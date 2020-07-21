///////////////////////////////////////
// main.cpp


// provides a windows shell for the application with an evaluator, result and transcript text pane.
// executes scheme code in a separate thread and displays the results.
// talks only to windows gui elements and the scheme engine
// the scheme engine may be separately extended; independent of this app; using DLLs and scheme libraries.
// this application  also provides a web browser pane for documentation;
// and a 2d graphics pane for drawings.
// some functions here operate on scheme values
// scheme has supporting functions that allow this app to evaluate expressions; in base.ss.

#define NOMINMAX
#include "stdafx.h"
#include "ContainerApp.h"
#include "Windows.h"
#include <WTypes.h>
#include <WinBase.h>
#include <MsHTML.h>
#include <comutil.h>

using namespace Gdiplus;

// chez scheme engine..
#include <scheme.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <MsXml2.h>
#include "OAIdl.h"
#include "Mainfrm.h"

 
// Windows SafeArray ATL
#include <atlbase.h>

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
 

// reg exp support

#pragma comment(lib, "re2.lib")
 
 

#define CALL0(who) Scall0(Stop_level_value(Sstring_to_symbol(who)))
#define CALL1(who, arg) Scall1(Stop_level_value(Sstring_to_symbol(who)), arg)
#define CALL2(who, arg, arg2) Scall2(Stop_level_value(Sstring_to_symbol(who)), arg, arg2)
#pragma comment(lib, "csv953.lib")

// alias some things we later remove
#define WinGetObject GetObjectW
#define WinSendMessage SendMessageW


#define USE_IMPORT_EXPORT

// read files
#include <iostream>
// strings
#include <locale>
#include <codecvt>


 
#include "csv.h"
#include <re2/re2.h>


//libs
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "msxml2.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")
#pragma comment(lib, "winspool.lib")

// IO streams
#pragma comment(lib, "msvcprtd.lib")
 



namespace Text {
	extern ptr utf8_string_separated_to_list(char* s, const char sep);
	extern std::wstring Widen(const std::string& in);
	ptr wideto_sstring(WCHAR* s);
	ptr const_utf8_to_sstring(const char* s);
	ptr utf8_to_sstring(char* s);
	std::string uri_decode(const std::string& s_src);
	std::string extract_search(std::string& in);
	std::string extract_path(std::string& in);

}
using namespace Text;

// global shared flags  
extern "C" __declspec(dllexport) bool launch_gui = true;
extern "C" __declspec(dllexport) bool launch_console = false;
extern "C" __declspec(dllexport) bool LogEvents = true;
extern "C" __declspec(dllexport) bool BeChatty = true;
extern "C" __declspec(dllexport) bool XL_SAFETY_ON = true;

void _init_graphics();
void init_commands();
void safe_cancel_commands();

ptr graphics_keys(void);
extern HWND image_hwnd;

extern "C" __declspec(dllexport) ptr EscapeKeyPressed()
{
	if (GetAsyncKeyState(VK_ESCAPE) != 0)
	{
		return Strue;
	}
	return Sfalse;
}


extern "C" __declspec(dllexport) bool inGuiMode()
{
	return launch_gui;
}

extern "C" __declspec(dllexport) bool inConsoleMode()
{
	return launch_console;
}

extern "C" __declspec(dllexport) bool LoggingOn()
{
	return LogEvents;
}

extern "C" __declspec(dllexport) ptr guiMode(bool flag)
{
	launch_gui = flag;
	return Snil;
}

extern "C" __declspec(dllexport) ptr guiModeQ(bool flag)
{
	if (launch_gui)
	{
		return Strue;
	}
	else
		return Snil;
}

extern "C" __declspec(dllexport) ptr consoleMode(bool flag)
{
	launch_console = flag;
	return Snil;
}

extern "C" __declspec(dllexport) ptr loggingMode(bool flag)
{
	LogEvents = flag;
	return Snil;
}

extern "C" __declspec(dllexport) ptr commandLine()
{
	return Sstring(GetCommandLineA());
}

extern "C" __declspec(dllexport) ptr QuitApplication()
{
	::ExitProcess(0);
	return Strue;
}

 
CDockContainerApp theApp;


extern "C" {
	void appendEditor(char* s);
	void appendTranscript(char* s);
	void appendTranscript1(const char* s);
	void appendTranscriptNL(const char* s);
	void appendTranscript2NL(const char* s, const char* s1);
	void appendTranscript3NL(const char* s, const char* s1, const char* s3);
	void set_inputed(char* s);
	void sc_setEditorFromFile(char* fname);
	ptr get_inputed();
	char *noncezero;
}

extern void eval_text(const char* text);
extern DWORD WINAPI exectask(LPVOID cmd);

extern HANDLE script_thread;
extern HANDLE g_script_mutex;

extern void start_time_stamp_timing();
 

std::wstring widen(const std::string& in)
{
	// Calculate target buffer size (not including the zero terminator).
	const auto len = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED,
	                                     in.c_str(), in.size(), nullptr, 0);
	if (len == 0)
	{
		std::wstring failed = L"";
		appendTranscriptNL("widen failed.");
		return failed;
	}

	std::wstring out(len, 0);
	MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED,
	                    in.c_str(), in.size(), &out[0], out.size());
	return out;
}


