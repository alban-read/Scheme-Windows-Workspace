
#include "stdafx.h"
#include "Mainfrm.h"
#include "Text.h"
#include "resource.h"
#include "WinUser.h"
#include <MsHTML.h>
#include <comutil.h>
#include <atlbase.h>
#define NOMINMAX
#include <re2/re2.h>

void eval_scite(HWND c);
void format_scite(HWND c);
void eval_selected_scite(HWND c);
void display_license_text();
extern "C" void appendTranscriptNL(char* s);
extern "C" void appendTranscript2NL(char* s0, char* s1);

void sc_setEditorFromFile(char* fname);
char* get_browser_selection_text();
LRESULT send_editor(HWND h, UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0);

std::wstring s2_ws(const std::string& str); // re2 for regular expressions

std::string ws_2s(const std::wstring& wstr);


CMainFrame::CMainFrame()
{
	c_dock_frame::SetView(m_View);
}

CMainFrame::~CMainFrame()
{
}

void CMainFrame::load_image_dockers()
{
	const DWORD dw_style = DS_CLIENTEDGE; // The style added to each docker
	const auto width = GetWindowRect().Size().cx;
	input = AddDockedChild(new CDockText, DS_DOCKED_LEFT | dw_style, width / 2.5, ID_DOCK_TEXT1);
	trans = AddDockedChild(new CDockImage, DS_DOCKED_LEFT | DS_DOCKED_RIGHT | dw_style, width / 3, ID_DOCK_GRAFF1);
	auto t = trans->AddDockedChild(new CDockTranscriptText, DS_DOCKED_BOTTOM | dw_style, 200, ID_DOCK_TEXT2);
	t->AddDockedChild(new CDockResponseText, DS_DOCKED_RIGHT | dw_style, width / 3, ID_DOCK_TEXT3);
}


void CMainFrame::load_full_image_dockers()
{
	const DWORD dw_style = DS_CLIENTEDGE; // The style added to each docker
	const auto width = GetWindowRect().Size().cx;
	image=AddDockedChild(new CDockImage, DS_DOCKED_LEFT | DS_DOCKED_RIGHT, dw_style,   ID_DOCK_GRAFF1);
	trans = image->AddDockedChild(new CDockTranscriptText, DS_DOCKED_BOTTOM | dw_style, 200, ID_DOCK_TEXT2);
	output=trans->AddDockedChild(new CDockResponseText, DS_DOCKED_CONTAINER | dw_style, 200, ID_DOCK_TEXT3);
	input=trans->AddDockedChild(new CDockText, DS_DOCKED_CONTAINER | dw_style, 200, ID_DOCK_TEXT1);
	
	
}


void CMainFrame::load_default_dockers()
{
	const auto width = GetWindowRect().Size().cx;
	const DWORD dw_style = DS_CLIENTEDGE; // The style added to each docker
	input = AddDockedChild(new CDockText, DS_DOCKED_LEFT | dw_style, width / 2, ID_DOCK_TEXT1);
	input->AddDockedChild(new CDockResponseText, DS_DOCKED_BOTTOM | dw_style, 300, ID_DOCK_TEXT3);
	trans = AddDockedChild(new CDockImage, DS_DOCKED_LEFT | DS_DOCKED_RIGHT | dw_style, width / 3, ID_DOCK_GRAFF1);
	trans->AddDockedChild(new CDockTranscriptText, DS_DOCKED_CONTAINER | dw_style, 200, ID_DOCK_TEXT2);
	 
}

void CMainFrame::load_browser_dockers()
{
	const auto width = GetWindowRect().Size().cx;
	const DWORD dw_style = DS_CLIENTEDGE; // The style added to each docker
	browser = AddDockedChild(new CDockBrowser, DS_DOCKED_LEFT | dw_style, width / 3, ID_DOCK_BRWSR1);
	input = AddDockedChild(new CDockText, DS_DOCKED_LEFT | dw_style, width / 3, ID_DOCK_TEXT1);
	input->AddDockedChild(new CDockResponseText, DS_DOCKED_BOTTOM | dw_style, 1000, ID_DOCK_TEXT3);
	trans = AddDockedChild(new CDockImage, DS_DOCKED_LEFT | DS_DOCKED_RIGHT | dw_style, width / 3, ID_DOCK_GRAFF1);
	trans->AddDockedChild(new CDockTranscriptText, DS_DOCKED_CONTAINER | dw_style, width / 3, ID_DOCK_TEXT2);
}

