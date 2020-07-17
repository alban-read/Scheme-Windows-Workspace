// Win32++   Version 8.4
// Release Date: 10th March 2017
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2017  David Nash
//
// Permission is hereby granted, free of charge, to
// any person obtaining a copy of this software and
// associated documentation files (the "Software"),
// to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice
// shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
////////////////////////////////////////////////////////


// Acknowledgements:
// ================
// A special thanks to James Brown and Jeff Glatt for publishing tutorials
// on docking and making these freely available. This code uses some of the
// ideas presented in those tutorials.
//
// Catch22 Docking Tutorials by James Brown:
// http://www.catch22.net/tuts
//
// Docking Toolbars in Plain C by Jeff Glatt
// http://www.codeproject.com/Articles/10224/Docking-Toolbars-in-Plain-C


///////////////////////////////////////////////////////
// wxx_docking.h
//  Declaration of the CDocker class

#ifndef _WIN32XX_DOCKING_H_
#define _WIN32XX_DOCKING_H_


#include "wxx_wincore.h"
#include "wxx_toolbar.h"
#include "wxx_tab.h"
#include "wxx_regkey.h"
#include "wxx_themes.h"
#include "default_resource.h"


// Ensure these are defined
#ifndef GA_ROOT
#define GA_ROOT					2
#endif

#ifndef WM_UNINITMENUPOPUP
#define WM_UNINITMENUPOPUP		0x0125
#endif

#ifndef WM_MENURBUTTONUP
#define WM_MENURBUTTONUP		0x0122
#endif


// Docking Styles
#define DS_DOCKED_LEFT			0x0001  // Dock the child left
#define DS_DOCKED_RIGHT			0x0002  // Dock the child right
#define DS_DOCKED_TOP			0x0004  // Dock the child top
#define DS_DOCKED_BOTTOM		0x0008  // Dock the child bottom
#define DS_NO_DOCKCHILD_LEFT	0x0010  // Prevent a child docking left
#define DS_NO_DOCKCHILD_RIGHT	0x0020  // Prevent a child docking right
#define DS_NO_DOCKCHILD_TOP		0x0040  // Prevent a child docking at the top
#define DS_NO_DOCKCHILD_BOTTOM	0x0080  // Prevent a child docking at the bottom
#define DS_NO_RESIZE			0x0100  // Prevent resizing
#define DS_NO_CAPTION			0x0200  // Prevent display of caption when docked
#define DS_NO_CLOSE				0x0400	// Prevent closing of a docker while docked
#define DS_NO_UNDOCK			0x0800  // Prevent undocking of a docker
#define DS_CLIENTEDGE			0x1000  // Has a 3D border when docked
#define DS_NO_FIXED_RESIZE		0x2000	// Perform a proportional resize instead of a fixed size resize on dock children
#define DS_DOCKED_CONTAINER		0x4000  // Dock a container within a container
#define DS_DOCKED_LEFTMOST      0x10000 // Leftmost outer docking
#define DS_DOCKED_RIGHTMOST     0x20000 // Rightmost outer docking
#define DS_DOCKED_TOPMOST		0x40000 // Topmost outer docking
#define DS_DOCKED_BOTTOMMOST	0x80000 // Bottommost outer docking

// Required for Dev-C++
#ifndef TME_NONCLIENT
  #define TME_NONCLIENT 0x00000010
#endif
#ifndef TME_LEAVE
  #define TME_LEAVE 0x000000002
#endif
#ifndef WM_NCMOUSELEAVE
  #define WM_NCMOUSELEAVE 0x000002A2
#endif


namespace Win32xx
{
	// Class declarations
	class c_dock_container;
	class c_docker;

	typedef Shared_Ptr<c_docker> DockPtr;

	struct container_info
	{
		c_string Title;
		int iImage;
		c_dock_container* pContainer;
		container_info() : iImage(0), pContainer(0) {}
	};

	///////////////////////////////////////
	// Declaration of the CDockContainer class
	//  A CDockContainer is a CTab window. A CTab has a view window, and optionally a toolbar control.
	//  A top level CDockContainer can contain other CDockContainers. The view for each container
	//  (including the top level container) along with possibly its toolbar, is displayed
	//  within the container parent's view page.
	class c_dock_container : public CTab
	{
	public:

		// Nested class. This is the Wnd for the window displayed over the client area
		// of the tab control.  The toolbar and view window are child windows of the
		// viewpage window. Only the ViewPage of the parent CDockContainer is displayed. It's
		// contents are updated with the view window of the relevant container whenever
		// a different tab is selected.
		class c_view_page : public CWnd
		{

		public:
			c_view_page() : m_pContainer(NULL), m_pView(NULL), m_pTab(NULL) {}
			virtual ~c_view_page() {}
			virtual CToolBar& GetToolBar() const	{return m_ToolBar;}
			virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
			virtual int OnCreate(CREATESTRUCT& cs);
			virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
			virtual void PreRegisterClass(WNDCLASS& wc);
			virtual void RecalcLayout();
			LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

			CWnd* GetTabCtrl() const						{ return m_pTab;}
			CWnd& GetView() const { assert(m_pView); return *m_pView; }
			c_dock_container* GetContainer() const;
			void SetContainer(c_dock_container* pContainer)	{ m_pContainer = pContainer; }
			void SetView(CWnd& wndView);

		private:
			mutable CToolBar m_ToolBar;
			c_dock_container* m_pContainer;
			c_string m_strTooltip;
			CWnd* m_pView;
			CWnd* m_pTab;
		};

	public:
		c_dock_container();
		virtual ~c_dock_container();
		virtual void AddContainer(c_dock_container* pContainer, BOOL Insert = FALSE);
		virtual void AddToolBarButton(UINT nID, BOOL IsEnabled = TRUE);
		virtual c_dock_container* GetContainerFromIndex(UINT nPage);
		virtual c_dock_container* GetContainerFromView(CWnd* pView) const;
		virtual int GetContainerIndex(c_dock_container* pContainer);
		virtual SIZE GetMaxTabTextSize();
		virtual c_view_page& GetViewPage() const	{ return m_ViewPage; }
		virtual int GetTabImageID(UINT nTab) const;
		virtual c_string GetTabText(UINT nTab) const;
		virtual void RecalcLayout();
		virtual void RemoveContainer(c_dock_container* pWnd);
		virtual void SelectPage(int nPage);
		virtual void SetTabSize();
		virtual void SetupToolBar();
		virtual void SwapTabs(UINT nTab1, UINT nTab2);

		// Attributes
		c_dock_container* GetActiveContainer() const;
		CWnd* GetActiveView() const;
		const std::vector<container_info>& GetAllContainers() const {return m_pContainerParent->m_vContainerInfo;}
		c_dock_container* GetContainerParent() const { return m_pContainerParent; }
		c_string& GetDockCaption() const	{ return m_csCaption; }
		c_docker* GetDocker() const		{ return m_pDocker; }
		HICON GetTabIcon() const		{ return m_hTabIcon; }
		LPCTSTR GetTabText() const		{ return m_strTabText; }
		virtual CToolBar& GetToolBar()	const { return GetViewPage().GetToolBar(); }
		std::vector<UINT>& GetToolBarData() { return m_vToolBarData; }
		CWnd& GetView()	const			{ return GetViewPage().GetView(); }
		void SetActiveContainer(c_dock_container* pContainer);
		void SetDockCaption(LPCTSTR szCaption) { m_csCaption = szCaption; }
		void SetHideSingleTab(BOOL HideSingle);
		void SetTabIcon(HICON hTabIcon) { m_hTabIcon = hTabIcon; }
		void SetTabIcon(UINT nID_Icon);
		void SetTabIcon(int i, HICON hIcon) { CTab::SetTabIcon(i, hIcon); }
		void SetTabText(LPCTSTR szText) { m_strTabText = szText; }
		void SetTabText(UINT nTab, LPCTSTR szText);
		void SetView(CWnd& Wnd) const;


	protected:
		virtual void OnAttach();
		virtual LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnNotifyReflect(WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnTCNSelChange(LPNMHDR pNMHDR);
		virtual void PreCreate(CREATESTRUCT& cs);
		virtual void SetToolBarImages(COLORREF crMask, UINT ToolBarID, UINT ToolBarHotID, UINT ToolBarDisabledID);
		LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		std::vector<container_info>& GetAll() const {return m_pContainerParent->m_vContainerInfo;}
		std::vector<container_info> m_vContainerInfo;	// vector of ContainerInfo structs
		std::vector<UINT> m_vToolBarData;				// vector of resource IDs for ToolBar buttons
		c_string m_strTabText;
		mutable c_string m_csCaption;
		CImageList m_imlToolBar;
		CImageList m_imlToolBarHot;
		CImageList m_imlToolBarDis;
		mutable c_view_page m_ViewPage;
		int m_iCurrentPage;
		c_docker* m_pDocker;
		c_dock_container* m_pContainerParent;
		HICON m_hTabIcon;
		int m_nTabPressed;
		BOOL m_IsHideSingleTab;
		CPoint m_OldMousePos;
	};

	typedef struct DRAGPOS
	{
		NMHDR hdr;
		POINT ptPos;
		UINT DockZone;
		c_docker* pDocker;
	} *LPDRAGPOS;


	/////////////////////////////////////////
	// Declaration of the CDocker class
	//  A CDocker window allows other CDocker windows to be "docked" inside it.
	//  A CDocker can dock on the top, left, right or bottom side of a parent CDocker.
	//  There is no theoretical limit to the number of CDockers within CDockers.
	class c_docker : public CWnd
	{
	public:
		//  A nested class for the splitter bar that separates the docked panes.
		class c_dock_bar : public CWnd
		{
		public:
			c_dock_bar();
			virtual ~c_dock_bar();
			virtual void OnDraw(CDC& dc);
			virtual void PreCreate(CREATESTRUCT& cs);
			virtual void PreRegisterClass(WNDCLASS& wc);
			virtual void SendNotify(UINT nMessageID);
			virtual void SetColor(COLORREF color);
			LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

			CBrush GetBrushBkgnd() const	{return m_brBackground;}
			c_docker& GetDocker() const		{assert (m_pDocker); return *m_pDocker;}
			int GetWidth() const			{return m_DockBarWidth;}
			void SetDocker(c_docker& Docker)	{m_pDocker = &Docker;}
			void SetWidth(int nWidth)		{m_DockBarWidth = nWidth;}

		protected:
			virtual LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam);

		private:
			c_dock_bar(const c_dock_bar&);				// Disable copy construction
			c_dock_bar& operator = (const c_dock_bar&); // Disable assignment operator

			c_docker* m_pDocker;
			DRAGPOS m_DragPos;
			CBrush m_brBackground;
			int m_DockBarWidth;
		};

		//  A nested class for the window inside a CDocker which includes all of this docked client.
		//  It's the remaining part of the CDocker that doesn't belong to the CDocker's children.
		//  The Docker's view window is a child window of CDockClient.
		class c_dock_client : public CWnd
		{
		public:
			c_dock_client();
			virtual ~c_dock_client() {}
			virtual void Draw3DBorder(RECT& Rect);
			virtual void DrawCaption();
			virtual void DrawCloseButton(CDC& DrawDC, BOOL Focus);
			virtual CRect GetCloseRect() const;
			virtual void SendNotify(UINT nMessageID);

			c_string& GetCaption() const		{ return m_csCaption; }
			CWnd& GetView() const			{ assert (m_pView); return *m_pView; }
			void SetDocker(c_docker* pDocker)	{ m_pDocker = pDocker;}
			void SetCaption(LPCTSTR szCaption)	{ m_csCaption = szCaption; }
			void SetCaptionColors(COLORREF Foregnd1, COLORREF Backgnd1, COLORREF ForeGnd2, COLORREF BackGnd2, COLORREF PenColor);
			void SetView(CWnd& wndView);

		protected:
			virtual LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT	OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT OnNCCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT OnNCHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT OnNCLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT OnNCLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT OnNCMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT OnNCMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT OnNCPaint(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);
			virtual void    PreRegisterClass(WNDCLASS& wc);
			virtual void    PreCreate(CREATESTRUCT& cs);
			LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

		private:
			LRESULT MouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
			c_dock_client(const c_dock_client&);				// Disable copy construction
			c_dock_client& operator = (const c_dock_client&);	// Disable assignment operator

			mutable c_string m_csCaption;
			CPoint m_Oldpt;
			c_docker* m_pDocker;
			CWnd* m_pView;
			BOOL m_IsClosePressed;
			BOOL m_IsOldFocusStored;
			BOOL m_IsCaptionPressed;
			BOOL m_IsTracking;
			COLORREF m_Foregnd1;
			COLORREF m_Backgnd1;
			COLORREF m_Foregnd2;
			COLORREF m_Backgnd2;
			COLORREF m_PenColor;
		};

		//  This nested class is used to indicate where a window could dock by
		//  displaying a blue tinted window.
		class c_dock_hint : public CWnd
		{
		public:
			c_dock_hint();
			virtual ~c_dock_hint();
			virtual RECT CalcHintRectContainer(c_docker* pDockTarget);
			virtual RECT CalcHintRectInner(c_docker* pDockTarget, c_docker* pDockDrag, UINT uDockSide);
			virtual RECT CalcHintRectOuter(c_docker* pDockDrag, UINT uDockSide);
			virtual void DisplayHint(c_docker* pDockTarget, c_docker* pDockDrag, UINT uDockSide);
			virtual void OnDraw(CDC& dc);
			virtual void PreCreate(CREATESTRUCT& cs);

		private:
			c_dock_hint(const c_dock_hint&);				// Disable copy construction
			c_dock_hint& operator = (const c_dock_hint&); // Disable assignment operator

			CBitmap m_bmBlueTint;
			UINT m_uDockSideOld;
		};

		class c_target : public CWnd
		{
		public:
			c_target() {}
			virtual ~c_target();
			virtual void OnDraw(CDC& dc);
			virtual void PreCreate(CREATESTRUCT& cs);

		protected:
			CBitmap m_bmImage;

		private:
			c_target(const c_target&);				// Disable copy construction
			c_target& operator = (const c_target&); // Disable assignment operator
		};

		class c_target_centre : public c_target
		{
		public:
			c_target_centre();
			virtual ~c_target_centre();
			virtual void OnDraw(CDC& dc);
			virtual int  OnCreate(CREATESTRUCT& cs);
			virtual BOOL CheckTarget(LPDRAGPOS pDragPos);
			BOOL IsOverContainer() { return m_IsOverContainer; }

		private:
			c_target_centre(const c_target_centre&);				// Disable copy construction
			c_target_centre& operator = (const c_target_centre&); // Disable assignment operator

			BOOL m_IsOverContainer;
			c_docker* m_pOldDockTarget;
		};

		class c_target_left : public c_target
		{
		public:
			c_target_left() {m_bmImage.LoadBitmap(IDW_SDLEFT);}
			virtual BOOL CheckTarget(LPDRAGPOS pDragPos);

		private:
			c_target_left(const c_target_left&);				// Disable copy construction
			c_target_left& operator = (const c_target_left&); // Disable assignment operator
		};

		class c_target_top : public c_target
		{
		public:
			c_target_top() {m_bmImage.LoadBitmap(IDW_SDTOP);}
			virtual BOOL CheckTarget(LPDRAGPOS pDragPos);
		private:
			c_target_top(const c_target_top&);				// Disable copy construction
			c_target_top& operator = (const c_target_top&); // Disable assignment operator
		};

		class c_target_right : public c_target
		{
		public:
			c_target_right() {m_bmImage.LoadBitmap(IDW_SDRIGHT);}
			virtual BOOL CheckTarget(LPDRAGPOS pDragPos);

		private:
			c_target_right(const c_target_right&);				// Disable copy construction
			c_target_right& operator = (const c_target_right&); // Disable assignment operator
		};

		class c_target_bottom : public c_target
		{
		public:
			c_target_bottom() {m_bmImage.LoadBitmap(IDW_SDBOTTOM);}
			virtual BOOL CheckTarget(LPDRAGPOS pDragPos);
		};

		friend class c_target_centre;
		friend class c_target_left;
		friend class c_target_top;
		friend class c_target_right;
		friend class c_target_bottom;
		friend class c_dock_client;
		friend class c_dock_container;

	public:
		// Operations
		c_docker();
		virtual ~c_docker();
		virtual c_docker* AddDockedChild(c_docker* pDocker, DWORD dwDockStyle, int DockSize, int nDockID = 0);
		virtual c_docker* AddUndockedChild(c_docker* pDocker, DWORD dwDockStyle, int DockSize, const RECT& rc, int nDockID = 0);
		virtual void Close();
		virtual void CloseAllDockers();
		virtual void Dock(c_docker* pDocker, UINT uDockSide);
		virtual void DockInContainer(c_docker* pDocker, DWORD dwDockStyle);
		virtual c_dock_container* GetContainer() const;
		virtual c_docker* GetActiveDocker() const;
		virtual c_docker* GetDockAncestor() const;
		virtual c_docker* GetDockFromID(int n_DockID) const;
		virtual c_docker* GetDockFromView(CWnd* pView) const;
		virtual c_docker* GetTopmostDocker() const;
		virtual int GetDockSize() const;
		virtual CTabbedMDI* GetTabbedMDI() const;
		virtual int GetTextHeight();
		virtual void Hide();
		virtual BOOL LoadContainerRegistrySettings(LPCTSTR szRegistryKeyName);
		virtual BOOL LoadDockRegistrySettings(LPCTSTR szRegistryKeyName);
		virtual void RecalcDockLayout();
		virtual BOOL SaveDockRegistrySettings(LPCTSTR szRegistryKeyName);
		virtual void Undock(CPoint pt, BOOL ShowUndocked = TRUE);
		virtual void UndockContainer(c_dock_container* pContainer, CPoint pt, BOOL ShowUndocked);
		virtual BOOL VerifyDockers();

		// Attributes
		virtual c_dock_bar& GetDockBar() const		{ return m_DockBar; }
		virtual c_dock_client& GetDockClient() const	{ return m_DockClient; }
		virtual c_dock_hint& GetDockHint() const		{ return m_pDockAncestor->m_DockHint; }
		virtual CRect GetViewRect() const			{ return GetClientRect(); }

		virtual CWnd& GetView() const						{ return GetDockClient().GetView(); }
		virtual void SetView(CWnd& wndView);

		const std::vector <DockPtr> & GetAllDockChildren() const	{return GetDockAncestor()->m_vAllDockChildren;}
		const std::vector <c_docker*> & GetDockChildren() const		{return m_vDockChildren;}
		const std::vector <c_docker*> & GetAllDockers()	const		{return m_vAllDockers;}
		int GetBarWidth() const				{return GetDockBar().GetWidth();}
		c_string& GetCaption() const			{return GetDockClient().GetCaption();}
		int GetDockID() const				{return m_nDockID;}
		c_docker* GetDockParent() const		{return m_pDockParent;}
		DWORD GetDockStyle() const			{return m_DockStyle;}
		BOOL IsChildOfDocker(HWND hWnd) const;
		BOOL IsDocked() const;
		BOOL IsDragAutoResize() const;
		BOOL IsRelated(HWND hWnd) const;
		BOOL IsUndocked() const;
		void SetBarColor(COLORREF color) {GetDockBar().SetColor(color);}
		void SetBarWidth(int nWidth) {GetDockBar().SetWidth(nWidth);}
		void SetCaption(LPCTSTR szCaption);
		void SetCaptionColors(COLORREF Foregnd1, COLORREF Backgnd1, COLORREF ForeGnd2, COLORREF BackGnd2, COLORREF PenColor = RGB(160, 150, 140));
		void SetCaptionHeight(int nHeight);
		void SetDockStyle(DWORD dwDockStyle);
		void SetDockSize(int DockSize);
		void SetDragAutoResize(BOOL AutoResize);
		BOOL SetRedraw(BOOL Redraw = TRUE);

	protected:
		virtual c_docker* NewDockerFromID(int idDock);
		virtual int  OnCreate(CREATESTRUCT& cs);
		virtual void OnDestroy();
		virtual LRESULT OnBarEnd(LPDRAGPOS pdp);
		virtual LRESULT OnBarMove(LPDRAGPOS pdp);
		virtual LRESULT OnBarStart(LPDRAGPOS pdp);
		virtual LRESULT OnDockEnd(LPDRAGPOS pdp);
		virtual LRESULT OnDockMove(LPDRAGPOS pdp);
		virtual LRESULT OnDockStart(LPDRAGPOS pdp);
		virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
		virtual void PreCreate(CREATESTRUCT& cs);
		virtual void PreRegisterClass(WNDCLASS& wc);
		
		// Not intended to be overwritten
		LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