static bool set_browser_feature_control_key(const std::wstring& feature, wchar_t* app_name, DWORD value)
{
	HKEY key;
	auto success = true;
	const std::wstring features_path(L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\");
	auto path(features_path + feature);

	auto n_error = RegCreateKeyEx(HKEY_CURRENT_USER, path.c_str(), 0, nullptr, REG_OPTION_VOLATILE, KEY_WRITE, nullptr,
	                              &key, nullptr);
	if (n_error != ERROR_SUCCESS)
	{
		success = false;
	}
	else
	{
		n_error = RegSetValueExW(key, app_name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(value));
		if (n_error != ERROR_SUCCESS)
		{
			success = false;
		}

		n_error = RegCloseKey(key);
		if (n_error != ERROR_SUCCESS)
		{
			success = false;
		}
	}
	return success;
}

std::wstring s2_ws(const std::string& str)
{
	using convert_type_x = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type_x, wchar_t> converter_x;
	return converter_x.from_bytes(str);
}

std::string ws_2s(const std::wstring& wstr)
{
	using convert_type_x = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type_x, wchar_t> converter_x;
	return converter_x.to_bytes(wstr);
}


template <typename Out>
void split(const std::string& s, const char delim, Out result)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		*(result++) = item;
	}
}

std::vector<std::string> split(const std::string& s, const char delim)
{
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}



static LONG get_string_reg_key( HKEY h_key, const std::wstring& str_value_name, std::wstring& str_value,
                               const std::wstring& str_default_value)
{
	str_value = str_default_value;
	BYTE sz_buffer[512];
	const auto bptr = sz_buffer;
	DWORD dw_buffer_size = sizeof(sz_buffer);
	const ULONG n_error = RegQueryValueExW(h_key, str_value_name.c_str(), nullptr, nullptr, bptr, &dw_buffer_size);
	if (ERROR_SUCCESS == n_error)
	{
		str_value.assign(reinterpret_cast<wchar_t*>(bptr), 512);
	}
	return n_error;
}

static DWORD get_browser_emulation_mode()
{
	auto browser_version = 7;
	std::wstring s_browser_version;
	HKEY key;
	auto success = true;
	//BYTE data[256];
	std::wstring path(L"SOFTWARE\\Microsoft\\Internet Explorer");
	auto n_error = RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_QUERY_VALUE, &key);

	// !!! set mode to please NOT BE IE8.
	auto mode = 11000;
 
	if (n_error != ERROR_SUCCESS)
	{
		success = false;
	}
	else
	{
		n_error = get_string_reg_key(key, L"svcVersion", s_browser_version, L"7");
		if (n_error != ERROR_SUCCESS)
		{
			n_error = get_string_reg_key(key, L"version", s_browser_version, L"7");
			if (n_error != ERROR_SUCCESS)
			{
				success = false;
			}
		}

		if (RegCloseKey(key) != ERROR_SUCCESS)
		{
			success = false;
		}
	}

	if (success)
	{
		browser_version = std::atoi(split(ws_2s(s_browser_version), '.').at(0).c_str());
		// convert base 16 number in s to int

		switch (browser_version)
		{
		case 7:
			mode = 7000;
			// Webpages containing standards-based !DOCTYPE directives are displayed in IE7 Standards mode. Default value for applications hosting the WebBrowser Control.
			break;
		case 8:
			mode = 8000;
			// Webpages containing standards-based !DOCTYPE directives are displayed in IE8 mode. Default value for Internet Explorer 8
			break;
		case 9:
			mode = 9000;
			// Internet Explorer 9. Webpages containing standards-based !DOCTYPE directives are displayed in IE9 mode. Default value for Internet Explorer 9.
			break;
		default:
			// use IE10 mode by default
			break;
		}
	}
	else
	{
		mode = -1;
	}

	return mode;
}


static void set_browser_feature_control()
{
	// http://msdn.microsoft.com/en-us/library/ee330720(v=vs.85).aspx
	const auto emulation_mode = get_browser_emulation_mode();

	if (emulation_mode > 0)
	{
		wchar_t file_name[MAX_PATH + 1];
		ZeroMemory(file_name, (MAX_PATH + 1) * sizeof(wchar_t));
		GetModuleFileNameW(nullptr, file_name, 256);
		auto splitted_file_name = split(ws_2s(file_name), '\\');
		ZeroMemory(file_name, (MAX_PATH + 1) * sizeof(wchar_t));
		auto exe_name = widen(splitted_file_name.at(splitted_file_name.size() - 1));
		memcpy(file_name, exe_name.c_str(), sizeof(wchar_t) * exe_name.length());

		set_browser_feature_control_key(L"FEATURE_BROWSER_EMULATION", file_name, emulation_mode);
		set_browser_feature_control_key(L"FEATURE_AJAX_CONNECTIONEVENTS", file_name, 1);
		set_browser_feature_control_key(L"FEATURE_GPU_RENDERING ", file_name, 1);
		set_browser_feature_control_key(L"FEATURE_IVIEWOBJECTDRAW_DMLT9_WITH_GDI  ", file_name, 0);
		set_browser_feature_control_key(L"FEATURE_NINPUT_LEGACYMODE", file_name, 0);
		set_browser_feature_control_key(L"FEATURE_DISABLE_NAVIGATION_SOUNDS", file_name, 1);
		set_browser_feature_control_key(L"FEATURE_SCRIPTURL_MITIGATION", file_name, 1);
		set_browser_feature_control_key(L"FEATURE_SPELLCHECKING", file_name, 0);
		set_browser_feature_control_key(L"FEATURE_STATUS_BAR_THROTTLING", file_name, 1);
		set_browser_feature_control_key(L"FEATURE_VALIDATE_NAVIGATE_URL", file_name, 1);
		set_browser_feature_control_key(L"FEATURE_WEBOC_DOCUMENT_ZOOM", file_name, 1);
		set_browser_feature_control_key(L"FEATURE_WEBOC_POPUPMANAGEMENT", file_name, 0);
		set_browser_feature_control_key(L"FEATURE_ADDON_MANAGEMENT", file_name, 0);
		set_browser_feature_control_key(L"FEATURE_WEBSOCKET", file_name, 1);
		set_browser_feature_control_key(L"FEATURE_WINDOW_RESTRICTIONS", file_name, 0);
	}
}

