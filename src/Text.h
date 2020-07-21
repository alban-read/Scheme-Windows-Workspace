///////////////////////////////////////////////////////
// Text.h - Declaration of the CViewText, CContainText, 
//          and CDockText classes

#ifndef TEXT_H
#define TEXT_H
#include "gdiplus.h" 
#include "EventSink.h"



// these flags control the launch mode.
extern "C" __declspec(dllimport) bool inGuiMode();

extern "C" __declspec(dllimport) bool inConsoleMode();

extern "C" __declspec(dllimport) bool LoggingOn();

// Declaration of the CViewText class
class CViewText : public CWnd
{
public:
	CViewText() {}
	virtual ~CViewText() {}
	virtual void append_text(char* text);
protected:
	virtual void PreCreate(CREATESTRUCT &cs);
	virtual int OnCreate(CREATESTRUCT&);
	virtual void OnInitialUpdate();
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT on_drop_files(UINT uMsg, WPARAM wParam, LPARAM lParam);


};

class CViewResponseText : public CWnd
{
public:
	CViewResponseText() {}
	virtual ~CViewResponseText() {}
 
protected:
	virtual void PreCreate(CREATESTRUCT &cs);
	virtual void OnInitialUpdate();

};


class CViewTranscriptText : public CWnd
{
public:
	CViewTranscriptText() {}
	virtual ~CViewTranscriptText() {}

protected:
	virtual void PreCreate(CREATESTRUCT &cs);
	virtual void OnInitialUpdate();

};

// Declaration of the CContainText class
class CContainText : public c_dock_container
{
public:
	CContainText();
	~CContainText() {}

private:
	CViewText m_ViewText;
};

 




 

class CDockBrowser : public c_docker
{
public:
	CDockBrowser();
	virtual ~CDockBrowser() {}
	virtual CWebBrowser& GetBrowser() const { return (CWebBrowser&)m_ViewBrowser; }

public:
	CWebBrowser	m_ViewBrowser;

};

// Declaration of the CDockText class
class CDockText : public c_docker
{
public:
	CDockText();
	virtual ~CDockText() {}

private:
	CContainText m_View;

};


// Declaration of the CContainText class
class CContainResponseText : public c_dock_container
{
public:
	CContainResponseText();
	~CContainResponseText() {}

private:
	CViewResponseText m_ViewResponseText;
};

// Declaration of the CContainText class
class CContainTranscriptText : public c_dock_container
{
public:
	CContainTranscriptText();
	~CContainTranscriptText() {}

private:
	CViewTranscriptText m_ViewTranscriptText;
};



class CDockResponseText : public c_docker
{
public:
	CDockResponseText();
	virtual ~CDockResponseText() {}

private:
	CContainResponseText m_View;

};

class CDockTranscriptText : public c_docker
{
public:
	CDockTranscriptText();
	virtual ~CDockTranscriptText() {}

private:
	CContainTranscriptText m_View;

};
 
class CViewImage : public CWnd
{
public:
	CViewImage() {}
	virtual ~CViewImage() {}
	virtual void OnInitialUpdate();
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnDraw(CDC* pDC);
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void PreCreate(CREATESTRUCT& cs);
};



// Declaration of the CContainText class
class CContainImage : public c_dock_container
{
public:
	CContainImage();
	~CContainImage() {}

public:
	CViewImage m_ViewImage;
};

class CDockImage : public c_docker
{
public:
	CDockImage();
	virtual ~CDockImage() {}

public:
	CContainImage m_View;

};


#endif // TEXT_H

