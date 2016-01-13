
#include "stdafx.h"
#include "Credential_Imp.h"

/*
	CXFilter
*/

HRESULT XCreFilter_CreateInstance(REFIID riid, void** ppv)
{
	HRESULT hr;
	
	CXFilter * pCreFilter = new CXFilter();
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

CXFilter::CXFilter():
	m_cRef(0)
{
	AddRef();
}

CXFilter::~CXFilter()
{
	Release();
}

HRESULT CXFilter::Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders)
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
				if (IsEqualGUID(rgclsidProviders[i],CLSID_V1PasswordCredentialProvider) ||
					IsEqualGUID(rgclsidProviders[i],CLSID_OnexCredentialProvider) ||
					IsEqualGUID(rgclsidProviders[i],CLSID_V1SmartcardCredentialProvider))
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
				if (IsEqualGUID(rgclsidProviders[i],CLSID_V1PasswordCredentialProvider) ||
					IsEqualGUID(rgclsidProviders[i],CLSID_OnexCredentialProvider) ||
					IsEqualGUID(rgclsidProviders[i],CLSID_V1SmartcardCredentialProvider))
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
				if (IsEqualGUID(rgclsidProviders[i],CLSID_V1PasswordCredentialProvider) ||
					IsEqualGUID(rgclsidProviders[i],CLSID_OnexCredentialProvider) ||
					IsEqualGUID(rgclsidProviders[i],CLSID_V1SmartcardCredentialProvider))
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
HRESULT CXFilter::UpdateRemoteCredential(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsOut)
{
	UNREFERENCED_PARAMETER(pcpcsIn);
	UNREFERENCED_PARAMETER(pcpcsOut);

	return E_NOTIMPL;
}