extern "C" __declspec(dllexport) ptr GetFullPath(char* path)
{
	const int BSIZE = 4096;
	DWORD retval = 0;
	BOOL success;
	WCHAR buffer[BSIZE];
	WCHAR** lppPart = nullptr;
	auto wide_path = Text::Widen(path);
	retval = GetFullPathNameW(wide_path.c_str(), BSIZE, buffer, lppPart);
	if (retval == 0)
	{
		return Snil;
	}
	return Text::wideto_sstring(buffer);
}



// wait for ms; while keeping active windows message pump
void wait(const long ms)
{
	const auto end = clock() + ms;
	do
	{
		MSG msg;
		if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		::Sleep(0);
	}
	while (clock() < end);
}


// spin on script engine lock; while keeping UI live.

bool spin(const int turns)
{
	auto dw_wait_result = WaitForSingleObject(g_script_mutex, 15);
	auto time_out = turns;
	while (--time_out > 0 && (dw_wait_result == WAIT_TIMEOUT))
	{
		wait(85);
		dw_wait_result = WaitForSingleObject(g_script_mutex, 15);
	}
	if (time_out == 0)
	{
		ReleaseMutex(g_script_mutex);
		return true;
	}
	return false;
}


bool wait_for_browser()
{

	auto* ibrowser = theApp.GetMainFrame().get_browser().GetIWebBrowser2();
	if (nullptr == ibrowser)
	{
		appendTranscriptNL("Browser is null.");
		return false;
	}
	wait(10);

	READYSTATE pl_rdy;
	auto hr = ibrowser->get_ReadyState(&pl_rdy);
	if (FAILED(hr))
	{
		appendTranscriptNL("Ready state read failed");
		return false;
	}

	auto wait_time = 5;

	while (pl_rdy <2)
	{
		appendTranscriptNL("Waiting for browser");
		wait(50);
		hr = ibrowser->get_ReadyState(&pl_rdy);
		
		if (wait_time-- < 0)
		{
			appendTranscriptNL("Browser wait timed out.");
			return false;
		}

	}
	return true;
}

void format_scite(HWND hc)
{
	
	appendTranscriptNL(";; --- Formatted text ---\r\n");
	// wait for engine; then call function in engine
	if (spin(10))
	{
		return;
	}
	try
	{
		CALL0("format-scite");
		ReleaseMutex(g_script_mutex);
	}
	catch (...)
	{
		appendTranscriptNL("Exception: executing format script.");
		ReleaseMutex(g_script_mutex);
	}

}


void invoke_browser_help()
{
	if (theApp.GetMainFrame().get_browser() == nullptr)
	{
		appendTranscriptNL("Show browser in dock then use browser help");
		return;
	}

    // wait for engine; then call function in engine or fail..
	if (spin(10))
	{
		return;
	}
	try
	{
		CALL0("help");
		ReleaseMutex(g_script_mutex);
	}
	catch (...)
	{
		appendTranscriptNL("Exception: executing script.");
		ReleaseMutex(g_script_mutex);
	}

}


void invoke_browser_license()
{
	if (theApp.GetMainFrame().get_browser() == nullptr)
	{
		appendTranscriptNL("Show browser in dock then use browser help");
		return;
	}

	// wait for engine; then call function in engine
	if (spin(10))
	{
		return;
	}
	try
	{
		CALL0("license");
		ReleaseMutex(g_script_mutex);
	}
	catch (...)
	{
		appendTranscriptNL("Exception: executing script.");
		ReleaseMutex(g_script_mutex);
	}

}


void invoke_browser_back()
{
	if (theApp.GetMainFrame().get_browser() == nullptr)
	{
		appendTranscriptNL("Show browser in dock then use browser help");
		return;
	}

	theApp.GetMainFrame().get_browser().GoBack();
}

void invoke_browser_forward()
{
	if (theApp.GetMainFrame().get_browser() == nullptr)
	{
		appendTranscriptNL("Show browser in dock then use browser help");
		return;
	}

	theApp.GetMainFrame().get_browser().GoForward();
}


void invoke_browser_welcome()
{
	if (theApp.GetMainFrame().get_browser() == nullptr)
	{
		appendTranscriptNL("Show browser in dock then use browser help");
		return;
	}

	if (spin(5))
	{
		return;
	}
	try
	{
		CALL0("welcome");
		ReleaseMutex(g_script_mutex);
	}
	catch (...)
	{
		appendTranscriptNL("Exception: executing script.");
 
	}
}

void invoke_browser_clear()
{
	if (theApp.GetMainFrame().get_browser() == nullptr)
	{
		appendTranscriptNL("Show browser in dock then use browser help");
		return;
	}
	// wait only briefly; this is on main thread.
	if (spin(5))
	{
		return;
	}
	try
	{
		CALL0("blank");
		ReleaseMutex(g_script_mutex);
	}
	catch (...)
	{
		appendTranscriptNL("Exception: executing script.");
		ReleaseMutex(g_script_mutex);
	}
}



// URL lists for browser handling.
RE2* re_allow_list;
RE2* re_block_list;

// this allows the browser to be controlled by a script
// almost never a good idea though.

ptr browser_connect_sink(char* w, char* b)
{
	if (re_allow_list != nullptr)
	{
		delete re_allow_list;
		re_allow_list = nullptr;
	}
	if (re_block_list != nullptr)
	{
		delete re_block_list;
		re_block_list = nullptr;
	}

	re_allow_list = new RE2(w, re2::RE2::Quiet);
	re_block_list = new RE2(b, re2::RE2::Quiet);

	if (wait_for_browser())
	{
		PostMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 506, 0, 0);
	}
	return Strue;
}

