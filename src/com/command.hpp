#pragma once

#include <windows.h>
#include <processthreadsapi.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include "defs.h"

#pragma comment(lib, "shlwapi.lib")

// ============================================================
// 子菜单 Command
// ============================================================

class SubCommand : public IExplorerCommand {
public:
    explicit SubCommand(const char* title, const char* command) : title_(title), command_(command) {}

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

        return SHStrDupA(title_.c_str(), title);
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

        std::string command = parseCommand(items);

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
    std::string title_;
    std::string command_;

    bool execute(std::string& command) {
        STARTUPINFOA si{};
        si.cb = sizeof(si);

        PROCESS_INFORMATION pi{};

        BOOL result = CreateProcessA(
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

    std::string parseCommand(IShellItemArray* items) {
        if (!items)
            return {};

        DWORD count = 0;

        if (FAILED(items->GetCount(&count)))
            return {};

        std::vector<std::string> paths;

        for (DWORD i = 0; i < count; ++i) {
            IShellItem* item = nullptr;

            if (FAILED(items->GetItemAt(i, &item)))
                continue;

            PWSTR widePath = nullptr;

            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &widePath))) {
                int size = WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    widePath,
                    -1,
                    nullptr,
                    0,
                    nullptr,
                    nullptr
                );

                if (size > 0) {
                    std::string path(size - 1, '\0');

                    WideCharToMultiByte(
                        CP_UTF8,
                        0,
                        widePath,
                        -1,
                        path.data(),
                        size,
                        nullptr,
                        nullptr
                    );

                    paths.emplace_back(std::move(path));
                }

                CoTaskMemFree(widePath);
            }

            item->Release();
        }

        std::string command = command_;

        auto replaceAll = [](std::string& str,
                            const std::string& from,
                            const std::string& to) {
            if (from.empty())
                return;

            size_t pos = 0;

            while ((pos = str.find(from, pos)) != std::string::npos) {
                str.replace(pos, from.length(), to);
                pos += to.length();
            }
        };

        // %1, %2, %3 ...
        for (size_t i = 0; i < paths.size(); ++i) {
            replaceAll(
                command,
                "%" + std::to_string(i + 1),
                "\"" + paths[i] + "\""
            );
        }

        // %*
        std::string allPaths;

        for (const auto& path : paths) {
            if (!allPaths.empty())
                allPaths += ' ';

            allPaths += "\"" + path + "\"";
        }

        replaceAll(command, "%*", allPaths);

        return command;
    }
};


// ============================================================
// IEnumExplorerCommand
// ============================================================

class CommandEnumerator : public IEnumExplorerCommand {
public:
    CommandEnumerator() {
        commands_.push_back(new SubCommand(
            "Open", 
            R"xxx(powershell -Command "Add-Type -AssemblyName PresentationFramework; [System.Windows.MessageBox]::Show('Hello open %1')")xxx"
        ));
        commands_.push_back(new SubCommand(
            "Open with Notepad",
            R"xxx(powershell -Command "Add-Type -AssemblyName PresentationFramework; [System.Windows.MessageBox]::Show('Notepad')")xxx"
        ));
        commands_.push_back(new SubCommand(
            "Something else",
            R"xxx(powershell -Command "Add-Type -AssemblyName PresentationFramework; [System.Windows.MessageBox]::Show('Something else')")xxx"
        ));
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
