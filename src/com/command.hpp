#pragma once

#include <windows.h>
#include <shobjidl.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

inline const GUID CLSID_Command = {
    0x12345678,
    0x1234,
    0x5678,
    {0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef}
};

class Command : public IExplorerCommand {
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv)
            return E_POINTER;

        *ppv = nullptr;

        if (riid == IID_IUnknown || riid == __uuidof(IExplorerCommand)) {
            *ppv = static_cast<IExplorerCommand*>(this);
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

    HRESULT STDMETHODCALLTYPE GetTitle(IShellItemArray*, LPWSTR* title) override {
        if (!title)
            return E_POINTER;

        return SHStrDupW(L"ContextForge Command", title);
    }

    HRESULT STDMETHODCALLTYPE GetIcon(IShellItemArray*, LPWSTR* icon) override {
        if (!icon)
            return E_POINTER;

        *icon = nullptr;

        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE GetToolTip(IShellItemArray*, LPWSTR* tooltip) override {
        if (!tooltip)
            return E_POINTER;

        *tooltip = nullptr;

        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE GetCanonicalName(GUID* name) override {
        if (!name)
            return E_POINTER;

        *name = CLSID_Command;

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetState(IShellItemArray*, BOOL, EXPCMDSTATE* state) override {
        if (!state)
            return E_POINTER;

        *state = ECS_ENABLED;

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Invoke(IShellItemArray* items, IBindCtx*) override {
        if (!items)
            return E_INVALIDARG;

        MessageBoxW(nullptr, L"Hello from MyCommand!", L"My Menu", MB_OK);

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetFlags(EXPCMDFLAGS* flags) override {
        if (!flags)
            return E_POINTER;

        *flags = ECF_DEFAULT;

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE EnumSubCommands(IEnumExplorerCommand** commands) override {
        if (!commands)
            return E_POINTER;

        *commands = nullptr;

        return E_NOTIMPL;
    }

private:
    ULONG refCount_ = 1;
};