HANDLE nav_thread = nullptr;


bool invoke_browser_on_navigate(char* url)
{
	if (spin(10))
	{
		return true;
	}

	// static to keep them alive
	static auto url_list = Snil;
	static auto arg_s0 = Snil;
	static auto arg_s1 = Snil;

	try
	{
		auto decoded = uri_decode(url);
		auto path = extract_path(decoded);
		url_list = Snil;
		auto search = extract_search(decoded);
		arg_s1 = Sstring(search.c_str());
		url_list = CALL2("cons", arg_s1, url_list);
		arg_s0 = Sstring(path.c_str());
		url_list = CALL2("cons", arg_s0, url_list);
		CALL1("OnNavigate", url_list);
	}
	catch (...)
	{
		appendTranscriptNL("Exception: executing function.");
		ReleaseMutex(g_script_mutex);
	}
	ReleaseMutex(g_script_mutex);
	return true;
}


ptr backtoforwardslash(char* s)
{
	std::string path = s;
	std::replace(path.begin(), path.end(), '\\', '/');
	return const_utf8_to_sstring(path.c_str());
}

ptr wait_on_browser()
{
	wait_for_browser();
	return Strue;
}

ptr wait_pump(const long ms)
{
	wait(ms);
	return Strue;
}


DWORD WINAPI exectask_navigate(LPVOID url)
{
	try
	{
		if (wait_for_browser())
		{
			PostMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 501, 0,
			             reinterpret_cast<LPARAM>(_strdup(static_cast<char*>(url))));
		}
	}
	catch (...)
	{
		appendTranscriptNL("Exception: navigating.");
	}
	return 0;
}

// this navigation action needs to exit the script engine a.s.a.p.
// because - it calls the script engine back using a message.
//

ptr navigate(char* url)
{
	try
	{
		if (wait_for_browser())
		{
			SendMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 501, 0,
			             reinterpret_cast<LPARAM>(_strdup(static_cast<char*>(url))));
		}
	}
	catch (...)
	{
		appendTranscriptNL("Exception: navigating.");
	}
	return Strue;
}


ptr browser_back()
{
	theApp.GetMainFrame().get_browser().GoBack();
	return Strue;
}

ptr browser_forward()
{
	theApp.GetMainFrame().get_browser().GoForward();
	return Strue;
}

ptr browser_home()
{
	theApp.GetMainFrame().get_browser().GoHome();
	return Strue;
}

ptr read_inner_html()
{
	HRESULT hr;
	auto s = Snil;

	if (theApp.GetMainFrame().get_browser() == nullptr)
	{
		appendTranscriptNL("Browser is null");
		return s;
	}

	if (wait_for_browser())
	{
		const WPARAM wp = 0;
		const LPARAM lp = 0;
		const auto p = reinterpret_cast<char*>(SendMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 503, wp, lp));
		try
		{
			s = Text::const_utf8_to_sstring(p);
			delete[] p;
		}
		catch (...)
		{
			appendTranscriptNL("ConvertBSTRToString exception");
			return s;
		}
	}
	return s;
}

ptr read_inner_text()
{
	
	auto s = Snil;

	if (theApp.GetMainFrame().get_browser() == nullptr)
	{
		appendTranscriptNL("Browser is null");
		return s;
	}

	if (wait_for_browser())
	{
		const WPARAM wp = 0;
		const LPARAM lp = 0;
		const auto p = reinterpret_cast<char*>(SendMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 504, wp, lp));
		try
		{
			s = const_utf8_to_sstring(p);
			delete[] p;
		}
		catch (...)
		{
			appendTranscriptNL("ConvertBSTRToString exception");
			return s;
		}
	}
	return s;
}

ptr read_title()
{
	
	auto s = Snil;

	if (theApp.GetMainFrame().get_browser() == nullptr)
	{
		appendTranscriptNL("Browser is null");
		return s;
	}

	if (wait_for_browser())
	{
		const WPARAM wp = 0;
		const LPARAM lp = 0;
		const auto p = reinterpret_cast<char*>(SendMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 505, wp, lp));
		try
		{
			s = const_utf8_to_sstring(p);
			delete[] p;
		}
		catch (...)
		{
			appendTranscriptNL("ConvertBSTRToString exception");
			return s;
		}
	}
	return s;
}


// run script in browser
ptr exec_browser(char* script)
{
	if (wait_for_browser())
	{
		SendMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 509, 0, reinterpret_cast<LPARAM>(script));
	}
	return Strue;
}

ptr get_browser_selection()
{
	auto s = Snil;
	if (theApp.GetMainFrame().get_browser() == nullptr)
	{
		appendTranscriptNL("Browser is null; so no selection.");
		return s;
	}

	if (wait_for_browser())
	{
		const WPARAM wp = 0;
		const LPARAM lp = 0;
		const auto p = reinterpret_cast<char*>(SendMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 513, wp, lp));
		try
		{
			s = const_utf8_to_sstring(p);
			delete[] p;
		}
		catch (...)
		{
			appendTranscriptNL("ConvertBSTRToString exception");
			return s;
		}
	}
	return s;
}


char* get_browser_selection_text()
{
	char* s = nullptr;
	if (theApp.GetMainFrame().get_browser() == nullptr)
	{
		appendTranscriptNL("Browser is null");
		return s;
	}

	if (wait_for_browser())
	{
		const WPARAM wp = 0;
		const LPARAM lp = 0;
		const auto p = reinterpret_cast<char*>(SendMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 513, wp, lp));
		try
		{
			s = _strdup(p);
			delete[] p;
		}
		catch (...)
		{
			appendTranscriptNL("ConvertBSTRToString exception");
			return s;
		}
	}
	return s;
}



