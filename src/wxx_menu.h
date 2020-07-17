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


////////////////////////////////////////////////////////
// wxx_menu.h
//  Declaration of the CMenu class

// Notes
//  1) Owner-drawn menus send the WM_MEASUREITEM and WM_DRAWITEM messages
//     to the window that owns the menu. To manage owner drawing for menus,
//     handle these two messages in the CWnd's WndProc function.
//
//  2) CMenu objects are reference counted and can be safely copied. They can
//     be returned by value from functions and passed by value in function 
//     arguments.
//
//  3) In those functions that use a MENUITEMINFO structure, its cbSize member
//     is automatically set to the correct value.

//  Program sample
//  --------------
//	void CView::CreatePopup()
//	{
// 		CPoint pt = GetCursorPos();
//
// 		// Create the menu
// 		CMenu Popup;
// 		Popup.CreatePopupMenu();
//
// 		// Add some menu items
// 		Popup.AppendMenu(MF_STRING, 101, _T("Menu Item &1"));
// 		Popup.AppendMenu(MF_STRING, 102, _T("Menu Item &2"));
// 		Popup.AppendMenu(MF_STRING, 103, _T("Menu Item &3"));
// 		Popup.AppendMenu(MF_SEPARATOR);
// 		Popup.AppendMenu(MF_STRING, 104, _T("Menu Item &4"));
//
// 		// Set menu item states
// 		Popup.CheckMenuRadioItem(101, 101, 101, MF_BYCOMMAND);
// 		Popup.CheckMenuItem(102, MF_BYCOMMAND | MF_CHECKED);
// 		Popup.EnableMenuItem(103, MF_BYCOMMAND | MF_GRAYED);
// 		Popup.SetDefaultItem(104);
//
// 		// Display the popup menu
// 		Popup.TrackPopupMenu(0, pt.x, pt.y, this);
//	}


#if !defined (_WIN32XX_MENU_H_) && !defined(_WIN32_WCE)
#define _WIN32XX_MENU_H_


#include "wxx_appcore0.h"
#include "wxx_cstring.h"


namespace Win32xx
{

	class c_menu
	{

	public:
		//Construction
		c_menu();
		c_menu(UINT nID);
		c_menu(HMENU hMenu);
		c_menu(const c_menu& rhs);
		c_menu& operator = (const c_menu& rhs);
		void operator = (const HMENU hImageList);
		~c_menu();

		//Initialization
		void Attach(HMENU hMenu);
		void create_menu() const;
		void create_popup_menu() const;
		void destroy_menu();
		HMENU Detach();

		HMENU get_handle() const;
		BOOL LoadMenu(LPCTSTR lpszResourceName) const;
		BOOL LoadMenu(UINT uIDResource) const;
		BOOL LoadMenuIndirect(const LPMENUTEMPLATE lpMenuTemplate) const;

		//Menu Operations
		BOOL track_popup_menu(UINT uFlags, int x, int y, HWND hWnd, LPCRECT lpRect = 0) const;
		BOOL track_popup_menu_ex(UINT uFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm) const;

		//Menu Item Operations
		BOOL AppendMenu(UINT uFlags, UINT_PTR uIDNewItem = 0, LPCTSTR lpszNewItem = NULL);
		BOOL AppendMenu(UINT uFlags, UINT_PTR uIDNewItem, HBITMAP hBitmap);
		UINT CheckMenuItem(UINT uIDCheckItem, UINT uCheck) const;
		BOOL CheckMenuRadioItem(UINT uIDFirst, UINT uIDLast, UINT uIDItem, UINT uFlags) const;
		BOOL delete_menu(UINT uPosition, UINT uFlags) const;
		UINT EnableMenuItem(UINT uIDEnableItem, UINT uEnable) const;
		int FindMenuItem(LPCTSTR szMenuString) const;
		UINT get_default_item(UINT gmdiFlags, BOOL ByPosition = FALSE) const;
		DWORD get_menu_context_help_id() const;

#if(WINVER >= 0x0500)	// Minimum OS required is Win2000
		BOOL GetMenuInfo(MENUINFO& mi) const;
		BOOL SetMenuInfo(const MENUINFO& mi) const;
#endif

