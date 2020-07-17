///////////////////////////////////////////////////////
// Output.h - Declaration of the CViewOutput, CContainOutput, 
//          and CDockOutput classes

#ifndef OUTPUT_H
#define OUTPUT_H


#include "resource.h"

// Declaration of the CViewText class
class CViewOutput : public CRichEdit
{
public:
	CViewOutput();
	virtual ~CViewOutput();

protected:
	virtual void OnAttach(); 
	virtual void PreCreate(CREATESTRUCT& cs);

};

// Declaration of the CContainText class
class CContainOutput : public c_dock_container
{
public:
	CContainOutput() 
	{
		SetView(m_ViewOutput); 
		SetDockCaption (_T("Output View - Docking container"));
		SetTabText(_T("Output"));
		SetTabIcon(IDI_TEXT);
	} 
	~CContainOutput() {}

private:
	CViewOutput m_ViewOutput;
};

// Declaration of the CDockText class
class c_dock_output : public c_docker
{
public:
	c_dock_output();
	virtual ~c_dock_output() {}

private:
	CContainOutput m_View;

};


#endif // OUTPUT_H