/*
	CXCreProvider
*/
HRESULT XCreProvider_CreateInstance(REFIID riid, void** ppv)
{
	HRESULT hr;	

	CXCreProvider* pProvider = new CXCreProvider();
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

/*
	CXCreProvider
*/

CXCreProvider::CXCreProvider():
	m_cRef(1)
{
	AddRef();
	
	m_rgpExtendCredentials = NULL;

	m_dwCredentialCount = 0;

	m_pOrgProvider = NULL;
	m_dwWrappedDescriptorCount = 0;	


	m_pcpe = NULL;
	m_pEventWin = NULL;
}

CXCreProvider::~CXCreProvider()
{
	CleanUpAllCredentials();

	if (m_pOrgProvider)
	{
		m_pOrgProvider->Release();
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
void CXCreProvider::OnConnectStatusChanged()
{
    if (m_pcpe != NULL)
    {
		OutputDebugString(L"OnConnectStatusChanged()");
        m_pcpe->CredentialsChanged(m_upAdviseContext);
    }
}

// Cleans up all credentials, including the memory used to allocate the array.
void CXCreProvider::CleanUpAllCredentials()
{
	// Iterate and clean up the array, if it exists.
	if (m_rgpExtendCredentials != NULL)
	{
		for (DWORD lcv = 0; lcv < m_dwCredentialCount; lcv++)
		{
			if (m_rgpExtendCredentials[lcv] != NULL)
			{
				m_rgpExtendCredentials[lcv]->Release();
				m_rgpExtendCredentials[lcv] = NULL;
			}
		}

		delete [] m_rgpExtendCredentials;
		m_rgpExtendCredentials = NULL;
	}
}

//
// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.  In this sample we have chosen to precreate the credentials 
// for the usage scenario passed in cpus instead of saving off cpus and only creating
// the credentials when we're asked to.
// This sample only handles the logon and unlock scenarios as those are the most common.
//
HRESULT CXCreProvider::SetUsageScenario(
	CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
	DWORD dwFlags
	)
{
	HRESULT hr;

	// Create the password credential provider and query its interface for an
	// ICredentialProvider we can use. Once it's up and running, ask it about the 
	// usage scenario being provided.
	IUnknown *pUnknown = NULL;
	
	//802.1x 
	//hr = CoCreateInstance(CLSID_OnexCredentialProvider, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pUnknown));

	//smart card
	//hr = CoCreateInstance(CLSID_SmartcardCredentialProvider, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pUnknown));

	//username+password
	hr = CoCreateInstance(CLSID_V1PasswordCredentialProvider, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pUnknown));
	if (SUCCEEDED(hr))
	{		
		hr = pUnknown->QueryInterface(IID_PPV_ARGS(&(m_pOrgProvider)));
		if (SUCCEEDED(hr))
		{
			hr = m_pOrgProvider->SetUsageScenario(cpus, dwFlags);			
			m_cpus = cpus;

			//增加窗口
			m_pEventWin = new CCmdEventWnd();
			if(NULL != m_pEventWin)
			{
				m_pEventWin->Initialize(this);
			}
		}
	}	

	if (FAILED(hr))
	{
		if (m_pOrgProvider != NULL)
		{
			m_pOrgProvider->Release();
			m_pOrgProvider = NULL;
		}

		if(NULL != m_pEventWin)
		{
			delete m_pEventWin;
			m_pEventWin = NULL;
		}
	}

	return hr;
}

//
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
// SetSerialization is currently optional, which is why it's not implemented in this sample.  
// If that changes in the future, we will update the sample.
// 
STDMETHODIMP CXCreProvider::SetSerialization(
	const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs
	)
{	
	HRESULT hr = E_UNEXPECTED;

	if (m_pOrgProvider != NULL)
	{
		hr = m_pOrgProvider->SetSerialization(pcpcs);
	}

	OutputDebugStringW(L"SetSerialization()");

	return hr;
}

//
// Called by LogonUI to give you a callback.  Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated
//
HRESULT CXCreProvider::Advise(
	ICredentialProviderEvents* pcpe,
	UINT_PTR upAdviseContext
	)
{
	HRESULT hr = E_UNEXPECTED;
	if (m_pOrgProvider != NULL)
	{
		hr = m_pOrgProvider->Advise(pcpe,upAdviseContext);

		m_pcpe = pcpe;
		m_pcpe->AddRef();
		m_upAdviseContext = upAdviseContext;
	}

	return hr;
}

//
// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
//
HRESULT CXCreProvider::UnAdvise()
{
	HRESULT hr = E_UNEXPECTED;

	if (m_pOrgProvider != NULL)
	{
		hr = m_pOrgProvider->UnAdvise();
	}

	if (m_pcpe != NULL)
    {
        m_pcpe->Release();
        m_pcpe = NULL;
    }

	return hr;
}

//
// Called by LogonUI to determine the number of fields in your tiles.  This
// does mean that all your tiles must have the same number of fields.
// This number must include both visible and invisible fields. If you want a tile
// to have different fields from the other tiles you enumerate for a given usage
// scenario you must include them all in this count and then hide/show them as desired 
// using the field descriptors.
//
HRESULT CXCreProvider::GetFieldDescriptorCount(
	DWORD* pdwCount
	)
{
	HRESULT hr = E_UNEXPECTED;

	if (m_pOrgProvider != NULL)
	{
		hr = m_pOrgProvider->GetFieldDescriptorCount(&m_dwWrappedDescriptorCount);
		if (SUCCEEDED(hr))
		{				
			*pdwCount = m_dwWrappedDescriptorCount;

			//add sms ctrls.
			*pdwCount = *pdwCount + SFI_MAX_FIELDS;
		}
	}

	return hr;
}

//
// Gets the field descriptor for a particular field
//
HRESULT CXCreProvider::GetFieldDescriptorAt(
	DWORD dwIndex, 
	CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
	)
{    
	HRESULT hr;	
	if (m_pOrgProvider != NULL)
	{
		if (ppcpfd != NULL)
		{
			if (dwIndex < m_dwWrappedDescriptorCount)
			{				
				hr = m_pOrgProvider->GetFieldDescriptorAt(dwIndex, ppcpfd);				
			}
			else //
			{
				// Offset into the descriptor count so we can index our own fields.
				dwIndex -= m_dwWrappedDescriptorCount;
				if (dwIndex < SFI_MAX_FIELDS)
				{
					hr = FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptors[dwIndex], ppcpfd);
					(*ppcpfd)->dwFieldID += m_dwWrappedDescriptorCount;
				}
				else
				{ 
					hr = E_INVALIDARG;
				}				
			}
		}
		else
		{ 
			hr = E_INVALIDARG;
		}
	}

	return hr;
}

//
// Sets pdwCount to the number of tiles that we wish to show at this time.
// Sets pdwDefault to the index of the tile which should be used as the default.
// The default tile is the tile which will be shown in the zoomed view by default. If 
// more than one provider specifies a default tile the behavior is undefined. 
// If *pbAutoLogonWithDefault is TRUE, LogonUI will immediately call GetSerialization
// on the credential you've specified as the default and will submit that credential
// for authentication without showing any further UI.
//
HRESULT CXCreProvider::GetCredentialCount(
	DWORD* pdwCount,
	DWORD* pdwDefault,
	BOOL* pbAutoLogonWithDefault
	)
{
	HRESULT hr = E_UNEXPECTED;
	DWORD dwDefault = 0;
	BOOL bAutoLogonWithDefault = FALSE;	

	//Make sure we've created the provider.
	if (m_pOrgProvider != NULL)
	{
		// This probably shouldn't happen, but in the event that this gets called after
		// we've already been through once, we want to clean up everything before 
		// allocating new stuff all over again.
		if (m_rgpExtendCredentials != NULL)
		{
			CleanUpAllCredentials();
		}

		// We need to know how many fields each credential has in order to initialize
		// our wrapper credentials, so we might as well do that here before anything else.
		DWORD count;
		hr = GetFieldDescriptorCount(&(count));
		if (SUCCEEDED(hr))
		{
			//Grab the credential count of the wrapped provider. We'll simply wrap each.
			hr = m_pOrgProvider->GetCredentialCount(&(m_dwCredentialCount), &(dwDefault), &(bAutoLogonWithDefault));
			if (SUCCEEDED(hr))
			{
				// Create an array of credentials for use.
				m_rgpExtendCredentials = new CXCreCredential*[m_dwCredentialCount];
				if (m_rgpExtendCredentials != NULL)
				{
					//Iterate each credential and make a wrapper.
					for (DWORD lcv = 0; SUCCEEDED(hr) && (lcv < m_dwCredentialCount); lcv++)
					{
						m_rgpExtendCredentials[lcv] = new CXCreCredential();
						if (m_rgpExtendCredentials[lcv] != NULL)
						{
							ICredentialProviderCredential *pCredential;
							hr = m_pOrgProvider->GetCredentialAt(lcv,&(pCredential));
							if (SUCCEEDED(hr))
							{
								// Set the Field State Pair and Field Descriptors for ppc's 
								// fields to the defaults (s_rgCredProvFieldDescriptors, 
								// and s_rgFieldStatePairs) and the value of SFI_USERNAME
								// to pwzUsername.									
								hr = m_rgpExtendCredentials[lcv]->Initialize(m_cpus,pCredential,m_dwWrappedDescriptorCount,m_pEventWin);
								if (FAILED(hr))
								{
									// If initialization failed, clean everything up.
									for (lcv = 0; lcv < m_dwCredentialCount; lcv++)
									{
										if (m_rgpExtendCredentials[lcv] != NULL)
										{
											// Release the pointer to account for the local reference.
											m_rgpExtendCredentials[lcv]->Release();
											m_rgpExtendCredentials[lcv] = NULL;
										}
									}
								}
								pCredential->Release();
							} // (End if _pWrappedProvider->GetCredentialAt succeeded.)
						} // (End if allocating _rgpCredentials[lcv] succeeded.)
						else
						{
							hr = E_OUTOFMEMORY;
						} 
					} // (End of _rgpCredentials allocation loop.)
				} // (End if for allocating _rgpCredentials succeeded.)
				else
				{
					hr = E_OUTOFMEMORY;
				}
			}				
		} // (End if _pWrappedProvider->GetCredentialCount succeeded.)
	} // (End if GetFieldDescriptorCount succeeded.)	

	if (FAILED(hr))
	{
		// Clean up.
		if (NULL != m_rgpExtendCredentials)
		{
			delete [] m_rgpExtendCredentials;
			m_rgpExtendCredentials = NULL;
		}		
	}
	else
	{
		*pdwCount = m_dwCredentialCount;
		*pdwDefault = dwDefault;
		*pbAutoLogonWithDefault = bAutoLogonWithDefault;
	}
	
	char szLog[256] = {0};
	sprintf(szLog,"CNT : %d,def : %d\r\n",*pdwCount,*pdwDefault);
	OutputDebugStringA(szLog);

	//是否可以自动连接.
	if(m_pEventWin->GetConnectedStatus())
	{
		*pdwDefault = 0;
	}

    *pbAutoLogonWithDefault = FALSE;

	return hr;
}

//
// Returns the credential at the index specified by dwIndex. This function is called by logonUI to enumerate
// the tiles.
// 
HRESULT CXCreProvider::GetCredentialAt(
	DWORD dwIndex, 
	ICredentialProviderCredential** ppcpc
	)
{
	HRESULT hr;

	//Validate parameters.
	if ((dwIndex < m_dwCredentialCount) && 
		(ppcpc != NULL) &&
		(m_rgpExtendCredentials != NULL) &&
		(m_rgpExtendCredentials[dwIndex] != NULL))
	{
		hr = m_rgpExtendCredentials[dwIndex]->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
	}
	else
	{
		hr = E_INVALIDARG;
	}		

	return hr;
}

/*
	CXCreCredential
*/

CXCreCredential::CXCreCredential():
m_cRef(1)
{
	DllAddRef();	

	m_pWrappedCredential = NULL;	
	m_pCredProvCredentialEvents = NULL;	

	m_dwSMSGetTypeIndex = 0;

	m_bCheckPass = FALSE;	
}

CXCreCredential::~CXCreCredential()
{		
	CleanupEvents();

	if (NULL != m_pWrappedCredential)
	{
		m_pWrappedCredential->Release();
		m_pWrappedCredential = NULL;
	}

	Release();
}

//返回系统控件的数目.
DWORD CXCreCredential::GetDescriptorCountOrg()
{
	return m_dwWrappedDescriptorCount;
}

// Initializes one credential with the field information passed in. We also keep track
// of our wrapped credential and how many fields it has.
HRESULT CXCreCredential::Initialize(
									  __in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
									  __in ICredentialProviderCredential *pWrappedCredential,
									  __in DWORD dwWrappedDescriptorCount,
									  __in CCmdEventWnd * pEventWin)
{
	HRESULT hr = S_OK;

	// Grab the credential we're wrapping for future reference.
	if (m_pWrappedCredential != NULL)
	{
		m_pWrappedCredential->Release();
	}	

	//用户名 + 密码的 验证模式
	m_pWrappedCredential = (ICredentialProviderCredential*)pWrappedCredential;
	m_pWrappedCredential->AddRef();

	//We also need to remember how many fields the inner credential has.
	m_dwWrappedDescriptorCount = dwWrappedDescriptorCount;	

	m_cpus = cpus;

	m_pCredProvCredentialEvents = NULL;
	m_pEventWin = pEventWin;

	return hr;
}

// LogonUI calls this in order to give us a callback in case we need to notify it of 
// anything. We'll also provide it to the wrapped credential.
HRESULT CXCreCredential::Advise(
								  __in ICredentialProviderCredentialEvents* pcpce
								  )
{
	HRESULT hr = S_OK;

	CleanupEvents();

	//We keep a strong reference on the real ICredentialProviderCredentialEvents
	//to ensure that the weak reference held by the CWrappedCredentialEvents is valid.
	m_pCredProvCredentialEvents = pcpce;
	m_pCredProvCredentialEvents->AddRef();
	
	if (m_pWrappedCredential != NULL)
	{
		hr = m_pWrappedCredential->Advise(pcpce);
	}
	
	//get parent window HWND.
	hr = m_pCredProvCredentialEvents->OnCreatingWindow(&m_hParentWnd);		

	return hr;
}

// LogonUI calls this to tell us to release the callback. 
// We'll also provide it to the wrapped credential.
HRESULT CXCreCredential::UnAdvise()
{
	HRESULT hr = S_OK;

	if (NULL != m_pWrappedCredential)
	{
		m_pWrappedCredential->UnAdvise();
	}	

	return hr;
}

// LogonUI calls this function when our tile is selected (zoomed)
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the 
// field definitions. In fact, we're just going to hand it off to the
// wrapped credential in case it wants to do something.
HRESULT CXCreCredential::SetSelected(__out BOOL* pbAutoLogon)  
{
	HRESULT hr = E_UNEXPECTED;

	if (NULL != m_pWrappedCredential)
	{
		hr = m_pWrappedCredential->SetSelected(pbAutoLogon);
		m_hWatchThread = CreateThread(NULL,0,WorkThread,this,0,NULL);

		if(NULL != m_pEventWin)
		{
			if(m_pEventWin->GetConnectedStatus())
			{
				*pbAutoLogon = TRUE;
			}
		}
	}

	return hr;
}

// Similarly to SetSelected, LogonUI calls this when your tile was selected
// and now no longer is. We'll let the wrapped credential do anything it needs.
HRESULT CXCreCredential::SetDeselected()
{
	HRESULT hr = E_UNEXPECTED;

	if (NULL != m_pWrappedCredential)
	{
		hr = m_pWrappedCredential->SetDeselected();
	}

	return hr;
}

// Get info for a particular field of a tile. Called by logonUI to get information to 
// display the tile. We'll check to see if it's for us or the wrapped credential, and then
// handle or route it as appropriate.
HRESULT CXCreCredential::GetFieldState(
	__in DWORD dwFieldID,
	__out CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
	__out CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis
	)
{
	HRESULT hr = E_UNEXPECTED;

	// Make sure we have a wrapped credential.
	if (NULL != m_pWrappedCredential)
	{
		// Validate parameters.
		if ((pcpfs != NULL) && (pcpfis != NULL))
		{
			if (IsFieldInWrappedCredential(dwFieldID))
			{
				hr = m_pWrappedCredential->GetFieldState(dwFieldID, pcpfs, pcpfis);	
				if (SUCCEEDED(hr))
				{
					if(SUBMIT_BUTTON_ID == dwFieldID || USER_PASSS_WORD_ID == dwFieldID)
					{
						*pcpfs = CPFS_HIDDEN;
					}
				}
			}
			else
			{
				FIELD_STATE_PAIR *pfsp = LookupLocalFieldStatePair(dwFieldID);				
				if (pfsp != NULL)
				{					
					*pcpfs = pfsp->cpfs;
					*pcpfis = pfsp->cpfis;
					
					hr = S_OK;
				}
				else
				{
					hr = E_INVALIDARG;
				}
			}
		}
		else
		{ 
			hr = E_INVALIDARG;
		}
	}

	OutputDebugStringW(L"GetFiledState()");

	return hr;
}

// Sets ppwsz to the string value of the field at the index dwFieldID. We'll check to see if 
// it's for us or the wrapped credential, and then handle or route it as appropriate.
HRESULT CXCreCredential::GetStringValue(
	__in DWORD dwFieldID, 
	__deref_out PWSTR* ppwsz
	)
{
	HRESULT hr = E_UNEXPECTED;

	// Make sure we have a wrapped credential.
	if (NULL != m_pWrappedCredential)
	{
		if (IsFieldInWrappedCredential(dwFieldID))
		{
			hr = m_pWrappedCredential->GetStringValue(dwFieldID, ppwsz);

			wchar_t wszOutLog[1024] = {0};
			swprintf(wszOutLog,L"GET - > %d:%s \r\n",dwFieldID,*ppwsz);
			OutputDebugStringW(wszOutLog);
		}
		else
		{
			FIELD_STATE_PAIR *pfsp = LookupLocalFieldStatePair(dwFieldID);
			if (pfsp != NULL)
			{	
				dwFieldID = dwFieldID - m_dwWrappedDescriptorCount;	

				hr = SHStrDupW(s_rgCredProvFieldDescriptors[dwFieldID].pszLabel,ppwsz);
			}
			else
			{
				hr = E_INVALIDARG;
			}
		}
	}

	return hr;
}


// Returns the number of items to be included in the combobox (pcItems), as well as the 
// currently selected item (pdwSelectedItem). We'll check to see if it's for us or the 
// wrapped credential, and then handle or route it as appropriate.
HRESULT CXCreCredential::GetComboBoxValueCount(
	__in DWORD dwFieldID, 
	__out DWORD* pcItems, 
	__out_range(<,*pcItems) DWORD* pdwSelectedItem
	)
{
	HRESULT hr = E_UNEXPECTED;

	// Make sure we have a wrapped credential.
	if (NULL != m_pWrappedCredential)
	{
		if (IsFieldInWrappedCredential(dwFieldID))
		{
			hr = m_pWrappedCredential->GetComboBoxValueCount(dwFieldID, pcItems, pdwSelectedItem);
		}		
		else
		{			
			/*if ((SFI_CODE_TYPE_COMBOBOX + m_dwWrappedDescriptorCount) == dwFieldID)
			{
				FIELD_STATE_PAIR *pfsp = LookupLocalFieldStatePair(dwFieldID);
				if (pfsp != NULL)
				{
					*pcItems = ARRAYSIZE(s_gLoginComboBoxStrings);
					*pdwSelectedItem = m_dwSMSGetTypeIndex;
					hr = S_OK;
				}
			}	*/		
		}
	}

	return hr;
}

// Called iteratively to fill the combobox with the string (ppwszItem) at index dwItem.
// We'll check to see if it's for us or the wrapped credential, and then handle or route 
// it as appropriate.
HRESULT CXCreCredential::GetComboBoxValueAt(
	__in DWORD dwFieldID, 
	__in DWORD dwItem,
	__deref_out PWSTR* ppwszItem
	)
{
	HRESULT hr = E_UNEXPECTED;

	//Make sure we have a wrapped credential.
	if (NULL != m_pWrappedCredential)
	{
		if (IsFieldInWrappedCredential(dwFieldID))
		{
			hr = m_pWrappedCredential->GetComboBoxValueAt(dwFieldID, dwItem, ppwszItem);
		}		
		else
		{	/*		
			if ((SFI_CODE_TYPE_COMBOBOX + m_dwWrappedDescriptorCount) == dwFieldID)
			{
				FIELD_STATE_PAIR *pfsp = LookupLocalFieldStatePair(dwFieldID);
				if ((pfsp != NULL) && (dwItem < ARRAYSIZE(s_gLoginComboBoxStrings)))
				{
					hr = SHStrDupW(s_gLoginComboBoxStrings[dwItem],ppwszItem);
				}
			}	*/	
		}
	}

	return hr;
}

// Called when the user changes the selected item in the combobox. We'll check to see if 
// it's for us or the wrapped credential, and then handle or route it as appropriate.
HRESULT CXCreCredential::SetComboBoxSelectedValue(
	__in DWORD dwFieldID,
	__in DWORD dwSelectedItem
	)
{
	HRESULT hr = E_UNEXPECTED;

	//Make sure we have a wrapped credential.
	if (NULL != m_pWrappedCredential)
	{
		if (IsFieldInWrappedCredential(dwFieldID))
		{
			hr = m_pWrappedCredential->SetComboBoxSelectedValue(dwFieldID, dwSelectedItem);
		}
		else
		{	/*	
			if ((SFI_CODE_TYPE_COMBOBOX + m_dwWrappedDescriptorCount) == dwFieldID)
			{
				FIELD_STATE_PAIR *pfsp = LookupLocalFieldStatePair(dwFieldID);
				if ((pfsp != NULL) && (dwSelectedItem < ARRAYSIZE(s_gLoginComboBoxStrings)))
				{
					m_dwSMSGetTypeIndex = dwSelectedItem;
					hr = S_OK;
				}
			}		*/
		}
	}

	return hr;
}

//------------- 
// The following methods are for logonUI to get the values of various UI elements and 
// then communicate to the credential about what the user did in that field. Even though
// we don't offer these field types ourselves, we need to pass along the request to the
// wrapped credential.
HRESULT CXCreCredential::GetBitmapValue(
	__in DWORD dwFieldID, 
	__out HBITMAP* phbmp
	)
{
	HRESULT hr = E_UNEXPECTED;

	OutputDebugStringW(L"GetBitmap \r\n");

	if (NULL != m_pWrappedCredential)
	{	
		HBITMAP hbmp;
		HPALETTE      hPalette;

		wchar_t wszBMPFilePath[MAX_PATH] = {0};
		
		GetWindowsDirectoryW(wszBMPFilePath,MAX_PATH);
		wcscat(wszBMPFilePath,L"\\SafeLogonEx.bmp");
		if (LoadBitmapFromBMPFile(wszBMPFilePath,&hbmp,&hPalette))
		{			
			hr = S_OK;
			*phbmp = hbmp;
		}		
		else
		{
			hr = m_pWrappedCredential->GetBitmapValue(dwFieldID, phbmp);
		}
	}

	return hr;
}

HRESULT CXCreCredential::GetSubmitButtonValue(
	__in DWORD dwFieldID,
	__out DWORD* pdwAdjacentTo
	)
{
	HRESULT hr = E_UNEXPECTED;

	if (NULL != m_pWrappedCredential)
	{		
		if (IsFieldInWrappedCredential(dwFieldID))
		{
			hr = m_pWrappedCredential->GetSubmitButtonValue(dwFieldID, pdwAdjacentTo);

			wchar_t wszOutLog[1024] = {0};
			swprintf(wszOutLog,L"SUBBTN -> %d:%d \r\n",dwFieldID,*pdwAdjacentTo);
			OutputDebugStringW(wszOutLog);
		}
		else
		{

		}				
	}	

	return hr;
}

HRESULT CXCreCredential::SetStringValue(
	__in DWORD dwFieldID,
	__in PCWSTR pwz
	)
{
	HRESULT hr = E_UNEXPECTED;

	if (NULL != m_pWrappedCredential)
	{
		if (IsFieldInWrappedCredential(dwFieldID))
		{
			hr = m_pWrappedCredential->SetStringValue(dwFieldID, pwz);	
			
			wchar_t wszOutLog[1024] = {0};
			swprintf(wszOutLog,L"SET - > %d:%s \r\n",dwFieldID,pwz);
			OutputDebugStringW(wszOutLog);
		}
		else
		{

		}
	}	
	
	return hr;
}

HRESULT CXCreCredential::GetCheckboxValue(
	__in DWORD dwFieldID, 
	__out BOOL* pbChecked,
	__deref_out PWSTR* ppwszLabel
	)
{
	HRESULT hr = E_UNEXPECTED;

	if (NULL != m_pWrappedCredential)
	{
		if (IsFieldInWrappedCredential(dwFieldID))
		{
			hr = m_pWrappedCredential->GetCheckboxValue(dwFieldID, pbChecked, ppwszLabel);
		}
		else
		{			
			if (SFI_CODE_TYPE_CHECK_BOX + m_dwWrappedDescriptorCount == dwFieldID)
			{
				hr = SHStrDupW(s_rgCredProvFieldDescriptors[dwFieldID - m_dwWrappedDescriptorCount].pszLabel,ppwszLabel);
				*pbChecked = m_bCheckPass;
			}
		}
	}

	return hr;
}

HRESULT CXCreCredential::SetCheckboxValue(
	__in DWORD dwFieldID, 
	__in BOOL bChecked
	)
{
	HRESULT hr = E_UNEXPECTED;

	if (NULL != m_pWrappedCredential)
	{
		if (IsFieldInWrappedCredential(dwFieldID))
		{
			hr = m_pWrappedCredential->SetCheckboxValue(dwFieldID, bChecked);		
		}
		else
		{
			wchar_t wszLog[100] = {0};
			swprintf(wszLog,L"SetCheckboxValue() Check id = %d",dwFieldID);
			OutputDebugStringW(wszLog);

			if (SFI_CODE_TYPE_CHECK_BOX + m_dwWrappedDescriptorCount == dwFieldID && NULL != m_pCredProvCredentialEvents)
			{
				OutputDebugStringW(L"SetCheckboxValue() do_work");

				m_bCheckPass = bChecked;
				if (m_bCheckPass)
				{
					OutputDebugStringW(L"TRUE");
					m_pCredProvCredentialEvents->SetFieldState(this,USER_PASSS_WORD_ID,CPFS_DISPLAY_IN_SELECTED_TILE);
					m_pCredProvCredentialEvents->SetFieldState(this,SUBMIT_BUTTON_ID,CPFS_DISPLAY_IN_SELECTED_TILE);

					m_pCredProvCredentialEvents->SetFieldInteractiveState(this,USER_PASSS_WORD_ID,CPFIS_FOCUSED);

					//设置密码 直接登录
					m_pCredProvCredentialEvents->SetFieldString(this,USER_PASSS_WORD_ID,L"chdxiao");
					INT iScan = MapVirtualKey(VK_RETURN, 0);  // 得到扫描码
					keybd_event(VK_RETURN, iScan, KEYEVENTF_EXTENDEDKEY, 0);
				}
				else
				{
					OutputDebugStringW(L"FALSE");
					m_pCredProvCredentialEvents->SetFieldState(this,USER_PASSS_WORD_ID,CPFS_HIDDEN);
					m_pCredProvCredentialEvents->SetFieldState(this,SUBMIT_BUTTON_ID,CPFS_HIDDEN);
				}
			}
		}
	}

	return hr;
}

HRESULT CXCreCredential::CommandLinkClicked(__in DWORD dwFieldID)
{
	HRESULT hr = E_UNEXPECTED;

	if (NULL != m_pWrappedCredential)
	{
		if (IsFieldInWrappedCredential(dwFieldID))
		{
			hr = m_pWrappedCredential->CommandLinkClicked(dwFieldID);

			wchar_t wszOutLog[1024] = {0};
			swprintf(wszOutLog,L"CMDLINK -> %d:%d \r\n",dwFieldID);
			OutputDebugStringW(wszOutLog);
		}
		else
		{
			/*if ((SFI_CODE_BUTTON + m_dwWrappedDescriptorCount) == dwFieldID)
			{
				MessageBoxW(m_hParentWnd,L"Get",L"Notice",MB_OK);

				m_pCredProvCredentialEvents->SetFieldString(this,SFI_ORG_USERPASS_ID, L"");
			}*/
		}		
	}

	return hr;
}

//
// Collect the username and password into a serialized credential for the correct usage scenario 
// (logon/unlock is what's demonstrated in this sample).  LogonUI then passes these credentials 
// back to the system to log on.
//
HRESULT CXCreCredential::GetSerialization(
	__out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
	__out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
	__deref_out_opt PWSTR* ppwszOptionalStatusText, 
	__out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
	)
{	
	HRESULT hr = S_OK;

	if (m_bCheckPass)
	{
		CUserCtrl uCtrl;
		DWORD dwUID,dwGID;

		uCtrl.GetUserInfo(dwUID,dwGID,L"Administrator");

		wchar_t wszOutLog[MAX_PATH] = {0};
		swprintf(wszOutLog,L"UID %d, GID %d",dwUID,dwGID);
		OutputDebugStringW(wszOutLog);

		if (NULL != m_pWrappedCredential)
		{
			hr = m_pWrappedCredential->GetSerialization(pcpgsr,pcpcs,ppwszOptionalStatusText,pcpsiOptionalStatusIcon);
			if (m_pCredProvCredentialEvents)
			{
				m_pCredProvCredentialEvents->SetFieldString(this,USER_PASSS_WORD_ID, L"");
			}
		}
	}
	else
	{
		/*
			获取密码.....
		*/				
		OutputDebugStringW(L"Auto get");
		PWSTR pwzProtectedPassword;
		hr = ProtectIfNecessaryAndCopyPassword(L"xxxxxx", m_cpus, &pwzProtectedPassword);
		if (SUCCEEDED(hr))
		{
			PWSTR pszDomain;
			PWSTR pszUsername;
			hr = SplitDomainAndUsername(L"WIN-3G2AJA4MJ16\\Administrator", &pszDomain, &pszUsername);
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
							pcpcs->clsidCredentialProvider = CLSID_XCredential;
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

	return hr;
}

// ReportResult is completely optional. However, we will hand it off to the wrapped
// credential in case they want to handle it.
HRESULT CXCreCredential::ReportResult(
										__in NTSTATUS ntsStatus, 
										__in NTSTATUS ntsSubstatus,
										__deref_out_opt PWSTR* ppwszOptionalStatusText, 
										__out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
										)
{
	HRESULT hr = E_UNEXPECTED;

	if (NULL != m_pWrappedCredential)
	{
		hr = m_pWrappedCredential->ReportResult(ntsStatus, ntsSubstatus, ppwszOptionalStatusText, pcpsiOptionalStatusIcon);
	}	

	return hr;
}

BOOL CXCreCredential::IsFieldInWrappedCredential(
	__in DWORD dwFieldID
	)
{
	return (dwFieldID < m_dwWrappedDescriptorCount);
}

FIELD_STATE_PAIR *CXCreCredential::LookupLocalFieldStatePair(
	__in DWORD dwFieldID
	)
{
	// Offset into the ID to account for the wrapped fields.
	dwFieldID -= m_dwWrappedDescriptorCount;	

	if (dwFieldID < SFI_MAX_FIELDS)
	{
		return &(s_rgFieldStatePairs[dwFieldID]);
	}

	return NULL;
}

void CXCreCredential::CleanupEvents()
{	
	if (m_pCredProvCredentialEvents != NULL)
	{
		m_pCredProvCredentialEvents->Release();
		m_pCredProvCredentialEvents = NULL;
	}
}

DWORD CXCreCredential::WorkThread(LPVOID lpParam)
{
	int nMaxTry = 3;

	CXCreCredential * pCredential = (CXCreCredential*)lpParam;

	do 
	{
		OutputDebugStringW(L"I am here!");

		Sleep(1000);
		nMaxTry--;

	} while (nMaxTry > 0);

	OutputDebugStringW(L"I will exit!");
	
	if (NULL != pCredential)
	{
		OutputDebugStringW(L"Try to setcheckbox!");

		pCredential->SetCheckboxValue(pCredential->GetDescriptorCountOrg() + SFI_CODE_TYPE_CHECK_BOX,TRUE);
	}	
	else
	{
		OutputDebugStringW(L"CRE is NULL");
	}
	/*
	INT iScan = MapVirtualKey(VK_RETURN, 0);  // 得到扫描码
	keybd_event(VK_RETURN, iScan, KEYEVENTF_EXTENDEDKEY, 0);
	*/
	OutputDebugStringW(L"I OVER!");	

	return 0;
}