// call javascript function in browser
// scheme list to function args; results of function to list.
LRESULT invoke_browser_function_call(const LPARAM list)
{
	static auto results = Snil;
	results = Snil;

	auto args = reinterpret_cast<ptr>(list);

	const auto is_string = Sstringp(args);
	if (is_string)
	{
		appendTranscriptNL(R"(List Needed e.g. '( "funcname" "arg1" ))");
		return reinterpret_cast<LRESULT>(results);
	}


	const auto argn = Sinteger32_value(CALL1("length", args));
	const auto fname = Scar(args);
	auto bytes = CALL1("string->utf8", fname);
	const long len = Sbytevector_length(bytes);
	const auto data = Sbytevector_data(bytes);
	const auto jfunc = static_cast<char *>(calloc(len + 1, sizeof(char)));
	memcpy(jfunc, data, len);
	//appendTranscript2NL("function name:", jfunc);

	auto ibrowser = theApp.GetMainFrame().get_browser().GetIWebBrowser2();
	if (nullptr == ibrowser)
	{
		appendTranscriptNL("Browser is null.");
		return reinterpret_cast<LRESULT>(results);
	}

	IDispatch* dispatch;
	auto hr = ibrowser->get_Document(&dispatch);
	if (FAILED(hr))
	{
		appendTranscriptNL("IDispatch failed");
		return reinterpret_cast<LRESULT>(results);
	}

	IHTMLDocument2* doc;
	hr = dispatch->QueryInterface(IID_IHTMLDocument2, reinterpret_cast<void**>(&doc));
	if (FAILED(hr))
	{
		appendTranscriptNL("Doc failed");
		return reinterpret_cast<LRESULT>(results);
	}

	IDispatch* scriptid;
	hr = doc->get_Script(&scriptid);
	if (FAILED(hr))
	{
		appendTranscriptNL("get script  failed");
		return reinterpret_cast<LRESULT>(results);
	}

	DISPID dispid = NULL;
	BSTR cmd = _bstr_t(jfunc);
	hr = scriptid->GetIDsOfNames(IID_NULL, &cmd, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
	if (FAILED(hr))
	{
		appendTranscriptNL("get function name failed");
		return reinterpret_cast<LRESULT>(results);
	}

	args = Scdr(args);
	const auto array_size = argn - 1;
	DISPPARAMS dispparams;
	memset(&dispparams, 0, sizeof dispparams);
	dispparams.cArgs = array_size;
	dispparams.rgvarg = new VARIANT[dispparams.cArgs];
	dispparams.cNamedArgs = 0;

	args = CALL1("reverse", args);
	for (auto i = 0; i < array_size; i++)
	{
		auto sparam = Scar(args);
		args = Scdr(args);

		const auto is_string = Sstringp(sparam);
		const auto is_fix_num = Sfixnump(sparam);
		const auto is_flo_num = Sflonump(sparam);

		if (is_string)
		{
			bytes = CALL1("string->utf8", sparam);
			int len = Sbytevector_length(bytes);
			auto data = Sbytevector_data(bytes);
			const auto thisparam = static_cast<char *>(calloc(len + 1, sizeof(char)));
			memcpy(thisparam, data, len);
			bytes = Snil;
			auto wparam = widen(thisparam);
			dispparams.rgvarg[i].bstrVal = SysAllocString(wparam.c_str());
			dispparams.rgvarg[i].vt = VT_BSTR;
			free(thisparam);
		}
		else if (is_fix_num)
		{
			const int num = Sfixnum_value(sparam);
			dispparams.rgvarg[i].intVal = num;
			dispparams.rgvarg[i].vt = VT_I4;
		}
		else if (is_flo_num)
		{
			const auto num = Sflonum_value(sparam);
			dispparams.rgvarg[i].dblVal = num;
			dispparams.rgvarg[i].vt = VT_R8;
		}
	}

	_variant_t va_result;

	EXCEPINFO excep_info;
	memset(&excep_info, 0, sizeof excep_info);
	auto n_arg_err = static_cast<UINT>(-1);
	try
	{
		hr = scriptid->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &dispparams, &va_result, &excep_info, &n_arg_err);
	}
	catch (...)
	{
	}

	delete[] dispparams.rgvarg;

	// process results
	switch (va_result.vt)
	{
	case VT_NULL:

	case VT_EMPTY:
		results = Sstring("");
		break;

	case VT_ERROR:
		results = Sstring("Error");
		break;

	case VT_BOOL: //see these
		if (va_result.boolVal != 0)
			results = Strue;
		results = Sfalse;
		break;

	case VT_I2:
		results = Sfixnum(va_result.iVal);
		break;

	case VT_I4:
		results = Sfixnum(va_result.lVal);
		break;

	case VT_R4:
		results = Sflonum(va_result.fltVal);
		break;

	case VT_R8:
		results = Sflonum(va_result.dblVal);
		break;

	case VT_BSTR: // all strings
		try
		{
			results = utf8_to_sstring(_com_util::ConvertBSTRToString(va_result.bstrVal));
		}
		catch (...)
		{
			results = Snil;
		}
		break;

	default:
		try
		{
			VariantChangeType(&va_result, &va_result, VARIANT_NOUSEROVERRIDE, VT_BSTR);
			results = utf8_to_sstring(_com_util::ConvertBSTRToString(va_result.bstrVal));
		}
		catch (...)
		{
			results = Snil;
		}
	}

	return reinterpret_cast<LRESULT>(results);
}


ptr callfunc(ptr args)
{
	if (wait_for_browser())
	{
		return reinterpret_cast<ptr>(SendMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 510, 0,
		                                          reinterpret_cast<LPARAM>(args)));
	}
	return Strue;
}


