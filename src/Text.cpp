
#include "stdafx.h"
#include "Text.h"
#include "resource.h"
#include "SciLexer.h"
#include "Scintilla.h"
#include "scheme/scheme.h"
#include "ContainerApp.h"
#include <thread>
#include <iostream>
#include <fstream>
#include <fmt/format.h>
#include <deque>

#define CALL0(who) Scall0(Stop_level_value(Sstring_to_symbol(who)))
#define CALL1(who, arg) Scall1(Stop_level_value(Sstring_to_symbol(who)), arg)
#define CALL2(who, arg, arg2) Scall2(Stop_level_value(Sstring_to_symbol(who)), arg, arg2)

extern bool sc_init;
extern CDockContainerApp theApp;
extern DWORD WINAPI  execstartup(LPVOID cmd);

// what mode are we running in.
extern bool launch_gui;
extern char* noncezero;
 
 

namespace GlobalGraphics {
	Gdiplus::Bitmap* __stdcall get_display_surface(void);
	int graphics_mode();
}


namespace Text {
	ptr utf8_string_separated_to_list(char* s, const char sep);
	void trim_utf8(std::string& hairy);
	int strlen_utf8(const char* s);
	ptr utf8_to_sstring(char* s);
	std::wstring widen(const std::string& in);
}

namespace Assoc {
	ptr sstring(const char* symbol, const char* value);
	ptr sflonum(const char* symbol, const float value);
	ptr sfixnum(const char* symbol, const int value);
	ptr sptr(const char* symbol, ptr value);
	ptr cons_sstring(const char* symbol, const char* value, ptr l);
	ptr cons_sbool(const char* symbol, bool value, ptr l);
	ptr cons_sptr(const char* symbol, ptr value, ptr l);
	ptr cons_sflonum(const char* symbol, const float value, ptr l);
	ptr cons_sfixnum(const char* symbol, const int value, ptr l);
}

struct stop_watch {
public:

	void reset() {

		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		pc_freq_ = double(li.QuadPart);
		QueryPerformanceCounter(&li);
		counter_start_ = li.QuadPart;
	}

	double get_counter() const
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return double(li.QuadPart - counter_start_) / pc_freq_ / 1000;
	}

	double seconds() const
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return double(li.QuadPart - counter_start_) / pc_freq_;
	}

private:
	__int64 counter_start_ = 0;
	double pc_freq_ = 0;
};



// A few basic colors
const COLORREF black = RGB(0, 0, 0);
const COLORREF white = RGB(255, 255, 255);
const COLORREF green = RGB(0, 0xCC, 0xC9);
const COLORREF red = RGB(255, 0, 0);
const COLORREF blue = RGB(0, 0, 245);
const COLORREF yellow = RGB(255, 255, 0);
const COLORREF magenta = RGB(255, 0, 255);
const COLORREF cyan = RGB(0, 255, 255);
const COLORREF gray = RGB(120, 120, 180);
const COLORREF orange = RGB(255, 102, 0);




LRESULT send_editor(HWND h, UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0)
{
	return ::SendMessage(h, Msg, wParam, lParam);
}

HWND response;
HWND transcript;
HWND inputed;

extern "C" void appendTranscriptNL(char *s);
extern "C" void appendTranscriptNL(char *s);
extern "C" void appendTranscript2NL(char *s, char *s1);

ptr utf8_sep_to_list(char* s, const char sep) {
	auto lst = Text::utf8_string_separated_to_list(s,sep);
	return lst;
}

HANDLE script_thread = nullptr;
HANDLE g_script_mutex;
HANDLE g_commands_mutex;
extern HANDLE g_image_rotation_mutex;

bool timing = true;

__int64 timers_counter_start;
double pc_freq;

void start_time_stamp_timing() {
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	pc_freq = double(li.QuadPart);
	QueryPerformanceCounter(&li);
	timers_counter_start = li.QuadPart;
}

double ellapsed_seconds_since_starting() {
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - timers_counter_start) / pc_freq;
}

// wait for a while for the last script thread to stop running
bool timeount_on_script_lock(const int turns)
{
	auto dw_wait_result = WaitForSingleObject(g_script_mutex, 100);
	auto time_out = turns;
	while (--time_out > 0 && (dw_wait_result == WAIT_TIMEOUT))
	{
		dw_wait_result = WaitForSingleObject(g_script_mutex, 100);
	}
	return time_out == 0;
}

 
 


std::deque<std::string> commands;

// script execution; 

bool cancelling = false;

void eval_text(const char* cmd)
{
	WaitForSingleObject(g_commands_mutex, INFINITE);
	commands.emplace_back(cmd);
	ReleaseMutex(g_commands_mutex);
}

void safe_cancel_commands()
{
	cancelling = true;
	WaitForSingleObject(g_commands_mutex, INFINITE);
	while (!commands.empty())
	{
		commands.pop_front();
	}
	commands.shrink_to_fit();
	ReleaseMutex(g_commands_mutex);
	Sleep(250);
}


void init_commands();