char* get_this_path(char* dest, const size_t dest_size);
DWORD WINAPI exectask_navigate(LPVOID url);
void load_html_if_exist(const char* html_relative)
{
	const auto maxpath = MAX_PATH + 80;
	char dest[maxpath];
	get_this_path(dest, maxpath);
	strcat_s(dest, html_relative);
	if (!(INVALID_FILE_ATTRIBUTES == GetFileAttributesA(dest) &&
		GetLastError() == ERROR_FILE_NOT_FOUND))
	{
		exectask_navigate(dest);
	}
}


void CMainFrame::load_default_browser_dockers()
{
	const DWORD dw_style = DS_CLIENTEDGE; // The style added to each docker
	const auto width = GetWindowRect().Size().cx;
	browser = AddDockedChild(new CDockBrowser, DS_DOCKED_LEFT | dw_style, width / 3, ID_DOCK_BRWSR1);
	input = AddDockedChild(new CDockText, DS_DOCKED_LEFT | dw_style, width / 3, ID_DOCK_TEXT1);
	input->AddDockedChild(new CDockResponseText, DS_DOCKED_BOTTOM | dw_style, 200, ID_DOCK_TEXT3);

	trans = AddDockedChild(new CDockImage, DS_DOCKED_LEFT | DS_DOCKED_RIGHT | dw_style, width / 3, ID_DOCK_GRAFF1);
	trans->AddDockedChild(new CDockTranscriptText, DS_DOCKED_CONTAINER | dw_style, 200, ID_DOCK_TEXT2);
	load_html_if_exist("/docs/readme.html");
}

void CMainFrame::load_full_browser_dockers()
{
	const DWORD dw_style = DS_CLIENTEDGE; // The style added to each docker
	browser = AddDockedChild(new CDockBrowser, DS_DOCKED_TOP | DS_DOCKED_LEFT | dw_style, 800, ID_DOCK_BRWSR1);
}


c_docker* CMainFrame::NewDockerFromID(int nID)
{
	c_docker* pDock = NULL;
	switch (nID)
	{
	case ID_DOCK_TEXT1:
		input = new CDockText;
		break;
	case ID_DOCK_TEXT2:
		output = new CDockResponseText;
		break;
	case ID_DOCK_TEXT3:
		output = new CDockTranscriptText;
		break;
	case ID_DOCK_GRAFF1:
		image = new CDockImage;
		break;
	case ID_DOCK_BRWSR1:
		browser = new CDockBrowser;
		break;
	default:
		TRACE("Unknown Dock ID\n");
		break;
	}

	return pDock;
}


#define WM_REDRAW_IMAGE    (WM_USER + 512)

std::wstring widen(const std::string& in);

LRESULT invoke_browser_function_call(LPARAM list);
LRESULT invoke_get_elements_by_id(LPARAM list);
LRESULT invoke_set_elements_by_id(LPARAM list);