// appends to browser document
ptr write_document(char* text)
{
	if (text != nullptr && strlen(text) > 0)
	{
		if (wait_for_browser())
		{
			SendMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 507, 0, reinterpret_cast<LPARAM>(_strdup(text)));
		}
	}
	return Strue;
}

// clears browser document
ptr clear_document()
{
	if (wait_for_browser())
	{
		SendMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 508, 0, 0);
	}
	return Strue;
}

// scrape elements out of Document by ID.
LRESULT invoke_get_elements_by_id(const LPARAM list)
{
	static auto results = Snil;
	results = Snil;

	auto args = reinterpret_cast<ptr>(list);

	const auto is_string = Sstringp(args);
	if (is_string)
	{
		appendTranscriptNL(R"(List was Needed e.g. '( "myid1" "myid2" ))");
		return reinterpret_cast<LRESULT>(results);
	}

	const auto argn = Sinteger32_value(CALL1("length", args));

	auto ibrowser = theApp.GetMainFrame().get_browser().GetIWebBrowser2();
	if (nullptr == ibrowser)
	{
		appendTranscriptNL("Browser is null.");
		return reinterpret_cast<LRESULT>(results);
	}

	IDispatch* dispatch;
	auto hr = ibrowser->get_Document(&dispatch);
	if (FAILED(hr))
	{
		appendTranscriptNL("IDispatch failed");
		return reinterpret_cast<LRESULT>(results);
	}

	// using doc 3
	IHTMLDocument3* doc;
	hr = dispatch->QueryInterface(IID_IHTMLDocument3, reinterpret_cast<void**>(&doc));
	if (FAILED(hr))
	{
		appendTranscriptNL("Doc failed");
		return reinterpret_cast<LRESULT>(results);
	}

	args = CALL1("reverse", args);
	for (auto i = 0; i < argn; i++)
	{
		auto sparam = Scar(args);
		args = Scdr(args);

		const auto is_string = Sstringp(sparam);
		if (is_string)
		{
			auto bytes = CALL1("string->utf8", sparam);
			const int len = Sbytevector_length(bytes);
			const auto data = Sbytevector_data(bytes);
			const auto thisparam = static_cast<char *>(calloc(len + 1, sizeof(char)));
			memcpy(thisparam, data, len);
			bytes = Snil;
			auto wparam = widen(thisparam);
			IHTMLElement* elem;

			hr = doc->getElementById(_bstr_t(wparam.c_str()), &elem);
			if (FAILED(hr))
			{
				appendTranscript2NL("Get Element failed:", thisparam);
				results = CALL2("cons", Sstring("!NotFound"), results);
				free(thisparam);
				continue;
			}
			free(thisparam);

			if (nullptr == elem)
			{
				appendTranscriptNL("Element null");
				results = CALL2("cons", Sstring("!NotFound"), results);
				continue;
			}

			BSTR inner_text;
			hr = elem->get_innerText(&inner_text);
			if (FAILED(hr))
			{
				appendTranscript2NL("Element failed:", thisparam);
				results = CALL2("cons", Sstring("!NotFound"), results);
				continue;
			}

			const auto p = _com_util::ConvertBSTRToString(inner_text);
			if (p == nullptr)
			{
				results = CALL2("cons", Sstring(""), results);
				continue;
			}

			results = CALL2("cons", const_utf8_to_sstring(p), results);
			SysFreeString(inner_text);
			delete[] p;
		}
	}
	return reinterpret_cast<LRESULT>(results);
}

ptr get_elements_by_id(char* id)
{
	if (wait_for_browser())
	{
		return reinterpret_cast<ptr>(SendMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 511, 0,
		                                          reinterpret_cast<LPARAM>(id)));
	}
	return Strue;
}

// '( ("myid1" "myvalue2") ("myid3" "myvalue3"))
LRESULT invoke_set_elements_by_id(const LPARAM list)
{
	static auto results = Snil;
	results = Snil;

	auto args = reinterpret_cast<ptr>(list);

	const auto is_string = Sstringp(args);
	if (is_string)
	{
		appendTranscriptNL(R"(List Needed e.g. '( "myid1" "myid2" ))");
		return reinterpret_cast<LRESULT>(results);
	}

	const auto argn = Sinteger32_value(CALL1("length", args));

	auto ibrowser = theApp.GetMainFrame().get_browser().GetIWebBrowser2();
	if (nullptr == ibrowser)
	{
		appendTranscriptNL("Browser is null.");
		return reinterpret_cast<LRESULT>(results);
	}

	IDispatch* dispatch;
	auto hr = ibrowser->get_Document(&dispatch);
	if (FAILED(hr))
	{
		appendTranscriptNL("IDispatch failed");
		return reinterpret_cast<LRESULT>(results);
	}

	// using doc 3
	IHTMLDocument3* doc;
	hr = dispatch->QueryInterface(IID_IHTMLDocument3, reinterpret_cast<void**>(&doc));
	if (FAILED(hr))
	{
		appendTranscriptNL("Doc failed");
		return reinterpret_cast<LRESULT>(results);
	}

	args = CALL1("reverse", args);
	for (auto i = 0; i < argn; i++)
	{
		// get key value
		auto kv = Scar(args);
		args = Scdr(args);

		const auto sparam = Scar(kv);
		auto bytes = CALL1("string->utf8", sparam);
		int len = Sbytevector_length(bytes);
		auto data = Sbytevector_data(bytes);
		const auto thisparam = static_cast<char *>(calloc(len + 1, sizeof(char)));
		memcpy(thisparam, data, len);
		bytes = Snil;

		auto wparam = widen(thisparam);

		const auto value = Scdr(kv);

		bytes = CALL1("string->utf8", value);
		len = Sbytevector_length(bytes);
		data = Sbytevector_data(bytes);
		const auto thisvalue = static_cast<char *>(calloc(len + 1, sizeof(char)));
		memcpy(thisvalue, data, len);
		bytes = Snil;
		auto wvalue = widen(thisvalue);

		IHTMLElement* elem;

		hr = doc->getElementById(_bstr_t(wparam.c_str()), &elem);
		if (FAILED(hr))
		{
			appendTranscript2NL("Get Element failed:", thisparam);
			free(thisparam);
			continue;
		}
		free(thisparam);

		if (nullptr == elem)
		{
			appendTranscriptNL("Element null");
			continue;
		}

		hr = elem->put_innerText(_bstr_t(wvalue.c_str()));
		if (FAILED(hr))
		{
			appendTranscript3NL("Put Element failed:", thisparam, thisvalue);
			continue;
		}
	}
	return reinterpret_cast<LRESULT>(results);
}


