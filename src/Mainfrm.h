/////////////////////////////////////////////
// Mainfrm.h

#ifndef MAINFRM_H
#define MAINFRM_H


#include "Simple.h"
#include "Text.h"



// Declaration of the CMainFrame class
class CMainFrame : public c_dock_frame
{
public:
	CMainFrame();
	virtual ~CMainFrame();
	void load_default_dockers();
	void load_image_dockers();
	void load_full_image_dockers();
	void load_browser_dockers();
	void load_default_browser_dockers();
	void load_full_browser_dockers();
	BOOL on_dock_close_all();
	BOOL on_dock_default();
	BOOL on_dock_image();
	BOOL on_dock_full_image();
	BOOL on_dock_browser();
	BOOL on_dock_full_browser();
	static BOOL on_browser_help();
	static BOOL on_browser_clear();
	static BOOL on_browser_welcome();
	static BOOL on_browser_back();
	static BOOL on_browser_forward();
	BOOL on_file_exit() const;

	CViewSimple m_View;
	c_docker* input{};
	c_docker* output{};
	c_docker* image{};
	c_docker* trans{};
	c_docker* browser{};
	HWND out_window{};
	CWebBrowser m_Bview;

	IWebBrowser2 * get_i_browser() const
	{
		return m_Bview.GetIWebBrowser2();
	}

	CWebBrowser& get_browser() const { return const_cast<CWebBrowser&>(m_Bview); }

	virtual int OnCreate(CREATESTRUCT&) override;
	virtual void OnInitialUpdate() override;
	virtual void connect_events();

	virtual IConnectionPoint* get_connection_point(REFIID riid);

	virtual void on_before_navigate2(const wchar_t * url, short *cancel);
	virtual void on_command_state_change(DISPPARAMS* p_disp_params);
	virtual void on_document_complete(DISPPARAMS* p_disp_params);
	virtual void on_download_begin(DISPPARAMS* p_disp_params);
	virtual void on_download_complete(DISPPARAMS* p_disp_params);
	virtual void on_navigate_complete2(DISPPARAMS* p_disp_params);
	virtual void on_new_window2(DISPPARAMS* pDispParams);
	virtual void on_progress_change(DISPPARAMS* p_disp_params);
	virtual void on_property_change(DISPPARAMS* pDispParams);
	virtual void on_status_text_change(DISPPARAMS* pDispParams);
	virtual void on_title_change(DISPPARAMS* p_disp_params);

	inline BOOL pre_translate_message(MSG* pMsg) const
	{
		// make del and tab keys work in the browser control.
		if (pMsg->wParam == VK_TAB || pMsg->wParam == VK_DELETE) {
			IOleInPlaceActiveObject* pao;
			auto *browser2 = get_browser().GetIWebBrowser2();
			if (browser2->QueryInterface(IID_IOleInPlaceActiveObject, reinterpret_cast<void**>(&pao)) == S_OK) {
				pao->TranslateAccelerator(pMsg);
				pao->Release();
			}

		}
		return FALSE;
	}

protected:
	virtual c_docker* NewDockerFromID(int nID) override;

	LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;


	void PreCreate(CREATESTRUCT& cs) override;
	BOOL SaveRegistrySettings() override;
	void SetupToolBar() override;




private:

	CEventSink	m_event_sink_;
	BOOL		m_completed_{};
	DWORD		m_event_cookie_{};


};

#endif //MAINFRM_H

