#include "command.hpp"

class ClassFactory : public IClassFactory {
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv)
            return E_POINTER;

        *ppv = nullptr;

        if (riid == IID_IUnknown || riid == IID_IClassFactory) {
            *ppv = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return InterlockedIncrement(reinterpret_cast<LONG*>(&refCount_));
    }

    ULONG STDMETHODCALLTYPE Release() override {
        ULONG count = InterlockedDecrement(reinterpret_cast<LONG*>(&refCount_));

        if (count == 0)
            delete this;

        return count;
    }

    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* outer, REFIID riid, void** object) override {
        if (!object)
            return E_POINTER;

        *object = nullptr;

        if (outer)
            return CLASS_E_NOAGGREGATION;

        Command* command = new Command();

        HRESULT hr = command->QueryInterface(riid, object);

        command->Release();

        return hr;
    }

    HRESULT STDMETHODCALLTYPE LockServer(BOOL) override {
        return S_OK;
    }

private:
    ULONG refCount_ = 1;
};

// DllGetClassObject / DllCanUnloadNow are already declared by the Windows SDK
// headers (combaseapi.h) as plain `extern "C"`, so adding __declspec(dllexport)
// on the definitions triggers:
//   warning: redeclaration of '...' should not add 'dllexport' attribute
// Export them with a linker directive instead.
#ifndef _M_IX86
#pragma comment(linker, "/export:DllGetClassObject")
#pragma comment(linker, "/export:DllCanUnloadNow")
#else
#pragma comment(linker, "/export:DllGetClassObject=_DllGetClassObject@12")
#pragma comment(linker, "/export:DllCanUnloadNow=_DllCanUnloadNow@0")
#endif

extern "C" HRESULT WINAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv) {
    if (!IsEqualCLSID(clsid, PROJECT_CLSID))
        return CLASS_E_CLASSNOTAVAILABLE;

    ClassFactory* factory = new ClassFactory();

    HRESULT hr = factory->QueryInterface(riid, ppv);

    factory->Release();

    return hr;
}

extern "C" HRESULT WINAPI DllCanUnloadNow() {
    return S_FALSE;
}

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID) {
    return TRUE;
}