LRESULT CMainFrame::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		// navigate to url.
	case WM_USER + 501:
	{
		const std::string url = reinterpret_cast<char*>(lParam);
		auto wurl = widen(url);
		delete reinterpret_cast<char*>(lParam);
		try
		{
			get_browser().Navigate(wurl.c_str(), NULL, nullptr, nullptr, nullptr);
		}
		catch (...)
		{
			appendTranscriptNL("Browser navigation error");
			return 0;
		};
		return 0;
	}

	// echo
	case WM_USER + 502:
	{
		return 502;
	}

	// readInnerHTML
	case WM_USER + 503:
	{
		auto ibrowser = get_browser().GetIWebBrowser2();
		if (nullptr == ibrowser) { return 0; }

		IDispatch* dispatch;
		auto hr = ibrowser->get_Document(&dispatch);
		if (FAILED(hr))
		{
			appendTranscriptNL("IDispatch failed");
			return 0;
		}

		IHTMLDocument2* doc;
		hr = dispatch->QueryInterface(IID_IHTMLDocument2, reinterpret_cast<void**>(&doc));
		if (FAILED(hr))
		{
			appendTranscriptNL("Doc failed");
			return 0;
		}

		IHTMLElement* lpBodyElm;

		try
		{
			hr = doc->get_body(&lpBodyElm);
			if (FAILED(hr))
			{
				appendTranscriptNL("Get Body error");
				return 0;
			}
		}
		catch (...)
		{
			appendTranscriptNL("Get Body exception");
			return 0;
		}


		BSTR inner_html;
		try
		{
			hr = lpBodyElm->get_innerHTML(&inner_html);
			if (FAILED(hr))
			{
				doc->Release();
				return 0;
			}
			lpBodyElm->Release();
		}
		catch (...)
		{
			appendTranscriptNL("Get HTML exception");
			return 0;
		}


		char* p;
		try
		{
			p = _com_util::ConvertBSTRToString(inner_html);
			if (p == nullptr)
			{
				appendTranscriptNL("HTML is null");
				doc->Release();
				return 0;
			}
			SysFreeString(inner_html);
		}
		catch (...)
		{
			appendTranscriptNL("ConvertBSTRToString exception");
			return 0;
		}

		doc->Release();
		return reinterpret_cast<LRESULT>(p);
	}

	// readInnerText
	case WM_USER + 504:
	{
		auto ibrowser = get_browser().GetIWebBrowser2();
		if (nullptr == ibrowser) { return 0; }

		IDispatch* dispatch;
		auto hr = ibrowser->get_Document(&dispatch);
		if (FAILED(hr))
		{
			appendTranscriptNL("IDispatch failed");
			return 0;
		}


		IHTMLDocument2* doc;
		hr = dispatch->QueryInterface(IID_IHTMLDocument2, reinterpret_cast<void**>(&doc));
		if (FAILED(hr))
		{
			appendTranscriptNL("Doc failed");
			return 0;
		}
		dispatch->Release();

		IHTMLElement* lp_body_elm;

		try
		{
			hr = doc->get_body(&lp_body_elm);
			if (FAILED(hr))
			{
				appendTranscriptNL("Get Body error");
				return 0;
			}
		}
		catch (...)
		{
			appendTranscriptNL("Get Body exception");
			return 0;
		}


		BSTR inner_text;
		try
		{
			hr = lp_body_elm->get_innerText(&inner_text);
			if (FAILED(hr))
			{
				doc->Release();
				return 0;
			}
			lp_body_elm->Release();
		}
		catch (...)
		{
			appendTranscriptNL("Get HTML exception");
			return 0;
		}


		char* p;
		try
		{
			p = _com_util::ConvertBSTRToString(inner_text);
			if (p == nullptr)
			{
				appendTranscriptNL("TEXT is null");
				doc->Release();
				return 0;
			}
			SysFreeString(inner_text);
		}
		catch (...)
		{
			appendTranscriptNL("ConvertBSTRToString exception");
			return 0;
		}

		doc->Release();
		return reinterpret_cast<LRESULT>(p);
	}

	// readTitle 
	case WM_USER + 505:
	{
		auto ibrowser = get_browser().GetIWebBrowser2();
		if (nullptr == ibrowser) { return 0; }

		IDispatch* dispatch;
		auto hr = ibrowser->get_Document(&dispatch);
		if (FAILED(hr))
		{
			appendTranscriptNL("IDispatch failed");
			return 0;
		}


		IHTMLDocument2* doc;
		hr = dispatch->QueryInterface(IID_IHTMLDocument2, reinterpret_cast<void**>(&doc));
		if (FAILED(hr))
		{
			appendTranscriptNL("Doc failed");
			return 0;
		}
		dispatch->Release();

		IHTMLElement* lp_body_elm;

		try
		{
			hr = doc->get_body(&lp_body_elm);
			if (FAILED(hr))
			{
				appendTranscriptNL("Get Body error");
				return 0;
			}
		}
		catch (...)
		{
			appendTranscriptNL("Get Body exception");
			return 0;
		}


		BSTR inner_text;
		try
		{
			hr = lp_body_elm->get_title(&inner_text);
			if (FAILED(hr))
			{
				doc->Release();
				return 0;
			}
			lp_body_elm->Release();
		}
		catch (...)
		{
			appendTranscriptNL("Get HTML exception");
			return 0;
		}

		char* p;
		try
		{
			p = _com_util::ConvertBSTRToString(inner_text);
			if (p == nullptr)
			{
				appendTranscriptNL("HTML is null");
				doc->Release();
				return 0;
			}
			SysFreeString(inner_text);
		}
		catch (...)
		{
			appendTranscriptNL("ConvertBSTRToString exception");
			return 0;
		}

		doc->Release();
		return reinterpret_cast<LRESULT>(p);
	}

	// wire up the browser control event sink
	// so scripts get browser events; via mainform
	case WM_USER + 506:
	{
		connect_events();
		return 0;
	}

	// write document; update browser contents very crash prone.
	case WM_USER + 507:
	{

		// Calculate target buffer size (not including the zero terminator).
		const auto len = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED,
			reinterpret_cast<char*>(lParam), -1, nullptr, 0);

		if (len == 0)
		{
			appendTranscriptNL("widen failed.");
			return 0;
		}

		// UTF8 does not fully survive the conversion experience.
		std::wstring out(len, 0);
		MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED,
			reinterpret_cast<char*>(lParam), -1, &out[0], out.size());

		auto ibrowser = get_i_browser();
		if (nullptr == ibrowser)
		{
			appendTranscriptNL("IDispatch failed");
			return 0;
		}

		IDispatch* dispatch;

		auto hr = ibrowser->get_Document(&dispatch);
		if (FAILED(hr))
		{
			appendTranscriptNL("IDispatch failed");
			return 0;
		}

		IHTMLDocument2* doc;
		hr = dispatch->QueryInterface(IID_IHTMLDocument2, reinterpret_cast<void**>(&doc));
		if (FAILED(hr))
		{
			appendTranscriptNL("Doc failed");
			return 0;
		}
		dispatch->Release();

		if (doc == nullptr)
		{
			appendTranscriptNL("Doc null");
			return 0;
		}


		const auto psa_strings = SafeArrayCreateVector(VT_VARIANT, 0, 1);
		if (psa_strings == nullptr)
		{
			appendTranscriptNL("Safearray null");
			doc->Release();
			return 0;
		}

		_variant_t* param;
		hr = SafeArrayAccessData(psa_strings, reinterpret_cast<LPVOID*>(&param));
		if (FAILED(hr))
		{
			appendTranscriptNL("safe array failed");
			return 0;
		}

		param->vt = VT_BSTR;
		param->bstrVal = SysAllocStringLen(out.c_str(), len);
		hr = SafeArrayUnaccessData(psa_strings);

		try
		{

			hr = doc->write(psa_strings);
			if (FAILED(hr))
			{
				appendTranscriptNL("Doc write failed");
				SafeArrayDestroy(psa_strings);
				return 0;
			}
		}
		catch (...)
		{
			appendTranscriptNL("Doc write exception");
		};

		// this is what used to crash when bstr was not allocated in the psa direct.
		SafeArrayDestroy(psa_strings);
		doc->Release();
		return 0;
	}

	// clear document; update browser contents
	// to clear document write "" and close; MSDN
	case WM_USER + 508:
	{


		auto ibrowser = get_i_browser();
		if (nullptr == ibrowser)
		{
			appendTranscriptNL("ibrowser failed");
			return 0;
		}
		IDispatch* dispatch;

		auto hr = ibrowser->get_Document(&dispatch);
		if (FAILED(hr))
		{
			appendTranscriptNL("IDispatch failed");
			return 0;
		}

		IHTMLDocument2* doc;
		hr = dispatch->QueryInterface(IID_IHTMLDocument2, reinterpret_cast<void**>(&doc));
		if (FAILED(hr))
		{
			appendTranscriptNL("Doc failed");
			return 0;
		}
		dispatch->Release();

		if (doc == nullptr)
		{
			appendTranscriptNL("Doc null");
			return 0;
		}


		const auto psa_strings = SafeArrayCreateVector(VT_VARIANT, 0, 1);
		if (psa_strings == nullptr)
		{
			appendTranscriptNL("Safearray null");
			doc->Release();
			return 0;
		}

		_variant_t* param;
		hr = SafeArrayAccessData(psa_strings, reinterpret_cast<LPVOID*>(&param));
		if (FAILED(hr))
		{
			appendTranscriptNL("safe array failed");
			return 0;
		}
		param->vt = VT_BSTR;
		param->bstrVal = SysAllocString(L"");
		hr = SafeArrayUnaccessData(psa_strings);
		try
		{
			hr = doc->write(psa_strings);
			if (FAILED(hr))
			{
				appendTranscriptNL("Doc write failed");
				SafeArrayDestroy(psa_strings);
				return 0;
			}
			hr = doc->close();
			if (FAILED(hr))
			{
				appendTranscriptNL("Doc close failed");
				return 0;
			}
		}
		catch (...)
		{
			appendTranscriptNL("Doc write exception");
		};
		SafeArrayDestroy(psa_strings);
		doc->Release();
		return 0;
	}

	// exec browser script
	case WM_USER + 509:
	{
		const std::string script = reinterpret_cast<char*>(lParam);
		auto wscript = widen(script);


		auto ibrowser = get_browser().GetIWebBrowser2();
		if (nullptr == ibrowser)
		{
			appendTranscriptNL("Browser is null.");
			return 0;
		}

		IDispatch* dispatch;
		auto hr = ibrowser->get_Document(&dispatch);
		if (FAILED(hr))
		{
			appendTranscriptNL("IDispatch failed");
			return 0;
		}
		IHTMLDocument2* doc;
		hr = dispatch->QueryInterface(IID_IHTMLDocument2, reinterpret_cast<void**>(&doc));
		if (FAILED(hr))
		{
			appendTranscriptNL("Doc failed");
			return 0;
		}
		dispatch->Release();

		if (doc == nullptr)
		{
			appendTranscriptNL("Doc null");
			return 0;
		}

		IHTMLWindow2* win;

		hr = doc->get_parentWindow(&win);
		if (FAILED(hr))
		{
			appendTranscriptNL("Win failed");
			return 0;
		}

		if (win == nullptr)
		{
			appendTranscriptNL("Win failed");
			return 0;
		}

		CComDispatchDriver disp_window;
		hr = win->QueryInterface(&disp_window);
		if (FAILED(hr))
		{
			appendTranscriptNL("Win dispatch failed");
			return 0;
		}

		CComVariant result;
		try
		{
			disp_window.Invoke1(L"eval", &CComVariant(wscript.c_str()), &result);
		}
		catch (...)
		{
		};

		win->Release();
		doc->Release();
		break;
	}


	// exec browser function by name
	// name of function and arguments in scheme list
	case WM_USER + 510:
	{
		return invoke_browser_function_call(lParam);
	}

	case WM_USER + 511:
	{
		return invoke_get_elements_by_id(lParam);
	}

	case WM_USER + 512:
	{
		return invoke_set_elements_by_id(lParam);
	}


	// read selected text
	case WM_USER + 513:
	{
		auto ibrowser = get_browser().GetIWebBrowser2();
		if (nullptr == ibrowser) { return 0; }

		IDispatch* dispatch;
		auto hr = ibrowser->get_Document(&dispatch);
		if (FAILED(hr))
		{
			appendTranscriptNL("IDispatch failed");
			return 0;
		}
		IHTMLDocument2* doc;
		hr = dispatch->QueryInterface(IID_IHTMLDocument2, reinterpret_cast<void**>(&doc));
		if (FAILED(hr))
		{
			appendTranscriptNL("Doc failed");
			return 0;
		}
		dispatch->Release();

		if (doc == nullptr)
		{
			appendTranscriptNL("Doc null");
			return 0;
		}

		CComPtr<IHTMLSelectionObject> pSelection;
		hr = doc->get_selection(&pSelection);
		if (FAILED(hr) || pSelection == NULL)
			return 0;

		CComPtr<IDispatch> pDispRange;
		hr = pSelection->createRange(&pDispRange);
		if (FAILED(hr) || pDispRange == NULL)
			return 0;
		CComPtr<IHTMLTxtRange> pRange;
		hr = pDispRange->QueryInterface(IID_IHTMLTxtRange,
			reinterpret_cast<void**>(&pRange));
		if (FAILED(hr) || pRange == NULL)
			return 0;

		BSTR text;
		pRange->get_text(&text);

		char* p = nullptr;
		try
		{
			p = _com_util::ConvertBSTRToString(text);
			if (p == nullptr)
			{
				appendTranscriptNL("TEXT is null");
				doc->Release();
				return 0;
			}
			SysFreeString(text);
		}
		catch (...)
		{
			appendTranscriptNL("ConvertBSTRToString exception");
			return 0;
		}

		doc->Release();
		return reinterpret_cast<LRESULT>(p);
	}
	case WM_USER + 514:
	{
		::SetFocus((HWND)(wParam));
		return TRUE;
	}

	case WM_USER + 520:
	{
		auto n = static_cast<int>(lParam);
		switch (n)
		{
		case 0:
			on_dock_default();
			break;
		case 1:
			on_dock_image();
			break;
		case 10:
			on_dock_full_image();
			break;
		case 2:
			on_dock_browser();
			break;
		case 20:
			on_dock_full_browser();
			break;
		case 4:
			on_dock_close_all();
			break;
		}
		return TRUE;
	}

	case WM_DROPFILES:
	{
		const int count = DragQueryFile(reinterpret_cast<HDROP>(wParam), 0xFFFFFFFF, nullptr, 0);
		if (count == 0) { return TRUE; }
		auto index = 0;
		const auto buffer = new char[64000];

		for (auto i = 0; i < count; i++)
		{
			const auto result_size = DragQueryFileA(reinterpret_cast<HDROP>(wParam), index, buffer, 4000);
			if (result_size == 0)
			{
				delete[] buffer;
				return TRUE;
			}
			index++;
			POINT xy;
			DragQueryPoint(reinterpret_cast<HDROP>(wParam), &xy);
			if (result_size == 0)
			{
				delete[] buffer;
				return TRUE;
			}

			sc_setEditorFromFile(buffer);

			delete[] buffer;
			DragFinish(reinterpret_cast<HDROP>(wParam));
			return TRUE;
		}
	}
	default:;
	}
	return WndProcDefault(uMsg, wParam, lParam);
}