		// Current declarations of message handlers
		virtual LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnDockActivated(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnDockDestroyed(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnExitSizeMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnNCLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		c_docker(const c_docker&);				// Disable copy construction
		c_docker& operator = (const c_docker&);	// Disable assignment operator
		std::vector <DockPtr> & GetAllChildren() const {return GetDockAncestor()->m_vAllDockChildren;}
		virtual c_docker* GetDockUnderDragPoint(POINT pt);
		void CheckAllTargets(LPDRAGPOS pDragPos);
		void CloseAllTargets();
		void DockOuter(c_docker* pDocker, DWORD dwDockStyle);
		void DrawAllCaptions();
		void DrawHashBar(HWND hBar, POINT Pos);
		void ConvertToChild(HWND hWndParent);
		void ConvertToPopup(const CRect& rc);
		void MoveDockChildren(c_docker* pDockTarget);
		void MoveDockInContainerChildren(c_docker* pDockTarget);
		void PromoteFirstChild();
		void RecalcDockChildLayout(CRect& rc);
		void ResizeDockers(LPDRAGPOS pdp);
		c_docker* SeparateFromDock();
		void SendNotify(UINT nMessageID);
		void SetUndockPosition(CPoint pt);
		std::vector<c_docker*> SortDockers();
		static BOOL CALLBACK EnumWindowsProc(HWND hWndTop, LPARAM lParam);

		mutable c_dock_bar		m_DockBar;
		mutable c_dock_hint		m_DockHint;
		mutable c_dock_client		m_DockClient;
		c_target_centre	m_TargetCentre;
		c_target_left		m_TargetLeft;
		c_target_top		m_TargetTop;
		c_target_right	m_TargetRight;
		CPoint			m_OldPoint;
		c_target_bottom	m_TargetBottom;
		c_docker*		m_pDockParent;
		c_docker*		m_pDockAncestor;

		std::vector <c_docker*> m_vDockChildren;		// Docker's immediate children
		std::vector <DockPtr> m_vAllDockChildren;	// All descendants of the DockAncestor (only used by the DockAncestor)
		std::vector <c_docker*> m_vAllDockers;		// DockAncestor + all descendants (only used by the DockAncestor)

		CRect m_rcBar;
		CRect m_rcChild;

		BOOL m_IsBlockMove;
		BOOL m_IsUndocking;
		BOOL m_IsClosing;
		BOOL m_IsDragging;
		BOOL m_IsDragAutoResize;
		int m_DockStartSize;
		int m_nDockID;
		int m_nRedrawCount;
		int m_NCHeight;
		DWORD m_dwDockZone;
		double m_DockSizeRatio;
		DWORD m_DockStyle;
		HWND m_hDockUnderPoint;
		CPoint m_DockPoint;

	}; // class CDocker

	struct dock_info
	{
		DWORD DockStyle;
		int DockSize;
		int DockID;
		int DockParentID;
		RECT Rect;
	};

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

	/////////////////////////////////////////////////////////////
	// Definitions for the CDockBar class nested within CDocker
	//
	inline c_docker::c_dock_bar::c_dock_bar() : m_pDocker(NULL), m_DockBarWidth(4)
	{
		ZeroMemory(&m_DragPos, sizeof(m_DragPos));
	}

	inline c_docker::c_dock_bar::~c_dock_bar()
	{
	}

	inline void c_docker::c_dock_bar::OnDraw(CDC& dc)
	{
		CRect rcClient = GetClientRect();
		dc.SelectObject(m_brBackground);
		dc.PatBlt(0, 0, rcClient.Width(), rcClient.Height(), PATCOPY);
	}

	inline LRESULT c_docker::c_dock_bar::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (!(m_pDocker->GetDockStyle() & DS_NO_RESIZE))
		{
			SendNotify(UWN_BARSTART);
			SetCapture();
		}

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_bar::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (!(m_pDocker->GetDockStyle() & DS_NO_RESIZE) && (GetCapture() == *this))
		{
			SendNotify(UWN_BAREND);
			ReleaseCapture();
		}

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_bar::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (!(m_pDocker->GetDockStyle() & DS_NO_RESIZE) && (GetCapture() == *this))
		{
			SendNotify(UWN_BARMOVE);
		}

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_bar::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (!(m_pDocker->GetDockStyle() & DS_NO_RESIZE))
		{
			HCURSOR hCursor;
			DWORD dwSide = GetDocker().GetDockStyle() & 0xF;
			if ((dwSide == DS_DOCKED_LEFT) || (dwSide == DS_DOCKED_RIGHT))
				hCursor = GetApp().LoadCursor(IDW_SPLITH);
			else
				hCursor = GetApp().LoadCursor(IDW_SPLITV);

			if (hCursor) SetCursor(hCursor);
			else TRACE("**WARNING** Missing cursor resource for slider bar\n");

			return TRUE;
		}
		else
			SetCursor(LoadCursor(nullptr, IDC_ARROW));

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline void c_docker::c_dock_bar::PreCreate(CREATESTRUCT& cs)
	{
		// Create a child window, initially hidden
		cs.style = WS_CHILD;
	}

	inline void c_docker::c_dock_bar::PreRegisterClass(WNDCLASS& wc)
	{
		wc.lpszClassName = _T("Win32++ Bar");
		wc.hbrBackground = m_brBackground;
	}

	inline void c_docker::c_dock_bar::SendNotify(UINT nMessageID)
	{
		// Send a splitter bar notification to the parent
		m_DragPos.hdr.code = nMessageID;
		m_DragPos.hdr.hwndFrom = GetHwnd();
		m_DragPos.ptPos = GetCursorPos();
		m_DragPos.ptPos.x += 1;
		m_DragPos.pDocker = m_pDocker;
		GetParent().SendMessage(WM_NOTIFY, 0L, reinterpret_cast<LPARAM>(&m_DragPos));
	}

	inline void c_docker::c_dock_bar::SetColor(COLORREF color)
	{
		// Useful colors:
		// GetSysColor(COLOR_BTNFACE)	// Default Grey
		// RGB(196, 215, 250)			// Default Blue

		m_brBackground.DeleteObject();
		m_brBackground.CreateSolidBrush(color);
	}

	inline LRESULT c_docker::c_dock_bar::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_SETCURSOR:		return OnSetCursor(uMsg, wParam, lParam);
		case WM_ERASEBKGND:		return 0L;
		case WM_LBUTTONDOWN:	return OnLButtonDown(uMsg, wParam, lParam);
		case WM_LBUTTONUP:		return OnLButtonUp(uMsg, wParam, lParam);
		case WM_MOUSEMOVE:		return OnMouseMove(uMsg, wParam, lParam);
		}

		// pass unhandled messages on for default processing
		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}


	////////////////////////////////////////////////////////////////
	// Definitions for the CDockClient class nested within CDocker
	//
	inline c_docker::c_dock_client::c_dock_client() : m_pDocker(nullptr), m_pView(nullptr), m_IsClosePressed(FALSE),
						m_IsOldFocusStored(FALSE), m_IsCaptionPressed(FALSE), m_IsTracking(FALSE)
	{
		m_Foregnd1 = RGB(32,32,32);
		m_Backgnd1 = RGB(190,207,227);
		m_Foregnd2 = GetSysColor(COLOR_BTNTEXT);
		m_Backgnd2 = GetSysColor(COLOR_BTNFACE);
		m_PenColor = RGB(160, 150, 140);
	}

	inline void c_docker::c_dock_client::Draw3DBorder(RECT& Rect)
	{
		// Imitates the drawing of the WS_EX_CLIENTEDGE extended style
		// This draws a 2 pixel border around the specified Rect
		CWindowDC dc(*this);
		CRect rcw = Rect;
		dc.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
		dc.MoveTo(0, rcw.Height());
		dc.LineTo(0, 0);
		dc.LineTo(rcw.Width(), 0);
		dc.CreatePen(PS_SOLID,1, GetSysColor(COLOR_3DDKSHADOW));
		dc.MoveTo(1, rcw.Height()-2);
		dc.LineTo(1, 1);
		dc.LineTo(rcw.Width()-2, 1);
		dc.CreatePen(PS_SOLID,1, GetSysColor(COLOR_3DHILIGHT));
		dc.MoveTo(rcw.Width()-1, 0);
		dc.LineTo(rcw.Width()-1, rcw.Height()-1);
		dc.LineTo(0, rcw.Height()-1);
		dc.CreatePen(PS_SOLID,1, GetSysColor(COLOR_3DLIGHT));
		dc.MoveTo(rcw.Width()-2, 1);
		dc.LineTo(rcw.Width()-2, rcw.Height()-2);
		dc.LineTo(1, rcw.Height()-2);
	}

