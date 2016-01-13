
#ifndef CREDENTIAL_IMP_H_
#define CREDENTIAL_IMP_H_

#include "../SrcLib/CreProviderHelp/CreProviderHelp.h"

#include "dllmain.h"
#include "SMS_Field.h"

#include "CmdEventWnd.h"

class CCmdEventWnd;

//
#define SFI_ORG_USERNAME_ID 2
#define SFI_ORG_USERPASS_ID 3

//
extern HRESULT XCreProvider_CreateInstance(REFIID riid,void** ppv);
extern HRESULT XCreFilter_CreateInstance(REFIID riid,void** ppv);


/*
	CXFilter ICredentialProviderFilter.
*/
class CXFilter : public ICredentialProviderFilter
{
public:	
	// IUnknown
	STDMETHOD_(ULONG, AddRef)()
	{
		return InterlockedIncrement(&m_cRef);
	}

	STDMETHOD_(ULONG, Release)()
	{
		LONG cRef = InterlockedDecrement(&m_cRef);
		if (!cRef)
		{
			delete this;
		}

		return cRef;
	}

	STDMETHOD (QueryInterface)(REFIID riid, void** ppv)
	{
		HRESULT hr;
		if (IID_IUnknown == riid ||
			IID_ICredentialProviderFilter == riid)		
		{
			*ppv = this;
			reinterpret_cast<IUnknown*>(*ppv)->AddRef();
			hr = S_OK;
		}
		else
		{
			*ppv = NULL;
			hr = E_NOINTERFACE;
		}

		return hr;
	}

public:
	// 
	friend	HRESULT XCreFilter_CreateInstance(REFIID riid, void** ppv);

	//Implementation of ICredentialProviderFilter
	IFACEMETHODIMP Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD
		dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders);
	IFACEMETHODIMP UpdateRemoteCredential(const
		CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn,
		CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsOut);

public:
	CXFilter();
	~CXFilter();

private:
	LONG m_cRef;
};

/*
	CXCreCredential µœ÷ICredentialProviderCredential.
*/
#define USER_NAME_ID 2
#define USER_PASSS_WORD_ID 3
#define SUBMIT_BUTTON_ID 12

class CXCreCredential : public ICredentialProviderCredential
{
public:
	CXCreCredential();
	~CXCreCredential();

public:
	// IUnknown
	STDMETHOD_(ULONG, AddRef)()
	{
		return InterlockedIncrement(&m_cRef);
	}

	STDMETHOD_(ULONG, Release)()
	{
		LONG cRef = InterlockedDecrement(&m_cRef);
		if (!cRef)
		{
			delete this;
		}

		return cRef;
	}

	STDMETHOD (QueryInterface)(REFIID riid, void** ppv)
	{
		HRESULT hr;
		if (ppv != NULL)
		{
			if (IID_IUnknown == riid ||
				IID_ICredentialProviderCredential == riid ||
				IID_ICredentialProviderFilter == riid)
			{
				*ppv = this;
				reinterpret_cast<IUnknown*>(*ppv)->AddRef();
				hr = S_OK;
			}
			else
			{
				*ppv = NULL;
				hr = E_NOINTERFACE;
			}
		}
		else
		{
			hr = E_INVALIDARG;
		}

		return hr;
	}

public:	
	//ICredentialProviderCredential
	IFACEMETHODIMP Advise(ICredentialProviderCredentialEvents* pcpce);
	IFACEMETHODIMP UnAdvise();

	IFACEMETHODIMP SetSelected(BOOL* pbAutoLogon);
	IFACEMETHODIMP SetDeselected();

	IFACEMETHODIMP GetFieldState(DWORD dwFieldID,
		CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
		CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis);

	IFACEMETHODIMP GetStringValue(DWORD dwFieldID, PWSTR* ppwz);
	IFACEMETHODIMP GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp);
	IFACEMETHODIMP GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, PWSTR* ppwzLabel);
	IFACEMETHODIMP GetComboBoxValueCount(DWORD dwFieldID, DWORD* pcItems, DWORD* pdwSelectedItem);
	IFACEMETHODIMP GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, PWSTR* ppwzItem);
	IFACEMETHODIMP GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo);

	IFACEMETHODIMP SetStringValue(DWORD dwFieldID, PCWSTR pwz);
	IFACEMETHODIMP SetCheckboxValue(DWORD dwFieldID, BOOL bChecked);
	IFACEMETHODIMP SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem);
	IFACEMETHODIMP CommandLinkClicked(DWORD dwFieldID);

	IFACEMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, 
		CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
		PWSTR* ppwzOptionalStatusText, 
		CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);
	IFACEMETHODIMP ReportResult(NTSTATUS ntsStatus, 
		NTSTATUS ntsSubstatus,
		PWSTR* ppwzOptionalStatusText, 
		CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);

