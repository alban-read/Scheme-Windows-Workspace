 

#include "stdafx.h"
#include "EventSink.h"
#include "Mainfrm.h"

extern "C" {
	void appendTranscriptNL(const char *s);
}

CEventSink::CEventSink() : m_cRefs(1), m_pSink(nullptr)
{
}

STDMETHODIMP_(ULONG) CEventSink::AddRef()
{
	return ++m_cRefs;
}

STDMETHODIMP CEventSink::QueryInterface(REFIID riid, void** ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = static_cast<IDispatch *>(this);
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else
	{
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

STDMETHODIMP CEventSink::GetIDsOfNames(REFIID riid, OLECHAR** rgsz_names, unsigned int cNames, LCID lcid, DISPID* rgdispid)
{
	UNREFERENCED_PARAMETER(static_cast<IID>(riid));
	UNREFERENCED_PARAMETER(rgsz_names);
	UNREFERENCED_PARAMETER(cNames);
	UNREFERENCED_PARAMETER(lcid);
	*rgdispid = DISPID_UNKNOWN;
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP CEventSink::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo** pptinfo)
{
	UNREFERENCED_PARAMETER(itinfo);
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(pptinfo);
	return E_NOTIMPL;
}

STDMETHODIMP CEventSink::GetTypeInfoCount(unsigned int* pctinfo)
{
	UNREFERENCED_PARAMETER(pctinfo);
	return E_NOTIMPL;
}

STDMETHODIMP CEventSink::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags,
	DISPPARAMS* pDispParams, VARIANT* pvarResult, EXCEPINFO* pexecinfo, unsigned int* puArgErr)
{
	UNREFERENCED_PARAMETER(static_cast<IID>(riid));
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(wFlags);
	UNREFERENCED_PARAMETER(pvarResult);
	UNREFERENCED_PARAMETER(pexecinfo);
	UNREFERENCED_PARAMETER(puArgErr);

	if (!m_pSink)
		return S_OK;

	switch (dispid)
	{

	case DISPID_BEFORENAVIGATE2:
		//appendTranscriptNL("DISPID_BEFORENAVIGATE2");
		m_pSink->on_before_navigate2(pDispParams->rgvarg[5].pvarVal->bstrVal, pDispParams->rgvarg[0].pboolVal);
		break;

	case DISPID_COMMANDSTATECHANGE:
		//appendTranscriptNL("DISPID_COMMANDSTATECHANGE");
		m_pSink->on_command_state_change(pDispParams);
		break;

	case DISPID_DOCUMENTCOMPLETE:
		//appendTranscriptNL("DISPID_DOCUMENTCOMPLETE");
		m_pSink->on_document_complete(pDispParams);
		break;

	case DISPID_DOWNLOADBEGIN:
		//appendTranscriptNL("DISPID_DOWNLOADBEGIN");
		m_pSink->on_download_begin(pDispParams);
		break;

	case DISPID_DOWNLOADCOMPLETE:
		//appendTranscriptNL("DISPID_DOWNLOADCOMPLETE");
		m_pSink->on_download_complete(pDispParams);
		break;

	case DISPID_NAVIGATECOMPLETE2:
		//appendTranscriptNL("DISPID_NAVIGATECOMPLETE2");
		m_pSink->on_navigate_complete2(pDispParams);
		break;

	case DISPID_NEWWINDOW2:
		//appendTranscriptNL("DISPID_NEWWINDOW2");
		m_pSink->on_new_window2(pDispParams);
		break;

	case DISPID_PROGRESSCHANGE:
		//appendTranscriptNL("DISPID_PROGRESSCHANGE");
		m_pSink->on_progress_change(pDispParams);
		break;

	case DISPID_PROPERTYCHANGE:		
		//appendTranscriptNL("DISPID_PROPERTYCHANGE");
		m_pSink->on_property_change(pDispParams);
		break;

	case DISPID_STATUSTEXTCHANGE:
		//appendTranscriptNL("DISPID_STATUSTEXTCHANGE");
		m_pSink->on_status_text_change(pDispParams);
		break;

	case DISPID_TITLECHANGE:
		//appendTranscriptNL("DISPID_TITLECHANGE");
		m_pSink->on_title_change(pDispParams);
		break;
	}

	return S_OK;
}

STDMETHODIMP_(ULONG) CEventSink::Release()
{
	return --m_cRefs;
}

