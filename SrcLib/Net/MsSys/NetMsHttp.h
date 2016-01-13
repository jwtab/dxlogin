
#ifndef NET_MS_HTTP_H_
#define NET_MS_HTTP_H_

#include <urlmon.h>
#include <WinInet.h>

#pragma comment(lib,"Wininet.lib")

//urlmon.dllµ¼³öº¯Êý.
typedef HRESULT (WINAPI *FUN_URLDownloadToFileW)(LPUNKNOWN,LPCTSTR,LPCTSTR,DWORD,LPBINDSTATUSCALLBACK); 

class CHttpFileCallback : public IBindStatusCallback
	,public IServiceProvider
	,public IHttpNegotiate

{
private:
	HMODULE m_hDll;
	FUN_URLDownloadToFileW m_pDownLoad;

	wchar_t m_wszFilePath[MAX_PATH];	

public:
	CHttpFileCallback(void)
	{
		m_pDownLoad = NULL;		
		m_hDll = LoadLibraryW(L"urlmon.dll");
		if (NULL != m_hDll)
		{
			m_pDownLoad = (FUN_URLDownloadToFileW)GetProcAddress(m_hDll,"URLDownloadToFileW");
		}
	};

	~CHttpFileCallback(void)
	{
		if (NULL != m_hDll)
		{
			FreeLibrary(m_hDll);
			m_hDll = NULL;
		}
	};

	BOOL GetTagetUrlFile(const wchar_t * url,const wchar_t * file_path)
	{
		int nMaxTry = 3;
		BOOL ret_fun = FALSE;
		HRESULT hr = S_OK;

		ZeroMemory(m_wszFilePath,MAX_PATH);

		wcscpy(m_wszFilePath,file_path);

		for(int index = 0; index < nMaxTry; index++)
		{
			//É¾³ý»º´æ.
			DeleteUrlCacheEntryW(url);

			if (NULL != m_pDownLoad)
			{
				hr = m_pDownLoad(NULL,url,file_path,0,this);
				if (SUCCEEDED(hr))
				{	
					ret_fun = true;
					break;
				}
				else
				{
					Sleep(1500);
				}			
			}
			else
			{
				break;
			}
		}

		return ret_fun;
	};

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		*ppv = NULL;

		if (IID_IServiceProvider == riid)
		{
			*ppv = static_cast<IServiceProvider*>(this);
		}
		else if (IID_IHttpNegotiate == riid)
		{
			*ppv = static_cast<IHttpNegotiate*>(this);
		}

		if ( NULL != *ppv )
		{
			((LPUNKNOWN)*ppv)->AddRef();
			return NOERROR;
		}

		return E_NOINTERFACE;
	};

	ULONG m_lRef;
	STDMETHODIMP_(ULONG) AddRef(void) 
	{
		InterlockedIncrement(&m_lRef);
		return m_lRef;
	}

	STDMETHODIMP_(ULONG) Release(void) 
	{
		if (!InterlockedDecrement(&m_lRef)) 
		{
			AddRef();	//prevent delete this run twice, when AddRef/Release occur between deconstruction.
			//delete this;
			return 0;
		}

		return m_lRef;
	}

	//IServiceProvider
	STDMETHODIMP QueryService( 
		/* [in] */ __in REFGUID guidService,
		/* [in] */ __in REFIID riid,
		/* [out] */ __deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		if (guidService == IID_IHttpNegotiate && riid == IID_IHttpNegotiate)
		{
			*ppvObject = this;
			return NOERROR;
		}

		return E_NOINTERFACE;
	};

	//IBindStatusCallback
	HRESULT STDMETHODCALLTYPE OnStartBinding( 
		/* [in] */ DWORD dwReserved,
		/* [in] */ __RPC__in_opt IBinding *pib)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetPriority( 
		/* [out] */ __RPC__out LONG *pnPriority)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE OnLowResource( 
		/* [in] */ DWORD reserved)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE OnProgress( 
		/* [in] */ ULONG ulProgress,
		/* [in] */ ULONG ulProgressMax,
		/* [in] */ ULONG ulStatusCode,
		/* [unique][in] */ __RPC__in_opt LPCWSTR szStatusText)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnStopBinding( 
		/* [in] */ HRESULT hresult,
		/* [unique][in] */ __RPC__in_opt LPCWSTR szError)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetBindInfo( 
		/* [out] */ DWORD *grfBINDF,
		/* [unique][out][in] */ BINDINFO *pbindinfo)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE OnDataAvailable( 
		/* [in] */ DWORD grfBSCF,
		/* [in] */ DWORD dwSize,
		/* [in] */ FORMATETC *pformatetc,
		/* [in] */ STGMEDIUM *pstgmed)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable( 
		/* [in] */ __RPC__in REFIID riid,
		/* [iid_is][in] */ __RPC__in_opt IUnknown *punk)
	{
		return E_NOTIMPL;
	}

	//IHttpNegotiate
	virtual HRESULT STDMETHODCALLTYPE BeginningTransaction( 
		/* [in] */ __RPC__in LPCWSTR szURL,
		/* [unique][in] */ __RPC__in_opt LPCWSTR szHeaders,
		/* [in] */ DWORD dwReserved,
		/* [out] */ __RPC__deref_out_opt LPWSTR *pszAdditionalHeaders)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnResponse( 
		/* [in] */ DWORD dwResponseCode,
		/* [unique][in] */ __RPC__in_opt LPCWSTR szResponseHeaders,
		/* [unique][in] */ __RPC__in_opt LPCWSTR szRequestHeaders,
		/* [out] */ __RPC__deref_out_opt LPWSTR *pszAdditionalRequestHeaders)
	{
		return E_NOTIMPL;
	}
};


#endif