	inline CRect c_docker::c_dock_client::GetCloseRect() const
	{
		// Calculate the close rect position in screen co-ordinates
		CRect rcClose;

		int gap = 4;
		CRect rc = GetWindowRect();
		int cx = GetSystemMetrics(SM_CXSMICON);
		int cy = GetSystemMetrics(SM_CYSMICON);

		rcClose.top = 2 + rc.top + m_pDocker->m_NCHeight/2 - cy/2;
		rcClose.bottom = 2 + rc.top + m_pDocker->m_NCHeight/2 + cy/2;
		rcClose.right = rc.right - gap;
		rcClose.left = rcClose.right - cx;

#if (WINVER >= 0x0500)
		if (GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
		{
			rcClose.left = rc.left + gap;
			rcClose.right = rcClose.left + cx;
		}
#endif


		return rcClose;
	}

	inline void c_docker::c_dock_client::DrawCaption()
	{
		if (IsWindow() && m_pDocker->IsDocked() && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
		{
			const auto focus = m_pDocker->IsChildOfDocker(GetFocus());
			m_IsOldFocusStored = FALSE;

			// Acquire the DC for our NonClient painting
			CWindowDC dc(*this);

			// Create and set up our memory DC
			CRect rc = GetWindowRect();
			CMemDC dcMem(dc);
			int rcAdjust = (GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_CLIENTEDGE)? 2 : 0;
			int Width = MAX(rc.Width() -rcAdjust, 0);
			int Height = m_pDocker->m_NCHeight + rcAdjust;
			dcMem.CreateCompatibleBitmap(dc, Width, Height);
			m_IsOldFocusStored = focus;

			// Set the font for the title
			NONCLIENTMETRICS info;
			ZeroMemory(&info, sizeof(NONCLIENTMETRICS));
			info.cbSize = GetSizeofNonClientMetrics();
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
			dcMem.CreateFontIndirect(info.lfStatusFont);

			// Set the Colours
			if (m_pDocker->GetActiveDocker() == m_pDocker)
			{
				dcMem.SetTextColor(m_Foregnd1);
				dcMem.CreateSolidBrush(m_Backgnd1);
				dcMem.SetBkColor(m_Backgnd1);
			}
			else
			{
				dcMem.SetTextColor(m_Foregnd2);
				dcMem.CreateSolidBrush(m_Backgnd2);
				dcMem.SetBkColor(m_Backgnd2);
			}

			// Draw the rectangle
			dcMem.CreatePen(PS_SOLID, 1, m_PenColor);
			dcMem.Rectangle(rcAdjust, rcAdjust, rc.Width() - rcAdjust, m_pDocker->m_NCHeight + rcAdjust);

			// Display the caption
			const auto cx = (m_pDocker->GetDockStyle() & DS_NO_CLOSE) ? 0 : GetSystemMetrics(SM_CXSMICON);
			CRect rcText(4 + rcAdjust, rcAdjust, rc.Width() - 4 - cx - rcAdjust, m_pDocker->m_NCHeight + rcAdjust);
			dcMem.DrawText(m_csCaption, m_csCaption.GetLength(), rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

			// Draw the close button
			if (!(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
				DrawCloseButton(dcMem, focus);

			// Draw the 3D border
			if (GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_CLIENTEDGE)
				Draw3DBorder(rc);

			// Copy the Memory DC to the window's DC
			dc.BitBlt(rcAdjust, rcAdjust, Width, Height, dcMem, rcAdjust, rcAdjust, SRCCOPY);
		}
	}

	inline void c_docker::c_dock_client::DrawCloseButton(CDC& DrawDC, BOOL Focus)
	{
		// The close button isn't displayed on Win95
		if (GetWinVersion() == 1400)  return;

		if (m_pDocker->IsDocked() && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
		{
			// Determine the close button's drawing position relative to the window
			CRect rcClose = GetCloseRect();
			UINT uState = GetCloseRect().PtInRect(GetCursorPos())? m_IsClosePressed && IsLeftButtonDown()? 2 : 1 : 0;
			ScreenToClient(rcClose);

			if (GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_CLIENTEDGE)
			{
				rcClose.OffsetRect(2, m_pDocker->m_NCHeight+2);
				if (GetWindowRect().Height() < (m_pDocker->m_NCHeight+4))
					rcClose.OffsetRect(-2, -2);
			}
			else
				rcClose.OffsetRect(0, m_pDocker->m_NCHeight-2);

			// Draw the outer highlight for the close button
			if (!IsRectEmpty(&rcClose))
			{
				switch (uState)
				{
				case 0:
					{
						// Normal button
						DrawDC.CreatePen(PS_SOLID, 1, RGB(232, 228, 220));
						DrawDC.MoveTo(rcClose.left, rcClose.bottom);
						DrawDC.LineTo(rcClose.right, rcClose.bottom);
						DrawDC.LineTo(rcClose.right, rcClose.top);
						DrawDC.LineTo(rcClose.left, rcClose.top);
						DrawDC.LineTo(rcClose.left, rcClose.bottom);
						break;
					}

				case 1:
					{
						// Popped up button
						// Draw outline, white at top, black on bottom
						DrawDC.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
						DrawDC.MoveTo(rcClose.left, rcClose.bottom);
						DrawDC.LineTo(rcClose.right, rcClose.bottom);
						DrawDC.LineTo(rcClose.right, rcClose.top);
						DrawDC.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
						DrawDC.LineTo(rcClose.left, rcClose.top);
						DrawDC.LineTo(rcClose.left, rcClose.bottom);
					}

					break;
				case 2:
					{
						// Pressed button
						// Draw outline, black on top, white on bottom
						DrawDC.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
						DrawDC.MoveTo(rcClose.left, rcClose.bottom);
						DrawDC.LineTo(rcClose.right, rcClose.bottom);
						DrawDC.LineTo(rcClose.right, rcClose.top);
						DrawDC.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
						DrawDC.LineTo(rcClose.left, rcClose.top);
						DrawDC.LineTo(rcClose.left, rcClose.bottom);
					}
					break;
				}

				// Manually Draw Close Button
				if (Focus)
					DrawDC.CreatePen(PS_SOLID, 1, m_Foregnd1);
				else
					DrawDC.CreatePen(PS_SOLID, 1, m_Foregnd2);

				DrawDC.MoveTo(rcClose.left + 3, rcClose.top +3);
				DrawDC.LineTo(rcClose.right - 2, rcClose.bottom -2);

				DrawDC.MoveTo(rcClose.left + 4, rcClose.top +3);
				DrawDC.LineTo(rcClose.right - 2, rcClose.bottom -3);

				DrawDC.MoveTo(rcClose.left + 3, rcClose.top +4);
				DrawDC.LineTo(rcClose.right - 3, rcClose.bottom -2);

				DrawDC.MoveTo(rcClose.right -3, rcClose.top +3);
				DrawDC.LineTo(rcClose.left + 2, rcClose.bottom -2);

				DrawDC.MoveTo(rcClose.right -3, rcClose.top +4);
				DrawDC.LineTo(rcClose.left + 3, rcClose.bottom -2);

				DrawDC.MoveTo(rcClose.right -4, rcClose.top +3);
				DrawDC.LineTo(rcClose.left + 2, rcClose.bottom -3);
			}
		}
	}

	inline LRESULT c_docker::c_dock_client::OnNCCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Sets the non-client area (and hence sets the client area)
		// This function modifies lParam

		if ((0 != m_pDocker) && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
		{
			if (m_pDocker->IsDocked())
			{
				auto rc = reinterpret_cast<LPRECT>(lParam);
				rc->top += m_pDocker->m_NCHeight;
			}
		}

		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_client::OnNCHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Identify which part of the non-client area the cursor is over
		if ((nullptr != m_pDocker) && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
		{
			if (m_pDocker->IsDocked())
			{
				CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

				// Indicate if the point is in the close button (except for Win95)
				if ((GetWinVersion() > 1400) && (GetCloseRect().PtInRect(pt)))
					return HTCLOSE;

				ScreenToClient(pt);

				// Indicate if the point is in the caption
				if (pt.y < 0)
					return HTCAPTION;
			}
		}
		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_client::OnNCLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if ((nullptr != m_pDocker) && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
		{
			if ((HTCLOSE == wParam) && !(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
			{
				m_IsClosePressed = TRUE;
				SetCapture();
			}

			m_IsCaptionPressed = TRUE;
			m_Oldpt.x = GET_X_LPARAM(lParam);
			m_Oldpt.y = GET_Y_LPARAM(lParam);
			if (m_pDocker->IsDocked())
			{
				// Give the view window focus unless its child already has it
				if (!GetView().IsChild(GetFocus()))
					m_pDocker->GetView().SetFocus();

				// Update the close button
				if ((0 != m_pDocker) && !(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
				{
					CWindowDC dc(*this);
					DrawCloseButton(dc, m_IsOldFocusStored);
				}

				return 0L;
			}
		}
		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_client::OnNCLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if ((0 != m_pDocker) && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
		{
			if ((HTCLOSE == wParam) && !(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
			{
				m_IsClosePressed = TRUE;
				SetCapture();
			}

			m_IsCaptionPressed = TRUE;
			m_Oldpt.x = GET_X_LPARAM(lParam);
			m_Oldpt.y = GET_Y_LPARAM(lParam);
			if (m_pDocker->IsDocked())
			{
				// Give the view window focus unless its child already has it
				if (!GetView().IsChild(GetFocus()))
					m_pDocker->GetView().SetFocus();

				// Update the close button
				if ((nullptr != m_pDocker) && !(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
				{
					CWindowDC dc(*this);
					DrawCloseButton(dc, m_IsOldFocusStored);
				}

				return 0L;
			}
		}

		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_client::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		ReleaseCapture();
		if ((nullptr != m_pDocker) && !(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
		{
			CWindowDC dc(*this);
			DrawCloseButton(dc, m_IsOldFocusStored);
			if ((0 != m_pDocker) && !(m_pDocker->GetDockStyle() & (DS_NO_CAPTION|DS_NO_CLOSE)))
			{
				m_IsCaptionPressed = FALSE;
				if (m_IsClosePressed && GetCloseRect().PtInRect(GetCursorPos()))
				{
					// Destroy the docker
					if (m_pDocker->GetContainer())
					{
						c_dock_container* pContainer = m_pDocker->GetContainer()->GetActiveContainer();
						c_docker* pDocker = m_pDocker->GetDockFromView(pContainer);
						assert(pDocker);
						pDocker->Close();
					}
					else
					{
						m_pDocker->Close();
					}
				}
			}
		}

		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_client::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_IsClosePressed = FALSE;
		ReleaseCapture();
		CWindowDC dc(*this);
		DrawCloseButton(dc, m_IsOldFocusStored);

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_client::MouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (!m_IsTracking)
		{
			TRACKMOUSEEVENT TrackMouseEventStruct;
			ZeroMemory(&TrackMouseEventStruct, sizeof(TRACKMOUSEEVENT));
			TrackMouseEventStruct.cbSize = sizeof(TrackMouseEventStruct);
			TrackMouseEventStruct.dwFlags = TME_LEAVE|TME_NONCLIENT;
			TrackMouseEventStruct.hwndTrack = *this;
			_TrackMouseEvent(&TrackMouseEventStruct);
			m_IsTracking = TRUE;
		}

		if ((0 != m_pDocker) && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
		{
			if (m_pDocker->IsDocked())
			{
				// Discard phantom mouse move messages
				if ( (m_Oldpt.x == GET_X_LPARAM(lParam) ) && (m_Oldpt.y == GET_Y_LPARAM(lParam)))
					return 0L;

				if (IsLeftButtonDown() && (wParam == HTCAPTION)  && (m_IsCaptionPressed))
				{
					assert(m_pDocker);
					m_pDocker->Undock(GetCursorPos());
				}

				// Update the close button
				if ((0 != m_pDocker) && !(m_pDocker->GetDockStyle() & DS_NO_CLOSE))
				{
					CWindowDC dc(*this);
					DrawCloseButton(dc, m_IsOldFocusStored);
				}
			}

			m_IsCaptionPressed = FALSE;
		}

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_client::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return MouseMove(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_client::OnNCMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_IsTracking = FALSE;
		CWindowDC dc(*this);
		if ((nullptr != m_pDocker) && !(m_pDocker->GetDockStyle() 
			& (DS_NO_CAPTION|DS_NO_CLOSE)) && m_pDocker->IsDocked())
			DrawCloseButton(dc, m_IsOldFocusStored);

		m_IsTracking = FALSE;

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_client::OnNCMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return MouseMove(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_client::OnNCPaint(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if ((nullptr != m_pDocker) && !(m_pDocker->GetDockStyle() & DS_NO_CAPTION))
		{
			if (m_pDocker->IsDocked())
			{
				DefWindowProc(WM_NCPAINT, wParam, lParam);
				DrawCaption();
				return 0;
			}
		}
		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::c_dock_client::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Reposition the View window to cover the DockClient's client area
		CRect rc = GetClientRect();
		GetView().SetWindowPos(nullptr, rc, SWP_SHOWWINDOW);

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline void c_docker::c_dock_client::PreRegisterClass(WNDCLASS& wc)
	{
		wc.lpszClassName = _T("Win32++ DockClient");
		wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	}

	inline void c_docker::c_dock_client::PreCreate(CREATESTRUCT& cs)
	{
		assert(m_pDocker);
		DWORD dwStyle = m_pDocker->GetDockStyle();
		if (dwStyle & DS_CLIENTEDGE)
			cs.dwExStyle = WS_EX_CLIENTEDGE;

#if (WINVER >= 0x0500)
		if (m_pDocker->GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
			cs.dwExStyle |= WS_EX_LAYOUTRTL;
#endif

	}

	inline void c_docker::c_dock_client::SendNotify(UINT nMessageID)
	{
		// Fill the DragPos structure with data
		DRAGPOS DragPos{};
		DragPos.hdr.code = nMessageID;
		DragPos.hdr.hwndFrom = GetHwnd();
		DragPos.ptPos = GetCursorPos();
		DragPos.pDocker = m_pDocker;

		// Send a DragPos notification to the docker
		GetParent().SendMessage(WM_NOTIFY, 0L, (LPARAM)&DragPos);
	}

	inline void c_docker::c_dock_client::SetCaptionColors(COLORREF Foregnd1, COLORREF Backgnd1, COLORREF Foregnd2, COLORREF Backgnd2, COLORREF PenColor)
	{
		// Set the colors used when drawing the caption
		// m_Foregnd1 Foreground colour (focused).  m_Backgnd1 Background colour (focused)
		// m_Foregnd2 Foreground colour (not focused). m_Backgnd2 Foreground colour (not focused)
		// m_PenColor Pen color used for drawing the outline.
		m_Foregnd1 = Foregnd1;
		m_Backgnd1 = Backgnd1;
		m_Foregnd2 = Foregnd2;
		m_Backgnd2 = Backgnd2;
		m_PenColor = PenColor;
	}

	inline void c_docker::c_dock_client::SetView(CWnd& wndView)
	{
		if (m_pView != &wndView)
		{
			// Hide the existing view window (if any)
			if (m_pView && m_pView->IsWindow())
			{
				// We can't change docker view if it is a DockContainer
				// Use CDockContainer::SetView to change the DockContainer's view instead
				assert(m_pView->SendMessage(UWM_GETCDOCKCONTAINER) == 0);

				m_pView->ShowWindow(SW_HIDE);
			}

			// Assign the view window
			m_pView = &wndView;

			if (IsWindow())
			{
				// The docker is already created, so create and position the new view too

				if (!GetView().IsWindow())
					GetView().Create(*this);
				else
				{
					GetView().SetParent(*this);
				}

				CRect rc = GetClientRect();
				GetView().SetWindowPos(NULL, rc, SWP_SHOWWINDOW);
			}
		}
	}

	inline LRESULT c_docker::c_dock_client::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_LBUTTONUP:			return OnLButtonUp(uMsg, wParam, lParam);
		case WM_MOUSEMOVE:			return OnMouseMove(uMsg, wParam, lParam);
		case WM_NCCALCSIZE:			return OnNCCalcSize(uMsg, wParam, lParam);
		case WM_NCHITTEST:			return OnNCHitTest(uMsg, wParam, lParam);
		case WM_NCLBUTTONDBLCLK:	return OnNCLButtonDblClk(uMsg, wParam, lParam);
		case WM_NCLBUTTONDOWN:		return OnNCLButtonDown(uMsg, wParam, lParam);
		case WM_NCMOUSEMOVE:		return OnNCMouseMove(uMsg, wParam, lParam);
		case WM_NCPAINT:			return OnNCPaint(uMsg, wParam, lParam);
		case WM_NCMOUSELEAVE:		return OnNCMouseLeave(uMsg, wParam, lParam);
		case WM_NOTIFY:				return m_pDocker->SendMessage(uMsg, wParam, lParam);
		case WM_WINDOWPOSCHANGED:	return OnWindowPosChanged(uMsg, wParam, lParam);
		}

		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}


	//////////////////////////////////////////////////////////////
	// Definitions for the CDockHint class nested within CDocker
	//
	inline c_docker::c_dock_hint::c_dock_hint() : m_uDockSideOld(0)
	{
	}

	inline c_docker::c_dock_hint::~c_dock_hint()
	{
	}

	inline RECT c_docker::c_dock_hint::CalcHintRectContainer(c_docker* pDockTarget)
	{
		// Calculate the hint window's position for container docking
		CRect rcHint = pDockTarget->GetDockClient().GetWindowRect();
		if (pDockTarget->GetDockClient().GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_CLIENTEDGE)
			rcHint.InflateRect(-2, -2);
		pDockTarget->ScreenToClient(rcHint);

		return rcHint;
	}

	inline RECT c_docker::c_dock_hint::CalcHintRectInner(c_docker* pDockTarget, c_docker* pDockDrag, UINT uDockSide)
	{
		assert(pDockTarget);
		assert(pDockDrag);

		BOOL RTL = FALSE;
#if (WINVER >= 0x0500)
		RTL = (pDockTarget->GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_LAYOUTRTL);
#endif

		// Calculate the hint window's position for inner docking
		c_dock_client* pDockClient = &pDockTarget->GetDockClient();
		CRect rcHint = pDockClient->GetWindowRect();

		if (pDockClient->GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_CLIENTEDGE)
			rcHint.InflateRect(-2, -2);
		pDockTarget->ScreenToClient(rcHint);

		int Width;
		CRect rcDockDrag = pDockDrag->GetWindowRect();
		CRect rcDockTarget = pDockClient->GetWindowRect();
		if ((uDockSide  == DS_DOCKED_LEFT) || (uDockSide  == DS_DOCKED_RIGHT))
		{
			Width = rcDockDrag.Width();
			if (Width >= (rcDockTarget.Width() - pDockDrag->GetBarWidth()))
				Width = MAX(rcDockTarget.Width()/2 - pDockDrag->GetBarWidth(), pDockDrag->GetBarWidth());
		}
		else
		{
			Width = rcDockDrag.Height();
			if (Width >= (rcDockTarget.Height() - pDockDrag->GetBarWidth()))
				Width = MAX(rcDockTarget.Height()/2 - pDockDrag->GetBarWidth(), pDockDrag->GetBarWidth());
		}
		switch (uDockSide)
		{
		case DS_DOCKED_LEFT:
			if (RTL)	rcHint.left = rcHint.right - Width;
			else		rcHint.right = rcHint.left + Width;

			break;
		case DS_DOCKED_RIGHT:
			if (RTL)	rcHint.right = rcHint.left + Width;
			else		rcHint.left = rcHint.right - Width;

			break;
		case DS_DOCKED_TOP:
			rcHint.bottom = rcHint.top + Width;
			break;
		case DS_DOCKED_BOTTOM:
			rcHint.top = rcHint.bottom - Width;
			break;
		}

		return rcHint;
	}

	inline RECT c_docker::c_dock_hint::CalcHintRectOuter(c_docker* pDockDrag, UINT uDockSide)
	{
		assert(pDockDrag);

		// Calculate the hint window's position for outer docking
		c_docker* pDockTarget = pDockDrag->GetDockAncestor();
		CRect rcHint = pDockTarget->GetViewRect();
		if (pDockTarget->GetDockClient().GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_CLIENTEDGE)
			rcHint.InflateRect(-2, -2);

		int Width;
		CRect rcDockDrag = pDockDrag->GetWindowRect();

		BOOL RTL = FALSE;
#ifdef WS_EX_LAYOUTRTL
		RTL = (pDockTarget->GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_LAYOUTRTL);
#endif

		int BarWidth = pDockDrag->GetBarWidth();
		CRect rcDockClient = pDockTarget->GetDockClient().GetClientRect();

		// Limit the docked size to half the parent's size if it won't fit inside parent
		if ((uDockSide == DS_DOCKED_LEFTMOST) || (uDockSide  == DS_DOCKED_RIGHTMOST))
		{
			Width = rcDockDrag.Width();
			if (Width >= rcDockClient.Width() - BarWidth)
				Width = MAX(rcDockClient.Width()/2 - BarWidth, BarWidth);
		}
		else
		{
			Width = rcDockDrag.Height();
			if (Width >= rcDockClient.Height() - BarWidth)
				Width = MAX(rcDockClient.Height()/2 - BarWidth, BarWidth);
		}
		switch (uDockSide)
		{
		case DS_DOCKED_LEFTMOST:
			if(RTL)	rcHint.left = rcHint.right - Width;
			else	rcHint.right = rcHint.left + Width;

			break;
		case DS_DOCKED_RIGHTMOST:
			if(RTL)	rcHint.right = rcHint.left + Width;
			else	rcHint.left = rcHint.right - Width;

			break;
		case DS_DOCKED_TOPMOST:
			rcHint.bottom = rcHint.top + Width;
			break;
		case DS_DOCKED_BOTTOMMOST:
			rcHint.top = rcHint.bottom - Width;
			break;
		}

		return rcHint;
	}

	inline void c_docker::c_dock_hint::DisplayHint(c_docker* pDockTarget, c_docker* pDockDrag, UINT uDockSide)
	{
		// Ensure a new hint window is created if dock side changes
		if (uDockSide != m_uDockSideOld)
		{
			Destroy();
			pDockTarget->RedrawWindow(RDW_NOERASE | RDW_UPDATENOW );
			pDockDrag->RedrawWindow();
		}
		m_uDockSideOld = uDockSide;

		if (!IsWindow())
		{
			CRect rcHint;

			if (uDockSide & 0xF)
				rcHint = CalcHintRectInner(pDockTarget, pDockDrag, uDockSide);
			else if (uDockSide & 0xF0000)
				rcHint = CalcHintRectOuter(pDockDrag, uDockSide);
			else if (uDockSide & DS_DOCKED_CONTAINER)
				rcHint = CalcHintRectContainer(pDockTarget);
			else
				return;

			// Save the Dock window's blue tinted bitmap
			CClientDC dcDesktop(NULL);
			CMemDC dcMem(dcDesktop);
			CRect rcBitmap = rcHint;
			CRect rcTarget = rcHint;
			pDockTarget->ClientToScreen(rcTarget);

			m_bmBlueTint.DeleteObject();
			m_bmBlueTint.CreateCompatibleBitmap(dcDesktop, rcBitmap.Width(), rcBitmap.Height());
			dcMem.SelectObject(m_bmBlueTint);
			dcMem.BitBlt(0, 0, rcBitmap.Width(), rcBitmap.Height(), dcDesktop, rcTarget.left, rcTarget.top, SRCCOPY);
			dcMem.DetachBitmap();
			m_bmBlueTint.TintBitmap(-64, -24, +128);

			// Create the Hint window
			if (!IsWindow())
			{
				Create(*pDockTarget);
			}

			// Adjust hint shape for container in container docking
			if ((uDockSide & DS_DOCKED_CONTAINER) && rcHint.Height() > 50)
			{
				CRgn Rgn;
				Rgn.CreateRectRgn(0, 0, rcHint.Width(), rcHint.Height() -25);
				assert(Rgn.GetHandle());
				CRgn Rgn2;
				Rgn2.CreateRectRgn(5, rcHint.Height() -25, 60, rcHint.Height());
				Rgn.CombineRgn(Rgn2, RGN_OR);
				SetWindowRgn(Rgn, FALSE);
			}

			pDockTarget->ClientToScreen(rcHint);
			SetWindowPos(NULL, rcHint, SWP_SHOWWINDOW|SWP_NOZORDER|SWP_NOACTIVATE);
		}
	}

	inline void c_docker::c_dock_hint::OnDraw(CDC& dc)
	{
		// Display the blue tinted bitmap
		CRect rc = GetClientRect();
		CMemDC dcMem(dc);
		dcMem.SelectObject(m_bmBlueTint);
		dc.BitBlt(0, 0, rc.Width(), rc.Height(), dcMem, 0, 0, SRCCOPY);
	}

	inline void c_docker::c_dock_hint::PreCreate(CREATESTRUCT& cs)
	{
		cs.style = WS_POPUP;

		// WS_EX_TOOLWINDOW prevents the window being displayed on the taskbar
		cs.dwExStyle = WS_EX_TOOLWINDOW;

		cs.lpszClass = _T("Win32++ DockHint");
	}


	////////////////////////////////////////////////////////////////
	// Definitions for the CTargetCentre class nested within CDocker
	//
	inline c_docker::c_target_centre::c_target_centre() : m_IsOverContainer(FALSE), m_pOldDockTarget(0)
	{
	}

	inline c_docker::c_target_centre::~c_target_centre()
	{
	}

	inline void c_docker::c_target_centre::OnDraw(CDC& dc)
	{
		// Load the target bitmaps
		CBitmap bmCentre(IDW_SDCENTER);
		CBitmap bmLeft(IDW_SDLEFT);
		CBitmap bmRight(IDW_SDRIGHT);
		CBitmap bmTop(IDW_SDTOP);
		CBitmap bmBottom(IDW_SDBOTTOM);

		// Grey out invalid dock targets
		DWORD dwStyle = m_pOldDockTarget->GetDockStyle();
		if (dwStyle & DS_NO_DOCKCHILD_LEFT)  bmLeft.TintBitmap(150, 150, 150);
		if (dwStyle & DS_NO_DOCKCHILD_TOP) 	 bmTop.TintBitmap(150, 150, 150);
		if (dwStyle & DS_NO_DOCKCHILD_RIGHT) bmRight.TintBitmap(150, 150, 150);
		if (dwStyle & DS_NO_DOCKCHILD_BOTTOM) bmBottom.TintBitmap(150, 150, 150);

		// Draw the dock targets
		if (bmCentre.GetHandle())	dc.DrawBitmap(0, 0, 88, 88, bmCentre, RGB(255,0,255));
		else TRACE("Missing docking resource: Target Centre\n");

		if (bmLeft.GetHandle()) dc.DrawBitmap(0, 29, 31, 29, bmLeft, RGB(255,0,255));
		else TRACE("Missing docking resource: Target Left\n");

		if (bmTop.GetHandle()) dc.DrawBitmap(29, 0, 29, 31, bmTop, RGB(255,0,255));
		else TRACE("Missing docking resource: Target Top\n");

		if (bmRight.GetHandle()) dc.DrawBitmap(55, 29, 31, 29, bmRight, RGB(255,0,255));
		else TRACE("Missing docking resource: Target Right\n");

		if (bmBottom.GetHandle()) dc.DrawBitmap(29, 55, 29, 31, bmBottom, RGB(255,0,255));
		else TRACE("Missing docking resource: Target Bottom\n");

		if (IsOverContainer())
		{
			CBitmap bmMiddle(IDW_SDMIDDLE);
			dc.DrawBitmap(31, 31, 25, 26, bmMiddle, RGB(255,0,255));
		}
	}

	inline int c_docker::c_target_centre::OnCreate(CREATESTRUCT& cs)
	{
		UNREFERENCED_PARAMETER(cs);

		// Use a region to create an irregularly shapped window
		POINT ptArray[16] = { {0,29}, {22, 29}, {29, 22}, {29, 0},
		                      {58, 0}, {58, 22}, {64, 29}, {87, 29},
		                      {87, 58}, {64, 58}, {58, 64}, {58, 87},
		                      {29, 87}, {29, 64}, {23, 58}, {0, 58} };

		CRgn rgnPoly;
		rgnPoly.CreatePolygonRgn(ptArray, 16, WINDING);
		SetWindowRgn(rgnPoly, FALSE);
		return 0;
	}

	inline BOOL c_docker::c_target_centre::CheckTarget(LPDRAGPOS pDragPos)
	{
		c_docker* pDockDrag = pDragPos->pDocker;
		assert( ::SendMessage(*pDockDrag, UWM_GETCDOCKER, 0, 0) );

		c_docker* pDockTarget = pDockDrag->GetDockUnderDragPoint(pDragPos->ptPos);
		if (NULL == pDockTarget) return FALSE;

		if (!IsWindow())	Create();
		m_IsOverContainer = (BOOL)pDockTarget->GetView().SendMessage(UWM_GETCDOCKCONTAINER);

		// Redraw the target if the dock target changes
		if (m_pOldDockTarget != pDockTarget)	Invalidate();
		m_pOldDockTarget = pDockTarget;

		int cxImage = 88;
		int cyImage = 88;

		CRect rcTarget = pDockTarget->GetDockClient().GetWindowRect();
		int xMid = rcTarget.left + (rcTarget.Width() - cxImage)/2;
		int yMid = rcTarget.top + (rcTarget.Height() - cyImage)/2;
		SetWindowPos(HWND_TOPMOST, xMid, yMid, cxImage, cyImage, SWP_NOACTIVATE|SWP_SHOWWINDOW);

		// Create the docking zone rectangles
		CPoint pt = pDragPos->ptPos;
		ScreenToClient(pt);
		CRect rcLeft(0, 29, 31, 58);
		CRect rcTop(29, 0, 58, 31);
		CRect rcRight(55, 29, 87, 58);
		CRect rcBottom(29, 55, 58, 87);
		CRect rcMiddle(31, 31, 56, 57);

		// Test if our cursor is in one of the docking zones
		if ((rcLeft.PtInRect(pt)) && !(pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_LEFT))
		{
			pDockDrag->m_IsBlockMove = TRUE;

			pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_LEFT);
			pDockDrag->m_dwDockZone = DS_DOCKED_LEFT;

			return TRUE;
		}
		else if ((rcTop.PtInRect(pt)) && !(pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_TOP))
		{
			pDockDrag->m_IsBlockMove = TRUE;
			pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_TOP);
			pDockDrag->m_dwDockZone = DS_DOCKED_TOP;
			return TRUE;
		}
		else if ((rcRight.PtInRect(pt)) && !(pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_RIGHT))
		{
			pDockDrag->m_IsBlockMove = TRUE;

			pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_RIGHT);
			pDockDrag->m_dwDockZone = DS_DOCKED_RIGHT;

			return TRUE;
		}
		else if ((rcBottom.PtInRect(pt)) && !(pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_BOTTOM))
		{
			pDockDrag->m_IsBlockMove = TRUE;
			pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_BOTTOM);
			pDockDrag->m_dwDockZone = DS_DOCKED_BOTTOM;
			return TRUE;
		}
		else if ((rcMiddle.PtInRect(pt)) && (IsOverContainer()))
		{
			pDockDrag->m_IsBlockMove = TRUE;
			pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_CONTAINER);
			pDockDrag->m_dwDockZone = DS_DOCKED_CONTAINER;
			return TRUE;
		}
		else
			return FALSE;
	}

	////////////////////////////////////////////////////////////////
	// Definitions for the CTarget class nested within CDocker
	// CTarget is the base class for a number of CTargetXXX classes
	inline c_docker::c_target::~c_target()
	{
	}

	inline void c_docker::c_target::OnDraw(CDC& dc)
	{
		BITMAP bm = m_bmImage.GetBitmapData();
		int cxImage = bm.bmWidth;
		int cyImage = bm.bmHeight;

		if (m_bmImage != 0)
			dc.DrawBitmap(0, 0, cxImage, cyImage, m_bmImage, RGB(255,0,255));
		else
			TRACE("Missing docking resource\n");
	}

	inline void c_docker::c_target::PreCreate(CREATESTRUCT& cs)
	{
		cs.style = WS_POPUP;
		cs.dwExStyle = WS_EX_TOPMOST|WS_EX_TOOLWINDOW;
		cs.lpszClass = _T("Win32++ DockTargeting");
	}


	////////////////////////////////////////////////////////////////
	// Definitions for the CTargetLeft class nested within CDocker
	//
	inline BOOL c_docker::c_target_left::CheckTarget(LPDRAGPOS pDragPos)
	{
		c_docker* pDockDrag = pDragPos->pDocker;
		assert( pDockDrag );

		CPoint pt = pDragPos->ptPos;
		c_docker* pDockTarget = pDockDrag->GetDockUnderDragPoint(pt)->GetTopmostDocker();
		if (pDockTarget != pDockDrag->GetDockAncestor())
		{
			Destroy();
			return FALSE;
		}

		if (pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_LEFT)
			return FALSE;

		BITMAP bm = m_bmImage.GetBitmapData();
		int cxImage = bm.bmWidth;
		int cyImage = bm.bmHeight;

		if (!IsWindow())
		{
			Create();
			CRect rc = pDockTarget->GetViewRect();
			pDockTarget->ClientToScreen(rc);
			int yMid = rc.top + (rc.Height() - cyImage)/2;
			SetWindowPos(HWND_TOPMOST, rc.left + 8, yMid, cxImage, cyImage, SWP_NOACTIVATE|SWP_SHOWWINDOW);
		}

		CRect rcLeft(0, 0, cxImage, cyImage);
		ScreenToClient(pt);

		// Test if our cursor is in one of the docking zones
		if (rcLeft.PtInRect(pt))
		{
			pDockDrag->m_IsBlockMove = TRUE;
			pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_LEFTMOST);
			pDockDrag->m_dwDockZone = DS_DOCKED_LEFTMOST;
			return TRUE;
		}

		return FALSE;
	}


	////////////////////////////////////////////////////////////////
	// Definitions for the CTargetTop class nested within CDocker
	//
	inline BOOL c_docker::c_target_top::CheckTarget(LPDRAGPOS pDragPos)
	{
		c_docker* pDockDrag = pDragPos->pDocker;
		assert( pDockDrag );

		CPoint pt = pDragPos->ptPos;
		c_docker* pDockTarget = pDockDrag->GetDockUnderDragPoint(pt)->GetTopmostDocker();
		if (pDockTarget != pDockDrag->GetDockAncestor())
		{
			Destroy();
			return FALSE;
		}

		if (pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_TOP)
			return FALSE;

		BITMAP bm = m_bmImage.GetBitmapData();
		int cxImage = bm.bmWidth;
		int cyImage = bm.bmHeight;

		if (!IsWindow())
		{
			Create();
			CRect rc = pDockTarget->GetViewRect();
			pDockTarget->ClientToScreen(rc);
			int xMid = rc.left + (rc.Width() - cxImage)/2;
			SetWindowPos(HWND_TOPMOST, xMid, rc.top + 8, cxImage, cyImage, SWP_NOACTIVATE|SWP_SHOWWINDOW);
		}

		CRect rcTop(0, 0, cxImage, cyImage);
		ScreenToClient(pt);

		// Test if our cursor is in one of the docking zones
		if (rcTop.PtInRect(pt))
		{
			pDockDrag->m_IsBlockMove = TRUE;
			pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_TOPMOST);
			pDockDrag->m_dwDockZone = DS_DOCKED_TOPMOST;
			return TRUE;
		}

		return FALSE;
	}


	////////////////////////////////////////////////////////////////
	// Definitions for the CTargetRight class nested within CDocker
	//
	inline BOOL c_docker::c_target_right::CheckTarget(LPDRAGPOS pDragPos)
	{
		c_docker* pDockDrag = pDragPos->pDocker;
		assert( pDockDrag );

		CPoint pt = pDragPos->ptPos;
		c_docker* pDockTarget = pDockDrag->GetDockUnderDragPoint(pt)->GetTopmostDocker();
		if (pDockTarget != pDockDrag->GetDockAncestor())
		{
			Destroy();
			return FALSE;
		}

		if (pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_RIGHT)
			return FALSE;

		BITMAP bm = m_bmImage.GetBitmapData();
		int cxImage = bm.bmWidth;
		int cyImage = bm.bmHeight;

		if (!IsWindow())
		{
			Create();
			CRect rc = pDockTarget->GetViewRect();
			pDockTarget->ClientToScreen(rc);
			int yMid = rc.top + (rc.Height() - cyImage)/2;
			SetWindowPos(HWND_TOPMOST, rc.right - 8 - cxImage, yMid, cxImage, cyImage, SWP_NOACTIVATE|SWP_SHOWWINDOW);
		}

		CRect rcRight(0, 0, cxImage, cyImage);
		ScreenToClient(pt);

		// Test if our cursor is in one of the docking zones
		if (rcRight.PtInRect(pt))
		{
			pDockDrag->m_IsBlockMove = TRUE;
			pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_RIGHTMOST);
			pDockDrag->m_dwDockZone = DS_DOCKED_RIGHTMOST;
			return TRUE;
		}

		return FALSE;
	}


	////////////////////////////////////////////////////////////////
	// Definitions for the CTargetBottom class nested within CDocker
	//
	inline BOOL c_docker::c_target_bottom::CheckTarget(LPDRAGPOS pDragPos)
	{
		c_docker* pDockDrag = pDragPos->pDocker;
		assert( pDockDrag );

		CPoint pt = pDragPos->ptPos;
		c_docker* pDockTarget = pDockDrag->GetDockUnderDragPoint(pt)->GetTopmostDocker();
		if (pDockTarget != pDockDrag->GetDockAncestor())
		{
			Destroy();
			return FALSE;
		}

		if (pDockTarget->GetDockStyle() & DS_NO_DOCKCHILD_BOTTOM)
			return FALSE;

		BITMAP bm = m_bmImage.GetBitmapData();
		int cxImage = bm.bmWidth;
		int cyImage = bm.bmHeight;

		if (!IsWindow())
		{
			Create();
			CRect rc = pDockTarget->GetViewRect();
			pDockTarget->ClientToScreen(rc);
			int xMid = rc.left + (rc.Width() - cxImage)/2;
			SetWindowPos(HWND_TOPMOST, xMid, rc.bottom - 8 - cyImage, cxImage, cyImage, SWP_NOACTIVATE|SWP_SHOWWINDOW);
		}
		CRect rcBottom(0, 0, cxImage, cyImage);
		ScreenToClient(pt);

		// Test if our cursor is in one of the docking zones
		if (rcBottom.PtInRect(pt))
		{
			pDockDrag->m_IsBlockMove = TRUE;
			pDockTarget->GetDockHint().DisplayHint(pDockTarget, pDockDrag, DS_DOCKED_BOTTOMMOST);
			pDockDrag->m_dwDockZone = DS_DOCKED_BOTTOMMOST;
			return TRUE;
		}

		return FALSE;
	}


	/////////////////////////////////////////
	// Definitions for the CDocker class
	//
	inline c_docker::c_docker() : m_pDockParent(NULL), m_IsBlockMove(FALSE), m_IsUndocking(FALSE),
		            m_IsClosing(FALSE), m_IsDragging(FALSE), m_IsDragAutoResize(TRUE), m_DockStartSize(0),
		            m_nDockID(0), m_nRedrawCount(0), m_NCHeight(0), m_dwDockZone(0), m_DockSizeRatio(1.0),
		            m_DockStyle(0), m_hDockUnderPoint(0)
	{
		// Assume this docker is the DockAncestor for now.
		m_pDockAncestor = this;
		m_vAllDockers.push_back(this);
		m_DockClient.SetDocker(this);
	}

	inline c_docker::~c_docker()
	{
		GetDockBar().Destroy();

		std::vector<DockPtr>::const_iterator iter;
		if (GetDockAncestor() == this)
		{
			// Destroy all dock descendants of this dock ancestor
			for (iter = GetAllChildren().begin(); iter != GetAllChildren().end(); ++iter)
			{
				(*iter)->Destroy();
			}
		}
	}

	inline c_docker* c_docker::AddDockedChild(c_docker* pDocker, DWORD dwDockStyle, int DockSize, int nDockID /* = 0*/)
	// This function creates the docker, and adds it to the docker hierarchy as docked
	{
		// Create the docker window as a child of the frame window.
		// This permanently sets the frame window as the docker window's owner,
		// even when its parent is subsequently changed.

		assert(pDocker);

		// Store the Docker's pointer in the DockAncestor's vector for later deletion
		GetAllChildren().push_back(DockPtr(pDocker));
		GetDockAncestor()->m_vAllDockers.push_back(pDocker);

		pDocker->SetDockStyle(dwDockStyle);
		pDocker->m_nDockID = nDockID;
		pDocker->m_pDockAncestor = GetDockAncestor();
		pDocker->m_pDockParent = this;
		HWND hwndFrame = GetDockAncestor()->GetAncestor();
		pDocker->Create(hwndFrame);
		pDocker->SetParent(*this);

		// Dock the docker window
		if (dwDockStyle & DS_DOCKED_CONTAINER)
			DockInContainer(pDocker, dwDockStyle);
		else
			Dock(pDocker, dwDockStyle);

		pDocker->SetDockSize(DockSize);

		// Issue TRACE warnings for any missing resources
		HMODULE hMod= GetApp().GetResourceHandle();

		if (!(dwDockStyle & DS_NO_RESIZE))
		{
			if (!FindResource(hMod, MAKEINTRESOURCE(IDW_SPLITH), RT_GROUP_CURSOR))
				TRACE("**WARNING** Horizontal cursor resource missing\n");
			if (!FindResource(hMod, MAKEINTRESOURCE(IDW_SPLITV), RT_GROUP_CURSOR))
				TRACE("**WARNING** Vertical cursor resource missing\n");
		}

		if (!(dwDockStyle & DS_NO_UNDOCK))
		{
			if (!FindResource(hMod, MAKEINTRESOURCE(IDW_SDCENTER), RT_BITMAP))
				TRACE("**WARNING** Docking center bitmap resource missing\n");
			if (!FindResource(hMod, MAKEINTRESOURCE(IDW_SDLEFT), RT_BITMAP))
				TRACE("**WARNING** Docking left bitmap resource missing\n");
			if (!FindResource(hMod, MAKEINTRESOURCE(IDW_SDRIGHT), RT_BITMAP))
				TRACE("**WARNING** Docking right bitmap resource missing\n");
			if (!FindResource(hMod, MAKEINTRESOURCE(IDW_SDTOP), RT_BITMAP))
				TRACE("**WARNING** Docking top bitmap resource missing\n");
			if (!FindResource(hMod, MAKEINTRESOURCE(IDW_SDBOTTOM), RT_BITMAP))
				TRACE("**WARNING** Docking center bottom resource missing\n");
		}

		if (dwDockStyle & DS_DOCKED_CONTAINER)
		{
			if (!FindResource(hMod, MAKEINTRESOURCE(IDW_SDMIDDLE), RT_BITMAP))
				TRACE("**WARNING** Docking container bitmap resource missing\n");
		}

		return pDocker;
	}

	inline c_docker* c_docker::AddUndockedChild(c_docker* pDocker, DWORD dwDockStyle, int DockSize, const RECT& rc, int nDockID /* = 0*/)
	// This function creates the docker, and adds it to the docker hierarchy as undocked
	{
		assert(pDocker);

		// Store the Docker's pointer in the DockAncestor's vector for later deletion
		GetAllChildren().push_back(DockPtr(pDocker));
		GetDockAncestor()->m_vAllDockers.push_back(pDocker);

		pDocker->SetDockSize(DockSize);
		pDocker->SetDockStyle(dwDockStyle & 0XFFFFFF0);
		pDocker->m_nDockID = nDockID;
		pDocker->m_pDockAncestor = GetDockAncestor();

		// Initially create the as a child window of the frame
		// This makes the frame window the owner of our docker
		HWND hFrame = GetDockAncestor()->GetAncestor();
		pDocker->Create(hFrame);
		pDocker->SetParent(*this);

		// Change the Docker to a POPUP window
		DWORD dwStyle = WS_POPUP| WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_VISIBLE;
		pDocker->SetWindowLongPtr(GWL_STYLE, dwStyle);
		pDocker->SetRedraw(FALSE);
		pDocker->SetParent(0);
		pDocker->SetWindowPos(0, rc, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
		pDocker->SetRedraw(TRUE);
		pDocker->RedrawWindow(RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_ALLCHILDREN);
		pDocker->SetWindowText(pDocker->GetCaption().c_str());

		return pDocker;
	}

	inline void c_docker::CheckAllTargets(LPDRAGPOS pDragPos)
	// Calls CheckTarget for each possible target zone
	{
		if (!GetDockAncestor()->m_TargetCentre.CheckTarget(pDragPos))
		{
			if (!GetDockAncestor()->m_TargetLeft.CheckTarget(pDragPos))
			{
				if(!GetDockAncestor()->m_TargetTop.CheckTarget(pDragPos))
				{
					if(!GetDockAncestor()->m_TargetRight.CheckTarget(pDragPos))
					{
						if(!GetDockAncestor()->m_TargetBottom.CheckTarget(pDragPos))
						{
							// Not in a docking zone, so clean up
							c_docker* pDockDrag = pDragPos->pDocker;
							if (pDockDrag)
							{
								if (pDockDrag->m_IsBlockMove)
									pDockDrag->RedrawWindow(RDW_FRAME|RDW_INVALIDATE);

								GetDockHint().Destroy();
								pDockDrag->m_dwDockZone = 0;
								pDockDrag->m_IsBlockMove = FALSE;
							}
						}
					}
				}
			}
		}
	}

	inline BOOL c_docker::VerifyDockers()
	// A diagnostic routine which verifies the integrity of the docking layout
	{
		BOOL Verified = TRUE;

		// Check dock ancestor
		std::vector<DockPtr>::const_iterator iter;

		for (iter = GetAllChildren().begin(); iter != GetAllChildren().end(); ++iter)
		{
			if (GetDockAncestor() != (*iter)->m_pDockAncestor)
			{
				TRACE("Invalid Dock Ancestor\n");
				Verified = FALSE;
			}
		}

		// Check presence of dock parent
		for (iter = GetAllChildren().begin(); iter != GetAllChildren().end(); ++iter)
		{
			if ((*iter)->IsUndocked() && (*iter)->m_pDockParent != 0)
			{
				TRACE("Error: Undocked dockers should not have a dock parent\n");
				Verified = FALSE;
			}

			if ((*iter)->IsDocked() && (*iter)->m_pDockParent == 0)
			{
				TRACE("Error: Docked dockers should have a dock parent\n");
				Verified = FALSE;
			}
		}

		// Check dock parent/child relationship
		for (iter = GetAllChildren().begin(); iter != GetAllChildren().end(); ++iter)
		{
			std::vector<c_docker*>::const_iterator iterChild;
			for (iterChild = (*iter)->m_vDockChildren.begin(); iterChild != (*iter)->m_vDockChildren.end(); ++iterChild)
			{
				if ((*iterChild)->m_pDockParent != (*iter).get())
				{
					TRACE("Error: Docking parent/Child information mismatch\n");
					Verified = FALSE;
				}

				if ((*iterChild)->GetParent() != (*iter).get()->GetHwnd())
				{
					TRACE("Error: Incorrect windows child parent relationship\n");
					Verified = FALSE;
				}
			}
		}

		// Check dock parent chain
		for (iter = GetAllChildren().begin(); iter != GetAllChildren().end(); ++iter)
		{
			c_docker* pDockTopLevel = (*iter)->GetTopmostDocker();
			if (pDockTopLevel->IsDocked())
				TRACE("Error: Top level parent should be undocked\n");
		}

		return Verified;
	}

	inline void c_docker::Close()
	{
		if (IsDocked())
		{
			// Undock the docker and hide it
			Hide();
		}

		// Destroy the docker and its children
		Destroy();
	}

	inline void c_docker::CloseAllDockers()
	{
		assert(this == GetDockAncestor());	// Must call CloseAllDockers from the DockAncestor

		std::vector <DockPtr>::const_iterator v;

		for (v = GetAllChildren().begin(); v != GetAllChildren().end(); ++v)
		{
			// The CDocker is destroyed when the window is destroyed
			(*v)->m_IsClosing = TRUE;
			(*v)->Destroy();	// Destroy the window
		}

		m_vDockChildren.clear();

		// Delete any child containers this container might have
		if (GetContainer())
		{
			std::vector<container_info> AllContainers = GetContainer()->GetAllContainers();
			std::vector<container_info>::const_iterator iter;
			for (iter = AllContainers.begin(); iter != AllContainers.end(); ++iter)
			{
				if (GetContainer() != (*iter).pContainer)
					GetContainer()->RemoveContainer((*iter).pContainer);
			}
		}


		RecalcDockLayout();
	}

	inline void c_docker::CloseAllTargets()
	{
		GetDockAncestor()->m_TargetCentre.Destroy();
		GetDockAncestor()->m_TargetLeft.Destroy();
		GetDockAncestor()->m_TargetTop.Destroy();
		GetDockAncestor()->m_TargetRight.Destroy();
		GetDockAncestor()->m_TargetBottom.Destroy();
	}

	inline void c_docker::Dock(c_docker* pDocker, UINT DockStyle)
	// Docks the specified docker inside this docker
	{
		assert(pDocker);

		pDocker->m_pDockParent = this;
		pDocker->m_IsBlockMove = FALSE;
		pDocker->SetDockStyle(DockStyle);
		m_vDockChildren.push_back(pDocker);
		pDocker->ConvertToChild(*this);

		// Limit the docked size to half the parent's size if it won't fit inside parent
		if (((DockStyle & 0xF)  == DS_DOCKED_LEFT) || ((DockStyle &0xF)  == DS_DOCKED_RIGHT))
		{
			int Width = GetDockClient().GetWindowRect().Width();
			int BarWidth = pDocker->GetBarWidth();
			if (pDocker->m_DockStartSize >= (Width - BarWidth))
				pDocker->SetDockSize(MAX(Width/2 - BarWidth, BarWidth));

			pDocker->m_DockSizeRatio = ((double)pDocker->m_DockStartSize) / (double)GetWindowRect().Width();
		}
		else
		{
			int Height = GetDockClient().GetWindowRect().Height();
			int BarWidth = pDocker->GetBarWidth();
			if (pDocker->m_DockStartSize >= (Height - BarWidth))
				pDocker->SetDockSize(MAX(Height/2 - BarWidth, BarWidth));

			pDocker->m_DockSizeRatio = ((double)pDocker->m_DockStartSize) / (double)GetWindowRect().Height();
		}

		// Redraw the docked windows
		if (GetAncestor().IsWindowVisible())
		{
			// Give the view window focus unless its child already has it
			if (!pDocker->GetView().IsChild(GetFocus()))
				pDocker->GetView().SetFocus();

			GetTopmostDocker()->SetRedraw(FALSE);
			RecalcDockLayout();
			GetTopmostDocker()->SetRedraw(TRUE);
			GetTopmostDocker()->RedrawWindow();

			// Update the Dock captions
			GetDockAncestor()->PostMessage(UWM_DOCKACTIVATE);
		}
	}

	inline void c_docker::DockInContainer(c_docker* pDocker, DWORD dwDockStyle)
	// Add a container to an existing container
	{
		if ((dwDockStyle & DS_DOCKED_CONTAINER) && GetContainer())
		{
			// Transfer any dock children to this docker
			pDocker->MoveDockInContainerChildren(this);

			// Transfer container children to the target container
			c_dock_container* pContainer = GetContainer();
			c_dock_container* pContainerSource = pDocker->GetContainer();

			std::vector<container_info>::reverse_iterator riter;
			std::vector<container_info> AllContainers = pContainerSource->GetAllContainers();
			for (riter = AllContainers.rbegin(); riter < AllContainers.rend(); ++riter)
			{
				c_dock_container* pContainerChild = (*riter).pContainer;
				if (pContainerChild != pContainerSource)
				{
					// Remove child container from pContainerSource
					pContainerChild->ShowWindow(SW_HIDE);
					pContainerSource->RemoveContainer(pContainerChild);
					c_docker* pDockChild = GetDockFromView(pContainerChild);
					assert(pDockChild);
					pDockChild->SetParent(*this);
					pDockChild->m_pDockParent = this;
				}
			}

			pDocker->m_pDockParent = this;
			pDocker->m_IsBlockMove = FALSE;
			pDocker->ShowWindow(SW_HIDE);
			pDocker->SetWindowLongPtr(GWL_STYLE, WS_CHILD);
			pDocker->SetDockStyle(dwDockStyle);
			pDocker->SetParent(*this);
			pDocker->GetDockBar().SetParent(*GetDockAncestor());

			// Insert the containers in reverse order
			for (riter = AllContainers.rbegin(); riter < AllContainers.rend(); ++riter)
			{
				pContainer->AddContainer( (*riter).pContainer, TRUE);
			}
		}

		// Redraw the docked windows
		if (GetAncestor().IsWindowVisible())
		{
			// Give the view window focus unless its child already has it
			if (!pDocker->GetView().IsChild(GetFocus()))
				pDocker->GetView().SetFocus();

			// Update the Dock captions
			GetDockAncestor()->PostMessage(UWM_DOCKACTIVATE);
		}
	}

	inline void c_docker::DockOuter(c_docker* pDocker, DWORD dwDockStyle)
	// Docks the specified docker inside the dock ancestor
	{
		assert(pDocker);

		pDocker->m_pDockParent = GetDockAncestor();

		DWORD OuterDocking = dwDockStyle & 0xF0000;
		DWORD DockSide = OuterDocking / 0x10000;
		dwDockStyle &= 0xFFF0FFFF;
		dwDockStyle |= DockSide;

		// Set the dock styles
		DWORD dwStyle = WS_CHILD | WS_VISIBLE;
		pDocker->m_IsBlockMove = FALSE;
		pDocker->SetWindowLongPtr(GWL_STYLE, dwStyle);
		pDocker->ShowWindow(SW_HIDE);
		pDocker->SetDockStyle(dwDockStyle);

		// Set the docking relationships
		const auto iter = GetDockAncestor()->m_vDockChildren.begin();
		GetDockAncestor()->m_vDockChildren.insert(iter, pDocker);
		pDocker->SetParent(*GetDockAncestor());
		pDocker->GetDockBar().SetParent(*GetDockAncestor());

		// Limit the docked size to half the parent's size if it won't fit inside parent
		if (((dwDockStyle & 0xF)  == DS_DOCKED_LEFT) || ((dwDockStyle &0xF)  == DS_DOCKED_RIGHT))
		{
			int Width = GetDockAncestor()->GetDockClient().GetWindowRect().Width();
			int BarWidth = pDocker->GetBarWidth();
			if (pDocker->m_DockStartSize >= (Width - BarWidth))
				pDocker->SetDockSize(MAX(Width/2 - BarWidth, BarWidth));

			pDocker->m_DockSizeRatio = ((double)pDocker->m_DockStartSize) / (double)GetDockAncestor()->GetWindowRect().Width();
		}
		else
		{
			int Height = GetDockAncestor()->GetDockClient().GetWindowRect().Height();
			int BarWidth = pDocker->GetBarWidth();
			if (pDocker->m_DockStartSize >= (Height - BarWidth))
				pDocker->SetDockSize(MAX(Height/2 - BarWidth, BarWidth));

			pDocker->m_DockSizeRatio = ((double)pDocker->m_DockStartSize) / (double)GetDockAncestor()->GetWindowRect().Height();
		}

		// Redraw the docked windows
		if (GetAncestor().IsWindowVisible())
		{
			// Give the view window focus unless its child already has it
			if (!pDocker->GetView().IsChild(GetFocus()))
				pDocker->GetView().SetFocus();

			GetTopmostDocker()->SetRedraw(FALSE);
			RecalcDockLayout();
			GetTopmostDocker()->SetRedraw(TRUE);
			GetTopmostDocker()->RedrawWindow();

			// Update the Dock captions
			GetDockAncestor()->PostMessage(UWM_DOCKACTIVATE);
		}
	}

	inline void c_docker::DrawAllCaptions()
	{
		std::vector<DockPtr>::const_iterator iter;
		for (iter = GetAllChildren().begin(); iter != GetAllChildren().end(); ++iter)
		{
			if ((*iter)->IsDocked())
				(*iter)->GetDockClient().DrawCaption();
		}
	}

	inline void c_docker::DrawHashBar(HWND hBar, POINT Pos)
	// Draws a hashed bar while the splitter bar is being dragged
	{
		c_dock_bar* pDockBar = static_cast<c_dock_bar*>(GetCWndPtr(hBar));
		if (NULL == pDockBar) return;
		c_docker& Docker = pDockBar->GetDocker();

		BOOL IsVertical = ((Docker.GetDockStyle() & 0xF) == DS_DOCKED_LEFT) || ((Docker.GetDockStyle() & 0xF) == DS_DOCKED_RIGHT);
		CClientDC dcBar(*this);

		WORD HashPattern[] = {0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA};
		CBitmap bmHash;
		CBrush brDithered;
		bmHash.CreateBitmap(8, 8, 1, 1, HashPattern);
		brDithered.CreatePatternBrush(bmHash);
		dcBar.SelectObject(brDithered);

		CRect rc = pDockBar->GetWindowRect();
		ScreenToClient(rc);
		int cx = rc.Width();
		int cy = rc.Height();
		int BarWidth = Docker.GetDockBar().GetWidth();

		if (IsVertical)
			dcBar.PatBlt(Pos.x - BarWidth/2, rc.top, BarWidth, cy, PATINVERT);
		else
			dcBar.PatBlt(rc.left, Pos.y - BarWidth/2, cx, BarWidth, PATINVERT);
	}

	inline c_dock_container* c_docker::GetContainer() const
	{
		// returns NULL if not a CDockContainer*
		if (IsWindow())
			return reinterpret_cast<c_dock_container*>(GetView().SendMessage(UWM_GETCDOCKCONTAINER));

		return NULL;
	}

	inline c_docker* c_docker::GetActiveDocker() const
	// Returns the docker that has a child window with keyboard focus. 
	// The active docker's caption is drawn with a different color.
	{
		c_docker* pDockActive = NULL;
		HWND hWndTest = ::GetFocus();

		while (hWndTest != 0)
		{
			HWND hWndParent = ::GetParent(hWndTest);
			if (hWndParent == hWndTest) break;		// could be owned window, not parent
			hWndTest = hWndParent;
			
			c_docker* pDock = (c_docker*)::SendMessage(hWndTest, UWM_GETCDOCKER, 0, 0);
			if (pDock)
			{
				pDockActive = pDock;
				break;
			}
		}

		return pDockActive;
	}

	inline c_docker* c_docker::GetDockAncestor() const
	// The GetDockAncestor function retrieves the pointer to the
	//  ancestor (root docker parent) of the Docker.
	{
		return m_pDockAncestor;
	}

	inline c_docker* c_docker::GetDockUnderDragPoint(POINT pt)
	// Retrieves the Docker whose view window contains the specified point.
	// Used when dragging undocked dockers over other dockers to provide
	// the docker which needs to display the dock targets and dock hints.
	{
		// Step 1: Find the top level Docker under the point
		// EnumWindows assigns the Docker under the point to m_hDockUnderPoint

		m_hDockUnderPoint = 0;
		m_DockPoint = pt;
		EnumWindows(EnumWindowsProc, (LPARAM)this);

		// Step 2: Find the docker child whose view window has the point
		c_docker* pDockTarget = NULL;
		if (m_hDockUnderPoint != 0)
		{
			HWND hDockTest = m_hDockUnderPoint;
			HWND hDockParent = m_hDockUnderPoint;

			while (IsRelated(hDockTest))
			{
				hDockParent = hDockTest;
				CPoint ptLocal = pt;
				::ScreenToClient(hDockParent, &ptLocal);
				hDockTest = ::ChildWindowFromPoint(hDockParent, ptLocal);
				if (hDockTest == hDockParent) break;
			}

			c_docker* pDockParent = reinterpret_cast<c_docker*>(::SendMessage(hDockParent, UWM_GETCDOCKER, 0, 0));
			assert(pDockParent);

			CRect rc = pDockParent->GetDockClient().GetWindowRect();
			if (rc.PtInRect(pt))
				pDockTarget = pDockParent;
		}

		return pDockTarget;
	}

	inline c_docker* c_docker::GetDockFromID(int n_DockID) const
	{
		std::vector<DockPtr>::const_iterator v;

		if (GetDockAncestor())
		{
			for (v = GetAllChildren().begin(); v != GetAllChildren().end(); ++v)
			{
				if (n_DockID == (*v)->GetDockID())
					return (*v).get();
			}
		}

		return 0;
	}

	inline c_docker* c_docker::GetDockFromView(CWnd* pView) const
	{
		c_docker* pDocker = 0;
		std::vector<DockPtr>::const_iterator iter;
		for (iter = GetAllChildren().begin(); iter != GetAllChildren().end(); ++iter)
		{
			if (&(*iter)->GetView() == pView)
				pDocker = (*iter).get();
		}

		if (&GetDockAncestor()->GetView() == pView)
			pDocker = GetDockAncestor();

		return pDocker;
	}

	inline int c_docker::GetDockSize() const
	{
		// Returns the size of the docker to be used if it is redocked
		// Note: This function returns 0 if the docker has the DS_DOCKED_CONTAINER style

		CRect rcParent;
		if (GetDockParent())
			rcParent = GetDockParent()->GetWindowRect();
		else
			rcParent = GetDockAncestor()->GetWindowRect();

		double DockSize = 0;
		if ((GetDockStyle() & DS_DOCKED_LEFT) || (GetDockStyle() & DS_DOCKED_RIGHT))
			DockSize = (double)(rcParent.Width()*m_DockSizeRatio);
		else if ((GetDockStyle() & DS_DOCKED_TOP) || (GetDockStyle() & DS_DOCKED_BOTTOM))
			DockSize = (double)(rcParent.Height()*m_DockSizeRatio);
		else if ((GetDockStyle() & DS_DOCKED_CONTAINER))
			DockSize = 0;

		return static_cast<int>(DockSize);
	}

	inline c_docker* c_docker::GetTopmostDocker() const
	// Returns the docker's parent at the top of the Z order.
	// Could be the dock ancestor or an undocked docker.
	{
		c_docker* pDockTopLevel = (c_docker* const)this;

		while(pDockTopLevel->GetDockParent())
		{
			assert (pDockTopLevel != pDockTopLevel->GetDockParent());
			pDockTopLevel = pDockTopLevel->GetDockParent();
		}

		return pDockTopLevel;
	}

	inline CTabbedMDI* c_docker::GetTabbedMDI() const
	{
		// returns NULL if not a CTabbedMDI*
		if (IsWindow())
			return reinterpret_cast<CTabbedMDI*>(GetView().SendMessage(UWM_GETCTABBEDMDI));

		return NULL;
	}

	inline int c_docker::GetTextHeight()
	{
		NONCLIENTMETRICS info;
		ZeroMemory(&info, sizeof(NONCLIENTMETRICS));
		info.cbSize = GetSizeofNonClientMetrics();
		SystemParametersInfo (SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
		LOGFONT lf = info.lfStatusFont;

		CClientDC dc(*this);
		dc.CreateFontIndirect(lf);
		CSize szText = dc.GetTextExtentPoint32(_T("Text"), lstrlen(_T("Text")));
		return szText.cy;
	}

	inline void c_docker::Hide()
	{
		// Undocks a docker (if needed) and hides it.
		// To unhide the docker, dock it.

		if (IsDocked())
		{
			c_dock_container* pContainer = GetContainer();

			if (pContainer)
			{
				if (pContainer == pContainer->GetContainerParent())
					UndockContainer(pContainer, GetCursorPos(), FALSE);
				else
					pContainer->GetContainerParent()->RemoveContainer(pContainer);
			}
			else
			{
				c_docker* pDockUndockedFrom = SeparateFromDock();
				pDockUndockedFrom->RecalcDockLayout();
			}
		}

		ShowWindow(SW_HIDE);
	}

	inline BOOL c_docker::IsChildOfDocker(HWND hWnd) const
	// returns true if the specified window is a child of this docker
	{
		while ((hWnd != NULL) && (hWnd != *GetDockAncestor()))
		{
			if ( hWnd == *this ) return TRUE;
			if (IsRelated(hWnd)) break;
			hWnd = ::GetParent(hWnd);
		}

		return FALSE;
	}

	inline BOOL c_docker::IsDocked() const
	{
		return (((m_DockStyle&0xF) || (m_DockStyle & DS_DOCKED_CONTAINER)) && !m_IsUndocking); // Boolean expression
	}

	inline BOOL c_docker::IsDragAutoResize() const
	{
		return m_IsDragAutoResize;
	}

	inline BOOL c_docker::IsRelated(HWND hWnd) const
	// Returns TRUE if the hWnd is a docker within this dock family
	{
		if (*GetDockAncestor() == hWnd) return TRUE;

		for (std::vector<DockPtr>::const_iterator iter = GetAllChildren().begin(); iter != GetAllChildren().end(); ++iter)
		{
			if ((*iter).get()->GetHwnd() == hWnd) return TRUE;
		}

		return FALSE;
	}

	inline BOOL c_docker::IsUndocked() const
	{
		return (!((m_DockStyle&0xF)|| (m_DockStyle & DS_DOCKED_CONTAINER)) && !m_IsUndocking); // Boolean expression
	}

	inline BOOL c_docker::LoadDockRegistrySettings(LPCTSTR szRegistryKeyName)
	// Recreates the docker layout based on information stored in the registry.
	// Assumes the DockAncestor window is already created.
	{
		BOOL IsLoaded = FALSE;

		if (szRegistryKeyName)
		{
			std::vector<dock_info> vDockList;
			c_string KeyName = _T("Software\\") + c_string(szRegistryKeyName) + _T("\\Dock Windows");
			CRegKey Key;
			if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, KeyName, KEY_READ))
			{
				DWORD BufferSize = sizeof(dock_info);
				dock_info di{};
				int i = 0;
				c_string SubKeyName;
				SubKeyName.Format(_T("DockChild%d"), i);

				// Fill the DockList vector from the registry
				while (ERROR_SUCCESS == Key.QueryBinaryValue(SubKeyName, &di, &BufferSize))
				{
					vDockList.push_back(di);
					i++;
					SubKeyName.Format(_T("DockChild%d"), i);
				}

				Key.Close();
				if (!vDockList.empty()) IsLoaded = TRUE;
			}

			try
			{
				// Add dockers without parents first
				std::vector<dock_info>::iterator iter;
				for (iter = vDockList.begin(); iter != vDockList.end() ; ++iter)
				{
					const auto di = (*iter);
					if (di.DockParentID == 0)
					{
						c_docker* pDocker = NewDockerFromID(di.DockID);
						if (!pDocker)
							throw CUserException(_T("Failed to add dockers without parents from registry"));

						if (di.DockStyle & 0xF)
							AddDockedChild(pDocker, di.DockStyle, di.DockSize, di.DockID);
						else
							AddUndockedChild(pDocker, di.DockStyle, di.DockSize, di.Rect, di.DockID);
					}
				}

				// Remove dockers without parents from vDockList
				for (auto n = static_cast<UINT>(vDockList.size()); n > 0; --n)
				{
					iter = vDockList.begin() + n-1;
					if ((*iter).DockParentID == 0)
						vDockList.erase(iter);
				}

				// Add remaining dockers
				while (!vDockList.empty())
				{
					bool bFound = false;
					for (iter = vDockList.begin(); iter != vDockList.end(); ++iter)
					{
						dock_info di = *iter;
						c_docker* pDockParent = GetDockFromID(di.DockParentID);

						if (pDockParent != 0)
						{
							c_docker* pDocker = NewDockerFromID(di.DockID);
							if(!pDocker)
								throw CUserException(_T("Failed to add dockers with parents from registry"));

							pDockParent->AddDockedChild(pDocker, di.DockStyle, di.DockSize, di.DockID);
							bFound = true;
							vDockList.erase(iter);
							break;
						}
					}

					if (!bFound)
						throw CUserException(_T("Orphaned dockers stored in registry"));

					if (!VerifyDockers())
						throw CUserException(_T("Dockers are in an inconsistant state"));
				}
			}

			catch (const CUserException& e)
			{
				Trace(e.GetText()); Trace("\n");
				IsLoaded = FALSE;
				CloseAllDockers();

				// Delete the bad key from the registry
				c_string strParentKey = _T("Software\\") + c_string(szRegistryKeyName);
				CRegKey ParentKey;
				if (ERROR_SUCCESS == ParentKey.Open(HKEY_CURRENT_USER, strParentKey, KEY_READ))
					ParentKey.RecurseDeleteKey(_T("Dock Windows"));
			}

		}

		if (IsLoaded)
			LoadContainerRegistrySettings(szRegistryKeyName);

		return IsLoaded;
	}


	inline BOOL c_docker::LoadContainerRegistrySettings(LPCTSTR szRegistryKeyName)
	{
		BOOL IsLoaded = FALSE;
		if (szRegistryKeyName)
		{
			// Load Dock container tab order and active container
			c_string KeyName = _T("Software\\") + c_string(szRegistryKeyName) + _T("\\Dock Windows");
			CRegKey Key;

			if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, KeyName, KEY_READ))
			{
				try
				{
					UINT uContainer = 0;
					c_string SubKeyName;
					SubKeyName.Format(_T("DockContainer%u"), uContainer);
					CRegKey ContainerKey;

					while (ERROR_SUCCESS == ContainerKey.Open(Key, SubKeyName, KEY_READ))
					{
						// Load tab order
						IsLoaded = TRUE;
						UINT nTab = 0;
						DWORD dwTabID;
						std::vector<UINT> vTabOrder;
						c_string strTabKey;
						strTabKey.Format(_T("Tab%u"), nTab);
						while (ERROR_SUCCESS == ContainerKey.QueryDWORDValue(strTabKey, dwTabID))
						{
							vTabOrder.push_back(dwTabID);
							strTabKey.Format(_T("Tab%u"), ++nTab);
						}

						// Set tab order
						DWORD dwParentID;
						if (ERROR_SUCCESS == ContainerKey.QueryDWORDValue(_T("Parent Container"), dwParentID))
						{
							c_docker* pDocker = GetDockFromID(dwParentID);
							if (!pDocker)
								throw CUserException(_T("Failed to get docker from ID"));

							c_dock_container* pParentContainer = pDocker->GetContainer();
							if (!pParentContainer)
								throw CUserException(_T("Failed to get parent container"));

							for (UINT uTab = 0; uTab < vTabOrder.size(); ++uTab)
							{
								c_docker* pOldDocker = GetDockFromView(pParentContainer->GetContainerFromIndex(uTab));
								if (!pOldDocker)
									throw CUserException(_T("Failed to get docker from view"));

								UINT uOldID = pOldDocker->GetDockID();

								const std::vector<UINT>::const_iterator it = std::find(vTabOrder.begin(), vTabOrder.end(), uOldID);
								auto uOldTab = static_cast<UINT>(it - vTabOrder.begin());

								if (uTab != uOldTab)
									pParentContainer->SwapTabs(uTab, uOldTab);
							}
						}

						// Set the active container
						DWORD nActiveContainer;
						if (ERROR_SUCCESS == ContainerKey.QueryDWORDValue(_T("Active Container"), nActiveContainer))
						{
							c_docker* pDocker = GetDockFromID(nActiveContainer);
							if (!pDocker)
								throw CUserException(_T("Failed to get docker from ID"));

							c_dock_container* pContainer = pDocker->GetContainer();
							if (!pContainer)
								throw CUserException(_T("Failed to get container view"));

							int nPage = pContainer->GetContainerIndex(pContainer);
							if (nPage >= 0)
								pContainer->SelectPage(nPage);
						}

						SubKeyName.Format(_T("DockContainer%u"), ++uContainer);
					}
				}

				catch (const CUserException& e)
				{
					Trace("*** Failed to load values from registry, using defaults. ***\n");
					Trace(e.GetText()); Trace("\n");
					CloseAllDockers();

					// Delete the bad key from the registry
					c_string strParentKey = _T("Software\\") + c_string(szRegistryKeyName);
					CRegKey ParentKey;
					if (ERROR_SUCCESS == ParentKey.Open(HKEY_CURRENT_USER, strParentKey, KEY_READ))
						ParentKey.RecurseDeleteKey(_T("Dock Windows"));
				}
			}
		}

		return IsLoaded;
	}

	inline void c_docker::MoveDockChildren(c_docker* pDockTarget)
	// Used internally by Dock and Undock
	{
		assert(pDockTarget);

		for (std::vector<c_docker*>::const_iterator iter = m_vDockChildren.begin(); iter != m_vDockChildren.end(); ++iter)
		{
			pDockTarget->m_vDockChildren.push_back(*iter);
			(*iter)->m_pDockParent = pDockTarget;
			(*iter)->SetParent(*pDockTarget);
			(*iter)->GetDockBar().SetParent(*pDockTarget);
		}
		m_vDockChildren.clear();
	}

	inline void c_docker::MoveDockInContainerChildren(c_docker* pDockTarget)
	{
		while (GetDockChildren().size() > 0)
		{
			std::vector<c_docker*>::const_iterator iter = m_vDockChildren.begin();
			c_docker* pDocker = *iter;
			DWORD dwDockStyle = pDocker->GetDockStyle() | DS_DOCKED_CONTAINER;
			pDocker->SeparateFromDock();
			pDockTarget->DockInContainer( pDocker, dwDockStyle );
		}
	}

	inline c_docker* c_docker::NewDockerFromID(int nID)
	// Used in LoadRegistrySettings. Creates a new Docker from the specified ID
	{
		UNREFERENCED_PARAMETER(nID);

		// Override this function to create the Docker objects as shown below

		c_docker* pDocker = nullptr;
	/*	switch(nID)
		{
		case ID_CLASSES:
			pDocker = new CDockClasses;
			break;
		case ID_FILES:
			pDocker = new CDockFiles;
			break;
		default:
			TRACE("Unknown Dock ID\n");
			break;
		} */

		return pDocker;
	}

	inline LRESULT c_docker::OnActivate(UINT, WPARAM wParam, LPARAM)
	{
		GetDockAncestor()->PostMessage(UWM_DOCKACTIVATE);

		if ((wParam != WA_INACTIVE) && (this != GetDockAncestor()) && IsUndocked())
		{
			// Give the view window focus unless its child already has it
			if (!GetView().IsChild(GetFocus()))
				GetView().SetFocus();
		}

		return 0L;
	}

	inline LRESULT c_docker::OnBarStart(LPDRAGPOS pdp)
	{
		CPoint pt = pdp->ptPos;
		ScreenToClient(pt);
		if (!IsDragAutoResize())
			DrawHashBar(pdp->hdr.hwndFrom, pt);
		m_OldPoint = pt;

		return 0L;
	}

	inline LRESULT c_docker::OnBarMove(LPDRAGPOS pdp)
	{
		CPoint pt = pdp->ptPos;
		ScreenToClient(pt);

		if (pt != m_OldPoint)
		{
			if (IsDragAutoResize())
				ResizeDockers(pdp);
			else
			{
				DrawHashBar(pdp->hdr.hwndFrom, m_OldPoint);
				DrawHashBar(pdp->hdr.hwndFrom, pt);
			}

			m_OldPoint = pt;
		}

		return 0L;
	}

	inline LRESULT c_docker::OnBarEnd(LPDRAGPOS pdp)
	{
		POINT pt = pdp->ptPos;
		ScreenToClient(pt);

		if (!IsDragAutoResize())
			DrawHashBar(pdp->hdr.hwndFrom, pt);

		ResizeDockers(pdp);
		return 0L;
	}

	inline int c_docker::OnCreate(CREATESTRUCT& cs)
	{
		UNREFERENCED_PARAMETER(cs);

#if (WINVER >= 0x0500)
		if (GetParent().GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
		{
			DWORD dwExStyle = (DWORD)GetWindowLongPtr(GWL_EXSTYLE);
			SetWindowLongPtr(GWL_EXSTYLE, dwExStyle | WS_EX_LAYOUTRTL);
		}
#endif

		// Create the various child windows
		GetDockClient().Create(*this);

		assert(&GetView());			// Use SetView in the docker's constructor to set the view window
		GetView().Create(GetDockClient());

		// Create the slider bar belonging to this docker
		GetDockBar().SetDocker(*this);
		if (GetDockAncestor() != this)
			GetDockBar().Create(*GetDockAncestor());

		// Now remove the WS_POPUP style. It was required to allow this window
		// to be owned by the frame window.
		SetWindowLongPtr(GWL_STYLE, WS_CHILD);
		SetParent(GetParent());		// Reinstate the window's parent

		// Set the default colour for the splitter bar if it hasn't already been set
		if (!GetDockBar().GetBrushBkgnd().GetHandle())
		{
			COLORREF rgbColour = GetSysColor(COLOR_BTNFACE);
			HWND hWndFrame = GetDockAncestor()->GetAncestor();

			ReBarTheme* pTheme = reinterpret_cast<ReBarTheme*>(::SendMessage(hWndFrame, UWM_GETRBTHEME, 0, 0));

			if (pTheme && pTheme->UseThemes && pTheme->clrBkgnd2 != 0)
				rgbColour = pTheme->clrBkgnd2;

			SetBarColor(rgbColour);
		}

		// Set the caption height based on text height
		m_NCHeight = MAX(20, GetTextHeight() + 5);

		c_dock_container* pContainer = GetContainer();
		if (pContainer)
		{
			SetCaption(pContainer->GetDockCaption().c_str());
		}

		return 0;
	}

	inline void c_docker::OnDestroy()
	{
		for (std::vector<c_docker*>::const_iterator iter = m_vDockChildren.begin(); iter != m_vDockChildren.end(); ++iter)
		{
			(*iter)->Destroy();
		}

		c_dock_container* pContainer = GetContainer();
		if (pContainer && IsUndocked())
		{
			if (pContainer->GetAllContainers().size() > 1)
			{
				// This container has children, so destroy them now
				const std::vector<container_info>& AllContainers = pContainer->GetAllContainers();
				for (std::vector<container_info>::const_iterator iter1 = AllContainers.begin(); iter1 < AllContainers.
				     end(); ++iter1)
				{
					if ((*iter1).pContainer != pContainer)
					{
						// Reset container parent before destroying the dock window
						c_docker* pDocker = GetDockFromView((*iter1).pContainer);
						assert(pDocker);
						if (pContainer->IsWindow())
							pContainer->SetParent(pDocker->GetDockClient());

						pDocker->Destroy();
					}
				}
			}
		}

		GetDockBar().Destroy();

		// Post a docker destroyed message
		if ( GetDockAncestor()->IsWindow() )
			GetDockAncestor()->PostMessage(UWM_DOCKDESTROYED, (WPARAM)this, 0L);
	}

	inline LRESULT c_docker::OnDockDestroyed(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(uMsg);
		UNREFERENCED_PARAMETER(lParam);
		c_docker* pDocker = reinterpret_cast<c_docker*>(wParam);

		assert( this == GetDockAncestor() );
		for (auto iter = GetAllChildren().begin(); iter != GetAllChildren().end(); ++iter)
		{
			if ((*iter).get() == pDocker)
			{
				GetAllChildren().erase(iter);
				break;
			}
		}

		std::vector<c_docker*>& Dockers = GetDockAncestor()->m_vAllDockers;
		for (std::vector<c_docker*>::iterator it = Dockers.begin(); it < Dockers.end(); ++it)
		{
			if ((*it) == pDocker)
			{
				Dockers.erase(it);
				break;
			}
		}

		return 0L;
	}

	inline LRESULT c_docker::OnDockActivated(UINT uMsg, WPARAM wParam, LPARAM lParam)
	// Called in response to a UWM_DOCKACTIVATE message
	{
		// Redraw captions to take account of focus change
		if (this == GetDockAncestor())
			DrawAllCaptions();

		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::OnDockStart(LPDRAGPOS pdp)
	{
		if (IsDocked())
		{
			Undock(GetCursorPos());
			SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(pdp->ptPos.x, pdp->ptPos.y));
		}

		return 0L;
	}

	inline LRESULT c_docker::OnDockMove(LPDRAGPOS pdp)
	{
		CheckAllTargets(pdp);
		return 0L;
	}

	inline LRESULT c_docker::OnDockEnd(LPDRAGPOS pdp)
	{
		c_docker* pDocker = pdp->pDocker;
		assert(pDocker);

		UINT DockZone = pdp->DockZone;
		CRect rc = pDocker->GetWindowRect();

		switch(DockZone)
		{
		case DS_DOCKED_LEFT:
		case DS_DOCKED_RIGHT:
			pDocker->SetDockSize(rc.Width());
			Dock(pDocker, pDocker->GetDockStyle() | DockZone);
			break;
		case DS_DOCKED_TOP:
		case DS_DOCKED_BOTTOM:
			pDocker->SetDockSize(rc.Height());
			Dock(pDocker, pDocker->GetDockStyle() | DockZone);
			break;
		case DS_DOCKED_CONTAINER:
			{
				DockInContainer(pDocker, pDocker->GetDockStyle() | DockZone);
				c_dock_container* pContainer = GetContainer();
				assert(pContainer);
				pContainer->SelectPage(0);
			}
			break;
		case DS_DOCKED_LEFTMOST:
		case DS_DOCKED_RIGHTMOST:
			pDocker->SetDockSize(rc.Width());
			DockOuter(pDocker, pDocker->GetDockStyle() | DockZone);
			break;
		case DS_DOCKED_TOPMOST:
		case DS_DOCKED_BOTTOMMOST:
			pDocker->SetDockSize(rc.Height());
			DockOuter(pDocker, pDocker->GetDockStyle() | DockZone);
			break;
		}

		GetDockHint().Destroy();
		CloseAllTargets();

		return 0L;
	}

	inline LRESULT c_docker::OnExitSizeMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(uMsg);
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		m_IsBlockMove = FALSE;
		m_IsDragging = FALSE;
		SendNotify(UWN_DOCKEND);

		return 0L;
	}

	inline LRESULT c_docker::OnMouseActivate(UINT, WPARAM, LPARAM)
	{
		CPoint pt = GetCursorPos();

		if (PtInRect(GetDockClient().GetWindowRect(), pt)) // only for this docker
		{
			GetDockAncestor()->PostMessage(UWM_DOCKACTIVATE);
		}

		return 0L;	// Return 0 to stop propogating this message to parent windows
	}

	inline LRESULT c_docker::OnNCLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_IsDragging = FALSE;
		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::OnNotify(WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(wParam);
		const auto pdp = reinterpret_cast<LPDRAGPOS>(lParam);

		if (IsWindowVisible())
		{
			switch (reinterpret_cast<LPNMHDR>(lParam)->code)
			{
			case UWN_BARSTART:		return OnBarStart(pdp);
			case UWN_BARMOVE:		return OnBarMove(pdp);
			case UWN_BAREND:		return OnBarEnd(pdp);
			case UWN_DOCKSTART:		return OnDockStart(pdp);
			case UWN_DOCKMOVE:		return OnDockMove(pdp);
			case UWN_DOCKEND:		return OnDockEnd(pdp);
			}
		}
		
		// The framework will call SetWindowLongPtr(DWLP_MSGRESULT, lr) for non-zero returns		
		return 0L;
	}

	inline LRESULT c_docker::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (this == GetTopmostDocker())
		{
			// Reposition the dock children
			if (IsUndocked() && IsWindowVisible() && !m_IsClosing)
				RecalcDockLayout();
		}

		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		if (this == GetDockAncestor())
		{
			COLORREF rgbColour = GetSysColor(COLOR_BTNFACE);
			HWND hWndFrame = GetDockAncestor()->GetAncestor();

			ReBarTheme* pTheme = reinterpret_cast<ReBarTheme*>(::SendMessage(hWndFrame, UWM_GETRBTHEME, 0, 0));

			if (pTheme && pTheme->UseThemes && pTheme->clrBand2 != 0)
				rgbColour = pTheme->clrBkgnd2;
			else
				rgbColour = GetSysColor(COLOR_BTNFACE);

			for (std::vector<DockPtr>::const_iterator iter = GetAllChildren().begin(); iter != GetAllChildren().end(); ++iter)
				(*iter)->SetBarColor(rgbColour);

			// Set the splitter bar colour for the docker ancestor
			SetBarColor(rgbColour);
		}

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(wParam&0xFFF0)
		{
		case SC_MOVE:
			// An undocked docker is being moved
			{
				BOOL IsEnabled = FALSE;
				m_IsDragging = TRUE;
				SetCursor(LoadCursor(NULL, IDC_ARROW));

				if (SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &IsEnabled, 0))
				{
					// Turn on DragFullWindows for this move
					SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, TRUE, 0, 0);

					// Process this message
					DefWindowProc(WM_SYSCOMMAND, wParam, lParam);

					// Return DragFullWindows to its previous state
					SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, IsEnabled, 0, 0);
					return 0L;
				}
			}
			break;
		case SC_CLOSE:
			// The close button is pressed on an undocked docker
			m_IsClosing = TRUE;
			break;
		}
		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Suspend dock drag moving while over dock zone
		if (m_IsBlockMove)
		{
			auto pWndPos = reinterpret_cast<LPWINDOWPOS>(lParam);
			pWndPos->flags |= SWP_NOMOVE|SWP_FRAMECHANGED;
			return 0;
		}

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_docker::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(wParam);

		if (m_IsDragging)
		{
			// Send a Move notification to the parent
			if ( IsLeftButtonDown() )
			{
				const auto wPos = reinterpret_cast<LPWINDOWPOS>(lParam);
				if ((!(wPos->flags & SWP_NOMOVE)) || m_IsBlockMove)
					SendNotify(UWN_DOCKMOVE);
			}
			else
			{
				CloseAllTargets();
				m_IsBlockMove = FALSE;
			}
		}

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline void c_docker::PreCreate(CREATESTRUCT& cs)
	{
		// Specify the WS_POPUP style to have this window owned
		if (this != GetDockAncestor())
			cs.style = WS_POPUP;

		cs.dwExStyle = WS_EX_TOOLWINDOW;
	}

	inline void c_docker::PreRegisterClass(WNDCLASS& wc)
	{
		wc.lpszClassName = _T("Win32++ Docker");
		wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	}

	inline void c_docker::RecalcDockChildLayout(CRect& rc)
	{
		// This function positions the Docker's dock children, the Dockers client area
		//  and draws the dockbar bars.

		// Notes:
		// 1) This function is called recursively.
		// 2) The client area and child dockers are positioned simultaneously with
		//      DeferWindowPos to avoid drawing errors in complex docker arrangements.
		// 3) The docker's client area contains the docker's caption (if any) and the docker's view window.

		// Note: All top level dockers are undocked, including the dock ancestor.

		BOOL RTL = FALSE;
#ifdef WS_EX_LAYOUTRTL
		RTL = (GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_LAYOUTRTL);
#endif

		if (IsDocked())
		{
			rc.OffsetRect(-rc.left, -rc.top);
		}

		HDWP hdwp = BeginDeferWindowPos((int)m_vDockChildren.size() +2);

		// Step 1: Calculate the position of each Docker child, DockBar, and Client window.
		//   The Client area = the docker rect minus the area of dock children and the dock bar (splitter bar).
		std::vector<c_docker*>::const_iterator iter;
		for (iter = m_vDockChildren.begin(); iter != m_vDockChildren.end(); ++iter)
		{
			CRect rcChild = rc;
			double DockSize = (*iter)->m_DockStartSize;
			int minSize = 30;

			// Calculate the size of the Docker children
			switch ((*iter)->GetDockStyle() & 0xF)
			{
			case DS_DOCKED_LEFT:
				if ((*iter)->GetDockStyle() & DS_NO_FIXED_RESIZE)
					DockSize = MIN((*iter)->m_DockSizeRatio*(GetWindowRect().Width()), rcChild.Width());

				if (RTL)
				{
					rcChild.left = rcChild.right - (int)DockSize;
					rcChild.left = MIN(rcChild.left, rc.right - minSize);
					rcChild.left = MAX(rcChild.left, rc.left + minSize);
				}
				else
				{
					rcChild.right = rcChild.left + (int)DockSize;
					rcChild.right = MAX(rcChild.right, rc.left + minSize);
					rcChild.right = MIN(rcChild.right, rc.right - minSize);
				}
				break;
			case DS_DOCKED_RIGHT:
				if ((*iter)->GetDockStyle() & DS_NO_FIXED_RESIZE)
					DockSize = MIN((*iter)->m_DockSizeRatio*(GetWindowRect().Width()), rcChild.Width());

				if (RTL)
				{
					rcChild.right = rcChild.left + (int)DockSize;
					rcChild.right = MAX(rcChild.right, rc.left + minSize);
					rcChild.right = MIN(rcChild.right, rc.right - minSize);
				}
				else
				{
					rcChild.left = rcChild.right - (int)DockSize;
					rcChild.left = MIN(rcChild.left, rc.right - minSize);
					rcChild.left = MAX(rcChild.left, rc.left + minSize);
				}

				break;
			case DS_DOCKED_TOP:
				if ((*iter)->GetDockStyle() & DS_NO_FIXED_RESIZE)
					DockSize = MIN((*iter)->m_DockSizeRatio*(GetWindowRect().Height()), rcChild.Height());

				rcChild.bottom = rcChild.top + (int)DockSize;
				rcChild.bottom = MAX(rcChild.bottom, rc.top + minSize);
				rcChild.bottom = MIN(rcChild.bottom, rc.bottom - minSize);
				break;
			case DS_DOCKED_BOTTOM:
				if ((*iter)->GetDockStyle() & DS_NO_FIXED_RESIZE)
					DockSize = MIN((*iter)->m_DockSizeRatio*(GetWindowRect().Height()), rcChild.Height());

				rcChild.top = rcChild.bottom - (int)DockSize;
				rcChild.top = MIN(rcChild.top, rc.bottom - minSize);
				rcChild.top = MAX(rcChild.top, rc.top + minSize);

				break;
			}

			if ((*iter)->IsDocked())
			{
				// Position this docker's children
				hdwp = (*iter)->DeferWindowPos(hdwp, nullptr, rcChild, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
				(*iter)->m_rcChild = rcChild;

				rc.SubtractRect(rc, rcChild);

				// Calculate the dimensions of the splitter bar
				CRect rcBar = rc;
				DWORD DockSide = (*iter)->GetDockStyle() & 0xF;

				if (DS_DOCKED_LEFT   == DockSide)
				{
					if (RTL) rcBar.left   = rcBar.right - (*iter)->GetBarWidth();
					else	 rcBar.right  = rcBar.left + (*iter)->GetBarWidth();
				}

				if (DS_DOCKED_RIGHT  == DockSide)
				{
					if (RTL) rcBar.right  = rcBar.left + (*iter)->GetBarWidth();
					else	 rcBar.left   = rcBar.right - (*iter)->GetBarWidth();
				}

				if (DS_DOCKED_TOP    == DockSide) rcBar.bottom = rcBar.top + (*iter)->GetBarWidth();
				if (DS_DOCKED_BOTTOM == DockSide) rcBar.top    = rcBar.bottom - (*iter)->GetBarWidth();

				// Save the splitter bar position. We will reposition it later.
				(*iter)->m_rcBar = rcBar;
				rc.SubtractRect(rc, rcBar);
			}
		}

		// Step 2: Position the Dock client and dock bar
		if (GetDockClient().IsWindow())
			hdwp = GetDockClient().DeferWindowPos(hdwp, nullptr, rc, SWP_SHOWWINDOW |SWP_FRAMECHANGED);
		EndDeferWindowPos(hdwp);

		// Position the dockbar. Only docked dockers have a dock bar.
		if (IsDocked())
		{
			CRect rcBar;
			rcBar.IntersectRect(m_rcBar, GetDockParent()->GetViewRect());

			// The SWP_NOCOPYBITS forces a redraw of the dock bar.
			GetDockBar().SetWindowPos(nullptr, rcBar, SWP_SHOWWINDOW|SWP_FRAMECHANGED|SWP_NOCOPYBITS);
		}

		// Step 3: Now recurse through the docker's children. They might have children of their own.
		for (iter = m_vDockChildren.begin(); iter != m_vDockChildren.end(); ++iter)
		{
			(*iter)->RecalcDockChildLayout((*iter)->m_rcChild);
		}
	}

	inline void c_docker::RecalcDockLayout()
	// Repositions the dock children of a top level docker
	{
		if (GetDockAncestor()->IsWindow())
		{
			CRect rc = GetTopmostDocker()->GetViewRect();
			GetTopmostDocker()->RecalcDockChildLayout(rc);
			GetTopmostDocker()->UpdateWindow();
		}
	}

	inline std::vector<c_docker*> c_docker::SortDockers()
	// Returns a vector of sorted dockers, used by SaveRegistrySettings.
	{
		std::vector<c_docker*> vSorted;

		// Add undocked top level dockers
		for (std::vector<DockPtr>::const_iterator itAll = GetAllChildren().begin(); itAll !=  GetAllChildren().end(); ++itAll)
		{
			if (!(*itAll)->GetDockParent())
				vSorted.push_back((*itAll).get());
		}

		// Add dock ancestor's children
		vSorted.insert(vSorted.end(), GetDockAncestor()->GetDockChildren().begin(), GetDockAncestor()->GetDockChildren().end());

		// Add other dock children
		int index = 0;
		std::vector<c_docker*>::const_iterator itSort = vSorted.begin();
		while (itSort < vSorted.end())
		{
			vSorted.insert(vSorted.end(), (*itSort)->GetDockChildren().begin(), (*itSort)->GetDockChildren().end());
			itSort = vSorted.begin() + (++index);
		}

		// Add dockers docked in containers
		std::vector<c_docker*> vDockContainers;
		for (itSort = vSorted.begin(); itSort< vSorted.end(); ++itSort)
		{
			if ((*itSort)->GetContainer())
				vDockContainers.push_back(*itSort);
		}

		for (itSort = vDockContainers.begin(); itSort < vDockContainers.end(); ++itSort)
		{
			c_dock_container* pContainer = (*itSort)->GetContainer();

			for (UINT i = 0; i < pContainer->GetAllContainers().size(); ++i)
			{
				c_dock_container* pChild = pContainer->GetContainerFromIndex(i);

				if (pChild != pContainer)
				{
					c_docker* pDocker = GetDockFromView(pChild);
					assert(pDocker);
					vSorted.push_back(pDocker);
				}
			}
		}

		return vSorted;
	}

	inline void c_docker::ResizeDockers(LPDRAGPOS pdp)
	// Called when the docker's splitter bar is dragged
	{
		assert(pdp);

		POINT pt = pdp->ptPos;
		ScreenToClient(pt);

		c_docker* pDocker = pdp->pDocker;
		assert(pDocker);

		RECT rcDock = pDocker->GetWindowRect();
		ScreenToClient(rcDock);

		int iBarWidth = pDocker->GetDockBar().GetWidth();
		double dBarWidth = iBarWidth;
		int DockSize;

		BOOL RTL = FALSE;
#ifdef WS_EX_LAYOUTRTL
		RTL = (GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_LAYOUTRTL);
#endif

		CRect rcDockParent = pDocker->m_pDockParent->GetWindowRect();

		switch (pDocker->GetDockStyle() & 0xF)
		{
		case DS_DOCKED_LEFT:
			if (RTL) DockSize = rcDock.right - MAX(pt.x, iBarWidth / 2) - static_cast<int>(.5 * dBarWidth);
			else     DockSize = MAX(pt.x, iBarWidth / 2) - rcDock.left - static_cast<int>(.5 * dBarWidth);

			DockSize = MAX(-iBarWidth, DockSize);
			pDocker->SetDockSize(DockSize);
			pDocker->m_DockSizeRatio = static_cast<double>(pDocker->m_DockStartSize) / static_cast<double>(rcDockParent.Width());
			break;
		case DS_DOCKED_RIGHT:
			if (RTL)  DockSize = MAX(pt.x, iBarWidth / 2) - rcDock.left - static_cast<int>(.5 * dBarWidth);
			else      DockSize = rcDock.right - MAX(pt.x, iBarWidth / 2) - static_cast<int>(.5 * dBarWidth);

			DockSize = MAX(-iBarWidth, DockSize);
			pDocker->SetDockSize(DockSize);
			pDocker->m_DockSizeRatio = static_cast<double>(pDocker->m_DockStartSize) / static_cast<double>(rcDockParent.Width());
			break;
		case DS_DOCKED_TOP:
			DockSize = MAX(pt.y, iBarWidth / 2) - rcDock.top - static_cast<int>(.5 * dBarWidth);
			DockSize = MAX(-iBarWidth, DockSize);
			pDocker->SetDockSize(DockSize);
			pDocker->m_DockSizeRatio = static_cast<double>(pDocker->m_DockStartSize) / static_cast<double>(rcDockParent.Height());
			break;
		case DS_DOCKED_BOTTOM:
			DockSize = rcDock.bottom - MAX(pt.y, iBarWidth / 2) - static_cast<int>(.5 * dBarWidth);
			DockSize = MAX(-iBarWidth, DockSize);
			pDocker->SetDockSize(DockSize);
			pDocker->m_DockSizeRatio = static_cast<double>(pDocker->m_DockStartSize) / static_cast<double>(rcDockParent.Height());
			break;
		}

		RecalcDockLayout();
	}

	inline BOOL c_docker::SaveDockRegistrySettings(LPCTSTR szRegistryKeyName)
	// Stores the docking configuration in the registry
	// NOTE: This function assumes that each docker has a unique DockID
	{
		std::vector<c_docker*> vSorted = SortDockers();
		std::vector<c_docker*>::const_iterator iter;
		std::vector<dock_info> vDockInfo;

		if (szRegistryKeyName)
		{
			CRegKey Key;
			CRegKey KeyDock;
			c_string KeyName = _T("Software\\") + c_string(szRegistryKeyName);

			try
			{
				if (!VerifyDockers())
					throw (CUserException(_T("Dockers are in an inconsistant state")));

				// Create the App's registry key
				if (ERROR_SUCCESS != Key.Create(HKEY_CURRENT_USER, KeyName))
					throw (CUserException(_T("Create Key failed")));

				if (ERROR_SUCCESS != Key.Open(HKEY_CURRENT_USER, KeyName))
					throw (CUserException(_T("Open Key failed")));

				// Remove Old Docking info ...
				Key.RecurseDeleteKey(_T("Dock Windows"));

				// Fill the DockInfo vector with the docking information
				for (iter = vSorted.begin(); iter !=  vSorted.end(); ++iter)
				{
					dock_info di{};
					ZeroMemory(&di, sizeof(dock_info));
					if (! (*iter)->IsWindow())
						throw (CUserException(_T("Can't save Docker in registry. \n")));

					di.DockID	 = (*iter)->GetDockID();
					di.DockStyle = (*iter)->GetDockStyle();
					di.DockSize  = (*iter)->GetDockSize();
					di.Rect		 = (*iter)->GetWindowRect();
					if ((*iter)->GetDockParent())
						di.DockParentID = (*iter)->GetDockParent()->GetDockID();

					vDockInfo.push_back(di);
				}

				if (ERROR_SUCCESS != Key.Create(Key, _T("Dock Windows")))
					throw (CUserException(_T("Create KeyDock failed")));

				if (ERROR_SUCCESS != KeyDock.Open(Key, _T("Dock Windows")))
					throw (CUserException(_T("Open KeyDock failed")));

				c_string SubKeyName;

				// Add the Dock windows information to the registry
				for (UINT u = 0; u < vDockInfo.size(); ++u)
				{
					dock_info di = vDockInfo[u];
					SubKeyName.Format(_T("DockChild%u"), u);
					if(ERROR_SUCCESS != KeyDock.SetBinaryValue(SubKeyName, &di, sizeof(dock_info)))
						throw (CUserException(_T("KeyDock SetBinaryValue failed")));
				}

				// Add dock container info to the registry
				UINT u1 = 0;
				for (iter = vSorted.begin(); iter != vSorted.end(); ++iter)
				{
					c_dock_container* pContainer = (*iter)->GetContainer();

					if (pContainer && ( !((*iter)->GetDockStyle() & DS_DOCKED_CONTAINER) ))
					{
						CRegKey KeyContainer;
						SubKeyName.Format(_T("DockContainer%u"), u1++);
						if (ERROR_SUCCESS != KeyDock.Create(KeyDock, SubKeyName))
							throw (CUserException(_T("Create KeyDockContainer failed")));

						if (ERROR_SUCCESS != KeyContainer.Open(KeyDock, SubKeyName))
							throw (CUserException(_T("Open KeyContainer failed")));

						// Store the container group's parent
						c_docker* pDocker = GetDockFromView(pContainer);
						if (pDocker == 0)
							throw CUserException(_T("Failed to get docker from container view"));
						int nID = pDocker->GetDockID();
						if (ERROR_SUCCESS != KeyContainer.SetDWORDValue(_T("Parent Container"), nID))
							throw (CUserException(_T("KeyContainer SetDWORDValue failed")));

						// Store the active (selected) container
						pDocker = GetDockFromView(pContainer->GetActiveContainer());
						if (pDocker == 0)
							throw CUserException(_T("Failed to get active container's docker"));
						nID = pDocker->GetDockID();
						if (ERROR_SUCCESS != KeyContainer.SetDWORDValue(_T("Active Container"), nID))
							throw (CUserException(_T("KeyContainer SetDWORDValue failed")));

						// Store the tab order
						for (UINT u2 = 0; u2 < pContainer->GetAllContainers().size(); ++u2)
						{
							SubKeyName.Format(_T("Tab%u"), u2);
							c_dock_container* pTab = pContainer->GetContainerFromIndex(u2);
							if (pTab == 0)
								throw CUserException(_T("Failed to get container from index"));
							pDocker = GetDockFromView(pTab);
							if (pDocker== 0)
								throw CUserException(_T("Failed to get docker from view"));
							int nTabID = pDocker->GetDockID();

							if (ERROR_SUCCESS != KeyContainer.SetDWORDValue(SubKeyName, nTabID))
								throw (CUserException(_T("RegSetValueEx failed")));
						}
					}
				}
			}

			catch (const CUserException& e)
			{
				Trace("*** Failed to save dock settings in registry. ***");
				Trace(e.GetText()); Trace("\n");

				// Roll back the registry changes by deleting the subkeys
				if (Key.GetKey())
				{
					Key.RecurseDeleteKey(_T("Dock Windows"));
				}

				return FALSE;
			}
		}

		return TRUE;
	}

	inline void c_docker::SendNotify(UINT nMessageID)
	// Sends a docking notification to the docker below the cursor
	{
		DRAGPOS DragPos{};
		DragPos.hdr.code = nMessageID;
		DragPos.hdr.hwndFrom = GetHwnd();
		DragPos.ptPos = GetCursorPos();
		DragPos.DockZone = m_dwDockZone;
		DragPos.pDocker = this;
		m_dwDockZone = 0;

		c_docker* pDocker = GetDockUnderDragPoint(DragPos.ptPos);

		if (pDocker)
			pDocker->SendMessage(WM_NOTIFY, 0L, reinterpret_cast<LPARAM>(&DragPos));
		else
		{
			if (GetDockHint().IsWindow())
				GetDockHint().Destroy();

			CloseAllTargets();
			m_IsBlockMove = FALSE;
		}
	}

	inline void c_docker::SetDockStyle(DWORD dwDockStyle)
	{
		if (IsWindow())
		{
			if ((dwDockStyle & DS_CLIENTEDGE) != (m_DockStyle & DS_CLIENTEDGE))
			{
				if (dwDockStyle & DS_CLIENTEDGE)
				{
					DWORD dwExStyle = static_cast<DWORD>(GetDockClient().GetWindowLongPtr(GWL_EXSTYLE))|WS_EX_CLIENTEDGE;
					GetDockClient().SetWindowLongPtr(GWL_EXSTYLE, static_cast<LONG_PTR>(dwExStyle));
					GetDockClient().RedrawWindow(RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
				}
				else
				{
					auto dw_ex_style = static_cast<DWORD>(GetDockClient().GetWindowLongPtr(GWL_EXSTYLE));
					dw_ex_style &= ~WS_EX_CLIENTEDGE;
					GetDockClient().SetWindowLongPtr(GWL_EXSTYLE, static_cast<LONG_PTR>(dw_ex_style));
					GetDockClient().RedrawWindow(RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
				}
			}

			RecalcDockLayout();
		}

		m_DockStyle = dwDockStyle;
	}

	inline void c_docker::SetCaption(LPCTSTR szCaption)
	// Sets the caption text
	{
		GetDockClient().SetCaption(szCaption);

		if (IsWindow())
			SetWindowText(szCaption);
	}

	inline void c_docker::SetCaptionColors(COLORREF Foregnd1, COLORREF Backgnd1, COLORREF ForeGnd2, COLORREF BackGnd2, COLORREF PenColor /*= RGB(160, 150, 140)*/)
	{
		GetDockClient().SetCaptionColors(Foregnd1, Backgnd1, ForeGnd2, BackGnd2, PenColor);
	}

	inline void c_docker::SetCaptionHeight(int nHeight)
	// Sets the height of the caption
	{
		m_NCHeight = nHeight;
		if (IsWindow())
		{
			RedrawWindow();
			RecalcDockLayout();
		}
	}

	inline void c_docker::SetDockSize(int DockSize)
	// Sets the size of a docked docker
	{
		if (IsDocked())
		{
			assert (m_pDockParent);
			CRect rc = m_pDockParent->GetWindowRect();
			switch (GetDockStyle() & 0xF)
			{
			case DS_DOCKED_LEFT:
			case DS_DOCKED_RIGHT:
				m_DockStartSize = MIN(DockSize,rc.Width());
				m_DockSizeRatio = static_cast<double>(m_DockStartSize)/static_cast<double>(rc.Width());
				break;
			case DS_DOCKED_TOP:
			case DS_DOCKED_BOTTOM:
				m_DockStartSize = MIN(DockSize,rc.Height());
				m_DockSizeRatio = static_cast<double>(m_DockStartSize)/static_cast<double>(rc.Height());
				break;
			}

			RecalcDockLayout();
		}
		else
		{
			m_DockStartSize = DockSize;
			m_DockSizeRatio = 1.0;
		}
	}

	inline void c_docker::SetDragAutoResize(BOOL AutoResize)
	{
		m_IsDragAutoResize = AutoResize;
	}

	inline void c_docker::SetView(CWnd& wndView)
	// Assigns the view window to the docker
	{
		GetDockClient().SetView(wndView);
		c_dock_container* pContainer = GetContainer();
		if (pContainer)
		{
			SetCaption(pContainer->GetDockCaption().c_str());
		}
	}

	inline void c_docker::PromoteFirstChild()
	// One of the steps required for undocking
	{
		// Promote our first child to replace ourself
		if (m_pDockParent)
		{
			std::vector<c_docker*>& vChild = m_pDockParent->m_vDockChildren;

			for (std::vector<c_docker*>::iterator iter = vChild.begin(); iter != vChild.end(); ++iter)
			{
				if ((*iter) == this)
				{
					if (m_vDockChildren.size() > 0)
						// swap our first child for ourself as a child of the parent
						(*iter) = m_vDockChildren[0];
					else
						// remove ourself as a child of the parent
						vChild.erase(iter);

					// Done
					break;
				}
			}
		}

		// Transfer styles and data and children to the child docker
		c_docker* pDockFirstChild = NULL;
		if (m_vDockChildren.size() > 0)
		{
			pDockFirstChild = m_vDockChildren[0];
			pDockFirstChild->m_DockStyle = (pDockFirstChild->m_DockStyle & 0xFFFFFFF0 ) | (m_DockStyle & 0xF);
			pDockFirstChild->m_DockStartSize = m_DockStartSize;
			pDockFirstChild->m_DockSizeRatio = m_DockSizeRatio;

			if (m_pDockParent)
			{
				pDockFirstChild->m_pDockParent = m_pDockParent;
				pDockFirstChild->SetParent(*m_pDockParent);
				pDockFirstChild->GetDockBar().SetParent(*m_pDockParent);
			}
			else
			{
				for (std::vector<c_docker*>::const_iterator iter = m_vDockChildren.begin() + 1; iter != m_vDockChildren.end(); ++iter)
					(*iter)->ShowWindow(SW_HIDE);

				pDockFirstChild->ConvertToPopup(GetWindowRect());
				pDockFirstChild->GetDockBar().ShowWindow(SW_HIDE);
			}

			m_vDockChildren.erase(m_vDockChildren.begin());
			MoveDockChildren(pDockFirstChild);
		}
	}

	inline void c_docker::ConvertToChild(HWND hWndParent)
	{
		DWORD dwStyle = WS_CHILD | WS_VISIBLE;
		SetWindowLongPtr(GWL_STYLE, dwStyle);
		SetParent(hWndParent);
		GetDockBar().SetParent(hWndParent);
	}

	inline void c_docker::ConvertToPopup(const CRect& rc)
	{
		// Change the window to an "undocked" style
		ShowWindow(SW_HIDE);
		const auto dwStyle = WS_POPUP| WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
		SetWindowLongPtr(GWL_STYLE, static_cast<LONG_PTR>(dwStyle));

		// Change the window's parent and reposition it
		GetDockBar().ShowWindow(SW_HIDE);
		SetWindowPos(0, 0, 0, 0, 0, SWP_NOSENDCHANGING|SWP_HIDEWINDOW|SWP_NOREDRAW);
		m_pDockParent = 0;
		SetParent(0);
		SetWindowPos(nullptr, rc, SWP_SHOWWINDOW|SWP_FRAMECHANGED|SWP_NOOWNERZORDER);
		GetDockClient().SetWindowPos(nullptr, GetClientRect(), SWP_SHOWWINDOW);

		SetWindowText(GetCaption().c_str());
	}

	inline c_docker* c_docker::SeparateFromDock()
	{
		// This performs some of the tasks required for undocking.
		// It is also used when a docker is hidden.
		c_docker* pDockUndockedFrom = GetDockParent();
		if (!pDockUndockedFrom && (m_vDockChildren.size() > 0))
			pDockUndockedFrom = m_vDockChildren[0];

		PromoteFirstChild();
		m_pDockParent = 0;

		GetDockBar().ShowWindow(SW_HIDE);
		m_DockStyle = m_DockStyle & 0xFFFFFFF0;
		m_DockStyle &= ~DS_DOCKED_CONTAINER;

		return pDockUndockedFrom;
	}

	inline BOOL c_docker::SetRedraw(BOOL Redraw /* = TRUE*/)
	{
		// Allows nested calls to SetRedraw.
		Redraw? ++m_nRedrawCount : --m_nRedrawCount ;

		return static_cast<BOOL>(SendMessage(WM_SETREDRAW, (m_nRedrawCount >= 0), 0L));
	}

	inline void c_docker::SetUndockPosition(CPoint pt)
	{
		m_IsUndocking = TRUE;
		CRect rc;
		rc = GetDockClient().GetWindowRect();
		CRect rcTest = rc;
		rcTest.bottom = MIN(rcTest.bottom, rcTest.top + m_NCHeight);
		if ( !rcTest.PtInRect(pt))
			rc.SetRect(pt.x - rc.Width()/2, pt.y - m_NCHeight/2, pt.x + rc.Width()/2, pt.y - m_NCHeight/2 + rc.Height());

		ConvertToPopup(rc);
		m_IsUndocking = FALSE;

		// Send the undock notification to the frame
		NMHDR nmhdr;
		ZeroMemory(&nmhdr, sizeof(NMHDR));
		nmhdr.hwndFrom = GetHwnd();
		nmhdr.code = UWN_UNDOCKED;
		nmhdr.idFrom = m_nDockID;
		HWND hFrame = GetDockAncestor()->GetAncestor();
		assert(hFrame);

		::SendMessage(hFrame, WM_NOTIFY, (WPARAM)m_nDockID, (LPARAM)&nmhdr);

		// Initiate the window move
		SetCursorPos(pt.x, pt.y);
		ScreenToClient(pt);
		PostMessage(WM_SYSCOMMAND, (WPARAM)(SC_MOVE|0x0002), MAKELPARAM(pt.x, pt.y));
	}

	inline void c_docker::Undock(CPoint pt, BOOL ShowUndocked)
	{
		// Return if we shouldn't undock
		if (GetDockStyle() & DS_NO_UNDOCK) return;

		// Undocking isn't supported on Win95
		if (1400 == GetWinVersion()) return;

		c_docker* pDockUndockedFrom = SeparateFromDock();

		// Position and draw the undocked window, unless it is about to be closed
		if (ShowUndocked)
		{
			SetUndockPosition(pt);
		}

		// Give the view window focus unless its child already has it
		if (!GetView().IsChild(GetFocus()))
			GetView().SetFocus();

		RecalcDockLayout();
		if ((pDockUndockedFrom) && (pDockUndockedFrom->GetTopmostDocker() != GetTopmostDocker()))
			pDockUndockedFrom->RecalcDockLayout();
	}

	inline void c_docker::UndockContainer(c_dock_container* pContainer, CPoint pt, BOOL ShowUndocked)
	{
		assert(pContainer);
		assert(this == GetDockFromView(pContainer->GetContainerParent()));

		// Return if we shouldn't undock
		if (GetDockFromView(pContainer)->GetDockStyle() & DS_NO_UNDOCK) return;

		if (GetDockFromView(pContainer) == GetDockAncestor()) return;

		// Undocking isn't supported on Win95
		if (1400 == GetWinVersion()) return;

		c_docker* pDockUndockedFrom = this;
		pDockUndockedFrom->ShowWindow(SW_HIDE);
		if (&GetView() == pContainer)
		{
			// The parent container is being undocked, so we need
			// to transfer our child containers to a different docker

			// Choose a new docker from among the dockers for child containers
			c_docker* pDockNew = 0;
			c_docker* pDockOld = GetDockFromView(pContainer);
			assert(pDockOld);
			std::vector<container_info> AllContainers = pContainer->GetAllContainers();
			std::vector<container_info>::const_iterator iter = AllContainers.begin();
			while ((nullptr == pDockNew) && (iter != AllContainers.end()))
			{
				if ((*iter).pContainer != pContainer)
				{
					pDockNew = (*iter).pContainer->GetDocker();
					assert(pDockNew);
				}

				++iter;
			}

			if (pDockNew)
			{
				// Move containers from the old docker to the new docker
				c_dock_container* pContainerNew = pDockNew->GetContainer();
				assert(pContainerNew);
				for (iter = AllContainers.begin(); iter != AllContainers.end(); ++iter)
				{
					if ((*iter).pContainer != pContainer)
					{
						c_dock_container* pChildContainer = (*iter).pContainer;
						pContainer->RemoveContainer(pChildContainer);
						if (pContainerNew != pChildContainer)
						{
							pContainerNew->AddContainer(pChildContainer);
							c_docker* pDocker = GetDockFromView(pChildContainer);
							assert(pDocker);
							pDocker->SetParent(*pDockNew);
							pDocker->m_pDockParent = pDockNew;
						}
					}
				}

				// Now transfer the old docker's settings to the new one.
				pDockUndockedFrom = pDockNew;
				pDockNew->m_DockStyle		= pDockOld->m_DockStyle;
				pDockNew->m_DockStartSize	= pDockOld->m_DockStartSize;
				pDockNew->m_DockSizeRatio	= pDockOld->m_DockSizeRatio;
				if (pDockOld->IsDocked())
				{
					pDockNew->m_pDockParent		= pDockOld->m_pDockParent;
					pDockNew->SetParent(pDockOld->GetParent());
				}
				else
				{
					CRect rc = pDockOld->GetWindowRect();
					pDockNew->ShowWindow(SW_HIDE);
					DWORD dwStyle = WS_POPUP| WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_VISIBLE;
					pDockNew->SetWindowLongPtr(GWL_STYLE, static_cast<LONG_PTR>(dwStyle));
					pDockNew->m_pDockParent = 0;
					pDockNew->SetParent(0);
					pDockNew->SetWindowPos(nullptr, rc, SWP_SHOWWINDOW|SWP_FRAMECHANGED| SWP_NOOWNERZORDER);
				}
				pDockNew->GetDockBar().SetParent(pDockOld->GetParent());
				pDockNew->GetView().SetFocus();

				// Transfer the Dock children to the new docker
				pDockOld->MoveDockChildren(pDockNew);

				// insert pDockNew into its DockParent's DockChildren vector
				if (pDockNew->m_pDockParent)
				{
					std::vector<c_docker*>::iterator p;
					for (p = pDockNew->m_pDockParent->m_vDockChildren.begin(); p != pDockNew->m_pDockParent->m_vDockChildren.end(); ++p)
					{
						if (*p == this)
						{
							pDockNew->m_pDockParent->m_vDockChildren.insert(p, pDockNew);
							break;
						}
					}
				}
			}
		}
		else
		{
			// This is a child container, so simply remove it from the parent
			c_dock_container* pContainerParent = GetContainer();
			assert(pContainerParent);
			pContainerParent->RemoveContainer(pContainer);
			pContainerParent->SetTabSize();
			pContainerParent->SetFocus();
			pContainerParent->GetViewPage().SetParent(*pContainerParent);
		}

		// Finally do the actual undocking
		c_docker* pDocker = GetDockFromView(pContainer);
		assert(pDocker);
		CRect rc = GetDockClient().GetWindowRect();
		ScreenToClient(rc);
		pDocker->GetDockClient().SetWindowPos(NULL, rc, SWP_SHOWWINDOW);
		pDocker->Undock(pt, ShowUndocked);
		pDocker->GetDockBar().SetParent(*GetDockAncestor());
		pDockUndockedFrom->ShowWindow();
		pDockUndockedFrom->RecalcDockLayout();
		pDocker->BringWindowToTop();
	}

	inline LRESULT c_docker::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_ACTIVATE:			return OnActivate(uMsg, wParam, lParam);
		case WM_SYSCOMMAND:			return OnSysCommand(uMsg, wParam, lParam);
		case WM_EXITSIZEMOVE:		return OnExitSizeMove(uMsg, wParam, lParam);
		case WM_MOUSEACTIVATE:		return OnMouseActivate(uMsg, wParam, lParam);
		case WM_NCLBUTTONDBLCLK:	return OnNCLButtonDblClk(uMsg, wParam, lParam);
		case WM_SIZE:				return OnSize(uMsg, wParam, lParam);
		case WM_SYSCOLORCHANGE:		return OnSysColorChange(uMsg, wParam, lParam);
		case WM_WINDOWPOSCHANGING:	return OnWindowPosChanging(uMsg, wParam, lParam);
		case WM_WINDOWPOSCHANGED:	return OnWindowPosChanged(uMsg, wParam, lParam);

		// Messages defined by Win32++
		case UWM_DOCKACTIVATE:		return OnDockActivated(uMsg, wParam, lParam);
		case UWM_DOCKDESTROYED:		return OnDockDestroyed(uMsg, wParam, lParam);
		case UWM_GETCDOCKER:		return reinterpret_cast<LRESULT>(this);
		}

		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

	inline BOOL CALLBACK c_docker::EnumWindowsProc(HWND hWndTop, LPARAM lParam)
	// Static callback function to enumerate top level dockers excluding
	// the one being dragged. Top level windows are enumurated in Z order.
	{
		c_docker* This = reinterpret_cast<c_docker*>(lParam);
		assert(dynamic_cast<c_docker*>(This));
		CPoint pt = This->m_DockPoint;

		// Update hWndTop if the DockAncestor is a child of the top level window
		if (::IsChild(hWndTop, This->GetDockAncestor()->GetHwnd()))
			hWndTop = This->GetDockAncestor()->GetHwnd();

		// Assign this docker's m_hDockUnderPoint
		if (This->IsRelated(hWndTop) && hWndTop != This->GetHwnd())
		{
			CRect rc;
			::GetWindowRect(hWndTop, &rc);
			if ( rc.PtInRect(pt) )
			{
				This->m_hDockUnderPoint = hWndTop;
				return FALSE;	// Stop enumerating
			}
		}

		return TRUE;	// Continue enumerating
	}


	//////////////////////////////////////
	// Declaration of the CDockContainer class
	inline c_dock_container::c_dock_container() : m_iCurrentPage(0), m_pDocker(0), m_hTabIcon(0),
		                                m_nTabPressed(-1), m_IsHideSingleTab(FALSE)
	{
		m_pContainerParent = this;
		m_ViewPage.SetContainer(this);
	}

	inline c_dock_container::~c_dock_container()
	{
		if (m_hTabIcon != 0)
			DestroyIcon(m_hTabIcon);
	}

	inline void c_dock_container::AddContainer(c_dock_container* pContainer, BOOL Insert /* = FALSE */)
	// Adds a container to the group. Set Insert to TRUE to insert the container as the
	//  first tab, or FALSE to add it as the last tab.
	{
		assert(pContainer);
		assert(this == m_pContainerParent); // Must be performed by parent container

		container_info ci;
		ci.pContainer = pContainer;
		ci.Title = pContainer->GetTabText();
		ci.iImage = GetODImageList().Add( pContainer->GetTabIcon() );
		int iNewPage = 0;
		if (Insert)
		{
			m_vContainerInfo.insert(m_vContainerInfo.begin(), ci);
		}
		else
		{
			iNewPage = static_cast<int>(m_vContainerInfo.size());
			m_vContainerInfo.push_back(ci);
		}

		if (IsWindow())
		{
			TCITEM tie;
			ZeroMemory(&tie, sizeof(TCITEM));
			tie.mask = TCIF_TEXT | TCIF_IMAGE;
			tie.iImage = ci.iImage;
			tie.pszText = const_cast<LPTSTR>(m_vContainerInfo[iNewPage].Title.c_str());
			InsertItem(iNewPage, &tie);
			SelectPage(iNewPage);
			SetTabSize();
		}

		pContainer->m_pContainerParent = this;
		if (pContainer->IsWindow())
		{
			// Set the parent container relationships
			pContainer->GetViewPage().SetParent(*this);
			pContainer->GetViewPage().ShowWindow(SW_HIDE);
			RecalcLayout();
		}
	}

	inline void c_dock_container::AddToolBarButton(UINT nID, BOOL IsEnabled /* = TRUE */)
	// Adds Resource IDs to toolbar buttons.
	// A resource ID of 0 is a separator
	{
		GetToolBarData().push_back(nID);
		GetToolBar().AddButton(nID, IsEnabled);
	}

	inline c_dock_container* c_dock_container::GetContainerFromIndex(UINT nPage)
	{
		c_dock_container* pContainer = nullptr;
		if (nPage < m_vContainerInfo.size())
			pContainer = m_vContainerInfo[nPage].pContainer;

		return pContainer;
	}

	inline c_dock_container* c_dock_container::GetActiveContainer() const
	{
		assert(m_pContainerParent);
		assert((int)m_pContainerParent->m_vContainerInfo.size() > m_pContainerParent->m_iCurrentPage);
		return m_pContainerParent->m_vContainerInfo[m_pContainerParent->m_iCurrentPage].pContainer;
	}

	inline CWnd* c_dock_container::GetActiveView() const
	// Returns a pointer to the active view window, or NULL if there is no active veiw.
	{
		assert (GetActiveContainer());
		return &GetActiveContainer()->GetView();
	}

	inline c_dock_container* c_dock_container::GetContainerFromView(CWnd* pView) const
	{
		assert(pView);

		c_dock_container* pViewContainer = 0;
		for (std::vector<container_info>::const_iterator iter = GetAll().begin(); iter != GetAll().end(); ++iter)
		{
			c_dock_container* pContainer = (*iter).pContainer;
			if (&pContainer->GetView() == pView)
				pViewContainer = pContainer;
		}

		return pViewContainer;
	}

	inline int c_dock_container::GetContainerIndex(c_dock_container* pContainer)
	{
		assert(pContainer);
		int iReturn = -1;

		for (int i = 0; i < (int)m_pContainerParent->m_vContainerInfo.size(); ++i)
		{
			if (m_pContainerParent->m_vContainerInfo[i].pContainer == pContainer)
				iReturn = i;
		}

		return iReturn;
	}

	inline SIZE c_dock_container::GetMaxTabTextSize()
	{
		CSize Size;

		// Allocate an iterator for the ContainerInfo vector
		std::vector<container_info>::const_iterator iter;

		for (iter = m_vContainerInfo.begin(); iter != m_vContainerInfo.end(); ++iter)
		{
			CSize TempSize;
			CClientDC dc(*this);
			NONCLIENTMETRICS info;
			ZeroMemory(&info, sizeof(NONCLIENTMETRICS));
			info.cbSize = GetSizeofNonClientMetrics();
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
			dc.CreateFontIndirect(info.lfStatusFont);
			TempSize = dc.GetTextExtentPoint32(iter->Title, lstrlen(iter->Title));
			if (TempSize.cx > Size.cx)
				Size = TempSize;
		}

		return Size;
	}

	inline int c_dock_container::GetTabImageID(UINT nTab) const
	{
		assert (nTab < GetAllContainers().size());
		return GetAllContainers()[nTab].iImage;
	}

	inline c_string c_dock_container::GetTabText(UINT nTab) const
	{
		assert (nTab < GetAllContainers().size());
		return GetAllContainers()[nTab].Title;
	}

	inline void c_dock_container::SetupToolBar()
	{
		// Use this function to set the Resource IDs for the toolbar(s).

/*		// Set the Resource IDs for the toolbar buttons
		AddToolBarButton( IDM_FILE_NEW   );
		AddToolBarButton( IDM_FILE_OPEN  );
		AddToolBarButton( IDM_FILE_SAVE  );
		AddToolBarButton( 0 );				// Separator
		AddToolBarButton( IDM_EDIT_CUT   );
		AddToolBarButton( IDM_EDIT_COPY  );
		AddToolBarButton( IDM_EDIT_PASTE );
		AddToolBarButton( 0 );				// Separator
		AddToolBarButton( IDM_FILE_PRINT );
		AddToolBarButton( 0 );				// Separator
		AddToolBarButton( IDM_HELP_ABOUT );
*/
	}

	inline void c_dock_container::OnAttach()
	{
		assert(&GetView());			// Use SetView in the constructor to set the view window

		// Create and assign the tab's image list
		GetODImageList().Create(16, 16, ILC_MASK|ILC_COLOR32, 0, 0);

		// Set the tab control's font
		NONCLIENTMETRICS info;
		ZeroMemory(&info, sizeof(NONCLIENTMETRICS));
		info.cbSize = GetSizeofNonClientMetrics();
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
		GetTabFont().CreateFontIndirect(info.lfStatusFont);
		SetFont(GetTabFont());

		container_info ci;
		ci.pContainer = this;
		ci.Title = GetTabText();
		ci.iImage = GetODImageList().Add( GetTabIcon() );
		m_vContainerInfo.push_back(ci);

		// Create the page window
		GetViewPage().Create(*this);

		// Create the toolbar
		GetToolBar().Create(GetViewPage());
		DWORD style = static_cast<DWORD>(GetToolBar().GetWindowLongPtr(GWL_STYLE));
		style |= CCS_NODIVIDER;
		GetToolBar().SetWindowLongPtr(GWL_STYLE, static_cast<LONG_PTR>(style));
		SetupToolBar();
		if (GetToolBarData().size() > 0)
		{
			// Set the toolbar images
			// A mask of 192,192,192 is compatible with AddBitmap (for Win95)
			if (!GetToolBar().SendMessage(TB_GETIMAGELIST,  0L, 0L))
				SetToolBarImages(RGB(192,192,192), IDW_MAIN, 0, 0);

			GetToolBar().SendMessage(TB_AUTOSIZE, 0L, 0L);
		}
		else
			GetToolBar().Destroy();

		SetFixedWidth(TRUE);
		SetOwnerDraw(TRUE);

		// Add tabs for each container.
		for (int i = 0; i < (int)m_vContainerInfo.size(); ++i)
		{
			// Add tabs for each view.
			TCITEM tie;
			ZeroMemory(&tie, sizeof(TCITEM));
			tie.mask = TCIF_TEXT | TCIF_IMAGE;
			tie.iImage = m_vContainerInfo[i].iImage;
			tie.pszText = const_cast<LPTSTR>(m_vContainerInfo[i].Title.c_str());
			InsertItem(i, &tie);
		}

		m_pDocker = reinterpret_cast<c_docker*>((GetParent().GetParent().SendMessage(UWM_GETCDOCKER)));
	}

	inline LRESULT c_dock_container::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Overrides CTab::OnLButtonDown

		UNREFERENCED_PARAMETER(wParam);

		CPoint pt(static_cast<DWORD>(lParam));
		TCHITTESTINFO info;
		ZeroMemory(&info, sizeof(TCHITTESTINFO));
		info.pt = pt;
		m_nTabPressed = HitTest(info);

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_dock_container::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Overrides CTab::OnLButtonUp and takes no action

		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);
		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_dock_container::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Overrides CTab::OnMouseLeave

		if (IsLeftButtonDown() && (m_nTabPressed >= 0))
		{
			if (GetDocker())
			{
				c_dock_container* pContainer = GetContainerFromIndex(m_iCurrentPage);
				GetDocker()->UndockContainer(pContainer, GetCursorPos(), TRUE);
			}
		}

		m_nTabPressed = -1;
		return CTab::OnMouseLeave(uMsg, wParam, lParam);
	}

	inline LRESULT c_dock_container::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		CPoint pt;
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);

		// Skip if mouse hasn't moved
		if ((pt.x == m_OldMousePos.x) && (pt.y == m_OldMousePos.y))
			return FALSE;

		m_OldMousePos.x = pt.x;
		m_OldMousePos.y = pt.y;

		if (IsLeftButtonDown())
		{
			TCHITTESTINFO info;
			ZeroMemory(&info, sizeof(TCHITTESTINFO));
			info.pt = CPoint((DWORD_PTR)lParam);
			int nTab = HitTest(info);
			if (nTab >= 0 && m_nTabPressed >= 0)
			{
				if (nTab !=  m_nTabPressed)
				{
					SwapTabs(nTab, m_nTabPressed);
					m_nTabPressed = nTab;
					SelectPage(nTab);
				}
			}
		}

		return CTab::OnMouseMove(uMsg, wParam, lParam);
	}

	inline LRESULT c_dock_container::OnNotifyReflect(WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(wParam);

		LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lParam);
		switch (pNMHDR->code)
		{
		case TCN_SELCHANGE:	return OnTCNSelChange(pNMHDR);
		}

		// The framework will call SetWindowLongPtr(DWLP_MSGRESULT, lr) for non-zero returns
		return 0L;
	}

	inline LRESULT c_dock_container::OnTCNSelChange(LPNMHDR pNMHDR)
	{
		UNREFERENCED_PARAMETER(pNMHDR);

		// Display the newly selected tab page
		int nPage = GetCurSel();
		SelectPage(nPage);

		return 0L;
	}

	inline LRESULT c_dock_container::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Sets the focus to the active view (or its child)
		HWND hwndPrevFocus = reinterpret_cast<HWND>(wParam);
		if (GetActiveView()->IsChild(hwndPrevFocus))
		{
			// return focus back to the child of the active view that had it before
			::SetFocus(hwndPrevFocus);
		}
		else
			GetActiveView()->SetFocus();

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT c_dock_container::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(uMsg);
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		RecalcLayout();
		return 0;
	}

	inline void c_dock_container::PreCreate(CREATESTRUCT& cs)
	{
		// For Tabs on the bottom, add the TCS_BOTTOM style
		CTab::PreCreate(cs);
		cs.style |= TCS_BOTTOM;
	}

	inline void c_dock_container::RecalcLayout()
	{
		if (GetContainerParent() == this)
		{
			// Set the tab sizes
			SetTabSize();

			// Position the View over the tab control's display area
			CRect rc = GetClientRect();
			AdjustRect(FALSE, &rc);
			c_dock_container* pContainer = m_vContainerInfo[m_iCurrentPage].pContainer;
			pContainer->GetViewPage().SetWindowPos(0, rc, SWP_SHOWWINDOW);
			RedrawWindow(RDW_INVALIDATE|RDW_NOCHILDREN);
		}
	}

	inline void c_dock_container::RemoveContainer(c_dock_container* pWnd)
	{
		assert(pWnd);

		// Remove the tab
		int iTab = GetContainerIndex(pWnd);
		DeleteItem(iTab);

		int iImage = -1;
		for (std::vector<container_info>::iterator iter = m_vContainerInfo.begin(); iter != m_vContainerInfo.end(); ++iter)
		{
			if (iter->pContainer == pWnd)
			{
				iImage = (*iter).iImage;
				if (iImage >= 0)
					RemoveImage(iImage);

				m_vContainerInfo.erase(iter);
				break;
			}
		}

		// Set the parent container relationships
		pWnd->GetViewPage().SetParent(*pWnd);
		pWnd->m_pContainerParent = pWnd;

		// Display next lowest page
		m_iCurrentPage = MAX(iTab -1, 0);
		if (IsWindow())
		{
			SelectPage(m_iCurrentPage);
			RecalcLayout();
		}
	}

	inline void c_dock_container::SelectPage(int nPage)
	{
		if (this != m_pContainerParent)
			m_pContainerParent->SelectPage(nPage);
		else
		{
			if ((nPage >= 0) && (nPage < static_cast<int>(m_vContainerInfo.size()) ))
			{
				if (GetCurSel() != nPage)
					SetCurSel(nPage);

				// Create the new container window if required
				if (!m_vContainerInfo[nPage].pContainer->IsWindow())
				{
					c_dock_container* pContainer = m_vContainerInfo[nPage].pContainer;
					pContainer->Create( GetParent() );
					pContainer->GetViewPage().SetParent(*this);
				}

				// Determine the size of the tab page's view area
				CRect rc = GetClientRect();
				AdjustRect(FALSE, &rc);

				// Swap the pages over
				c_dock_container* pNewContainer = m_vContainerInfo[nPage].pContainer;
				for (std::vector<container_info>::const_iterator it = m_vContainerInfo.begin(); it != m_vContainerInfo.end(); ++it)
				{
					(*it).pContainer->GetViewPage().ShowWindow(SW_HIDE);
				}

				pNewContainer->GetViewPage().SetWindowPos(0, rc, SWP_SHOWWINDOW);
				pNewContainer->GetViewPage().GetView().SetFocus();

				// Adjust the docking caption
				if (GetDocker())
				{
					GetDocker()->SetCaption(pNewContainer->GetDockCaption());
					GetDocker()->RedrawWindow();
				}

				m_iCurrentPage = nPage;
			}
		}
	}

	inline void c_dock_container::SetActiveContainer(c_dock_container* pContainer)
	{
		int nPage = GetContainerIndex(pContainer);
		assert (0 <= nPage);
		SelectPage(nPage);
	}

	inline void c_dock_container::SetHideSingleTab(BOOL HideSingle)
	// Only display tabs if there are two or more.
	{
		m_IsHideSingleTab = HideSingle;
		RecalcLayout();
	}

	inline void c_dock_container::SetTabIcon(UINT nID_Icon)
	{
		const auto hIcon = static_cast<HICON>(GetApp().LoadImage(nID_Icon, IMAGE_ICON, 0, 0, LR_SHARED));
		SetTabIcon(hIcon);
	}

	inline void c_dock_container::SetTabSize()
	{
		CRect rc = GetClientRect();
		AdjustRect(FALSE, &rc);
		if (rc.Width() < 0 )
			rc.SetRectEmpty();

		int nItemWidth = 0;
		int nItemHeight = 1;
		if ((GetItemCount() != 1) || !m_IsHideSingleTab)
		{
			nItemWidth = MIN(25 + GetMaxTabTextSize().cx, (rc.Width()-2)/(int)m_vContainerInfo.size());
			nItemHeight = MAX(20, GetTextHeight() + 5);
		}
		SendMessage(TCM_SETITEMSIZE, 0L, MAKELPARAM(nItemWidth, nItemHeight));
	}

	inline void c_dock_container::SetTabText(UINT nTab, LPCTSTR szText)
	{
		c_dock_container* pContainer = GetContainerParent()->GetContainerFromIndex(nTab);
		pContainer->SetTabText(szText);

		CTab::SetTabText(nTab, szText);
	}

	inline void c_dock_container::SetToolBarImages(COLORREF crMask, UINT ToolBarID, UINT ToolBarHotID, UINT ToolBarDisabledID)
	// Either sets the imagelist or adds/replaces bitmap depending on ComCtl32.dll version
	// Assumes the width of the button image = height, minimum width = 16
	// The colour mask is ignored for 32bit bitmaps, but is required for 24bit bitmaps
	// The colour mask is often grey RGB(192,192,192) or magenta (255,0,255)
	// The hot and disabled bitmap resources can be 0
	// A disabled image list is created from the normal image list if one isn;t provided.
	{
		// ToolBar ImageLists require Comctl32.dll version 4.7 or later
		if (GetComCtlVersion() < 470)
		{
			// We are using COMCTL32.DLL version 4.0, so we can't use an ImageList.
			// Instead we simply set the bitmap.
			GetToolBar().SetBitmap(ToolBarID);
			return;
		}

		// Set the button images
		CBitmap Bitmap(ToolBarID);
		assert(Bitmap.GetHandle());

		BITMAP bm = Bitmap.GetBitmapData();
		int cy = bm.bmHeight;
		int cx  = MAX(bm.bmHeight, 16);

		m_imlToolBar.DeleteImageList();
		m_imlToolBar.Create(cx, cy, ILC_COLOR32 | ILC_MASK, 0, 0);
		m_imlToolBar.Add(Bitmap, crMask);
		GetToolBar().SetImageList(m_imlToolBar);

		if (ToolBarHotID)
		{
			CBitmap BitmapHot(ToolBarHotID);
			assert(BitmapHot);

			m_imlToolBarHot.DeleteImageList();
			m_imlToolBarHot.Create(cx, cy, ILC_COLOR32 | ILC_MASK, 0, 0);
			m_imlToolBarHot.Add(BitmapHot, crMask);
			GetToolBar().SetHotImageList(m_imlToolBarHot);
		}

		if (ToolBarDisabledID)
		{
			CBitmap BitmapDisabled(ToolBarDisabledID);
			assert(BitmapDisabled);

			m_imlToolBarDis.DeleteImageList();
			m_imlToolBarDis.Create(cx, cy, ILC_COLOR32 | ILC_MASK, 0, 0);
			m_imlToolBarDis.Add(BitmapDisabled, crMask);
			GetToolBar().SetDisableImageList( m_imlToolBarDis );
		}
		else
		{
			m_imlToolBarDis.DeleteImageList();
			m_imlToolBarDis.CreateDisabledImageList(m_imlToolBar);
			GetToolBar().SetDisableImageList( m_imlToolBarDis );
		}
	}

	inline void c_dock_container::SetView(CWnd& Wnd) const
	{
		GetViewPage().SetView(Wnd);
	}

	inline void c_dock_container::SwapTabs(UINT nTab1, UINT nTab2)
	{
		assert (nTab1 < GetContainerParent()->m_vContainerInfo.size());
		assert (nTab2 < GetContainerParent()->m_vContainerInfo.size());

		if (nTab1 != nTab2)
		{
			container_info ci1 = GetContainerParent()->m_vContainerInfo[nTab1];
			container_info ci2 = GetContainerParent()->m_vContainerInfo[nTab2];

			TCITEM item1;
			ZeroMemory(&item1, sizeof(TCITEM));
			item1.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_RTLREADING | TCIF_STATE | TCIF_TEXT;
			item1.cchTextMax = ci1.Title.GetLength()+1;
			item1.pszText = const_cast<LPTSTR>(ci1.Title.c_str());
			GetItem(nTab1, &item1);

			TCITEM item2;
			ZeroMemory(&item2, sizeof(TCITEM));
			item2.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_RTLREADING | TCIF_STATE | TCIF_TEXT;
			item2.cchTextMax = ci2.Title.GetLength()+1;
			item2.pszText = const_cast<LPTSTR>(ci2.Title.c_str());
			GetItem(nTab2, &item2);

			SetItem(nTab1, &item2);
			SetItem(nTab2, &item1);
			GetContainerParent()->m_vContainerInfo[nTab1] = ci2;
			GetContainerParent()->m_vContainerInfo[nTab2] = ci1;
		}
	}

	inline LRESULT c_dock_container::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:	return OnLButtonDown(uMsg, wParam, lParam);
		case WM_LBUTTONUP:		return OnLButtonUp(uMsg, wParam, lParam);
		case WM_MOUSELEAVE:		return OnMouseLeave(uMsg, wParam, lParam);
		case WM_MOUSEMOVE:		return OnMouseMove(uMsg, wParam, lParam);
		case WM_SETFOCUS:		return OnSetFocus(uMsg, wParam, lParam);
		case WM_SIZE:			return OnSize(uMsg, wParam, lParam);
		case UWM_GETCDOCKCONTAINER:	return reinterpret_cast<LRESULT>(this);
		}

		// pass unhandled messages on to CTab for processing
		return CTab::WndProcDefault(uMsg, wParam, lParam);
	}


	///////////////////////////////////////////
	// Declaration of the nested CViewPage class

	inline c_dock_container* c_dock_container::c_view_page::GetContainer() const
	{
		assert(m_pContainer);
		return m_pContainer;
	}

	inline BOOL c_dock_container::c_view_page::OnCommand(WPARAM wParam, LPARAM lParam)
	{
		BOOL IsHandled = FALSE;
		if (GetContainer()->GetActiveContainer() && GetContainer()->GetActiveContainer()->IsWindow())
			IsHandled = static_cast<BOOL>(GetContainer()->GetActiveContainer()->SendMessage(WM_COMMAND, wParam, lParam));

		return IsHandled;
	}

	inline int c_dock_container::c_view_page::OnCreate(CREATESTRUCT& cs)
	{
		UNREFERENCED_PARAMETER(cs);
		if (m_pView)
			m_pView->Create(*this);

		return 0;
	}

	inline LRESULT c_dock_container::c_view_page::OnNotify(WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(wParam);

		switch (reinterpret_cast<LPNMHDR>(lParam)->code)
		{

		// Display tooltips for the toolbar
		case TTN_GETDISPINFO:
			{
				const auto iIndex =  GetToolBar().HitTest();
				auto lp_disp_info = reinterpret_cast<LPNMTTDISPINFO>(lParam);
				if (iIndex >= 0)
				{
					int nID = GetToolBar().GetCommandID(iIndex);
					if (nID > 0)
					{
						m_strTooltip = LoadString(nID);
						lp_disp_info->lpszText = const_cast<LPTSTR>(m_strTooltip.c_str());
					}
					else
						m_strTooltip = _T("");
				}
			}
			break;
		case NM_CUSTOMDRAW:
			{
				if (reinterpret_cast<LPNMHDR>(lParam)->hwndFrom == GetToolBar())
				{
					// Pass Toolbar's custom draw up to CFrame
					return GetAncestor().SendMessage(WM_NOTIFY, wParam, lParam);
				}
			}
			break;
		} // switch LPNMHDR

		// The framework will call SetWindowLongPtr(DWLP_MSGRESULT, lr) for non-zero returns		
		return 0L;
	}

	inline void c_dock_container::c_view_page::PreRegisterClass(WNDCLASS& wc)
	{
		wc.lpszClassName = _T("Win32++ TabPage");
		wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	}

	inline void c_dock_container::c_view_page::RecalcLayout()
	{
		CRect rc = GetClientRect();
		if (GetToolBar().IsWindow())
		{
			GetToolBar().SendMessage(TB_AUTOSIZE, 0L, 0L);
			CRect rcToolBar = GetToolBar().GetClientRect();
			rc.top += rcToolBar.Height();
		}

		GetView().SetWindowPos(NULL, rc, SWP_SHOWWINDOW);
	}

	inline void c_dock_container::c_view_page::SetView(CWnd& wndView)
	// Sets or changes the View window displayed within the container
	{
		if (m_pView != &wndView)
		{
			// Hide the existing view window (if any)
			if (m_pView && m_pView->IsWindow())
				m_pView->ShowWindow(SW_HIDE);

			// Assign the view window
			m_pView = &wndView;

			if (IsWindow())
			{
				if (!GetView().IsWindow())
					GetView().Create(*this);
				else
				{
					GetView().SetParent(*this);
					GetView().ShowWindow();
				}

				// The new view must not be a dockcontainer
				assert(GetView().SendMessage(UWM_GETCDOCKCONTAINER) == 0);

				RecalcLayout();
			}
		}
	}


	inline LRESULT c_dock_container::c_view_page::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_SIZE:
			RecalcLayout();
			break;
		}

		// pass unhandled messages on for default processing
		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

} // namespace Win32xx

#endif // _WIN32XX_DOCKING_H_

