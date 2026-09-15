#pragma once

#include <windows.h>
#include <processthreadsapi.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include "defs.h"
#include "menu_item.h"
#include "win11_menu.h"

#pragma comment(lib, "shlwapi.lib")

// UTF-8 转 UTF-16
inline std::wstring toWide(const std::string& value) {
    if (value.empty())
        return {};

    int size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);

    if (size <= 0)
        return {};

    std::wstring result(size, L'\0');

    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, result.data(), size);

    result.resize(size - 1);

    return result;
}

// ============================================================
// 子菜单 Command
// ============================================================

class SubCommand : public IExplorerCommand {
public:
    explicit SubCommand(const std::wstring& title, const std::wstring& command)
        : title_(title), command_(command) {}

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
        return InterlockedIncrement(&refCount_);
    }

    ULONG STDMETHODCALLTYPE Release() override {
        ULONG count = InterlockedDecrement(&refCount_);

        if (count == 0)
            delete this;

        return count;
    }

    HRESULT STDMETHODCALLTYPE GetTitle(IShellItemArray*, LPWSTR* title) override {
        if (!title)
            return E_POINTER;

        return SHStrDupW(title_.c_str(), title);
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

        *name = PROJECT_CLSID;
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

        std::wstring command = parseCommand(items);

        if (command.empty())
            return E_INVALIDARG;

        if (!execute(command))
            return HRESULT_FROM_WIN32(GetLastError());

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
    std::wstring title_;
    std::wstring command_;

    bool execute(std::wstring& command) {
        STARTUPINFOW si{};
        si.cb = sizeof(si);

        PROCESS_INFORMATION pi{};

        BOOL result = CreateProcessW(
            nullptr,
            command.data(),
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            nullptr,
            &si,
            &pi
        );

        if (!result)
            return false;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return true;
    }

    std::wstring parseCommand(IShellItemArray* items) {
        if (!items)
            return {};

        DWORD count = 0;

        if (FAILED(items->GetCount(&count)))
            return {};

        std::vector<std::wstring> paths;

        for (DWORD i = 0; i < count; ++i) {
            IShellItem* item = nullptr;

            if (FAILED(items->GetItemAt(i, &item)))
                continue;

            PWSTR widePath = nullptr;

            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &widePath))) {
                paths.emplace_back(widePath);

                CoTaskMemFree(widePath);
            }

            item->Release();
        }

        return substitute(command_, paths);
    }

    // 替换 %1、%2 ... 和 %*，插入的路径不会被再次替换
    std::wstring substitute(const std::wstring& command, const std::vector<std::wstring>& paths) {
        std::wstring result;

        for (size_t i = 0; i < command.size();) {
            if (command[i] != L'%' || i + 1 >= command.size()) {
                result += command[i];
                ++i;
                continue;
            }

            wchar_t next = command[i + 1];

            if (next == L'*') {
                for (size_t n = 0; n < paths.size(); ++n) {
                    if (n > 0)
                        result += L' ';

                    result += quote(paths[n]);
                }

                i += 2;
                continue;
            }

            if (next >= L'0' && next <= L'9') {
                size_t end = i + 1;
                size_t value = 0;

                while (end < command.size() && command[end] >= L'0' && command[end] <= L'9') {
                    value = value * 10 + static_cast<size_t>(command[end] - L'0');
                    ++end;
                }

                if (value >= 1 && value <= paths.size()) {
                    result += quote(paths[value - 1]);
                    i = end;
                    continue;
                }
            }

            result += command[i];
            ++i;
        }

        return result;
    }

    std::wstring quote(const std::wstring& value) {
        return L"\"" + value + L"\"";
    }
};


// ============================================================
// IEnumExplorerCommand
// ============================================================

class CommandEnumerator : public IEnumExplorerCommand {
public:
    CommandEnumerator() {
        std::vector<MenuItem> menu_items = Win11Menu::get_items();

        for (const auto& mi : menu_items) {
            commands_.push_back(new SubCommand(
                toWide(mi.label),
                toWide(mi.command)
            ));
        }
    }

    ~CommandEnumerator() {
        for (auto* command : commands_)
            command->Release();
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv)
            return E_POINTER;

        *ppv = nullptr;

        if (riid == IID_IUnknown || riid == __uuidof(IEnumExplorerCommand)) {
            *ppv = static_cast<IEnumExplorerCommand*>(this);
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return InterlockedIncrement(&refCount_);
    }

    ULONG STDMETHODCALLTYPE Release() override {
        ULONG count = InterlockedDecrement(&refCount_);

        if (count == 0)
            delete this;

        return count;
    }

    HRESULT STDMETHODCALLTYPE Next(ULONG celt, IExplorerCommand** rgelt, ULONG* pceltFetched) override {
        if (!rgelt)
            return E_POINTER;

        if (pceltFetched)
            *pceltFetched = 0;

        ULONG fetched = 0;

        while (fetched < celt && index_ < commands_.size()) {
            rgelt[fetched] = commands_[index_];
            commands_[index_]->AddRef();

            ++index_;
            ++fetched;
        }

        if (pceltFetched)
            *pceltFetched = fetched;

        return fetched == celt ? S_OK : S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE Skip(ULONG celt) override {
        index_ += celt;

        if (index_ > commands_.size())
            index_ = commands_.size();

        return index_ == commands_.size() ? S_FALSE : S_OK;
    }

    HRESULT STDMETHODCALLTYPE Reset() override {
        index_ = 0;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Clone(IEnumExplorerCommand** ppenum) override {
        if (!ppenum)
            return E_POINTER;

        *ppenum = nullptr;

        auto* clone = new CommandEnumerator();
        clone->index_ = index_;

        *ppenum = clone;

        return S_OK;
    }

private:
    ULONG refCount_ = 1;
    std::vector<IExplorerCommand*> commands_;
    size_t index_ = 0;
};


// ============================================================
// 父菜单 Command
// ============================================================

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
        return InterlockedIncrement(&refCount_);
    }

    ULONG STDMETHODCALLTYPE Release() override {
        ULONG count = InterlockedDecrement(&refCount_);

        if (count == 0)
            delete this;

        return count;
    }

    HRESULT STDMETHODCALLTYPE GetTitle(IShellItemArray*, LPWSTR* title) override {
        if (!title)
            return E_POINTER;

        return SHStrDupW(L"ContextForge", title);
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

        *name = PROJECT_CLSID;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetState(IShellItemArray*, BOOL, EXPCMDSTATE* state) override {
        if (!state)
            return E_POINTER;

        *state = ECS_ENABLED;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Invoke(IShellItemArray*, IBindCtx*) override {
        // 父菜单一般不会执行 Invoke
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE GetFlags(EXPCMDFLAGS* flags) override {
        if (!flags)
            return E_POINTER;

        // ★ 告诉 Explorer：这个 Command 有子菜单
        *flags = ECF_HASSUBCOMMANDS;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE EnumSubCommands(IEnumExplorerCommand** commands) override {
        if (!commands)
            return E_POINTER;

        *commands = nullptr;

        auto* enumerator = new CommandEnumerator();
        *commands = enumerator;

        return S_OK;
    }

private:
    ULONG refCount_ = 1;
};
