//////////////////////////////////////////////
// ContainerApp.cpp


#include "stdafx.h"
#include "ContainerApp.h"


// Definitions for the CDockContainerApp class
CDockContainerApp::CDockContainerApp()
{
}

CDockContainerApp::~CDockContainerApp()
{
}

BOOL CDockContainerApp::InitInstance()
{
	//Create the Frame Window
	if (m_Frame.Create() == nullptr)	
	{
		// We get here if the Frame creation fails
		
		::MessageBox(nullptr, _T("Failed to create Frame window"), _T("ERROR"), MB_ICONERROR);
		return FALSE; // returning FALSE ends the application
	}

	return TRUE;
}