DWORD WINAPI  process_commands(LPVOID x)
{

	// scheme engine begins
	WaitForSingleObject(g_script_mutex,INFINITE);
    execstartup(L"");
	ReleaseMutex(g_script_mutex);

	int ticks = 0;

	// now watch for scripts to run.
	while (true) {

	 
		if (GetAsyncKeyState(VK_ESCAPE) != 0)
		{
			cancelling = true;
			WaitForSingleObject(g_commands_mutex, INFINITE);
			while (!commands.empty())
			{
				commands.pop_front();
			}
			commands.shrink_to_fit();
			ReleaseMutex(g_commands_mutex);
		}

	
		WaitForSingleObject(g_script_mutex, INFINITE);
		if (commands.empty()) {
			auto ok = theApp.GetMainFrame().GetStatusBar().SetPartText(0, L"Ready");
		}
		ReleaseMutex(g_script_mutex);
		while (commands.empty())
		{
			Sleep(25);
		}
		WaitForSingleObject(g_commands_mutex, INFINITE);
		std::string eval;
		int pending_commands;
		if (!commands.empty()) {
			eval = commands.front();
			commands.pop_front();
			pending_commands = commands.size();
			if (pending_commands > 0) {
				auto text_message = Text::widen(fmt::format("Busy: {0} in queue.", pending_commands));
				auto ok = theApp.GetMainFrame().GetStatusBar().SetPartText(0, text_message.c_str());
			}
		}
		ReleaseMutex(g_commands_mutex);

		WaitForSingleObject(g_script_mutex, INFINITE);
		try {
		
			if (pending_commands == 0) {
				auto ok = theApp.GetMainFrame().GetStatusBar().SetPartText(0, L"Busy");
			}
			if (timing==true) {
				auto t1 = ellapsed_seconds_since_starting();
				CALL1("eval->string", Sstring(eval.c_str()));
				auto t2 = ellapsed_seconds_since_starting() - t1;
				if (inGuiMode() &&
					theApp.GetMainFrame().GetHwnd() != nullptr) {
					auto ellapsed = Text::widen(fmt::format("{0}", t2));
					auto ok = theApp.GetMainFrame().GetStatusBar().SetPartText(4, ellapsed.c_str());
				}
			}
			else {
				CALL1("eval->string", Sstring(eval.c_str()));
			}
		}
		catch (...) {
			ReleaseMutex(g_script_mutex);
		}
		// run some gc
		ticks++;
		if (ticks % 10 == 0) {
			CALL0("gc");
		}
		ReleaseMutex(g_script_mutex);
		::Sleep(20);
	}	
 
}

// this is the scheme thread 
void init_commands() {


	// script exec background thread
	static auto cmd_thread = CreateThread(
		nullptr,
		0,
		process_commands,
		nullptr,
		0,
		nullptr);
}


void cancel_commands()
{
	cancelling = true;
	WaitForSingleObject(g_commands_mutex, INFINITE);
	while (!commands.empty())
	{
		commands.pop_front();
	}
	commands.shrink_to_fit();
	ReleaseMutex(g_commands_mutex);
	Sleep(250);
}



void eval_scite(HWND hc)
{
	const auto l = send_editor(hc, SCI_GETLENGTH) + 1;
	auto *cmd = new(std::nothrow) char[l];
	if (cmd == nullptr) {
		return;
	}
	memset(cmd, 0, l);
	send_editor(hc, SCI_GETTEXT, l, reinterpret_cast<LPARAM>(cmd));
	eval_text(cmd); 
}

char *sc_getText(HWND hc) {
	const auto l = send_editor(hc, SCI_GETLENGTH) + 1;
	auto cmd = new(std::nothrow) char[l];
	memset(cmd, 0, l);
	send_editor(hc, SCI_GETTEXT, l, reinterpret_cast<LPARAM>(cmd));
	return cmd;
}

const char *sc_getExprText() {
	return sc_getText(inputed);
}

char* get_browser_selection_text();
void eval_selected_scite(HWND hc)
{
	auto ok=theApp.GetMainFrame().GetStatusBar().SetPartText(4, L"--.--");
	ptr result;

	// if in text pane
	if (hc == response || hc == transcript || hc == inputed) {

		const int l = send_editor(hc, SCI_GETSELTEXT) + 1;
		auto *cmd = new(std::nothrow) char[l];
		if (cmd == nullptr) {
			return;
		}
		memset(cmd, 0, l);
		send_editor(hc, SCI_GETSELTEXT, l, reinterpret_cast<LPARAM>(cmd));
	 
		try {
			eval_text(cmd);
		}
		catch (...) {
			auto err=theApp.GetMainFrame().GetStatusBar().SetPartText(4, L"ERROR");
			return;
		}
		return;
	}

	// check browser; oddly this does work fine.
	char* cmd=get_browser_selection_text();
	 
	appendTranscript2NL("\r\n;; -- evaluating browser selection -- \r\n ", cmd);
	try {
		eval_text(cmd);
	}
	catch (...) {
		auto err = theApp.GetMainFrame().GetStatusBar().SetPartText(4, L"ERROR");
	}

}

