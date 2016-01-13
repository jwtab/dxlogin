
#ifndef CREDENTIAL_V2_IMP_H_
#define CREDENTIAL_V2_IMP_H_

#include <credentialprovider.h>
#include <Shlwapi.h>
#include <propkey.h>

#include "dllmain.h"
#include "CmdEventWnd.h"

class CCmdEventWnd;

DEFINE_GUID(Identity_LocalUserProvider, 0xA198529B, 0x730F, 0x4089, 0xB6, 0x46, 0xA1, 0x25, 0x57, 0xF5, 0x66, 0x5E);

struct REPORT_RESULT_STATUS_INFO
{
	NTSTATUS ntsStatus;
	NTSTATUS ntsSubstatus;
	PWSTR     pwzMessage;
	CREDENTIAL_PROVIDER_STATUS_ICON cpsi;
};

#include "../SrcLib/CreProviderHelp/CreProviderHelp.h"

//友员函数
extern HRESULT XCreProviderV2_CreateInstance(REFIID riid,void** ppv);
extern HRESULT XCreFilterV2_CreateInstance(REFIID riid,void** ppv);


/*
	CXFilterV2过滤功能的ICredentialProviderFilter.
*/
class CXFilterV2 : public ICredentialProviderFilter
{
public:	
	//IUnknown
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
	friend	HRESULT XCreFilterV2_CreateInstance(REFIID riid, void** ppv);

	//Implementation of ICredentialProviderFilter
	IFACEMETHODIMP Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD
		dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders);
	IFACEMETHODIMP UpdateRemoteCredential(const
		CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn,
		CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsOut);

public:
	CXFilterV2();
	~CXFilterV2();

private:
	LONG m_cRef;
};

/*
	CXCreCredential实现ICredentialProviderCredential  ICredentialProviderCredentialWithFieldOptions.
*/
class CXCreCredentialV2 : public ICredentialProviderCredential2, ICredentialProviderCredentialWithFieldOptions
{
public:
	CXCreCredentialV2();
	virtual ~CXCreCredentialV2();

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
		static const QITAB qit[] =
		{			
			QITABENT(CXCreCredentialV2, ICredentialProviderCredential), // IID_ICredentialProviderCredential
			QITABENT(CXCreCredentialV2, ICredentialProviderCredential2), // IID_ICredentialProviderCredential2
			QITABENT(CXCreCredentialV2, ICredentialProviderCredentialWithFieldOptions), //IID_ICredentialProviderCredentialWithFieldOptions
			{0},
		};
		
		return QISearch(this, qit, riid, ppv);
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

	// ICredentialProviderCredential2
	IFACEMETHODIMP GetUserSid(_Outptr_result_nullonfailure_ PWSTR *ppszSid);

	// ICredentialProviderCredentialWithFieldOptions
	IFACEMETHODIMP GetFieldOptions(DWORD dwFieldID,
		_Out_ CREDENTIAL_PROVIDER_CREDENTIAL_FIELD_OPTIONS *pcpcfo);

public:
	HRESULT Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,	_In_ ICredentialProviderUser *pcpUser,CCmdEventWnd * pEventWin);

private:
	long                                    m_cRef;
	CREDENTIAL_PROVIDER_USAGE_SCENARIO      m_cpus;                                          // The usage scenario for which we were enumerated.
	
	PWSTR                                   m_rgFieldStrings[SFI_MAX_FIELDS];                // An array holding the string value of each field. This is different from the name of the field held in _rgCredProvFieldDescriptors.
	PWSTR                                   m_pszUserSid;
	PWSTR                                   m_pszQualifiedUserName;                          // The user name that's used to pack the authentication buffer
	ICredentialProviderCredentialEvents2*    m_pCredProvCredentialEvents;                    // Used to update fields.
	// CredentialEvents2 for Begin and EndFieldUpdates.

	HWND                                    m_hParentWnd;

	///////////////////////////////////////////////
	BOOL                                    m_fChecked;                                      // Tracks the state of our checkbox.
	DWORD                                   m_dwComboIndex;                                  // Tracks the current index of our combobox.
	BOOL                                    m_fShowControls;                                 // Tracks the state of our show/hide controls link.
	BOOL                                    m_fIsLocalUser;                                  // If the cred prov is assosiating with a local user tile

	CCmdEventWnd * m_pEventWin;
};

/*
	CXCreProviderV2实现ICredentialProvider ICredentialProviderSetUserArray.
*/
class CXCreProviderV2 : public ICredentialProvider,
						public ICredentialProviderSetUserArray
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
		static const QITAB qit[] =
		{
			QITABENT(CXCreProviderV2, ICredentialProvider), // IID_ICredentialProvider
			QITABENT(CXCreProviderV2, ICredentialProviderSetUserArray), // IID_ICredentialProviderSetUserArray
			{0},
		};

		return QISearch(this, qit, riid, ppv);
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

	//ICredentialProviderSetUserArray
	IFACEMETHODIMP SetUserArray(_In_ ICredentialProviderUserArray *users);

	friend HRESULT XCreProviderV2_CreateInstance(REFIID riid, void** ppv);

public:
	CXCreProviderV2();
	__override ~CXCreProviderV2();

	void ReleaseEnumeratedCredentials();
	void CreateEnumeratedCredentials();	
	HRESULT EnumerateCredentials();	

	void OnConnectStatusChanged();

private:
	long                                    m_cRef;            // Used for reference counting.

	DWORD                                     m_dwCredentialCount;
	CXCreCredentialV2                       **m_pCredential;    //Credential V2
	BOOL                                    m_fRecreateEnumeratedCredentials;
	CREDENTIAL_PROVIDER_USAGE_SCENARIO      m_cpus;
	ICredentialProviderUserArray            *m_pCredProviderUserArray;

	//event window
	ICredentialProviderEvents   *m_pcpe;                    // Used to tell our owner to re-enumerate credentials.
	CCmdEventWnd  * m_pEventWin;       // Emulates external events.
	UINT_PTR      m_upAdviseContext;       // Used to tell our owner who we are when asking to 
                                           // re-enumerate credentials.
};

#endif