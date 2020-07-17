//////////////////////////////////////////////////////////
// Classes.h - Declaration of CViewClasses, CContainClasses,
//              and CDockClasses classes


#ifndef CLASSES_H
#define CLASSES_H


#include "MyCombo.h"


// Declaration of the CViewClasses class
class c_view_classes : public c_tree_view
{
public:
	c_view_classes();
	virtual ~c_view_classes();
	virtual HTREEITEM AddItem(HTREEITEM hParent, LPCTSTR sxText, int iImage);
	
protected:
	virtual void OnAttach();
	virtual void OnDestroy();
	virtual LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void PreCreate(CREATESTRUCT& cs);
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	CImageList m_imlNormal;
	
};

// Declaration of the CContainClasses class
class CContainClasses : public c_dock_container
{
public:
	CContainClasses();
	virtual ~CContainClasses() {}
	virtual void AddCombo();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnFileNew();
	virtual BOOL OnHelpAbout();
	virtual void SetupToolBar();

private:
	c_view_classes m_ViewClasses;
	CMyCombo m_ComboBoxEx;
};

// Declaration of the CDockClasses class
class CDockClasses : public c_docker
{
public:
	CDockClasses(); 
	virtual ~CDockClasses() {}

private:
	CContainClasses m_Classes;

};



#endif  //CLASSES_H