ptr set_elements_by_id(char* id)
{
	if (wait_for_browser())
	{
		return reinterpret_cast<ptr>(SendMessageA(
			theApp.GetMainFrame().GetHwnd(), WM_USER + 512, 0, reinterpret_cast<LPARAM>(id)));
	}
	return Strue;
}


// gui window layout
ptr window_layout(const int n)
{
	SendMessageA(theApp.GetMainFrame().GetHwnd(), WM_USER + 520, 0, static_cast<LPARAM>(n));
	wait(10);
	return Strue;
}

ptr set_repaint_timer(const int n)
{
	KillTimer(image_hwnd, 1000);
	if (n > 0) {
		SetTimer(image_hwnd, 1000, n, NULL);
	}
	return Strue;
}


char* get_this_path(char* dest, const size_t dest_size)
{
	if (!dest) return nullptr;
	auto length = GetModuleFileNameA(nullptr, dest, dest_size);
	if (MAX_PATH > dest_size) return nullptr;
	PathRemoveFileSpecA(dest);
	return dest;
}



// how scales the image to the pane.
__declspec(dllexport) ptr GRUPDATE(const int how)
{
	if (nullptr == theApp.GetMainFrame().GetDockFromID(ID_DOCK_GRAFF1))
	{
		return Snil;
	}

	if (how == 0)
	{
		
		InvalidateRect(image_hwnd, 0, 0);
	}
	if (how == 1)
	{
		theApp.GetMainFrame().GetDockFromID(ID_DOCK_GRAFF1)->RedrawWindow();
	}
	if (how == 2)
	{
		theApp.GetMainFrame().RedrawWindow();
	}
	if (how == 3)
	{
		theApp.GetMainFrame().GetDockFromID(ID_DOCK_GRAFF1)->GetView().Invalidate(false);
	}
	if (how == 4)
	{
		theApp.GetMainFrame().GetDockFromID(ID_DOCK_GRAFF1)->Invalidate(false);
	}
	if (how == 5)
	{
		theApp.GetMainFrame().Invalidate(false);
	}
	if (how == 6)
	{
		theApp.GetMainFrame().GetDockFromID(ID_DOCK_GRAFF1)->GetView().InvalidateRect(false);
	}
	return Strue;
}
 
__declspec(dllexport) ptr GRACTIVATE()
{
	if (nullptr == theApp.GetMainFrame().GetDockFromID(ID_DOCK_GRAFF1))
	{
		return Snil;
	}
	SendMessageA(0, WM_USER + 514, (WPARAM)image_hwnd, 0);
	return Strue;
}


void redirect_io_to_console()
{
	// Create a console for this application
	const auto console_wnd = GetConsoleWindow();
	DWORD dw_process_id;
	GetWindowThreadProcessId(console_wnd, &dw_process_id);
	if (GetCurrentProcessId() == dw_process_id)
	{
	}
	else
	{
		AllocConsole();
	}
	// Get STDOUT handle
	auto console_output = GetStdHandle(STD_OUTPUT_HANDLE);
	const auto system_output = _open_osfhandle(intptr_t(console_output), _O_TEXT);
	auto c_output_handle = _fdopen(system_output, "w");

	// Get STDERR handle
	auto console_error = GetStdHandle(STD_ERROR_HANDLE);
	const auto system_error = _open_osfhandle(intptr_t(console_error), _O_TEXT);
	auto c_error_handle = _fdopen(system_error, "w");

	// Get STDIN handle
	auto console_input = GetStdHandle(STD_INPUT_HANDLE);
	const auto system_input = _open_osfhandle(intptr_t(console_input), _O_TEXT);
	auto c_input_handle = _fdopen(system_input, "r");

	//make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
	std::ios::sync_with_stdio(true);

	// Redirect the CRT standard input, output, and error handles to the console
	freopen_s(&c_input_handle, "CONIN$", "r", stdin);
	freopen_s(&c_output_handle, "CONOUT$", "w", stdout);
	freopen_s(&c_error_handle, "CONOUT$", "w", stderr);

	std::wcout.clear();
	std::cout.clear();
	std::wcerr.clear();
	std::cerr.clear();
	std::wcin.clear();
	std::cin.clear();
}


static bool coinitialized = false;

static void custom_init()
{
}


DWORD WINAPI  garbage_collect(LPVOID cmd);
DWORD WINAPI  busy_indicator(LPVOID cmd);

void register_boot_file(const char *boot_file, bool& already_loaded)
{
	const auto maxpath = MAX_PATH + 80;
	char dest[maxpath];
	get_this_path(dest, MAX_PATH + 80);
	strcat_s(dest, boot_file);

	if (!already_loaded &&
		!(INVALID_FILE_ATTRIBUTES == GetFileAttributesA(dest) &&
			GetLastError() == ERROR_FILE_NOT_FOUND))
	{
		already_loaded = true;
		Sregister_boot_file(dest);
	}
}