DWORD e0 = 0;

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	// OnCommand responds to menu and and toolbar input
	const UINT nID = LOWORD(wParam);
	switch (nID)
	{
	case IDM_FILE_EXIT: return on_file_exit();
	case IDM_DOCK_DEFAULT: return on_dock_default();
	case IDM_DOCK_IMAGELAYOUT: return on_dock_image();
	case ID_DOCKING_BROWSERLAYOUT: return on_dock_browser();
	case IDM_DOCK_CLOSEALL: return on_dock_close_all();
	case IDW_VIEW_STATUSBAR: return OnViewStatusBar();
	case IDW_VIEW_TOOLBAR: return OnViewToolBar();
	case IDM_HELP_ABOUT: return OnHelp();
	case ID_HELP_HELPBROWSER: return on_browser_help();
	case ID_BROWSER_HELP: return on_browser_help();
	case ID_BROWSER_CLEAR: return on_browser_clear();
	case ID_BROWSER_WELCOME: return on_browser_welcome();
	case ID_BACKBROWSER: return on_browser_back();
	case ID_FOWARDBROWSER: return on_browser_forward();
	case IDM_EDIT_CUT:
	{
		const auto focus = ::GetFocus();
		SendMessage(focus, WM_CUT, 0, 0);
		break;
	}


	case IDM_EDIT_COPY:
	{
		const auto focus = ::GetFocus();
		if (focus == get_browser().GetHwnd())
		{
			// does windows cut n paste actually work?

			auto ibrowser = get_browser().GetIWebBrowser2();
			if (nullptr == ibrowser) { return 0; }

			IDispatch* dispatch;
			auto hr = ibrowser->get_Document(&dispatch);
			if (FAILED(hr))
			{
				appendTranscriptNL("IDispatch failed");
				return 0;
			}
			IHTMLDocument2* doc;
			hr = dispatch->QueryInterface(IID_IHTMLDocument2, reinterpret_cast<void**>(&doc));
			if (FAILED(hr))
			{
				appendTranscriptNL("Doc failed");
				return 0;
			}
			dispatch->Release();

			if (doc == nullptr)
			{
				appendTranscriptNL("Doc null");
				return 0;
			}

			CComPtr<IHTMLSelectionObject> pSelection;
			hr = doc->get_selection(&pSelection);
			if (FAILED(hr) || pSelection == NULL)
				return 0;

			CComPtr<IDispatch> pDispRange;
			hr = pSelection->createRange(&pDispRange);
			if (FAILED(hr) || pDispRange == NULL)
				return 0;
			CComPtr<IHTMLTxtRange> pRange;
			hr = pDispRange->QueryInterface(IID_IHTMLTxtRange,
				reinterpret_cast<void**>(&pRange));
			if (FAILED(hr) || pRange == NULL)
				return 0;

			BSTR text;
			pRange->get_text(&text);

			char* p = nullptr;
			try
			{
				p = _com_util::ConvertBSTRToString(text);
				if (p == nullptr)
				{
					appendTranscriptNL("TEXT is null");
					doc->Release();
					return 0;
				}
				SysFreeString(text);
			}
			catch (...)
			{
				appendTranscriptNL("ConvertBSTRToString exception");
				return 0;
			}

			doc->Release();

			const size_t len = strlen(p) + 1;
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
			memcpy(GlobalLock(hMem), p, len);
			GlobalUnlock(hMem);
			OpenClipboard(0);
			EmptyClipboard();
			SetClipboardData(CF_TEXT, hMem);
			CloseClipboard();
			break;
		}

		SendMessage(focus, WM_COPY, 0, 0);
		break;
	}


	case IDM_EDIT_UNDO:
	{
		const auto focus = ::GetFocus();
		SendMessage(focus, WM_UNDO, 0, 0);
		break;
	}

	case IDM_EDIT_PASTE:
	{
		const auto focus = ::GetFocus();
		SendMessage(focus, WM_PASTE, 0, 0);
		break;
	}

	case ID_ACTIONS_EXECUTESELECTED:
	{
		eval_selected_scite(::GetFocus());
		break;
	}

	case ID_VIEW_CLEAR:
	{
		send_editor(::GetFocus(), SCI_CLEARALL);
		break;
	}

	case ID_FORMAT:
	{
		format_scite(::GetFocus());
		break;
	}

	case ID_HELP_LICENSETEXT:
	{
		display_license_text();
		break;
	}

	// key debounce only once per second.
	case ID_ACTIONS_EXECUTE:
	{
		if (GetTickCount() - e0 > 500)
		{
			eval_scite(::GetFocus());
		}
		else
		{
			appendTranscriptNL("Step away from the execute key for half a second.");
		}
		e0 = GetTickCount();
		break;
	}
	default: return FALSE;
	}

	return FALSE;
}