public:
	HRESULT Initialize( 
		__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
		__in ICredentialProviderCredential *pWrappedCredential,
		__in DWORD dwWrappedDescriptorCount,
		__in CCmdEventWnd * pEventWin);

	DWORD GetDescriptorCountOrg();

public:
	static DWORD WINAPI WorkThread(LPVOID lpParam);

private:
	BOOL                                  IsFieldInWrappedCredential(__in DWORD dwFieldID);
	FIELD_STATE_PAIR                     *LookupLocalFieldStatePair(__in DWORD dwFieldID);
	void                                  CleanupEvents(); 

	LONG								 m_cRef;
	
	HWND								 m_hParentWnd;
	CREDENTIAL_PROVIDER_USAGE_SCENARIO   m_cpus;	

	ICredentialProviderCredential        * m_pWrappedCredential;  //Our wrapped credential.
	DWORD                                  m_dwWrappedDescriptorCount;  //The number of fields in our wrapped credential.

	DWORD m_dwSMSGetTypeIndex;

	//check box
	BOOL m_bCheckPass;

	HANDLE m_hWatchThread;

public:
	ICredentialProviderCredentialEvents* m_pCredProvCredentialEvents;

	CCmdEventWnd * m_pEventWin;
};

/*
	CXCreProvider ICredentialProvider
*/
class CXCreProvider : public ICredentialProvider
{
public:
	// IUnknown
	STDMETHOD_(ULONG, AddRef)()
	{
		return InterlockedIncrement(&m_cRef);
	}

	STDMETHOD_(ULONG, Release)()
	{
		LONG cRef = InterlockedDecrement(&m_cRef);
		if (!cRef)
		{
			delete this;
		}

		return cRef;
	}

	STDMETHOD (QueryInterface)(REFIID riid, void** ppv)
	{
		HRESULT hr;

		if (IID_IUnknown == riid || 
			IID_ICredentialProvider == riid)
		{
			*ppv = static_cast<IUnknown*>(this);
			reinterpret_cast<IUnknown*>(*ppv)->AddRef();

			hr = S_OK;
		}
		else
		{
			*ppv = NULL;
			hr = E_NOINTERFACE;
		}	

		return hr;
	}

public:
	IFACEMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags);
	IFACEMETHODIMP SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

	IFACEMETHODIMP Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext);
	IFACEMETHODIMP UnAdvise();

	IFACEMETHODIMP GetFieldDescriptorCount(DWORD* pdwCount);
	IFACEMETHODIMP GetFieldDescriptorAt(DWORD dwIndex,  CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);

	IFACEMETHODIMP GetCredentialCount(DWORD* pdwCount,
		DWORD* pdwDefault,
		BOOL* pbAutoLogonWithDefault);
	IFACEMETHODIMP GetCredentialAt(DWORD dwIndex, 
		ICredentialProviderCredential** ppcpc);

	friend HRESULT XCreProvider_CreateInstance(REFIID riid, void** ppv);

public:
	CXCreProvider();
	__override ~CXCreProvider();

public:
    void OnConnectStatusChanged();

private:
	void CleanUpAllCredentials();	

	LONG                m_cRef;
	
	CXCreCredential   ** m_rgpExtendCredentials;          //												

	ICredentialProvider * m_pOrgProvider;         // Our wrapped provider.
	DWORD               m_dwCredentialCount;         // The number of credentials provided by our wrapped provider.
	DWORD               m_dwWrappedDescriptorCount;  // The number of fields on each tile of our wrapped provider's 
	// credentials.	

	CREDENTIAL_PROVIDER_USAGE_SCENARIO m_cpus;  //

	ICredentialProviderEvents   *m_pcpe;                    // Used to tell our owner to re-enumerate credentials.
	CCmdEventWnd  * m_pEventWin;       // Emulates external events.
	UINT_PTR      m_upAdviseContext;       // Used to tell our owner who we are when asking to 
                                           // re-enumerate credentials.
};

#endif