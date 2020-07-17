
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

namespace Text {
	extern ptr utf8_string_separated_to_list(char* s, const char sep);
	void trim_utf8(std::string& hairy);
	int strlen_utf8(const char* s);
	ptr utf8_to_sstring(char* s);
	std::wstring widen(const std::string& in);
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

 
DWORD WINAPI  garbage_collect(LPVOID cmd)
{
	while (true) {
		auto dw_wait_result = WaitForSingleObject(g_script_mutex, 200);
		while (dw_wait_result == WAIT_TIMEOUT)
		{
			dw_wait_result = WaitForSingleObject(g_script_mutex, 200);
		}
		CALL0("gc");
		ReleaseMutex(g_script_mutex);
		Sleep(500);
	}
}



std::deque<std::string> commands;

// script execution; 

bool cancelling = false;

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


DWORD WINAPI  process_commands(LPVOID x)
{
	while (true) {

		while (commands.empty())
		{
			Sleep(10);
		}

		std::string eval;
		WaitForSingleObject(g_commands_mutex, INFINITE);
		if (!commands.empty()) {

			eval = commands.front();
			commands.pop_front();
		}
		ReleaseMutex(g_commands_mutex);

		WaitForSingleObject(g_script_mutex, INFINITE);
		try {
			if (timing==true) {
				auto t1 = ellapsed_seconds_since_starting();
				CALL1("eval->string", Sstring(eval.c_str()));
				auto t2 = ellapsed_seconds_since_starting() - t1;
				if (inGuiMode() &&
					theApp.GetMainFrame().GetHwnd() != nullptr) {
					auto ellapsed = Text::widen(fmt::format("{0}", t2));
					auto sbw = theApp.GetMainFrame().GetStatusBar().SetPartWidth(3, 200);
					auto ok = theApp.GetMainFrame().GetStatusBar().SetPartText(3, ellapsed.c_str());
				}
			}
			else {
				CALL1("eval->string", Sstring(eval.c_str()));
			}
		}
		catch (...) {

		}
		ReleaseMutex(g_script_mutex);
		::Sleep(20);
	}
}

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

DWORD WINAPI  busy_indicator(LPVOID x)
{
	RECT rect;
	int last_pending_commands = -1;
	while (true) {
		int pending_commands;
		WaitForSingleObject(g_commands_mutex, INFINITE);
		pending_commands = commands.size();
		ReleaseMutex(g_commands_mutex);

		if (pending_commands != last_pending_commands) {
			last_pending_commands = pending_commands;
			auto ok = theApp.GetMainFrame().GetStatusBar().SetPartText(0, L"Ready");
			if (pending_commands > 0) {
				auto text_message = Text::widen(fmt::format("Busy: {0} in queue.", pending_commands));
				auto ok = theApp.GetMainFrame().GetStatusBar().SetPartText(0, text_message.c_str());
			}
		}
		Sleep(250);
	}
}

VOID CALLBACK watch_dog(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	// This GUI HANG should not happen given that scripts execute on entirely separate threads
	const PDWORD_PTR result = nullptr;
	if (inGuiMode() &&
		theApp.GetMainFrame().GetHwnd() != nullptr
		&& SendMessageTimeout(theApp.GetMainFrame().GetHwnd(), WM_USER + 502, 0, 0, SMTO_ABORTIFHUNG, 1000,
			result) == 0)
	{
		auto* e = new char[2048];
		sprintf_s(e, 2000, "App hang at %5.3f seconds!", ellapsed_seconds_since_starting());
		appendTranscriptNL(e);
		auto ok = theApp.GetMainFrame().GetStatusBar().SetPartText(0, L"APP Busy");

	}
}

void start_watch_dog() {

	HANDLE timer_queue = nullptr;
	const auto seconds = 1;
	HANDLE h_timer = nullptr;
	if (nullptr == timer_queue)
	{
		timer_queue = CreateTimerQueue();
	}
	try {
		CreateTimerQueueTimer(&h_timer, timer_queue,
			static_cast<WAITORTIMERCALLBACK>(watch_dog), nullptr, 10 * 1000, seconds * 1000, 0);
	}
	catch (const CException& e)
	{
		// Display the exception and quit
		MessageBox(nullptr, e.GetText(), AtoT(e.what()), MB_ICONERROR);
		return;
	}
}


void eval_text(const char* cmd)
{
	WaitForSingleObject(g_commands_mutex, INFINITE);
	commands.emplace_back(cmd);
	ReleaseMutex(g_commands_mutex);
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
	auto ok=theApp.GetMainFrame().GetStatusBar().SetPartText(3, L"--.--");
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
			auto err=theApp.GetMainFrame().GetStatusBar().SetPartText(3, L"!!.!!");
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
		auto err = theApp.GetMainFrame().GetStatusBar().SetPartText(3, L"B!.!!");
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
"map max member memq memv min "
"negative? null? numerator "
"pair? positive? procedure? "
"quasiquote quote quotient "
"read real? remainder reverse "
"seq sequence set! set-car! set-cdr! sqrt sublist symbol? "
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


LRESULT CViewImage::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{

	case WM_SIZE:
	case WM_PAINT:
	case WM_WINDOWPOSCHANGED:

	{
		// display background image; or hatch
		auto dc = this->GetDC();

		if (background != nullptr) {

			const auto cw = GetClientRect().Width();
			const auto ch = GetClientRect().Height();
			auto *bmp = new Gdiplus::Bitmap(cw, ch, PixelFormat32bppRGB);
			Gdiplus::Graphics g2(bmp);
			Gdiplus::Pen black(Gdiplus::Color::Black, 1);
			const auto h = background->GetHeight();
			const auto w = background->GetWidth();

			// if not in a fill mode have a border to draw.
			if (_graphics_mode != 4) {
				Gdiplus::Brush* brush = new Gdiplus::HatchBrush(Gdiplus::HatchStyle::HatchStyleLargeCheckerBoard,
				                                                Gdiplus::Color::DarkGray, Gdiplus::Color::LightGray);
				g2.FillRectangle(brush, 0, 0, cw, ch);
				delete brush;
			}

			Gdiplus::Graphics g(dc.GetHDC());

			switch (_graphics_mode) {
			case 0: // 1:1
				g2.DrawRectangle(&black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(background, 5, 5, w, h);
				break;
			case 1: // 1:2
				g2.DrawRectangle(&black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(background, 5, 5, w / 2, h / 2);
				break;
			case 2: // 1:4
				g2.DrawRectangle(&black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(background, 5, 5, w / 4, h / 4);
				break;
			case 3: // 2:1
				g2.DrawRectangle(&black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(background, 5, 5, w * 2, h * 2);
				break;
			case 4: // Fill
				g.DrawImage(background, 1, 1, cw - 2, ch - 2);
				break;
			case 5: // 2:3
				g2.DrawRectangle(&black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(background, 1, 1, w * 2 / 3, h * 2 / 3);
				break;
			case 6: // 3:4
				g2.DrawRectangle(&black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(background, 1, 1, w * 3 / 4, h * 3 / 4);
				break;
			case 7: // 3:2
				g2.DrawRectangle(&black, 0, 0, cw - 1, ch - 1);
				g2.DrawImage(background, 1, 1, w * 3 / 2, h * 3 / 2);
				break;

			}
			if (_graphics_mode != 4) {
				g.DrawImage(bmp, 0, 0, cw, ch);
				delete bmp;
			}


		}
		else {

			//// just a nice hatch style for the backdrop
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
	background = nullptr;
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