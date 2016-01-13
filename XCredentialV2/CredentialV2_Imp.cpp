
#include "stdafx.h"
#include "CredentialV2_Imp.h"

/*
	CXFilterV2
*/

HRESULT XCreFilterV2_CreateInstance(REFIID riid, void** ppv)
{
	HRESULT hr;
	
	CXFilterV2 * pCreFilter = new CXFilterV2();
	if (NULL != pCreFilter)
	{
		hr = pCreFilter->QueryInterface(riid,ppv);
		pCreFilter->Release();
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

CXFilterV2::CXFilterV2():
	m_cRef(1)
{
	AddRef();
}

CXFilterV2::~CXFilterV2()
{
	Release();
}

HRESULT CXFilterV2::Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders)
{
	HRESULT hr = S_OK;
	
	for (int i = 0; i < (int)cProviders; i++)
	{
		wchar_t wszGUID[1024] = {0};
		swprintf_s(wszGUID,1024,L"{%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}",rgclsidProviders[i].Data1,rgclsidProviders[i].Data2,rgclsidProviders[i].Data3, 
			rgclsidProviders[i].Data4[0], rgclsidProviders[i].Data4[1], rgclsidProviders[i].Data4[2], rgclsidProviders[i].Data4[3],  
			rgclsidProviders[i].Data4[4], rgclsidProviders[i].Data4[5], rgclsidProviders[i].Data4[6], rgclsidProviders[i].Data4[7]);
		OutputDebugString(wszGUID);
	}
	
	switch (cpus)
	{
	case CPUS_LOGON:
		{
			for (int i = 0; i < (int)cProviders; i++)
			{			
				if (IsEqualGUID(rgclsidProviders[i],CLSID_PasswordCredentialProvider) ||
					IsEqualGUID(rgclsidProviders[i],CLSID_OnexCredentialProvider) ||
					IsEqualGUID(rgclsidProviders[i],CLSID_SmartcardCredentialProvider))
				{
					rgbAllow[i] = FALSE;
				}
			}

			break;
		}

	case CPUS_UNLOCK_WORKSTATION:
		{
			for (int i = 0; i < (int)cProviders; i++)
			{			
				if (IsEqualGUID(rgclsidProviders[i],CLSID_PasswordCredentialProvider) ||
					IsEqualGUID(rgclsidProviders[i],CLSID_OnexCredentialProvider) ||
					IsEqualGUID(rgclsidProviders[i],CLSID_SmartcardCredentialProvider))
				{
					rgbAllow[i] = FALSE;
				}
			}

			break;
		}

	case CPUS_CHANGE_PASSWORD:
		{
			break;
		}

	case CPUS_CREDUI:
		{
			break;
		}

	case CPUS_PLAP:
		{			
			for (int i = 0; i < (int)cProviders; i++)
			{			
				if (IsEqualGUID(rgclsidProviders[i],CLSID_PasswordCredentialProvider) ||
					IsEqualGUID(rgclsidProviders[i],CLSID_OnexCredentialProvider) ||
					IsEqualGUID(rgclsidProviders[i],CLSID_SmartcardCredentialProvider))
				{
					rgbAllow[i] = FALSE;
				}
			}

			break;
		}

	default:
		{
			break;
		}
	}

	return hr;
}

//Used for remote sessions, not implemented here
HRESULT CXFilterV2::UpdateRemoteCredential(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsOut)
{
	UNREFERENCED_PARAMETER(pcpcsIn);
	UNREFERENCED_PARAMETER(pcpcsOut);

	return E_NOTIMPL;
}


/*
	CXCreProviderV2
*/
HRESULT XCreProviderV2_CreateInstance(REFIID riid, void** ppv)
{
	HRESULT hr;	

	CXCreProviderV2* pProvider = new CXCreProviderV2();
	if (NULL != pProvider)
	{
		hr = pProvider->QueryInterface(riid, ppv);
		pProvider->Release();
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

//CXCreProviderV2
CXCreProviderV2::CXCreProviderV2():
	m_cRef(1),
	m_pCredential(NULL),
	m_pCredProviderUserArray(NULL)
{
	AddRef();

	m_dwCredentialCount = 0;

	m_pcpe = NULL;
	m_pEventWin = NULL;
}

CXCreProviderV2::~CXCreProviderV2()
{
	ReleaseEnumeratedCredentials();

	if (m_pCredProviderUserArray != NULL)
	{
		m_pCredProviderUserArray->Release();
		m_pCredProviderUserArray = NULL;
	}

	if(NULL != m_pEventWin)
	{
		delete m_pEventWin;		
		m_pEventWin = NULL;
	}

	Release();
}

// This method acts as a callback for the hardware emulator. When it's called, it simply
// tells the infrastructure that it needs to re-enumerate the credentials.
void CXCreProviderV2::OnConnectStatusChanged()
{
    if (m_pcpe != NULL)
    {
		OutputDebugString(L"OnConnectStatusChanged()");
        m_pcpe->CredentialsChanged(m_upAdviseContext);
    }
}

// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.
HRESULT CXCreProviderV2::SetUsageScenario(
	CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
	DWORD dwFlags
	)
{
	HRESULT hr;

	// Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
	// that we're not designed for that scenario.
	switch (cpus)
	{
	case CPUS_LOGON:
	case CPUS_UNLOCK_WORKSTATION:
		// The reason why we need _fRecreateEnumeratedCredentials is because ICredentialProviderSetUserArray::SetUserArray() is called after ICredentialProvider::SetUsageScenario(),
		// while we need the ICredentialProviderUserArray during enumeration in ICredentialProvider::GetCredentialCount()
		m_cpus = cpus;
		m_fRecreateEnumeratedCredentials = true;
		hr = S_OK;

		//增加窗口
		m_pEventWin = new CCmdEventWnd();
		if(NULL != m_pEventWin)
		{
			m_pEventWin->Initialize(this);
		}

		break;

	case CPUS_CHANGE_PASSWORD:
	case CPUS_CREDUI:
		hr = E_NOTIMPL;
		break;

	default:
		hr = E_INVALIDARG;
		break;
	}

	return hr;
}

// SetSerialization takes the kind of buffer that you would normally return to LogonUI for
// an authentication attempt.  It's the opposite of ICredentialProviderCredential::GetSerialization.
// GetSerialization is implement by a credential and serializes that credential.  Instead,
// SetSerialization takes the serialization and uses it to create a tile.
//
// SetSerialization is called for two main scenarios.  The first scenario is in the credui case
// where it is prepopulating a tile with credentials that the user chose to store in the OS.
// The second situation is in a remote logon case where the remote client may wish to
// prepopulate a tile with a username, or in some cases, completely populate the tile and
// use it to logon without showing any UI.
//
// If you wish to see an example of SetSerialization, please see either the SampleCredentialProvider
// sample or the SampleCredUICredentialProvider sample.  [The logonUI team says, "The original sample that
// this was built on top of didn't have SetSerialization.  And when we decided SetSerialization was
// important enough to have in the sample, it ended up being a non-trivial amount of work to integrate
// it into the main sample.  We felt it was more important to get these samples out to you quickly than to
// hold them in order to do the work to integrate the SetSerialization changes from SampleCredentialProvider
// into this sample.]
STDMETHODIMP CXCreProviderV2::SetSerialization(
	const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs
	)
{	
	return E_NOTIMPL;
}

// Called by LogonUI to give you a callback.  Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated.
HRESULT CXCreProviderV2::Advise(
	ICredentialProviderEvents* pcpe,
	UINT_PTR upAdviseContext
	)
{
	m_pcpe = pcpe;
	m_pcpe->AddRef();

	m_upAdviseContext = upAdviseContext;

	return E_NOTIMPL;
}

//Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
HRESULT CXCreProviderV2::UnAdvise()
{
	if (m_pcpe != NULL)
    {
        m_pcpe->Release();
        m_pcpe = NULL;
    }
	
	return E_NOTIMPL;
}

// Called by LogonUI to determine the number of fields in your tiles.  This
// does mean that all your tiles must have the same number of fields.
// This number must include both visible and invisible fields. If you want a tile
// to have different fields from the other tiles you enumerate for a given usage
// scenario you must include them all in this count and then hide/show them as desired
// using the field descriptors.
HRESULT CXCreProviderV2::GetFieldDescriptorCount(
	DWORD* pdwCount
	)
{
	*pdwCount = SFI_MAX_FIELDS;
	return S_OK;
}

// Gets the field descriptor for a particular field.
HRESULT CXCreProviderV2::GetFieldDescriptorAt(
	DWORD dwIndex, 
	CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
	)
{    
	HRESULT hr;
	*ppcpfd = NULL;

	// Verify dwIndex is a valid field.
	if ((dwIndex < SFI_MAX_FIELDS) && ppcpfd)
	{
		hr = FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptors[dwIndex], ppcpfd);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets pdwCount to the number of tiles that we wish to show at this time.
// Sets pdwDefault to the index of the tile which should be used as the default.
// The default tile is the tile which will be shown in the zoomed view by default. If
// more than one provider specifies a default the last used cred prov gets to pick
// the default. If *pbAutoLogonWithDefault is TRUE, LogonUI will immediately call
// GetSerialization on the credential you've specified as the default and will submit
// that credential for authentication without showing any further UI.
HRESULT CXCreProviderV2::GetCredentialCount(
	DWORD* pdwCount,
	DWORD* pdwDefault,
	BOOL* pbAutoLogonWithDefault
	)
{	
	*pdwDefault = CREDENTIAL_PROVIDER_NO_DEFAULT;
	*pbAutoLogonWithDefault = FALSE;

	if (m_fRecreateEnumeratedCredentials)
	{
		m_fRecreateEnumeratedCredentials = FALSE;
		ReleaseEnumeratedCredentials();
		CreateEnumeratedCredentials();
	}

	*pdwCount = m_dwCredentialCount;

	//是否可以自动连接.
	if(m_pEventWin->GetConnectedStatus())
	{
		*pdwDefault = 0;
	}

	return S_OK;
}

// Returns the credential at the index specified by dwIndex. This function is called by logonUI to enumerate
// the tiles.
HRESULT CXCreProviderV2::GetCredentialAt(
	DWORD dwIndex, 
	ICredentialProviderCredential** ppcpc
	)
{
	HRESULT hr = E_INVALIDARG;
	*ppcpc = NULL;

	if ((dwIndex < m_dwCredentialCount) && ppcpc)
	{
		hr = m_pCredential[dwIndex]->QueryInterface(IID_PPV_ARGS(ppcpc));
	}

	return hr;
}

// This function will be called by LogonUI after SetUsageScenario succeeds.
// Sets the User Array with the list of users to be enumerated on the logon screen.
HRESULT CXCreProviderV2::SetUserArray(_In_ ICredentialProviderUserArray *users)
{
	if (m_pCredProviderUserArray)
	{
		m_pCredProviderUserArray->Release();
	}

	m_pCredProviderUserArray = users;
	m_pCredProviderUserArray->AddRef();

	return S_OK;
}


void CXCreProviderV2::CreateEnumeratedCredentials()
{
	switch (m_cpus)
	{
	case CPUS_LOGON:
	case CPUS_UNLOCK_WORKSTATION:
		{
			EnumerateCredentials();
			break;
		}
	default:
		break;
	}
}

void CXCreProviderV2::ReleaseEnumeratedCredentials()
{
	if (m_pCredential != NULL)
	{
		for (int i = 0; i < m_dwCredentialCount; i++)
		{
			m_pCredential[i]->Release();
			m_pCredential[i] = NULL;
		}

		delete []m_pCredential;
		m_pCredential = NULL;
	}
}

HRESULT CXCreProviderV2::EnumerateCredentials()
{
	HRESULT hr = E_UNEXPECTED;
	if (NULL != m_pCredProviderUserArray)
	{
		m_pCredProviderUserArray->GetCount(&m_dwCredentialCount);
		if (m_dwCredentialCount > 0)
		{
			m_pCredential = new CXCreCredentialV2*[m_dwCredentialCount];
			for (int index = 0; index < m_dwCredentialCount; index++)
			{
				ICredentialProviderUser *pCredUser = NULL;
				m_pCredential[index] = new CXCreCredentialV2();
				hr = m_pCredProviderUserArray->GetAt(index,&pCredUser);
				if (SUCCEEDED(hr))
				{					
					if (m_pCredential[index] != NULL)
					{
						hr = m_pCredential[index]->Initialize(m_cpus,pCredUser,m_pEventWin);
						if (FAILED(hr))
						{
							m_pCredential[index]->Release();
							m_pCredential[index] = NULL;
						}
					}
					else
					{
						hr = E_OUTOFMEMORY;
					}

					pCredUser->Release();
					pCredUser = NULL;
				}
			}			
		}
	}

	return hr;
}

/*
	CXCreCredentialV2
*/
CXCreCredentialV2::CXCreCredentialV2():
	m_cRef(1),
	m_pCredProvCredentialEvents(NULL),
	m_pszUserSid(NULL),
	m_pszQualifiedUserName(NULL),
	m_fIsLocalUser(FALSE),
	m_fChecked(FALSE),
	m_fShowControls(FALSE),
	m_dwComboIndex(0)
{
	AddRef();
	
	ZeroMemory(m_rgFieldStrings, sizeof(m_rgFieldStrings));

	m_hParentWnd = NULL;
	m_dwComboIndex = 0;
}

CXCreCredentialV2::~CXCreCredentialV2()
{		
	if (m_rgFieldStrings[SFI_PASSWORD])
	{
		size_t lenPassword = wcslen(m_rgFieldStrings[SFI_PASSWORD]);
		SecureZeroMemory(m_rgFieldStrings[SFI_PASSWORD], lenPassword * sizeof(*m_rgFieldStrings[SFI_PASSWORD]));
	}

	for (int i = 0; i < ARRAYSIZE(m_rgFieldStrings); i++)
	{
		CoTaskMemFree(m_rgFieldStrings[i]);
	}

	CoTaskMemFree(m_pszUserSid);
	CoTaskMemFree(m_pszQualifiedUserName);

	Release();
}

// Initializes one credential with the field information passed in.
// Set the value of the SFI_LARGE_TEXT field to pwzUsername.
HRESULT CXCreCredentialV2::Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
									  _In_ ICredentialProviderUser *pcpUser,
									  CCmdEventWnd * pEventWin)
{
	HRESULT hr = S_OK;
	m_cpus = cpus;

	GUID guidProvider;
	pcpUser->GetProviderID(&guidProvider);
	m_fIsLocalUser = (guidProvider == Identity_LocalUserProvider);

	m_pEventWin = pEventWin;

	// Initialize the String value of all the fields.
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Safe Logon", &m_rgFieldStrings[SFI_LABEL]);
	}
	
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"", &m_rgFieldStrings[SFI_PASSWORD]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Submit", &m_rgFieldStrings[SFI_SUBMIT_BUTTON]);
	}
		
	if (SUCCEEDED(hr))
	{
		hr = pcpUser->GetStringValue(PKEY_Identity_QualifiedUserName, &m_pszQualifiedUserName);
	}
	/*
	if (SUCCEEDED(hr))
	{
		PWSTR pszUserName;
		pcpUser->GetStringValue(PKEY_Identity_UserName, &pszUserName);
		if (pszUserName != nullptr)
		{
			wchar_t szString[256];
			StringCchPrintf(szString, ARRAYSIZE(szString), L"User Name: %s", pszUserName);
			hr = SHStrDupW(szString, &m_rgFieldStrings[SFI_FULLNAME_TEXT]);
			CoTaskMemFree(pszUserName);
		}
		else
		{
			hr =  SHStrDupW(L"User Name is NULL", &m_rgFieldStrings[SFI_FULLNAME_TEXT]);
		}
	}
	if (SUCCEEDED(hr))
	{
		PWSTR pszDisplayName;
		pcpUser->GetStringValue(PKEY_Identity_DisplayName, &pszDisplayName);
		if (pszDisplayName != nullptr)
		{
			wchar_t szString[256];
			StringCchPrintf(szString, ARRAYSIZE(szString), L"Display Name: %s", pszDisplayName);
			hr = SHStrDupW(szString, &m_rgFieldStrings[SFI_DISPLAYNAME_TEXT]);
			CoTaskMemFree(pszDisplayName);
		}
		else
		{
			hr = SHStrDupW(L"Display Name is NULL", &m_rgFieldStrings[SFI_DISPLAYNAME_TEXT]);
		}
	}
	
	if (SUCCEEDED(hr))
	{
		PWSTR pszLogonStatus;
		pcpUser->GetStringValue(PKEY_Identity_LogonStatusString, &pszLogonStatus);
		if (pszLogonStatus != nullptr)
		{
			wchar_t szString[256];
			StringCchPrintf(szString, ARRAYSIZE(szString), L"Logon Status: %s", pszLogonStatus);
			hr = SHStrDupW(szString, &m_rgFieldStrings[SFI_LOGONSTATUS_TEXT]);
			CoTaskMemFree(pszLogonStatus);
		}
		else
		{
			hr = SHStrDupW(L"Logon Status is NULL", &m_rgFieldStrings[SFI_LOGONSTATUS_TEXT]);
		}
	}
	*/
	if (SUCCEEDED(hr))
	{
		hr = pcpUser->GetSid(&m_pszUserSid);
	}

	return hr;
}

// LogonUI calls this in order to give us a callback in case we need to notify it of anything.
HRESULT CXCreCredentialV2::Advise(__in ICredentialProviderCredentialEvents* pcpce
								  )
{
	HRESULT hr = S_FALSE;

	if (NULL != m_pCredProvCredentialEvents)
	{
		m_pCredProvCredentialEvents->Release();
	}

	hr = pcpce->QueryInterface(IID_PPV_ARGS(&m_pCredProvCredentialEvents));
	if (SUCCEEDED(hr))
	{
		m_pCredProvCredentialEvents->OnCreatingWindow(&m_hParentWnd);
	}

	return hr;
}

// LogonUI calls this to tell us to release the callback.
HRESULT CXCreCredentialV2::UnAdvise()
{
	if (NULL != m_pCredProvCredentialEvents)
	{
		m_pCredProvCredentialEvents->Release();
		m_pCredProvCredentialEvents = NULL;
	}	

	return S_OK;
}

// LogonUI calls this function when our tile is selected (zoomed)
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the
// field definitions. But if you want to do something
// more complicated, like change the contents of a field when the tile is
// selected, you would do it here.
HRESULT CXCreCredentialV2::SetSelected(__out BOOL* pbAutoLogon)  
{
	*pbAutoLogon = FALSE;

	if(NULL != m_pEventWin)
	{
		if(m_pEventWin->GetConnectedStatus())
		{
			*pbAutoLogon = TRUE;
		}
	}

	return S_OK;
}

// Similarly to SetSelected, LogonUI calls this when your tile was selected
// and now no longer is. The most common thing to do here (which we do below)
// is to clear out the password field.
HRESULT CXCreCredentialV2::SetDeselected()
{
	HRESULT hr = S_OK;
	if (m_rgFieldStrings[SFI_PASSWORD])
	{
		size_t lenPassword = wcslen(m_rgFieldStrings[SFI_PASSWORD]);
		SecureZeroMemory(m_rgFieldStrings[SFI_PASSWORD], lenPassword * sizeof(*m_rgFieldStrings[SFI_PASSWORD]));

		CoTaskMemFree(m_rgFieldStrings[SFI_PASSWORD]);
		hr = SHStrDupW(L"", &m_rgFieldStrings[SFI_PASSWORD]);

		if (SUCCEEDED(hr) && m_pCredProvCredentialEvents)
		{
			m_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, m_rgFieldStrings[SFI_PASSWORD]);
		}
	}

	return hr;
}

