# ContextForge

A small command-line tool that adds custom entries to the Windows File Explorer context menu.

ContextForge supports two different menu backends:

- **`win10`** – classic shell menu. Entries are written to the current user's registry
  (`HKCU\Software\Classes\...\shell`), so they work on Windows 10 (and on Windows 11 in the
  legacy menu).
- **`win11`** – the modern Windows 11 context menu. Entries are stored in a JSON config file and
  exposed through a COM `IExplorerCommand` handler that is packaged as a lightweight MSIX/AppX
  package.

Each entry has a label, a program, and arguments. The arguments may contain `%1`, `%2`, ... and `%*`
placeholders, which are replaced with the paths of the selected files/folders when the entry is
invoked. `%*` expands to all selected paths at once. The program path is quoted automatically, so it
does not need to be quoted on the command line.

## Requirements

- Windows 10 1809+ / Windows 11
- [CMake](https://cmake.org/) 3.16 or newer
- A C++20 compiler — [Clang/LLVM](https://clang.llvm.org/) 17+ targeting MinGW-w64
  (e.g. [LLVM-MinGW](https://github.com/mstorsjo/llvm-mingw) or MSYS2's `clang`)
- [Ninja](https://ninja-build.org/)
- [Git](https://git-scm.com/install/)

Make sure `cmake`, `ninja`, `clang++` and `git` are available on your `PATH`.

## Build

```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++
cmake --build build --parallel 4
```

The `-DCMAKE_CXX_COMPILER=clang++` is only needed if another compiler (e.g. GCC or MSVC) is also on
your `PATH`; CMake otherwise picks it up automatically.

The build produces two artifacts in the `bin/` directory:

- `ContextForge.exe` – the command-line tool
- `ContextForgeCOM.dll` – the COM server used by the Windows 11 menu

`AppxManifest.xml` and `logo.png` are copied into `bin/` automatically, so the folder is ready to be
registered as an AppX package.

> Equivalent commands are available as VS Code tasks (`configure` and `build`) in
> `.vscode/tasks.json`.

## Usage

### 1. Install the AppX package (Windows 11 menu only)

The `win11` mode needs a one-time package registration so that File Explorer can load the COM
server. This step requires **Developer Mode**:

1. Open **Settings → System → For developers** and turn on **Developer Mode**.
2. Open PowerShell **in the `bin/` directory** (where `AppxManifest.xml` was copied) and run:

   ```powershell
   Add-AppxPackage -Register .\AppxManifest.xml
   ```

3. You can turn **Developer Mode** off again once the command has finished.

If the package is not installed, the tool prints these instructions for you.

### 2. Uninstall the AppX package (Windows 11 menu only)

The package is registered under the publisher `ContextForge`, so you can look it up and see its
`Name` with:

```powershell
Get-AppxPackage -Publisher *ContextForge*
```

To remove it, unregister it directly by the `Name` (the `Identity` name from `AppxManifest.xml`):

```powershell
Get-AppxPackage -Name d585db51-2e03-4e89-9b4c-080885988226 | Remove-AppxPackage
```

Or filter by publisher and remove it in a single step:

```powershell
Get-AppxPackage -Publisher *ContextForge* | Remove-AppxPackage
```

Either way this removes the Windows 11 context menu from File Explorer.

**Developer Mode is not required** to uninstall. `win11_menu.json` is left untouched — delete it
manually if you also want to discard your saved entries.

### 3. Manage entries

Add an entry:

```powershell
# Windows 11 modern menu
.\bin\ContextForge.exe add --mode win11 "Open in VS Code" "code" "%1"

# Classic shell menu
.\bin\ContextForge.exe add --mode win10 "Open in VS Code" "code" "%1"
```

The last argument holds the arguments passed to the program. Use `%1 %2` to forward individual
selected paths, or `%*` to forward all selected paths at once:

```powershell
.\bin\ContextForge.exe add --mode win10 "this is a label" "path/to/program.exe" "%1 %2"
.\bin\ContextForge.exe add --mode win10 "this is a label" "path/to/program.exe" "%*"
```

If the program lives somewhere other than your `PATH`, just pass its path — quoting is handled for
you:

```bat
.\bin\ContextForge.exe add --mode win11 "Open in Program" "D:\Program Files\program\program.exe" "%*"
```

List all entries of a mode:

```powershell
.\bin\ContextForge.exe list --mode win11
```

Remove an entry by its name:

```powershell
.\bin\ContextForge.exe remove --mode win11 ContextForge_xxxxxxxx
```

`add` generates a unique name (`ContextForge_` plus a random suffix); use `list` to obtain the exact
name before removing.

### Options

| Subcommand | Option            | Description                              |
| ---------- | ----------------- | ---------------------------------------- |
| `add`      | `--mode, -m`      | Menu backend: `win10` or `win11`         |
| `add`      | `label`           | Text shown in the context menu           |
| `add`      | `program`         | Program to run (quoted automatically)    |
| `add`      | `args`            | Arguments for the program (supports `%1`, `%*`, ...)|
| `remove`   | `--mode, -m`      | Menu backend: `win10` or `win11`         |
| `remove`   | `name`            | Entry name as returned by `list`         |
| `list`     | `--mode, -m`      | Menu backend: `win10` or `win11`         |

## How it works

- `win10` entries live directly in the registry under `HKCU\Software\Classes\*\shell`.
- `win11` entries are stored in `win11_menu.json` next to the executable (created on first use). The
  packaged `ContextForgeCOM.dll` implements `IExplorerCommand`; `EnumSubCommands` reads the JSON file
  and builds one submenu item per entry, replacing the placeholders in `args` with the selected paths
  before launching `program`.

## License

See [LICENSE](LICENSE).