		UINT get_menu_item_count() const;
		UINT get_menu_item_id(int nPos) const;
		BOOL GetMenuItemInfo(UINT uItem, MENUITEMINFO& MenuItemInfo, BOOL ByPosition = FALSE) const;
		UINT GetMenuState(UINT uID, UINT uFlags) const;
		int GetMenuString(UINT uIDItem, LPTSTR lpString, int nMaxCount, UINT uFlags) const;
		int GetMenuString(UINT uIDItem, c_string& rString, UINT uFlags) const;
		c_menu get_sub_menu(int nPos) const;
		BOOL InsertMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem = 0, LPCTSTR lpszNewItem = NULL) const;
		BOOL InsertMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, HBITMAP hBitmap) const;
		BOOL InsertMenuItem(UINT uItem, MENUITEMINFO& MenuItemInfo, BOOL ByPosition = FALSE) const;
		BOOL insert_popup_menu(UINT uPosition, UINT uFlags, HMENU hPopupMenu, LPCTSTR lpszNewItem) const;
		BOOL ModifyMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem = 0, LPCTSTR lpszNewItem = NULL) const;
		BOOL ModifyMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, HBITMAP hBitmap) const;
		BOOL RemoveMenu(UINT uPosition, UINT uFlags) const;
		BOOL set_default_item(UINT uItem, BOOL ByPosition = FALSE) const;
		BOOL set_menu_context_help_id(DWORD dwContextHelpId) const;
		BOOL set_menu_item_bitmaps(UINT uPosition, UINT uFlags, HBITMAP hbmUnchecked, HBITMAP hbmChecked) const;
		BOOL SetMenuItemInfo(UINT uItem, MENUITEMINFO& MenuItemInfo, BOOL ByPosition = FALSE) const;

		//Operators
		operator HMENU () const;

	private:
		void add_to_map() const;
		void Release();
		BOOL remove_from_map() const;
		CMenu_Data* m_pData;

	};

} // namespace Win32xx

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{
	////////////////////////////////////////
	// Global function
	//
	inline UINT GetSizeofMenuItemInfo()
	{
		// For Win95 and NT, cbSize needs to be 44
		if (1400 == (GetWinVersion()) || (2400 == GetWinVersion()))
			return CCSIZEOF_STRUCT(MENUITEMINFO, cch);

		return sizeof(MENUITEMINFO);
	}

	////////////////////////////////////////
	// Definitions of CMenu
	//
	inline c_menu::c_menu()
	{
		m_pData = new CMenu_Data;
	}

	inline c_menu::c_menu(UINT nID)
	{
		m_pData = new CMenu_Data;

		const auto menu = ::LoadMenu(GetApp().GetResourceHandle(), MAKEINTRESOURCE(nID));
		Attach(menu);
		m_pData->IsManagedMenu = TRUE;
	}

	inline c_menu::c_menu(HMENU hMenu)
	{
		m_pData = new CMenu_Data;
		Attach(hMenu);
	}

	inline c_menu::c_menu(const c_menu& rhs)
	// Note: A copy of a CMenu is a clone of the original.
	//       Both objects manipulate the one HMENU.
	{
		m_pData = rhs.m_pData;
		InterlockedIncrement(&m_pData->Count);
	}

	inline c_menu& c_menu::operator = (const c_menu& rhs)
	// Note: A copy of a CImageList is a clone of the original.
	{
		if (this != &rhs)
		{
			InterlockedIncrement(&rhs.m_pData->Count);
			Release();
			m_pData = rhs.m_pData;
		}

		return *this;
	}

	inline void c_menu::operator = (const HMENU hMenu)
	{
		Attach(hMenu);
	}

	inline c_menu::~c_menu()
	{
		Release();
	}

	inline void c_menu::add_to_map() const
	// Store the HMENU and CMenu pointer in the HMENU map
	{
		assert( &GetApp() );
		assert(m_pData);
		assert(m_pData->hMenu);

		GetApp().AddCMenuData(m_pData->hMenu, m_pData);
	}

	inline void c_menu::Release()
	{
		assert(m_pData);

		if (InterlockedDecrement(&m_pData->Count) == 0)
		{
			if (m_pData->hMenu != NULL)
			{
				if (m_pData->IsManagedMenu)
				{
					::DestroyMenu(m_pData->hMenu);
				}

				remove_from_map();
			}

			delete m_pData;
			m_pData = 0;
		}
	}

	inline BOOL c_menu::remove_from_map() const
	{
		auto success = FALSE;

		if ( &GetApp() )
		{
			// Allocate an iterator for our HMENU map
			std::map<HMENU, CMenu_Data*, CompareHMENU>::iterator m;

			CWinApp& App = GetApp();
			App.m_csMapLock.Lock();
			m = App.m_mapCMenuData.find(m_pData->hMenu);
			if (m != App.m_mapCMenuData.end())
			{
				// Erase the Menu pointer entry from the map
				App.m_mapCMenuData.erase(m);
				success = TRUE;
			}

			App.m_csMapLock.Release();
		}

		return success;
	}


	inline BOOL c_menu::AppendMenu(UINT uFlags, UINT_PTR uIDNewItem /*= 0*/, LPCTSTR lpszNewItem /*= NULL*/)
	// Appends a new item to the end of the specified menu bar, drop-down menu, submenu, or shortcut menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::AppendMenu(m_pData->hMenu, uFlags, uIDNewItem, lpszNewItem);
	}

	inline BOOL c_menu::AppendMenu(UINT uFlags, UINT_PTR uIDNewItem, HBITMAP hBitmap)
	// Appends a new item to the end of the specified menu bar, drop-down menu, submenu, or shortcut menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::AppendMenu(m_pData->hMenu, uFlags, uIDNewItem, (LPCTSTR)hBitmap);
	}

	inline void c_menu::Attach(HMENU hMenu)
	// Attaches an existing menu to this CMenu
	// The hMenu can be NULL
	{
		assert(m_pData);

		if (hMenu != m_pData->hMenu)
		{
			// Release any existing menu
			if (m_pData->hMenu != 0)
			{
				Release();
				m_pData = new CMenu_Data;
			}

			if (hMenu)
			{
				// Add the menu to this CMenu
				CMenu_Data* pCMenuData = GetApp().GetCMenuData(hMenu);
				if (pCMenuData)
				{
					delete m_pData;
					m_pData = pCMenuData;
					InterlockedIncrement(&m_pData->Count);
				}
				else
				{
					m_pData->hMenu = hMenu;
					add_to_map();
				}
			}
		}
	}

	inline UINT c_menu::CheckMenuItem(UINT uIDCheckItem, UINT uCheck) const
	// Sets the state of the specified menu item's check-mark attribute to either selected or clear.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::CheckMenuItem(m_pData->hMenu, uIDCheckItem, uCheck);
	}

	inline BOOL c_menu::CheckMenuRadioItem(UINT uIDFirst, UINT uIDLast, UINT uIDItem, UINT uFlags) const
	// Checks a specified menu item and makes it a radio item. At the same time, the function clears
	//  all other menu items in the associated group and clears the radio-item type flag for those items.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::CheckMenuRadioItem(m_pData->hMenu, uIDFirst, uIDLast, uIDItem, uFlags);
	}

	inline void c_menu::create_menu() const
	// Creates an empty menu.
	{
		assert(m_pData);
		assert(NULL == m_pData->hMenu);
		m_pData->hMenu = ::CreateMenu();
		add_to_map();
		m_pData->IsManagedMenu = TRUE;
	}

	inline void c_menu::create_popup_menu() const
	// Creates a drop-down menu, submenu, or shortcut menu. The menu is initially empty.
	{
		assert(m_pData);
		assert(NULL == m_pData->hMenu);
		m_pData->hMenu = ::CreatePopupMenu();
		add_to_map();
		m_pData->IsManagedMenu = TRUE;
	}

	inline BOOL c_menu::delete_menu(UINT uPosition, UINT uFlags) const
	// Deletes an item from the specified menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::DeleteMenu(m_pData->hMenu, uPosition, uFlags);
	}

	inline void c_menu::destroy_menu()
	// Destroys the menu and frees any memory that the menu occupies.
	{
		assert(m_pData);
		if (::IsMenu(m_pData->hMenu))
			::DestroyMenu( Detach() );
	}

	inline HMENU c_menu::Detach()
	// Detaches the HMENU from all CMenu objects.
	{
		assert(m_pData);
		const auto h_menu = m_pData->hMenu;
		remove_from_map();
		m_pData->hMenu = nullptr;
		m_pData->vSubMenus.clear();

		if (m_pData->Count > 0)
		{
			if (InterlockedDecrement(&m_pData->Count) == 0)
			{
				delete m_pData;
			}
		}

		m_pData = new CMenu_Data;

		return h_menu;
	}

	inline HMENU c_menu::get_handle() const
	// Returns the HMENU assigned to this CMenu
	{
		assert(m_pData);
		return m_pData->hMenu;
	}

	inline UINT c_menu::EnableMenuItem(UINT uIDEnableItem, UINT uEnable) const
	// Enables, disables, or grays the specified menu item.
	// The uEnable parameter must be a combination of either MF_BYCOMMAND or MF_BYPOSITION
	// and MF_ENABLED, MF_DISABLED, or MF_GRAYED.
	// MF_DISABLED and MF_GRAYED are different without XP themes, but the same with XP themes enabled.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::EnableMenuItem(m_pData->hMenu, uIDEnableItem, uEnable);
	}

	inline int c_menu::FindMenuItem(LPCTSTR szMenuString) const
	// Finds the position of a menu item with the specified string.
	// Returns -1 if the string is not found.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));

		c_string str;
		const int count = get_menu_item_count();
		for (auto i = 0; i < count; i++)
		{
			if (GetMenuString(i, str, MF_BYPOSITION))
			{
				if (str == szMenuString)
					return i;
			}
		}

		return -1;
	}

	inline UINT c_menu::get_default_item(UINT gmdiFlags, BOOL ByPosition /*= FALSE*/) const
	// Determines the default menu item.
	// The gmdiFlags parameter specifies how the function searches for menu items.
	// This parameter can be zero or more of the following values: GMDI_GOINTOPOPUPS; GMDI_USEDISABLED.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::GetMenuDefaultItem(m_pData->hMenu, ByPosition, gmdiFlags);
	}

	inline DWORD c_menu::get_menu_context_help_id() const
	// Retrieves the Help context identifier associated with the menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::GetMenuContextHelpId(m_pData->hMenu);
	}

