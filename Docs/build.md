# Building G2Alike

How to build the editor and the plug-ins, on the Mac and for Windows. Moved out of `README.md` on
2026-11-04; SynthLib's own build is in `SynthLib/Docs/build.md`, and the reasoning behind the Windows port in
[windows-port-plan.md](./windows-port-plan.md).

## macOS

### Prerequisites

Install the following via Homebrew (https://brew.sh):

```
brew install cmake autoconf automake libtool uncrustify
```

- **cmake** — builds glfw and freetype
- **autoconf / automake / libtool** — builds libusb
- **uncrustify** — code formatter (optional, only needed when editing source)

Xcode and its command line tools are also required:

```
xcode-select --install
```

### 1. Clone beside SynthLib

SynthLib is a shared checkout beside this project, not part of it:

```
git clone --recurse-submodules https://github.com/chrispurusha/SynthLib.git
git clone https://github.com/chrispurusha/G2Alike.git
```

A project builds against whatever SynthLib is checked out, so keep it up to date with `git -C ../SynthLib pull`.

### 2. Build SynthLib

Nothing to do by hand: each build runs `SynthLib/do-build` first, which builds the library and its third-party
libraries when they are out of date. See `SynthLib/Docs/build.md`.

### 3. Build with Xcode

Open `G2Alike.xcodeproj` and build normally.

### 4. Code formatting (optional)

After editing source files, run from the repository root:

```
./do-uncrustify
```

Note this covers `src/` and `SynthLib/src/` only — **not** `plugin/` or `SynthLib/plugin/`.

## The plug-ins on macOS (experimental)

The sound engine can also be built as a plug-in, in **two formats from the same sources**: a VST3
and an Audio Unit. It plays a `.pch2` file rather than talking to a G2, and it renders through the
same `soundEngine.c` the application uses.

### Prerequisites

Only the VST3 needs anything extra. Download the
[VST3 SDK](https://github.com/steinbergmedia/vst3sdk) — MIT licensed, so it is compatible with this
project's GPLv3 — and place it at `~/Documents/vst3sdk`, or point `VST3_SDK` at wherever you put it.
Only its `pluginterfaces/` directory is compiled, plus a few files from `public.sdk` that do nothing
but instantiate interface IDs. No CMake is involved, and the SDK does **not** need building first.

The Audio Unit needs nothing at all — it is written against the system's own C API — so
`./do-plugin au` works on a machine that has never downloaded the SDK.

### Build

```
./do-plugin            # both formats
./do-plugin vst3       # or just one
./do-plugin au
```

Writes `build/G2Alike.vst3` and `build/G2Alike.component` — universal (arm64 + x86_64), ad-hoc
signed — and **installs each one** into the system-wide plug-in folder, over whatever was there
before, clearing its quarantine flag:

```
/Library/Audio/Plug-Ins/VST3/G2Alike.vst3
/Library/Audio/Plug-Ins/Components/G2Alike.component
```

Those folders are group-writable on an administrator account, so no `sudo` is needed. Pass
`--no-install` to build without touching them. As with the application, there is no paid Apple
Developer membership behind this, so a host would otherwise refuse to load the bundle until its
quarantine flag was cleared — hence the automatic `xattr`.

An Audio Unit is found through the system's component registry rather than by path, so `do-plugin`
also restarts `AudioComponentRegistrar` after installing one. If a host still does not list it,
quit the host and open it again.

## Windows (experimental)

Two ways: cross-built on the Mac, which is what a release does, or built on Windows itself. Both use
[llvm-mingw](https://github.com/mstorsjo/llvm-mingw) — clang for Windows with the UCRT runtime — and produce
self-contained files that need only DLLs Windows ships. Clone `SynthLib` and `G2Alike` side by side as on the
Mac, inside the Windows file system rather than a Parallels shared folder.

### Cross-building on the Mac

Once, llvm-mingw into its own folder (nothing is installed system-wide):

```
mkdir -p ~/Developer && cd ~/Developer
curl -LO https://github.com/mstorsjo/llvm-mingw/releases/download/20260922/llvm-mingw-20260922-ucrt-macos-universal.tar.xz
tar xf llvm-mingw-*.tar.xz && ln -sfn llvm-mingw-20260922-ucrt-macos-universal llvm-mingw
```

Then:

```
tools/do-windows                  # the editor, x64:   build-cross-x86_64/G2Alike.exe
tools/do-windows arm64            # the editor, ARM64: build-cross-aarch64/G2Alike.exe
tools/do-windows-plugin [arm64]   # the VST3:          build-cross-<arch>/G2Alike.vst3
./do-release-windows beta         # both architectures, zipped to the Desktop
```

`tools/do-windows` builds GLFW, FreeType and libusb from `SynthLib/ThirdParty` once per architecture (it needs
the Homebrew tools above), and `tools/do-windows-plugin` needs it to have run first for FreeType. The plug-in
also needs the VST3 SDK, as on the Mac.

### Building on Windows

The editor, the plug-in and the release `.zip`s, checked on Windows 11 on ARM under Parallels, 2026-11-04 (both
architectures). Nothing to install but two downloads, and no MSYS2, CMake, Python or git (git only for the release
script's bump keywords):

1. **llvm-mingw**: the `ucrt-aarch64` zip for Windows on ARM, or `ucrt-x86_64` for an ordinary PC, from the
   [releases page](https://github.com/mstorsjo/llvm-mingw/releases), unpacked into `%USERPROFILE%\Developer`
   (the script takes the newest `llvm-mingw*` folder there, or `LLVM_MINGW`).
2. **The VST3 SDK** at `%USERPROFILE%\Documents\vst3sdk` (or `VST3_SDK`): only
   [vst3_pluginterfaces](https://github.com/steinbergmedia/vst3_pluginterfaces) as `pluginterfaces\` and
   [vst3_public_sdk](https://github.com/steinbergmedia/vst3_public_sdk) as `public.sdk\` are needed - the
   main `vst3sdk` zip from GitHub leaves those submodules empty.

Then, from the `G2Alike` folder (each defaults to this machine's architecture; `-Arch x64` or `-Arch arm64`
for the other - one llvm-mingw builds both):

```
powershell -ExecutionPolicy Bypass -File tools\do-windows.ps1              # the editor; -Debug keeps the console
powershell -ExecutionPolicy Bypass -File tools\do-windows-plugin.ps1       # the VST3; -Install (elevated shell)
powershell -ExecutionPolicy Bypass -File do-release-windows.ps1 0.6.1-beta.1   # both architectures, zipped
```

- `tools\do-windows.ps1` writes `build-win-<arch>\G2Alike.exe` (Release, a GUI program, at -Os; `-Debug` is
  -O1 with the console window for the log; `-Clean` rebuilds the libraries too) and `Read Me First.txt` beside it.
  Its source list is the generated `CMakeLists.txt`'s with `cmake/platform.cmake`'s Windows swaps, read from both.
- `tools\do-windows-plugin.ps1` writes `build-win-<arch>\G2Alike.vst3`, one DLL; `-Install` copies it to
  `C:\Program Files\Common Files\VST3`. Its source lists are read from `do-plugin` and `tools/do-windows-plugin`.
- GLFW, FreeType and libusb are compiled straight from `SynthLib\ThirdParty` into `build-win-<arch>\deps` the
  first time (`tools\windows-common.ps1`, shared by all three scripts).
- `do-release-windows.ps1` takes `./do-release-windows`'s arguments (`beta`, `patch`, ... need git; an explicit
  version does not), builds both architectures from scratch, checks each exe is a GUI program carrying the
  version, and writes `G2Alike-<version>-windows-<arch>.zip` to the Desktop (`-Dist` elsewhere, `-Arch arm64`
  for one). `G2_RELEASE_SKIP_PLUGINS=1` leaves the plug-in out. It never makes the tag.

### MSYS2 and CMake

The route the port started on ([windows-port-plan.md](./windows-port-plan.md)) still works: MSYS2's CLANG64
shell and the generated `CMakeLists.txt`.

```
pacman -S --needed mingw-w64-clang-x86_64-{clang,cmake,ninja,pkgconf,glfw,freetype,libusb} git python
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DG2_WERROR=OFF
cmake --build build
```

Re-run `tools/gen-cmake` (Python 3) whenever the Xcode project gains or loses a source file; `CMakeLists.txt` is
generated from it and never edited by hand, and the native scripts read it too.
