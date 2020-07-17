////////////////////////////////////////////
// Simple.h - Declaration of the CViewSimple 
//            and CDockSimple classes

#ifndef SIMPLE_H
#define SIMPLE_H


enum DockIDs
{
	ID_DOCK_CLASSES1 = 1,
	ID_DOCK_CLASSES2 = 2,
	ID_DOCK_FILES1 = 3,
	ID_DOCK_FILES2 = 4,
	ID_DOCK_TEXT1 = 5,
	ID_DOCK_TEXT2 = 7,
	ID_DOCK_TEXT3 = 8,
	ID_DOCK_GRAFF1 = 9,
	ID_DOCK_GRAFF2 = 10,
	ID_DOCK_BRWSR1 = 20
};

// Declaration of the CViewSimple class
class CViewSimple : public CWnd
{
public:
	CViewSimple() {}
	virtual ~CViewSimple() {}

protected:
	virtual void OnDraw(CDC& dc);
	virtual LRESULT on_mouse_activate(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT on_size(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


#endif // SIMPLE_H