#if(WINVER >= 0x0500)
// minimum OS required : Win2000

	inline BOOL CMenu::GetMenuInfo(MENUINFO& mi) const
	// Retrieves the menu information.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::GetMenuInfo(m_pData->hMenu, &mi);
	}

	inline BOOL CMenu::SetMenuInfo(const MENUINFO& mi) const
	// Sets the menu information from the specified MENUINFO structure.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::SetMenuInfo(m_pData->hMenu, &mi);
	}

#endif

	inline UINT c_menu::get_menu_item_count() const
	// Retrieves the number of menu items.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::GetMenuItemCount(m_pData->hMenu);
	}

	inline UINT c_menu::get_menu_item_id(int nPos) const
	// Retrieves the menu item identifier of a menu item located at the specified position
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::GetMenuItemID(m_pData->hMenu, nPos);
	}

	inline BOOL c_menu::GetMenuItemInfo(UINT uItem, MENUITEMINFO& MenuItemInfo, BOOL ByPosition /*= FALSE*/) const
	// retrieves information about the specified menu item.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		MenuItemInfo.cbSize = GetSizeofMenuItemInfo();
		return ::GetMenuItemInfo(m_pData->hMenu, uItem, ByPosition, &MenuItemInfo);
	}

	inline UINT c_menu::GetMenuState(UINT uID, UINT uFlags) const
	// Retrieves the menu flags associated with the specified menu item.
	// Possible values for uFlags are: MF_BYCOMMAND (default) or MF_BYPOSITION.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::GetMenuState(m_pData->hMenu, uID, uFlags);
	}

	inline int c_menu::GetMenuString(UINT uIDItem, LPTSTR lpString, int nMaxCount, UINT uFlags) const
	// Copies the text string of the specified menu item into the specified buffer.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		assert(lpString);
		return ::GetMenuString(m_pData->hMenu, uIDItem, lpString, nMaxCount, uFlags);
	}

	inline int c_menu::GetMenuString(UINT uIDItem, c_string& rString, UINT uFlags) const
	// Copies the text string of the specified menu item into the specified buffer.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		int n = ::GetMenuString(m_pData->hMenu, uIDItem, rString.GetBuffer(MAX_MENU_STRING), MAX_MENU_STRING, uFlags);
		rString.ReleaseBuffer();
		return n;
	}

	inline c_menu c_menu::get_sub_menu(int nPos) const
	// Retrieves the CMenu object of a pop-up menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		c_menu* pMenu = new c_menu;
		pMenu->m_pData->hMenu = ::GetSubMenu(m_pData->hMenu, nPos);
		pMenu->m_pData->IsManagedMenu = FALSE;
		m_pData->vSubMenus.push_back(pMenu);
		return *pMenu;
	}

	inline BOOL c_menu::InsertMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem /*= 0*/, LPCTSTR lpszNewItem /*= NULL*/) const
	// Inserts a new menu item into a menu, moving other items down the menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::InsertMenu(m_pData->hMenu, uPosition, uFlags, uIDNewItem, lpszNewItem);
	}

	inline BOOL c_menu::InsertMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, HBITMAP hBitmap) const
	// Inserts a new menu item into a menu, moving other items down the menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::InsertMenu(m_pData->hMenu, uPosition, uFlags, uIDNewItem, (LPCTSTR)hBitmap);
	}

	inline BOOL c_menu::InsertMenuItem(UINT uItem, MENUITEMINFO& MenuItemInfo, BOOL ByPosition /*= FALSE*/) const
	// Inserts a new menu item at the specified position in the menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		MenuItemInfo.cbSize = GetSizeofMenuItemInfo();
		return ::InsertMenuItem(m_pData->hMenu, uItem, ByPosition, &MenuItemInfo);
	}

	inline BOOL c_menu::insert_popup_menu(UINT uPosition, UINT uFlags, HMENU hPopupMenu, LPCTSTR lpszNewItem) const
	// Inserts a popup menu item at the specified position in the menu.
	{
		assert(hPopupMenu);
		assert(IsMenu(m_pData->hMenu));

		// Ensure the correct flags are set
		uFlags &= ~MF_BITMAP;
		uFlags &= ~MF_OWNERDRAW;
		uFlags |= MF_POPUP;

		return ::InsertMenu(m_pData->hMenu, uPosition, uFlags, (UINT_PTR)hPopupMenu, lpszNewItem);
	}

	inline BOOL c_menu::LoadMenu(LPCTSTR lpszResourceName) const
	// Loads the menu from the specified windows resource.
	{
		assert(m_pData);
		assert(NULL == m_pData->hMenu);
		assert(lpszResourceName);
		m_pData->hMenu = ::LoadMenu(GetApp().GetResourceHandle(), lpszResourceName);
		if (m_pData->hMenu != 0) add_to_map();
		return nullptr != m_pData->hMenu;
	}

	inline BOOL c_menu::LoadMenu(UINT uIDResource) const
	// Loads the menu from the specified windows resource.
	{
		assert(m_pData);
		assert(NULL == m_pData->hMenu);
		m_pData->hMenu = ::LoadMenu(GetApp().GetResourceHandle(), MAKEINTRESOURCE(uIDResource));
		if (m_pData->hMenu != 0) add_to_map();
		return nullptr != m_pData->hMenu;
	}

	inline BOOL c_menu::LoadMenuIndirect(const LPMENUTEMPLATE lpMenuTemplate) const
	// Loads the specified menu template and assigns it to this CMenu.
	{
		assert(m_pData);
		assert(NULL == m_pData->hMenu);
		assert(lpMenuTemplate);
		m_pData->hMenu = ::LoadMenuIndirect(lpMenuTemplate);
		if (m_pData->hMenu) add_to_map();
		return nullptr != m_pData->hMenu;
	}

	inline BOOL c_menu::ModifyMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem /*= 0*/, LPCTSTR lpszNewItem /*= NULL*/) const
	// Changes an existing menu item. This function is used to specify the content, appearance, and behavior of the menu item.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::ModifyMenu(m_pData->hMenu, uPosition, uFlags, uIDNewItem, lpszNewItem);
	}

	inline BOOL c_menu::ModifyMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, HBITMAP hBitmap) const
	// Changes an existing menu item. This function is used to specify the content, appearance, and behavior of the menu item.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::ModifyMenu(m_pData->hMenu, uPosition, uFlags, uIDNewItem, (LPCTSTR)hBitmap);
	}

	inline BOOL c_menu::RemoveMenu(UINT uPosition, UINT uFlags) const
	// Deletes a menu item or detaches a submenu from the menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::RemoveMenu(m_pData->hMenu, uPosition, uFlags);
	}

	inline BOOL c_menu::set_default_item(UINT uItem, BOOL ByPosition /*= FALSE*/) const
	//  sets the default menu item for the menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::SetMenuDefaultItem(m_pData->hMenu, uItem, ByPosition);
	}

	inline BOOL c_menu::set_menu_context_help_id(DWORD dwContextHelpId) const
	// Associates a Help context identifier with the menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::SetMenuContextHelpId(m_pData->hMenu, dwContextHelpId);
	}

	inline BOOL c_menu::set_menu_item_bitmaps(UINT uPosition, UINT uFlags, HBITMAP hbmUnchecked, HBITMAP hbmChecked) const
	// Associates the specified bitmap with a menu item.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::SetMenuItemBitmaps(m_pData->hMenu, uPosition, uFlags, hbmUnchecked, hbmChecked);
	}

	inline BOOL c_menu::SetMenuItemInfo(UINT uItem, MENUITEMINFO& MenuItemInfo, BOOL ByPosition /*= FALSE*/) const
	// Changes information about a menu item.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		MenuItemInfo.cbSize = GetSizeofMenuItemInfo();
		return ::SetMenuItemInfo(m_pData->hMenu, uItem, ByPosition, &MenuItemInfo);
	}

	inline BOOL c_menu::track_popup_menu(UINT uFlags, int x, int y, HWND hWnd, LPCRECT lpRect /*= 0*/) const
	// Displays a shortcut menu at the specified location and tracks the selection of items on the menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::TrackPopupMenu(m_pData->hMenu, uFlags, x, y, 0, hWnd, lpRect);
	}

	inline BOOL c_menu::track_popup_menu_ex(UINT uFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm) const
	// Displays a shortcut menu at the specified location and tracks the selection of items on the shortcut menu.
	{
		assert(m_pData);
		assert(IsMenu(m_pData->hMenu));
		return ::TrackPopupMenuEx(m_pData->hMenu, uFlags, x, y, hWnd, lptpm);
	}

	inline c_menu::operator HMENU () const
	// Retrieves the menu's handle.
	{
		assert(m_pData);
		return m_pData->hMenu;
	}


}	// namespace Win32xx

#endif	// _WIN32XX_MENU_H_