int CMainFrame::OnCreate(CREATESTRUCT& cs)
{
	SetUseToolBar(FALSE); // Don't use a ToolBar

	return c_dock_frame::OnCreate(cs);
}

BOOL CMainFrame::on_dock_default()
{
	SetRedraw(FALSE);
	CloseAllDockers();
	load_default_dockers();
	SetRedraw(TRUE);
	RedrawWindow(RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
 
	return TRUE;
}


BOOL CMainFrame::on_dock_image()
{
	SetRedraw(FALSE);
	CloseAllDockers();
	load_image_dockers();
	SetRedraw(TRUE);
	RedrawWindow(RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

	return TRUE;
}
BOOL CMainFrame::on_dock_full_image()
{
	SetRedraw(FALSE);
	CloseAllDockers();
	load_full_image_dockers();
	SetRedraw(TRUE);
	RedrawWindow(RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

	return TRUE;
}


BOOL CMainFrame::on_dock_browser()
{
	SetRedraw(FALSE);
	CloseAllDockers();
	load_default_browser_dockers();
	SetRedraw(TRUE);
	RedrawWindow(RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

	return TRUE;
}

BOOL CMainFrame::on_dock_full_browser()
{
	SetRedraw(FALSE);
	CloseAllDockers();
	load_full_browser_dockers();
	SetRedraw(TRUE);
	return RedrawWindow(RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
}


void invoke_browser_help();

BOOL CMainFrame::on_browser_help()
{
	invoke_browser_help();
	return TRUE;
}


void invoke_browser_clear();

BOOL CMainFrame::on_browser_clear()
{
	invoke_browser_clear();
	return TRUE;
}

void invoke_browser_welcome();

BOOL CMainFrame::on_browser_welcome()
{
	invoke_browser_welcome();
	return TRUE;
}

void invoke_browser_back();

BOOL CMainFrame::on_browser_back()
{
	invoke_browser_back();
	return TRUE;
}

void invoke_browser_forward();

BOOL CMainFrame::on_browser_forward()
{
	invoke_browser_forward();
	return TRUE;
}

BOOL CMainFrame::on_dock_close_all()
{
	CloseAllDockers();
	return TRUE;
}

BOOL CMainFrame::on_file_exit() const
{
	// Issue a close request to the frame
	return PostMessage(WM_CLOSE);
}

void post_gui_load_script();

void CMainFrame::OnInitialUpdate()
{
	auto h_icon = static_cast<HICON>(LoadImageW(GetModuleHandle(nullptr),
		MAKEINTRESOURCE(IDI_MAINICON),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		0));
	if (h_icon)
	{
		SendMessage(GetHwnd(), WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(h_icon));
		SendMessage(GetHwnd(), WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(h_icon));
	}

	SetDockStyle(DS_CLIENTEDGE);
	DragAcceptFiles(true);
	load_default_dockers();

	// display 
	ShowWindow(GetInitValues().ShowCmd);

	// run user script
	post_gui_load_script();
}


void CMainFrame::PreCreate(CREATESTRUCT& cs)
{
	// Call the base class function first
	c_dock_frame::PreCreate(cs);

	// Hide the window initially by removing the WS_VISIBLE style
	cs.style &= ~WS_VISIBLE;
	
}


BOOL CMainFrame::SaveRegistrySettings()
{
	return FALSE;
}

void CMainFrame::SetupToolBar()
{
	// Set the Resource IDs for the toolbar buttons
	//AddToolBarButton(IDM_FILE_NEW, FALSE);
	//AddToolBarButton(IDM_FILE_OPEN, FALSE);
	//AddToolBarButton(IDM_FILE_SAVE, FALSE);

	//AddToolBarButton(0);	// Separator
	//AddToolBarButton(IDM_EDIT_CUT, FALSE);
	//AddToolBarButton(IDM_EDIT_COPY, FALSE);
	//AddToolBarButton(IDM_EDIT_PASTE, FALSE);

	//AddToolBarButton(0);	// Separator
	//AddToolBarButton(IDM_FILE_PRINT, FALSE);

	//AddToolBarButton(0);	// Separator
	//AddToolBarButton(IDM_HELP_ABOUT);

}


// events from browser view


void CMainFrame::connect_events()
{
	m_event_sink_.SetSink(this);

	auto p_unk = get_browser().GetAXWindow().GetUnknown();
	if (!p_unk)
		return;

	auto pcp = get_connection_point(DIID_DWebBrowserEvents2);
	if (!pcp)
		return;

	pcp->Advise(&m_event_sink_, &m_event_cookie_);
	pcp->Release();
	p_unk->Release();
}

IConnectionPoint* CMainFrame::get_connection_point(REFIID riid)
{
	auto p_unk = get_browser().GetAXWindow().GetUnknown();
	if (!p_unk)
		return nullptr;

	IConnectionPointContainer* pcpc;
	IConnectionPoint* pcp = nullptr;

	const auto hr = p_unk->QueryInterface(IID_IConnectionPointContainer, reinterpret_cast<void**>(&pcpc));
	if (SUCCEEDED(hr))
	{
		pcpc->FindConnectionPoint(riid, &pcp);
		pcpc->Release();
	}

	p_unk->Release();

	return pcp;
}


// browser frame events
void CMainFrame::on_command_state_change(DISPPARAMS* p_disp_params)
{
	UNREFERENCED_PARAMETER(p_disp_params);
	//appendTranscriptNL("OnCommandStateChange");
}

void CMainFrame::on_document_complete(DISPPARAMS* p_disp_params)
{
	UNREFERENCED_PARAMETER(p_disp_params);
	//appendTranscriptNL("OnDocumentComplete");
}

void CMainFrame::on_download_begin(DISPPARAMS* p_disp_params)
{
	UNREFERENCED_PARAMETER(p_disp_params);
	//appendTranscriptNL("OnDownloadBegin");
}

void CMainFrame::on_download_complete(DISPPARAMS* p_disp_params)
{
	UNREFERENCED_PARAMETER(p_disp_params);
	//appendTranscriptNL("OnDownloadComplete");
}


void CMainFrame::on_title_change(DISPPARAMS* p_disp_params)
{
	UNREFERENCED_PARAMETER(p_disp_params);
	//appendTranscriptNL("OnTitleChange");
}


void CMainFrame::on_progress_change(DISPPARAMS* p_disp_params)
{
	UNREFERENCED_PARAMETER(p_disp_params);
	//appendTranscriptNL("OnProgressChange");
}

void CMainFrame::on_property_change(DISPPARAMS* pDispParams)
{
	UNREFERENCED_PARAMETER(pDispParams);
	//appendTranscriptNL("OnPropertyChange");
}

extern RE2* re_allow_list;
extern RE2* re_block_list;
bool invoke_browser_on_navigate(char* url);

void CMainFrame::on_before_navigate2(const wchar_t* url, short* cancel)
{
	*cancel = TRUE;
	auto surl = ws_2s(url);

	// the url is allow listed so we script does not look at it. (static content)
	if (re_allow_list != nullptr && re2::RE2::PartialMatch(surl, *re_allow_list))
	{
		*cancel = FALSE;
		return;
	}

	// the url is not blocklisted so script does look at it. (not an app query)
	if (re_block_list != nullptr && !re2::RE2::PartialMatch(surl, *re_block_list))
	{
		*cancel = FALSE;
		return;
	}


	if (invoke_browser_on_navigate(const_cast<char*>(surl.c_str())))
	{
		return;
	}
	*cancel = FALSE;
}


void CMainFrame::on_navigate_complete2(DISPPARAMS* p_disp_params)
{
	UNREFERENCED_PARAMETER(p_disp_params);
}


// This application supports a singe browser pane only
void CMainFrame::on_new_window2(DISPPARAMS* pDispParams)
{
	UNREFERENCED_PARAMETER(pDispParams);
}


void CMainFrame::on_status_text_change(DISPPARAMS* pDispParams)
{

	UNREFERENCED_PARAMETER(pDispParams);
	m_completed_ = true;
}
