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

extern HRESULT CSample_CreateInstance(REFIID riid, void **ppv);

class CClassFactory : public IClassFactory {
public:
    CClassFactory() : _cRef(1) {
    }

    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv) override {
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

    IFACEMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv) {
        HRESULT hr;
        if (!pUnkOuter) {
            hr = CSample_CreateInstance(riid, ppv);
        } else {
            *ppv = NULL;
            hr = CLASS_E_NOAGGREGATION;
        }
        return hr;
    }

    IFACEMETHODIMP LockServer(BOOL block) {
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

HRESULT CClassFactory_CreateInstance(REFCLSID rclsid, REFIID riid, void **ppv) {
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

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv) {
    return CClassFactory_CreateInstance(rclsid, riid, ppv);
}

STDAPI_(BOOL) DllMain(HINSTANCE hinstDll, DWORD dwReason, void *) {
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