// Get info for a particular field of a tile. Called by logonUI to get information
// to display the tile.
HRESULT CXCreCredentialV2::GetFieldState(
	__in DWORD dwFieldID,
	__out CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
	__out CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis
	)
{
	HRESULT hr;

	// Validate our parameters.
	if ((dwFieldID < ARRAYSIZE(s_rgFieldStatePairs)))
	{
		*pcpfs = s_rgFieldStatePairs[dwFieldID].cpfs;
		*pcpfis = s_rgFieldStatePairs[dwFieldID].cpfis;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets ppwsz to the string value of the field at the index dwFieldID
HRESULT CXCreCredentialV2::GetStringValue(
	__in DWORD dwFieldID, 
	__deref_out PWSTR* ppwsz
	)
{
	HRESULT hr;
	*ppwsz = nullptr;

	// Check to make sure dwFieldID is a legitimate index
	if (dwFieldID < ARRAYSIZE(s_rgCredProvFieldDescriptors))
	{
		// Make a copy of the string and return that. The caller
		// is responsible for freeing it.
		hr = SHStrDupW(m_rgFieldStrings[dwFieldID], ppwsz);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Returns the number of items to be included in the combobox (pcItems), as well as the
// currently selected item (pdwSelectedItem).
HRESULT CXCreCredentialV2::GetComboBoxValueCount(
	__in DWORD dwFieldID, 
	__out DWORD* pcItems, 
	__out_range(<,*pcItems) DWORD* pdwSelectedItem
	)
{
	HRESULT hr = S_OK;
	*pcItems = 0;
	*pdwSelectedItem = 0;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(s_rgCredProvFieldDescriptors) &&
		(CPFT_COMBOBOX == s_rgCredProvFieldDescriptors[dwFieldID].cpft) /*&&
		dwFieldID == SFI_COMBOBOX_TYPE*/)
	{
		*pcItems = ARRAYSIZE(s_gLoginComboBoxStrings);
		*pdwSelectedItem = m_dwComboIndex;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return hr;
}

// Called iteratively to fill the combobox with the string (ppwszItem) at index dwItem.
HRESULT CXCreCredentialV2::GetComboBoxValueAt(
	__in DWORD dwFieldID, 
	__in DWORD dwItem,
	__deref_out PWSTR* ppwszItem
	)
{
	HRESULT hr = S_OK;
	*ppwszItem = nullptr;
	
	//Validate parameters.
	if (dwFieldID < ARRAYSIZE(s_rgCredProvFieldDescriptors) &&
		(CPFT_COMBOBOX == s_rgCredProvFieldDescriptors[dwFieldID].cpft) /*&&
		dwFieldID == SFI_COMBOBOX_TYPE*/)
	{
		hr = SHStrDupW(s_gLoginComboBoxStrings[dwItem], ppwszItem);
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return hr;
}

// Called when the user changes the selected item in the combobox.
HRESULT CXCreCredentialV2::SetComboBoxSelectedValue(
	__in DWORD dwFieldID,
	__in DWORD dwSelectedItem
	)
{
	HRESULT hr = S_OK;
	
	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(s_rgCredProvFieldDescriptors) &&
		(CPFT_COMBOBOX == s_rgCredProvFieldDescriptors[dwFieldID].cpft) /*&&
		dwFieldID == SFI_COMBOBOX_TYPE*/)
	{
		m_dwComboIndex = dwSelectedItem;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return hr;
}

// Get the image to show in the user tile.
HRESULT CXCreCredentialV2::GetBitmapValue(
	__in DWORD dwFieldID, 
	__out HBITMAP* phbmp
	)
{
	HRESULT hr;
	*phbmp = NULL;

	if (SFI_TILEIMAGE == dwFieldID)
	{
		HBITMAP hbmp = LoadBitmap(g_hinst, MAKEINTRESOURCE(101));
		if (hbmp != NULL)
		{
			hr = S_OK;
			*phbmp = hbmp;
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets pdwAdjacentTo to the index of the field the submit button should be
// adjacent to. We recommend that the submit button is placed next to the last
// field which the user is required to enter information in. Optional fields
// should be below the submit button.
HRESULT CXCreCredentialV2::GetSubmitButtonValue(
	__in DWORD dwFieldID,
	__out DWORD* pdwAdjacentTo
	)
{
	HRESULT hr;

	if (SFI_SUBMIT_BUTTON == dwFieldID)
	{
		// pdwAdjacentTo is a pointer to the fieldID you want the submit button to
		// appear next to.
		*pdwAdjacentTo = SFI_PASSWORD;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets the value of a field which can accept a string as a value.
// This is called on each keystroke when a user types into an edit field.
HRESULT CXCreCredentialV2::SetStringValue(
	__in DWORD dwFieldID,
	__in PCWSTR pwz
	)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(s_rgCredProvFieldDescriptors) &&
		(CPFT_EDIT_TEXT == s_rgCredProvFieldDescriptors[dwFieldID].cpft ||
		CPFT_PASSWORD_TEXT == s_rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		PWSTR *ppwszStored = &m_rgFieldStrings[dwFieldID];
		CoTaskMemFree(*ppwszStored);
		hr = SHStrDupW(pwz, ppwszStored);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Returns whether a checkbox is checked or not as well as its label.
HRESULT CXCreCredentialV2::GetCheckboxValue(
	__in DWORD dwFieldID, 
	__out BOOL* pbChecked,
	__deref_out PWSTR* ppwszLabel
	)
{
	HRESULT hr = E_UNEXPECTED;
	*ppwszLabel = nullptr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(s_rgCredProvFieldDescriptors) &&
		(CPFT_CHECKBOX == s_rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		*pbChecked = m_fChecked;		
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets whether the specified checkbox is checked or not.
HRESULT CXCreCredentialV2::SetCheckboxValue(
	__in DWORD dwFieldID, 
	__in BOOL bChecked
	)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(s_rgCredProvFieldDescriptors) &&
		(CPFT_CHECKBOX == s_rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		m_fChecked = bChecked;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Returns the number of items to be included in the combobox (pcItems), as well as the
// currently selected item (pdwSelectedItem).
HRESULT CXCreCredentialV2::CommandLinkClicked(__in DWORD dwFieldID)
{
	HRESULT hr = S_OK;

	CREDENTIAL_PROVIDER_FIELD_STATE cpfsShow = CPFS_HIDDEN;

	//Validate parameter.
	if (dwFieldID < ARRAYSIZE(s_rgCredProvFieldDescriptors) &&
		(CPFT_COMMAND_LINK == s_rgCredProvFieldDescriptors[dwFieldID].cpft) /*&&
		dwFieldID == SFI_GETCODE_LINK*/)
	{
		::MessageBoxW(m_hParentWnd, L"Get Coder", L"Notice", 0);
	}
	
	return hr;
}

// Collect the username and password into a serialized credential for the correct usage scenario
// (logon/unlock is what's demonstrated in this sample).  LogonUI then passes these credentials
// back to the system to log on.
HRESULT CXCreCredentialV2::GetSerialization(
	__out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
	__out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
	__deref_out_opt PWSTR* ppwszOptionalStatusText, 
	__out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
	)
{	
	HRESULT hr = E_UNEXPECTED;
	*pcpgsr = CPGSR_NO_CREDENTIAL_NOT_FINISHED;
	*ppwszOptionalStatusText = nullptr;
	*pcpsiOptionalStatusIcon = CPSI_NONE;
	ZeroMemory(pcpcs, sizeof(*pcpcs));

	// For local user, the domain and user name can be split from _pszQualifiedUserName (domain\username).
	// CredPackAuthenticationBuffer() cannot be used because it won't work with unlock scenario.
	if (m_fIsLocalUser)
	{
		PWSTR pwzProtectedPassword;
		hr = ProtectIfNecessaryAndCopyPassword(L"xxxxxx"/*m_rgFieldStrings[SFI_PASSWORD]*/, m_cpus, &pwzProtectedPassword);
		if (SUCCEEDED(hr))
		{
			PWSTR pszDomain;
			PWSTR pszUsername;
			hr = SplitDomainAndUsername(m_pszQualifiedUserName, &pszDomain, &pszUsername);
			if (SUCCEEDED(hr))
			{
				KERB_INTERACTIVE_UNLOCK_LOGON kiul;
				hr = KerbInteractiveUnlockLogonInit(pszDomain, pszUsername, pwzProtectedPassword, m_cpus, &kiul);
				if (SUCCEEDED(hr))
				{
					// We use KERB_INTERACTIVE_UNLOCK_LOGON in both unlock and logon scenarios.  It contains a
					// KERB_INTERACTIVE_LOGON to hold the creds plus a LUID that is filled in for us by Winlogon
					// as necessary.
					hr = KerbInteractiveUnlockLogonPack(kiul, &pcpcs->rgbSerialization, &pcpcs->cbSerialization);
					if (SUCCEEDED(hr))
					{
						ULONG ulAuthPackage;
						hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);
						if (SUCCEEDED(hr))
						{
							pcpcs->ulAuthenticationPackage = ulAuthPackage;
							pcpcs->clsidCredentialProvider = CLSID_XCredentialV2;
							// At this point the credential has created the serialized credential used for logon
							// By setting this to CPGSR_RETURN_CREDENTIAL_FINISHED we are letting logonUI know
							// that we have all the information we need and it should attempt to submit the
							// serialized credential.
							*pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
						}
					}
				}
				CoTaskMemFree(pszDomain);
				CoTaskMemFree(pszUsername);
			}
			CoTaskMemFree(pwzProtectedPassword);
		}
	}
	else
	{
		DWORD dwAuthFlags = CRED_PACK_PROTECTED_CREDENTIALS | CRED_PACK_ID_PROVIDER_CREDENTIALS;

		// First get the size of the authentication buffer to allocate
		if (!CredPackAuthenticationBuffer(dwAuthFlags, m_pszQualifiedUserName, const_cast<PWSTR>(m_rgFieldStrings[SFI_PASSWORD]), nullptr, &pcpcs->cbSerialization) &&
			(GetLastError() == ERROR_INSUFFICIENT_BUFFER))
		{
			pcpcs->rgbSerialization = static_cast<byte *>(CoTaskMemAlloc(pcpcs->cbSerialization));
			if (pcpcs->rgbSerialization != nullptr)
			{
				hr = S_OK;

				// Retrieve the authentication buffer
				if (CredPackAuthenticationBuffer(dwAuthFlags, m_pszQualifiedUserName, const_cast<PWSTR>(m_rgFieldStrings[SFI_PASSWORD]), pcpcs->rgbSerialization, &pcpcs->cbSerialization))
				{
					ULONG ulAuthPackage;
					hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);
					if (SUCCEEDED(hr))
					{
						pcpcs->ulAuthenticationPackage = ulAuthPackage;
						pcpcs->clsidCredentialProvider = CLSID_XCredentialV2;

						// At this point the credential has created the serialized credential used for logon
						// By setting this to CPGSR_RETURN_CREDENTIAL_FINISHED we are letting logonUI know
						// that we have all the information we need and it should attempt to submit the
						// serialized credential.
						*pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
					}
				}
				else
				{
					hr = HRESULT_FROM_WIN32(GetLastError());
					if (SUCCEEDED(hr))
					{
						hr = E_FAIL;
					}
				}

				if (FAILED(hr))
				{
					CoTaskMemFree(pcpcs->rgbSerialization);
				}
			}
			else
			{
				hr = E_OUTOFMEMORY;
			}
		}
	}

	return hr;
}

static const REPORT_RESULT_STATUS_INFO s_rgLogonStatusInfo[] =
{
	{ STATUS_LOGON_FAILURE, 0, L"Incorrect password or username.", CPSI_ERROR, },
	{ STATUS_ACCOUNT_RESTRICTION, STATUS_ACCOUNT_DISABLED, L"The account is disabled.", CPSI_WARNING },
};

// ReportResult is completely optional.  Its purpose is to allow a credential to customize the string
// and the icon displayed in the case of a logon failure.  For example, we have chosen to
// customize the error shown in the case of bad username/password and in the case of the account
// being disabled.
HRESULT CXCreCredentialV2::ReportResult(
										__in NTSTATUS ntsStatus, 
										__in NTSTATUS ntsSubstatus,
										__deref_out_opt PWSTR* ppwszOptionalStatusText, 
										__out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
										)
{
	*ppwszOptionalStatusText = nullptr;
	*pcpsiOptionalStatusIcon = CPSI_NONE;

	DWORD dwStatusInfo = (DWORD)-1;

	// Look for a match on status and substatus.
	for (DWORD i = 0; i < ARRAYSIZE(s_rgLogonStatusInfo); i++)
	{
		if (s_rgLogonStatusInfo[i].ntsStatus == ntsStatus && s_rgLogonStatusInfo[i].ntsSubstatus == ntsSubstatus)
		{
			dwStatusInfo = i;
			break;
		}
	}

	if ((DWORD)-1 != dwStatusInfo)
	{
		if (SUCCEEDED(SHStrDupW(s_rgLogonStatusInfo[dwStatusInfo].pwzMessage, ppwszOptionalStatusText)))
		{
			*pcpsiOptionalStatusIcon = s_rgLogonStatusInfo[dwStatusInfo].cpsi;
		}
	}

	// If we failed the logon, try to erase the password field.
	if (FAILED(HRESULT_FROM_NT(ntsStatus)))
	{
		if (m_pCredProvCredentialEvents)
		{
			m_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, L"");
		}
	}

	// Since nullptr is a valid value for *ppwszOptionalStatusText and *pcpsiOptionalStatusIcon
	// this function can't fail.
	return S_OK;
}

// Gets the SID of the user corresponding to the credential.
HRESULT CXCreCredentialV2::GetUserSid(_Outptr_result_nullonfailure_ PWSTR *ppszSid)
{
	*ppszSid = nullptr;
	HRESULT hr = E_UNEXPECTED;
	if (NULL != m_pszUserSid)
	{
		hr = SHStrDupW(m_pszUserSid, ppszSid);
	}

	// Return S_FALSE with a null SID in ppszSid for the
	// credential to be associated with an empty user tile.

	return hr;
}

// GetFieldOptions to enable the password reveal button and touch keyboard auto-invoke in the password field.
HRESULT CXCreCredentialV2::GetFieldOptions(DWORD dwFieldID,
										   _Out_ CREDENTIAL_PROVIDER_CREDENTIAL_FIELD_OPTIONS *pcpcfo)
{
	*pcpcfo = CPCFO_NONE;

	if (dwFieldID == SFI_PASSWORD)
	{
		*pcpcfo = CPCFO_ENABLE_PASSWORD_REVEAL;
	}
	else if (dwFieldID == SFI_TILEIMAGE)
	{
		// *pcpcfo = CPCFO_ENABLE_TOUCH_KEYBOARD_AUTO_INVOKE;
	}

	return S_OK;
}