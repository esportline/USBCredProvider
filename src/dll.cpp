//
// Created by enimaloc on 22/11/2025.
//

#include <windows.h>
#include <unknwn.h>
#include "guid.h"
#include "dll.h"
#include "helpers.h"

static long g_cRef = 0;
HINSTANCE g_hinst = NULL;

extern HRESULT CSample_CreateInstance(_In_opt_ REFIID riid, _Outptr_ void **ppv);

class CClassFactory : public IClassFactory {
public:
    CClassFactory() : _cRef(1) {
    }

    IFACEMETHODIMP QueryInterface(_In_opt_ REFIID riid, _Outptr_ void **ppv) override {
        static const QITAB qit[] = {
            QITABENT(CClassFactory, IClassFactory),
            {0},
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef() {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release() {
        long cRef = InterlockedDecrement(&_cRef);
        if (!cRef) {
            delete this;
        }
        return cRef;
    }

    IFACEMETHODIMP CreateInstance(_In_opt_ IUnknown *pUnkOuter, _In_opt_ REFIID riid, _Outptr_ void **ppv) {
        HRESULT hr;
        if (!pUnkOuter) {
            hr = CSample_CreateInstance(riid, ppv);
        } else {
            *ppv = NULL;
            hr = CLASS_E_NOAGGREGATION;
        }
        return hr;
    }

    IFACEMETHODIMP LockServer(_In_opt_ BOOL block) {
        if (block) {
            DllAddRef();
        } else {
            DllRelease();
        }
        return S_OK;
    }

private:
    ~CClassFactory() {
    }

    long _cRef;
};

HRESULT CClassFactory_CreateInstance(_In_opt_ REFCLSID rclsid, _In_opt_ REFIID riid, _Outptr_ void **ppv) {
    *ppv = NULL;
    HRESULT hr;
    if (CLSID_CSample == rclsid) {
        CClassFactory *pcf = new CClassFactory();
        if (pcf) {
            hr = pcf->QueryInterface(riid, ppv);
            pcf->Release();
        } else {
            hr = E_OUTOFMEMORY;
        }
    } else {
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }
    return hr;
}

void DllAddRef() {
    InterlockedIncrement(&g_cRef);
}

void DllRelease() {
    InterlockedDecrement(&g_cRef);
}

STDAPI DllCanUnloadNow() {
    return (g_cRef > 0) ? S_FALSE : S_OK;
}

STDAPI DllGetClassObject(_In_opt_ REFCLSID rclsid, _In_opt_ REFIID riid, _Outptr_ void **ppv) {
    return CClassFactory_CreateInstance(rclsid, riid, ppv);
}

STDAPI_(BOOL) DllMain(_In_opt_ HINSTANCE hinstDll, _In_opt_ DWORD dwReason, _In_opt_ void *) {
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDll);
            break;
        default:
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    g_hinst = hinstDll;
    return TRUE;
}