void show_line_numbers(HWND h) {
	send_editor(h, SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
	send_editor(h, SCI_SETMARGINWIDTHN, 0, 48);
	send_editor(h, SCI_SETMARGINWIDTHN, 1, 0);
}

void set_a_style(HWND h, int style, COLORREF fore, COLORREF back = RGB(0xFF, 0xFF, 0xEA), int size = -1, const char *face = 0, bool bold = false, bool italic = false)
{
	send_editor(h, SCI_STYLESETFORE, style, fore);
	send_editor(h, SCI_STYLESETBACK, style, back);
	if (size >= 8)
		send_editor(h, SCI_STYLESETSIZE, style, size);
	if (face)
		send_editor(h, SCI_STYLESETFONT, style, reinterpret_cast<LPARAM>(face));
	if (bold)
		send_editor(h, SCI_STYLESETBOLD, style, 1);
	if (italic)
		send_editor(h, SCI_STYLESETITALIC, style, 1);
}


void sc_setText(HWND h, char *text) {
	int l = strlen(text);
	send_editor(h, SCI_SETTEXT, l, reinterpret_cast<LPARAM>(text));
}


void sc_appendText(HWND h, char *text) {
	const int l = strlen(text);
	send_editor(h, SCI_APPENDTEXT, l, reinterpret_cast<LPARAM>(text));
	Sleep(10);
}

void sc_setTextFromFile(HWND h, char *fname) {
	std::ifstream f(fname);
	std::stringstream buffer;
	buffer << f.rdbuf();
	buffer.seekg(0, std::ios::end);
	const int size = buffer.tellg();
	send_editor(h, SCI_SETTEXT, size, reinterpret_cast<LPARAM>(buffer.str().c_str()));
}

void sc_setEditorFromFile(char *fname) {
	sc_setTextFromFile(inputed, fname);
}

// response back from scheme to our evaluate request.
extern "C" {

	void set_inputed(char *s)
	{
		sc_setText(inputed, s);
	}

	ptr get_inputed()
	{
		const int l = send_editor(inputed, SCI_GETLENGTH) + 1;
		auto cmd = new(std::nothrow) char[l];
		memset(cmd, 0, l);
		send_editor(inputed, SCI_GETTEXT, l, reinterpret_cast<LPARAM>(cmd));
		const auto s = Sstring(cmd);
		delete[] cmd;
		return s;
	}

	void appendEditor(char *s)
	{
		sc_setText(response, s);
		Sleep(10);
	}
	void appendTranscript(char *s)
	{
		if (inGuiMode()) {
			sc_appendText(transcript, s);
		}
		if (inConsoleMode()) {
			std::cout << s << std::endl;
		}
	}
	void appendTranscriptNL(char *s)
	{
		if (inGuiMode()) {
			sc_appendText(transcript, s);
			sc_appendText(transcript, "\r\n");
		}
		if (inConsoleMode()) {
			std::cout << s << std::endl;
		}
	}


	// called from scheme engine.
	void appendTranscript1(char *s)
	{
		if (inGuiMode()) {
			sc_appendText(transcript, s);
		}
		if (inConsoleMode()) {
			std::cout << s << std::endl;
		}
	}

	void appendTranscript2(char *s, char *s1)
	{
		if (inGuiMode()) {
			sc_appendText(transcript, s);
			sc_appendText(transcript, " ");
			sc_appendText(transcript, s1);
		}
		if (inConsoleMode()) {
			std::cout << s << "" << s1 << std::endl;
		}
	}
	void appendTranscript2NL(char *s, char *s1)
	{
		if (inGuiMode()) {
			sc_appendText(transcript, s);
			sc_appendText(transcript, " ");
			sc_appendText(transcript, s1);
			sc_appendText(transcript, "\r\n");
		}
		if (inConsoleMode()) {
			std::cout << s << "" << s1 << std::endl;
		}
	}
	void appendTranscript3NL(char *s, char *s1, char *s3)
	{
		if (inGuiMode()) {
			sc_appendText(transcript, s);
			sc_appendText(transcript, " ");
			sc_appendText(transcript, s1);
			sc_appendText(transcript, " ");
			sc_appendText(transcript, s3);
			sc_appendText(transcript, "\r\n");
		}
		if (inConsoleMode()) {
			std::cout << s << "" << s1 << " " << s3 << std::endl;
		}
	}
	
	void clear_transcript()
	{
		if (inGuiMode()) {
			send_editor(transcript, SCI_CLEARALL);
		}
	}
}


static const char g_scheme[] =
"abs any append append! apply assoc assq assv begin boolean? "
"caaaar caaadr caaar caadar caaddr caadr caar cadaar cadadr cadar caddar cadddr caddr cadr car "
"case case-lambda call-with-values catch "
"cdaaar cdaadr cdaar cdadar cdaddr cdadr cdar cddaar cddadr cddar cdddar cddddr cdddr cddr cdr "
"char? char=? char>? char<? char<=? char>=? char->integer "
"cond cons cons* define define-syntax denominator "
"display do dotimes elseeq? eof-object? eqv? eval exact? expt "
"field filter find first floor for format for-each format from "
"hash "
"if in inexact? integer? integer->char import iota "
"lambda last-pair length let let* letrec let-rec list list? list-ref  list->string list-tail "
"map max member memq memv min mod"
"negative? null? numerator "
"pair? positive? procedure? "
"quasiquote quote quotient "
"read real? remainder reverse "
"seq sequence set! set-car! set-cdr! sqrt step sublist symbol? "
"string? string=? string-append string-copy string-length string->list string-ref  string-set! substring syntax-rules "
"to try truncate  "
"unless unquote "
"values vector vector? vector-length vector->list vector-ref vector-set! "
"while when where write ";

static const char g_scheme2[] =
"and and-let* nil not or null? ";
static const char g_scheme3[] =
" ";
static const char g_scheme4[] =
"#f #t '()";

struct s_scintilla_colors
{
	int         iItem;
	COLORREF    rgb;
	int			size;
	char*		face;
	bool		bold;
	bool		italic;
};

static s_scintilla_colors scheme[] =
{	//	item					colour				sz	face					bold	italic
	{ SCE_C_COMMENT,          RGB(160,82,45),		11, "Consolas",	false, true },
	{ SCE_C_COMMENTLINE,      RGB(184,138,0),		11, "Consolas",	false, false },
	{ SCE_C_COMMENTDOC,       RGB(32,178,170),		11, "Consolas",	false, false },
	{ SCE_C_NUMBER,           RGB(138,43,226),		11, "Consolas",	false, false },
	{ SCE_C_STRING,           RGB(140,140,140),		11, "Consolas",	false, false },
	{ SCE_C_CHARACTER,        RGB(100,100,100),		11, "Consolas",	false, false },
	{ SCE_C_UUID,             cyan,					11, "Consolas",	false, false },
	{ SCE_C_OPERATOR,         black,				11, "Consolas",	false, false },
	{ SCE_C_WORD,             RGB(0,138,184),		11, "Consolas",	true,  false },
	{ SCE_C_WORD2,            RGB(0,138,184),		11, "Consolas",	false,  false },
	{ SCE_C_REGEX,			  RGB(186,85,211),		11, "Consolas",	false, false },
	{ -1,                     0 }
};



void show_folds(HWND h) {

	send_editor(h, SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
	send_editor(h, SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));
	send_editor(h, SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
	send_editor(h, SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.html"), reinterpret_cast<LPARAM>("1"));

	send_editor(h, SCI_SETMARGINWIDTHN, 1, 0);
	send_editor(h, SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
	send_editor(h, SCI_SETMARGINMASKN, 1, SC_MASK_FOLDERS);
	send_editor(h, SCI_SETMARGINWIDTHN, 1, 20);
	send_editor(h, SCI_SETMARGINSENSITIVEN, 1, 1);

	send_editor(h, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
	send_editor(h, SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
	send_editor(h, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
	send_editor(h, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
	send_editor(h, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
	send_editor(h, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
	send_editor(h, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
	send_editor(h, SCI_SETFOLDFLAGS, 16, 0);
}

void hide_folds(HWND h) {

	send_editor(h, SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("0"));
	send_editor(h, SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));
	send_editor(h, SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("0"));
	send_editor(h, SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.html"), reinterpret_cast<LPARAM>("0"));
	send_editor(h, SCI_SETMARGINMASKN, 1, 0);
	send_editor(h, SCI_SETMARGINWIDTHN, 1, 0);
	send_editor(h, SCI_SETMARGINSENSITIVEN, 1, 0);
}

void initialize_editor(HWND h) {

	inputed = h;

	send_editor(h, SCI_SETCODEPAGE, SC_CP_UTF8);
	send_editor(h, SCI_SETLEXER, SCLEX_LISP);

	// Set tab width
	send_editor(h, SCI_SETTABWIDTH, 4);

	// line wrap (avoid h scroll bars)
	send_editor(h, SCI_SETWRAPMODE, 1);
	send_editor(h, SCI_SETWRAPVISUALFLAGS, 2);
	send_editor(h, SCI_SETWRAPSTARTINDENT, 6);

	// auto sel
	send_editor(h, SCI_AUTOCSETSEPARATOR, ',');
	send_editor(h, SCI_AUTOCSETMAXHEIGHT, 9);

	// dwell/ used for brackets
	send_editor(h, SCI_SETMOUSEDWELLTIME, 500);
	set_a_style(h, STYLE_DEFAULT, gray, RGB(0xFF, 0xFF, 0xEA), 11, "Consolas");

	// Set caret foreground color
	send_editor(h, SCI_SETCARETFORE, RGB(0, 0, 255));

	// Set selection color
	send_editor(h, SCI_SETSELBACK, TRUE, RGB(0xF0, 0xF0, 0xFE));

	// Set all styles
	send_editor(h, SCI_STYLECLEARALL);

	for (long i = 0; scheme[i].iItem != -1; i++)
		set_a_style(h, scheme[i].iItem,
			scheme[i].rgb,
			RGB(0xFF, 0xFF, 0xEA),
			scheme[i].size,
			scheme[i].face,
			scheme[i].bold,
			scheme[i].italic);

	set_a_style(h, STYLE_BRACELIGHT, orange, RGB(0xFF, 0xFF, 0xEA), 11, "Consolas");
	set_a_style(h, STYLE_BRACEBAD, red, RGB(0xFF, 0xFF, 0xEA), 11, "Consolas");


	send_editor(h, SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(g_scheme));
	send_editor(h, SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(g_scheme2));
	send_editor(h, SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(g_scheme3));
	send_editor(h, SCI_SETKEYWORDS, 3, reinterpret_cast<LPARAM>(g_scheme4));

	
	send_editor(h, SCI_ENSUREVISIBLEENFORCEPOLICY, 2);
	send_editor(h, SCI_GOTOLINE, 2);
}



void initialize_response_editor(HWND h) {

	response = h;
 
	send_editor(h, SCI_SETCODEPAGE, SC_CP_UTF8);
	send_editor(h, SCI_SETLEXER, SCLEX_LISP);

	// Set tab width
	send_editor(h, SCI_SETTABWIDTH, 4);

	// line wrap (avoid h scroll bars)
	send_editor(h, SCI_SETWRAPMODE, 1);
	send_editor(h, SCI_SETWRAPVISUALFLAGS, 2);
	send_editor(h, SCI_SETWRAPSTARTINDENT, 6);

	// auto sel
	send_editor(h, SCI_AUTOCSETSEPARATOR, ',');
	send_editor(h, SCI_AUTOCSETMAXHEIGHT, 9);

	// dwell/ used for brackets
	send_editor(h, SCI_SETMOUSEDWELLTIME, 500);
	set_a_style(h, STYLE_DEFAULT, gray, RGB(0xFF, 0xFF, 0xEA), 11, "Consolas");

	// Set caret foreground color
	send_editor(h, SCI_SETCARETFORE, RGB(0, 0, 255));

	// Set selection color
	send_editor(h, SCI_SETSELBACK, TRUE, RGB(0xF0, 0xF0, 0xFE));

	// Set all styles
	send_editor(h, SCI_STYLECLEARALL);

	for (long i = 0; scheme[i].iItem != -1; i++)
		set_a_style(h, scheme[i].iItem,
			scheme[i].rgb,
			RGB(0xFF, 0xFF, 0xEA),
			scheme[i].size,
			scheme[i].face,
			scheme[i].bold,
			scheme[i].italic);

	set_a_style(h, STYLE_BRACELIGHT, orange, RGB(0xFF, 0xFF, 0xEA), 11, "Consolas");
	set_a_style(h, STYLE_BRACEBAD, red, RGB(0xFF, 0xFF, 0xEA), 11, "Consolas");


	send_editor(h, SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(g_scheme));
	send_editor(h, SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(g_scheme2));
	send_editor(h, SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(g_scheme3));
	send_editor(h, SCI_SETKEYWORDS, 3, reinterpret_cast<LPARAM>(g_scheme4));


	send_editor(h, SCI_ENSUREVISIBLEENFORCEPOLICY, 2);
	send_editor(h, SCI_GOTOLINE, 2);
	char buffer[250];
	snprintf(buffer, 50, ";; Scheme version %s\n", Skernel_version());
	sc_setText(h, buffer);
}


void initialize_transcript_editor(HWND h) {

	transcript = h;

	send_editor(h, SCI_SETCODEPAGE, SC_CP_UTF8);
	// Set tab width
	send_editor(h, SCI_SETTABWIDTH, 4);

	// line wrap (avoid h scroll bars)
	send_editor(h, SCI_SETWRAPMODE, 1);
	send_editor(h, SCI_SETWRAPVISUALFLAGS, 2);
	send_editor(h, SCI_SETWRAPSTARTINDENT, 6);

	// auto sel
	send_editor(h, SCI_AUTOCSETSEPARATOR, ',');
	send_editor(h, SCI_AUTOCSETMAXHEIGHT, 9);

	// dwell
	send_editor(h, SCI_SETMOUSEDWELLTIME, 500);
	set_a_style(h, STYLE_DEFAULT, gray, RGB(0xFF, 0xFF, 0xEA), 10, "Consolas");

	// Set caret foreground color
	send_editor(h, SCI_SETCARETFORE, RGB(0, 0, 255));

	// Set selection color
	send_editor(h, SCI_SETSELBACK, TRUE, RGB(0xF0, 0xF0, 0xFE));

	// Set all styles
	send_editor(h, SCI_STYLECLEARALL);
	sc_setText(h, ";; Transcript\r\n");

}

static bool is_brace(const int c)
{
	switch (c)
	{
	case '(':
	case ')':
	case '[':
	case ']':
	case '{':
	case '}':
		return true;
	default: 
		return false;
	}
}


// this is hooked into the notify event in the header files.
// this just adds brace highlighting.

void scintillate(SCNotification *N, LPARAM lParam) {
	const auto h = static_cast<HWND>(N->nmhdr.hwndFrom);

	if (N->nmhdr.code == SCN_CHARADDED) {
		auto c = N->ch;
		const int lpos = send_editor(h, SCI_GETCURRENTPOS);
		const int line = send_editor(h, SCI_LINEFROMPOSITION, lpos);
	}

	else if (N->nmhdr.code == SCN_UPDATEUI) {
		const int lpos = send_editor(h, SCI_GETCURRENTPOS);
		const int c = send_editor(h, SCI_GETCHARAT, lpos, 0);
		if (is_brace(c)) {
			const int bracepos = send_editor(h, SCI_BRACEMATCH, lpos, 0);
			if (bracepos != -1) {
				send_editor(h, SCI_BRACEHIGHLIGHT, lpos, bracepos);
			}
			else {
				SendMessage(h, SCI_BRACEBADLIGHT, lpos, 0);
			}
		}
	}
	else if (N->nmhdr.code == SCN_DWELLSTART) {
		const int lpos = send_editor(h, SCI_GETCURRENTPOS);
		const int c = send_editor(h, SCI_GETCHARAT, lpos, 0);
		if (is_brace(c)) {
			const int bracepos = send_editor(h, SCI_BRACEMATCH, lpos, 0);
			if (bracepos != -1) {
				send_editor(h, SCI_BRACEHIGHLIGHT, lpos, bracepos);
			}
			else {
				SendMessage(h, SCI_BRACEBADLIGHT, lpos, 0);
			}
		}
	}
}

 
 

///////////////////////////////////////////////
// CViewText functions

void CViewText::OnInitialUpdate()
{
	HWND h = this->GetHwnd();
	initialize_editor(h);
}

void CViewText::PreCreate(CREATESTRUCT &cs)
{
	cs.lpszClass = _T("Scintilla");

}

int CViewText::OnCreate(CREATESTRUCT&)
{

	return 0;
}


void CViewText::append_text(char* text)
{
	const auto h = this->GetHwnd();
	sc_setText(h, text);
}


void CViewResponseText::OnInitialUpdate()
{
	const auto h = this->GetHwnd();
	initialize_response_editor(h);

}

void CViewResponseText::PreCreate(CREATESTRUCT &cs)
{
	cs.lpszClass = _T("Scintilla");
}

void CViewTranscriptText::OnInitialUpdate()
{
	const auto h = this->GetHwnd();
	initialize_transcript_editor(h);
}

void CViewTranscriptText::PreCreate(CREATESTRUCT &cs)
{
	cs.lpszClass = _T("Scintilla");
}




//////////////////////////////////////////////
//  Definitions for the CDockBrowserclass
CDockBrowser::CDockBrowser()
{
	c_docker::SetView(theApp.GetMainFrame().m_Bview);

}

 

//////////////////////////////////////////////
//  Definitions for the CContainText class
CContainText::CContainText()
{
	SetDockCaption(_T("Evaluator View"));
	SetTabText(_T("Evaluator"));
	SetTabIcon(IDI_TEXT);
	SetView(m_ViewText);

}

//////////////////////////////////////////////
//  Definitions for the CContainText class
CContainResponseText::CContainResponseText()
{
	SetDockCaption(_T("Response View"));
	SetTabText(_T("Response"));
	SetTabIcon(IDI_TEXT);
	SetView(m_ViewResponseText);
}

//////////////////////////////////////////////
//  Definitions for the CContainText class
CContainTranscriptText::CContainTranscriptText()
{
	SetDockCaption(_T("Transcript View"));
	SetTabText(_T("Transcript"));
	SetTabIcon(IDI_TEXT);
	SetView(m_ViewTranscriptText);
}

//////////////////////////////////////////////
//  Definitions for the CDockText class
CDockText::CDockText()
{
	// Set the view window to our edit control
	c_docker::SetView(m_View);


	// Set the width of the splitter bar
	SetBarWidth(4);
}

//////////////////////////////////////////////
//  Definitions for the CDockText class
CDockResponseText::CDockResponseText()
{
	// Set the view window to our edit control
	c_docker::SetView(m_View);

	// Set the width of the splitter bar
	SetBarWidth(4);
}
//////////////////////////////////////////////
//  Definitions for the CDockText class
CDockTranscriptText::CDockTranscriptText()
{
	// Set the view window to our edit control
	c_docker::SetView(m_View);

	// Set the width of the splitter bar
	SetBarWidth(4);
}


LRESULT CViewText::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg)
	{
	default: ;
	}
	return WndProcDefault(uMsg, wParam, lParam);
}


LRESULT CViewText::on_drop_files(UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	return 0L;
}



// graphic viewer
// windows procedure


// gui window layout

Gdiplus::Pen pen_black(Gdiplus::Color::Black, 1);

struct {
	int when;
	boolean left;
	boolean right;
	boolean up;
	boolean down;
	boolean ctrl;
	boolean space;
	int key_code;
} graphics_keypressed;


ptr graphics_keys(void) {
	ptr a = Snil;
	a = Assoc::cons_sbool("left", graphics_keypressed.left,a);
	a = Assoc::cons_sbool("right", graphics_keypressed.right, a);
	a = Assoc::cons_sbool("up", graphics_keypressed.up, a);
	a = Assoc::cons_sbool("down", graphics_keypressed.down, a);
	a = Assoc::cons_sbool("ctrl", graphics_keypressed.ctrl, a);
	a = Assoc::cons_sbool("space", graphics_keypressed.space, a);
	a = Assoc::cons_sfixnum("key", graphics_keypressed.key_code, a);
	a = Assoc::cons_sfixnum("recent", GetTickCount() - graphics_keypressed.when, a);
	return a;
}


Gdiplus::Bitmap* image_view_bitmap = nullptr;
int previous_cw = 0;
int previous_ch = 0;

 


LRESULT CViewImage::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto hdc = this->GetDC();
	switch (uMsg)
	{

	case WM_KEYDOWN:
		graphics_keypressed.ctrl = false;
		graphics_keypressed.left = false;
		graphics_keypressed.right = false;
		graphics_keypressed.down = false;
		graphics_keypressed.up = false;
		graphics_keypressed.space = false;
		graphics_keypressed.key_code = wParam;
		graphics_keypressed.when = GetTickCount();
		switch (wParam) {

		case VK_CONTROL:
			graphics_keypressed.ctrl = true;
			break;
		case VK_LEFT:
			graphics_keypressed.left = true;
			break;
		case VK_RIGHT:
			graphics_keypressed.right = true;
			break;
		case VK_UP:
			graphics_keypressed.up = true;
			break;
		case VK_DOWN:
			graphics_keypressed.down = true;
			break;
		case VK_SPACE:
			graphics_keypressed.space = true;
			break;

		}
		break;
	 
	case WM_TIMER:
	// fast display timer
		if (GlobalGraphics::get_display_surface() != nullptr) {

			const auto cw = GetClientRect().Width();
			const auto ch = GetClientRect().Height();
			const auto h = GlobalGraphics::get_display_surface()->GetHeight();
			const auto w = GlobalGraphics::get_display_surface()->GetWidth();
			Gdiplus::Graphics g(hdc);
			g.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
			g.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
			g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
			g.SetSmoothingMode(Gdiplus::SmoothingModeNone);
			g.SetInterpolationMode(Gdiplus::InterpolationModeDefault);
			g.DrawRectangle(&pen_black, 0, 0, cw - 1, ch - 1);
			g.DrawImage(GlobalGraphics::get_display_surface(), 5, 5, w, h);
			ReleaseMutex(g_image_rotation_mutex);
		}
		break;
	case WM_PAINT:
	case WM_SHOWWINDOW:
	case WM_SIZE:
	case WM_ACTIVATE:
	case WM_WINDOWPOSCHANGED:
	{
		// display background image; or hatch; behind our image
		int _graphics_mode = GlobalGraphics::graphics_mode();
		if (GlobalGraphics::get_display_surface() != nullptr) {

			const auto cw = GetClientRect().Width();
			const auto ch = GetClientRect().Height();


			if (previous_ch != ch || previous_ch != ch) {
				if (image_view_bitmap != nullptr) {
					delete image_view_bitmap;
					image_view_bitmap = nullptr;

				}
			}

			if (image_view_bitmap == nullptr) {
				appendTranscriptNL("*new screen image*");
				image_view_bitmap = new Gdiplus::Bitmap(cw, ch, PixelFormat32bppRGB);
			}

			previous_cw = cw;
			previous_ch = ch;

			// g2 can draw onto image_view.
			Gdiplus::Graphics g2(image_view_bitmap);

			const auto h = GlobalGraphics::get_display_surface()->GetHeight();
			const auto w = GlobalGraphics::get_display_surface()->GetWidth();

			// if not in a fill mode have a border to draw.
			if (_graphics_mode != 4 ) {
				Gdiplus::Brush* brush = new Gdiplus::HatchBrush(Gdiplus::HatchStyle::HatchStyleLargeCheckerBoard,
					Gdiplus::Color::DarkGray, Gdiplus::Color::LightGray);
				g2.FillRectangle(brush, 0, 0, cw, ch);
				delete brush;
			}

			Gdiplus::Graphics g(hdc);

			WaitForSingleObject(g_image_rotation_mutex, INFINITE);
		
			switch (_graphics_mode) {
			case 0:
				g2.DrawRectangle(&pen_black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(GlobalGraphics::get_display_surface(), 5, 5, w, h);
				break;
			case 1: // 1:2
				g2.DrawRectangle(&pen_black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(GlobalGraphics::get_display_surface(), 5, 5, w / 2, h / 2);
				break;
			case 2: // 1:4
				g2.DrawRectangle(&pen_black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(GlobalGraphics::get_display_surface(), 5, 5, w / 4, h / 4);
				break;
			case 3: // 2:1
				g2.DrawRectangle(&pen_black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(GlobalGraphics::get_display_surface(), 5, 5, w * 2, h * 2);
				break;
			case 4: // Fill direct to hdc window.
				g.DrawImage(GlobalGraphics::get_display_surface(), 1, 1, cw - 2, ch - 2);
				break;
			case 5: // 2:3
				g2.DrawRectangle(&pen_black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(GlobalGraphics::get_display_surface(), 1, 1, w * 2 / 3, h * 2 / 3);
				break;
			case 6: // 3:4
				g2.DrawRectangle(&pen_black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(GlobalGraphics::get_display_surface(), 1, 1, w * 3 / 4, h * 3 / 4);
				break;
			case 7: // 3:2
				g2.DrawRectangle(&pen_black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(GlobalGraphics::get_display_surface(), 1, 1, w * 3 / 2, h * 3 / 2);
				break;
			case 8: // 1:1 - faster
				g2.DrawRectangle(&pen_black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(GlobalGraphics::get_display_surface(), 1, 1, w * 3 / 2, h * 3 / 2);
				break;
			}
			ReleaseMutex(g_image_rotation_mutex);
			if (_graphics_mode != 4 ) {
				// draw third buffer 
				g.DrawImage(image_view_bitmap, 0, 0, cw, ch);
			}
			
		}
		else {

			//// just a nice hatch style for the backdrop
			const auto cw = GetClientRect().Width();
			const auto ch = GetClientRect().Height();
			Gdiplus::Pen black_pen(Gdiplus::Color::Black, 1);
			Gdiplus::Brush* brush = new Gdiplus::HatchBrush(Gdiplus::HatchStyle::HatchStyleLargeCheckerBoard,
				Gdiplus::Color::DarkGray, Gdiplus::Color::LightGray);
			Gdiplus::Graphics g(hdc);
			g.FillRectangle(brush, 0, 0, cw, ch);
			g.DrawRectangle(&black_pen, 0, 0, cw - 1, ch - 1);

			delete brush;
		}
		break;
	}

	case WM_ERASEBKGND:
		return FALSE;
	default: ;
	}

	return WndProcDefault(uMsg, wParam, lParam);
}



BOOL CViewImage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);


	switch (LOWORD(wParam))
	{

		/*case ID_GRAPHICS_TOCLIPBOARD:
			ViewToClipBoard(this);
			break;

		case ID_GRAPHICVIEW_CLEAR:
			this->setBackground(NULL);
			this->Invalidate();

		case ID_SIZE_SCALE_DOUBLE:
			this->setMode(3);
			this->Invalidate();
			break;

		case ID_SIZE_FULL:
			this->setMode(0);
			this->Invalidate();
			break;

		case ID_SIZE_HALF:
			this->setMode(1);
			this->Invalidate();
			break;

		case ID_SIZE_QUARTER:
			this->setMode(2);
			this->Invalidate();
			break;

		case ID_SIZE_FILLDISPLAY:
			this->setMode(4);
			this->Invalidate();
			break;


		case ID_GRAPHICVIEW_REFRESH:
			this->Invalidate();
			break;

		case ID_SIZE_SCALE3:
			this->setMode(5);
			this->Invalidate();
			break;

		case ID_SIZE_SCALE4:
			this->setMode(6);
			this->Invalidate();
			break;

		case ID_SIZE_SCALE5:
			this->setMode(7);
			this->Invalidate();
			break;
	*/
	default: ;
	}
	return true;

}


void CViewImage::PreCreate(CREATESTRUCT &cs)
{
 
}

HWND image_hwnd;
void CViewImage::OnInitialUpdate()
{
	const auto h = this->GetHwnd();
	image_hwnd = h;
	const auto dc = this->GetDC();
	const auto cw = GetClientRect().Width();
	const auto ch = GetClientRect().Height();
	Gdiplus::Pen black_pen(Gdiplus::Color::Black, 1);
	Gdiplus::Brush* brush = new Gdiplus::HatchBrush(Gdiplus::HatchStyle::HatchStyleLargeCheckerBoard,
	                                                Gdiplus::Color::DarkGray, Gdiplus::Color::LightGray);
	Gdiplus::Graphics g(dc.GetHDC());
	g.FillRectangle(brush, 0, 0, cw, ch);
	g.DrawRectangle(&black_pen, 0, 0, cw - 1, ch - 1);

	delete brush;
}


void CViewImage::OnDraw(CDC* pDC)
{

}


CContainImage::CContainImage()
{
	SetDockCaption(_T("Image View"));
	SetTabText(_T("Image"));
	SetTabIcon(IDI_TEXT);
	SetView(m_ViewImage);
}

//////////////////////////////////////////////
//  Definitions for the CDockText class
CDockImage::CDockImage()
{
	// Set the view window to our edit control
	c_docker::SetView(m_View);

	// Set the width of the splitter bar
	SetBarWidth(3);
}


/// LICENSE TEXT
void invoke_browser_license();
void display_license_text() {
	invoke_browser_license();
}