void load_script_ifexists(const char* script_relative)
{
	const auto maxpath = MAX_PATH + 80;
	char dest[maxpath];
	get_this_path(dest, maxpath);
	strcat_s(dest, script_relative);
	if (!(INVALID_FILE_ATTRIBUTES == GetFileAttributesA(dest) &&
		GetLastError() == ERROR_FILE_NOT_FOUND))
	{
		CALL1("load", Sstring(dest));
	}
}


DWORD WINAPI execstartup(LPVOID cmd);
void abnormal_exit() 
{
	appendTranscript("scheme engine died.");
	
	exit(1);
}


// we are running ahead of the GUI opening
DWORD WINAPI execstartup(LPVOID cmd)
{

	try
	{

		Sscheme_init(abnormal_exit);

		bool register_petite = false;
		bool register_cs = false;

	
		register_boot_file("\\boot\\petite.boot", register_petite);
		register_boot_file("petite.boot", register_petite);
		register_boot_file("\\boot\\scheme.boot", register_cs);
		register_boot_file("scheme.boot", register_cs);
 

		if (!register_cs && !register_petite)
		{
			MessageBox(nullptr,
			           L"The BOOT FILES could not be loaded.",
			           L"Looking for petite.boot and cs.boot",
			           MB_OK | MB_ICONERROR);
			exit(1);
		}


		Sbuild_heap("DockContainer.exe", custom_init);

		// export functions in this app so scheme can see them
		// some shell functions
		Sforeign_symbol("eval_respond", static_cast<void *>(appendEditor));
		Sforeign_symbol("append_transcript", static_cast<void *>(appendTranscript1));
		Sforeign_symbol("setInputed", static_cast<void *>(set_inputed));
		Sforeign_symbol("getInputed", static_cast<ptr>(get_inputed));
		Sforeign_symbol("graphicsUpdate", static_cast<ptr>(GRUPDATE));
		Sforeign_symbol("graphicsActivate", static_cast<ptr>(GRACTIVATE));
		Sforeign_symbol("EscapeKeyPressed", static_cast<ptr>(EscapeKeyPressed));
		

		// browser code
		Sforeign_symbol("backtoforwardslash", static_cast<ptr>(backtoforwardslash));
		Sforeign_symbol("navigate", static_cast<ptr>(navigate));
		Sforeign_symbol("waitOnBrowser", static_cast<ptr>(wait_on_browser));
		Sforeign_symbol("readInnerHTML", static_cast<ptr>(read_inner_html));
		Sforeign_symbol("readInnerText", static_cast<ptr>(read_inner_text));
		Sforeign_symbol("readTitle", static_cast<ptr>(read_title));
		Sforeign_symbol("readSelectedText", static_cast<ptr>(get_browser_selection()));
		Sforeign_symbol("browserConnectSink", static_cast<ptr>(browser_connect_sink));
		Sforeign_symbol("writeDocument", static_cast<ptr>(write_document));
		Sforeign_symbol("execBrowser", static_cast<ptr>(exec_browser));
		Sforeign_symbol("callfunc", static_cast<ptr>(callfunc));
		Sforeign_symbol("clearDocument", static_cast<ptr>(clear_document));
		Sforeign_symbol("getElementsById", static_cast<ptr>(get_elements_by_id));

		// set the window layout policy
		Sforeign_symbol("WindowLayout", static_cast<ptr>(window_layout));
		Sforeign_symbol("set_repaint_timer", static_cast<ptr>(set_repaint_timer));
		
		Sforeign_symbol("GetFullPath", static_cast<ptr>(GetFullPath));

		_init_graphics();
		Sforeign_symbol("graphics_keys", static_cast<ptr>(graphics_keys));
 
		// load scripts
		load_script_ifexists("\\scripts\\base.ss");
		load_script_ifexists("\\scripts\\init.ss");
		
		load_script_ifexists("\\scripts\\env.ss");

		CALL1("suppress-greeting", Strue);
		CALL1("waiter-prompt-string", Sstring(""));

		load_script_ifexists("\\scripts\\browser.ss");
		load_script_ifexists("\\scripts\\appstart.ss");

	}
	catch (const CException& e)
	{
		MessageBox(nullptr, e.GetText(), AtoT(e.what()), MB_ICONERROR);
		return 0;
	}

	return 0;
}
// runs on initial update
void post_gui_load_script()
{
	load_script_ifexists("\\scripts\\initialupdate.ss");

}

void start_com()
{
	if (!coinitialized)
	{
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		OleInitialize(nullptr);
		coinitialized = true;
	}
}

// no lexer no app.
bool load_lexer()
{
	const auto lexer_lib = LoadLibrary(L"scilexer.DLL");
	if (lexer_lib == nullptr)
	{
		MessageBox(nullptr,
		           L"The Scintilla SciLexer DLL could not be loaded.",
		           L"Error loading Scintilla",
		           MB_OK | MB_ICONERROR);
		exit(1);
	}
	return false;
}

extern __declspec(dllexport) HANDLE g_image_rotation_mutex;
 
int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	set_browser_feature_control();
 
	start_com();
 
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	g_script_mutex = CreateMutex(nullptr, FALSE, nullptr);
	g_image_rotation_mutex=CreateMutex(nullptr, FALSE, nullptr);

	start_time_stamp_timing();

	try
	{
		load_lexer();
		init_commands(); 
		try
		{
			const auto val = theApp.Run();
			return val;
		}
		catch (const CException& e)
		{
			MessageBox(nullptr, e.GetText(), AtoT(e.what()), MB_ICONERROR);
			return -1;
		}
	}


	// catch all unhandled CException types
	catch (const CException& e)
	{
		// Display the exception and quit
		MessageBox(nullptr, e.GetText(), AtoT(e.what()), MB_ICONERROR);
		return -1;
	}
}
