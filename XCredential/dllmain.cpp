// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "Credential_Imp.h"

static LONG g_cRef = 0;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hThisMod = hModule;

			DisableThreadLibraryCalls(hModule);
			break;
		}

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

/*
	CClassFactory µœ÷ IClassFactory.
*/
class CClassFactory : public IClassFactory
{
public:
	// IUnknown
	STDMETHOD_(ULONG,AddRef)()
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
			if (IID_IClassFactory == riid || IID_IUnknown == riid)
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
		}
		else
		{
			hr = E_INVALIDARG;
		}

		return hr;
	}

	// IClassFactory
	STDMETHOD (CreateInstance)(IUnknown* pUnkOuter, REFIID riid, void** ppv)
	{
		HRESULT hr;
		if (!pUnkOuter)
		{			
			wchar_t wszGUID[1024] = {0};
			swprintf_s(wszGUID,1024,L"{%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}",riid.Data1,riid.Data2,riid.Data3, 
				riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],  
				riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);

#ifdef _DEBUG
		DebugBreak();
#endif

			if (IID_ICredentialProvider == riid)
			{
				hr = XCreProvider_CreateInstance(riid,ppv);
			}
			else if (IID_ICredentialProviderFilter == riid)
			{
				hr = XCreFilter_CreateInstance(riid,ppv);
			}
		}
		else
		{
			hr = CLASS_E_NOAGGREGATION;
		}

		return hr;
	}

	STDMETHOD (LockServer)(BOOL bLock)
	{
		if (bLock)
		{
			DllAddRef();
		}
		else
		{
			DllRelease();
		}

		return S_OK;
	}

private:
	CClassFactory() : m_cRef(1) {}
	~CClassFactory(){}

private:
	LONG m_cRef;

	friend HRESULT CClassFactory_CreateInstance(REFCLSID rclsid, REFIID riid, void** ppv);
};

HRESULT CClassFactory_CreateInstance(REFCLSID rclsid, REFIID riid, void** ppv)
{
	HRESULT hr;
	if (CLSID_XCredential == rclsid)
	{		
		CClassFactory* pcf = new CClassFactory();
		if (NULL != pcf)
		{
			hr = pcf->QueryInterface(riid,ppv);
			pcf->Release();
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		hr = CLASS_E_CLASSNOTAVAILABLE;
	}

	return hr;
}

//DLL entry point.
STDAPI DllCanUnloadNow()
{	
	HRESULT hr;
	if (g_cRef > 0)
	{
		hr = S_FALSE;   // cocreated objects still exist, don't unload
	}
	else
	{
		hr = S_OK;      // refcount is zero, ok to unload
	}

	return hr;
}

// DLL entry point.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
	return CClassFactory_CreateInstance(rclsid,riid,ppv);
}

void DllAddRef()
{
	InterlockedIncrement(&g_cRef);
}

void DllRelease()
{
	InterlockedDecrement(&g_cRef);
}