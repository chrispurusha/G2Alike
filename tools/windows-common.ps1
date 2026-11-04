# windows-common.ps1 - what the native Windows build scripts share; dot-sourced, not run.
# tools\do-windows.ps1 (the editor), tools\do-windows-plugin.ps1 (the VST3), do-release-windows.ps1.
# See Docs/build.md.

$ErrorActionPreference = 'Stop'

$Root = Split-Path -Parent $PSScriptRoot    # this file's folder, tools\, wherever it is sourced from
$SynthLib = Join-Path (Split-Path -Parent $Root) 'SynthLib'
$Third = "$SynthLib\ThirdParty"

function Get-Toolchain([string]$arch) {
    $llvm = $env:LLVM_MINGW
    if (-not $llvm) {
        $llvm = Get-ChildItem "$env:USERPROFILE\Developer" -Directory -Filter 'llvm-mingw*' -ErrorAction SilentlyContinue |
                Sort-Object Name -Descending | Select-Object -First 1 -ExpandProperty FullName
    }
    $triple = if ($arch -eq 'arm64') { 'aarch64-w64-mingw32' } else { 'x86_64-w64-mingw32' }
    $t = [pscustomobject]@{
        Llvm   = $llvm
        Triple = $triple
        CC     = "$llvm\bin\$triple-clang.exe"
        CXX    = "$llvm\bin\$triple-clang++.exe"
        AR     = "$llvm\bin\llvm-ar.exe"
        Objdump = "$llvm\bin\llvm-objdump.exe"
        Out    = Join-Path $Root "build-win-$arch"
    }
    if (-not (Test-Path $t.CC)) { throw "no llvm-mingw at '$llvm' - set LLVM_MINGW (Docs/build.md)" }
    New-Item -ItemType Directory -Force "$($t.Out)\deps" | Out-Null
    return $t
}

function Quote($s) { if ($s -match '[\s"]') { '"' + ($s -replace '"', '\"') + '"' } else { $s } }

function New-CompileJob($exe, $src, $obj, [string[]]$arguments) {
    [pscustomobject]@{ Exe = $exe; Src = $src; Obj = $obj
        Args = (@($arguments) + @('-c', (Quote $src), '-o', (Quote $obj))) -join ' ' }
}

# Runs the jobs, as many at once as there are processors; throws naming every file that failed
function Invoke-Compile($jobs) {
    $max = [Environment]::ProcessorCount
    $running = @()
    $failed = @()
    foreach ($j in $jobs) {
        while (@($running | Where-Object { -not $_.P.HasExited }).Count -ge $max) { Start-Sleep -Milliseconds 50 }
        $errFile = "$($j.Obj).err"
        $p = Start-Process -FilePath $j.Exe -ArgumentList $j.Args -NoNewWindow -PassThru -RedirectStandardError $errFile
        $null = $p.Handle   # without this, ExitCode reads null once the process has gone
        $running += [pscustomobject]@{ P = $p; J = $j; Err = $errFile }
    }
    foreach ($r in $running) {
        $r.P.WaitForExit()
        $msg = Get-Content $r.Err -Raw -ErrorAction SilentlyContinue
        if ($msg) { Write-Host $msg }
        Remove-Item $r.Err -ErrorAction SilentlyContinue
        if ($r.P.ExitCode -ne 0 -or -not (Test-Path $r.J.Obj)) { $failed += (Split-Path -Leaf $r.J.Src) }
    }
    if ($failed) { throw "compile failed: $($failed -join ', ')" }
}

# Compiles a list of sources into one static library, once; returns its path
function Build-Library($t, [string]$name, [string[]]$sources, [string[]]$arguments) {
    $lib = "$($t.Out)\deps\lib$name.a"
    if (Test-Path $lib) { return $lib }
    Write-Host "==> $name for $($t.Triple)"
    $objDir = "$($t.Out)\$name-obj"
    New-Item -ItemType Directory -Force $objDir | Out-Null
    $jobs = foreach ($s in $sources) {
        New-CompileJob $t.CC $s (Join-Path $objDir (([IO.Path]::GetFileNameWithoutExtension($s)) + '.o')) $arguments
    }
    Invoke-Compile $jobs
    & $t.AR rcs $lib @($jobs | ForEach-Object Obj)
    if ($LASTEXITCODE) { throw "llvm-ar failed for $name" }
    return $lib
}

# FreeType from SynthLib\ThirdParty, its own CMake source list, no CMake
function Build-FreeType($t) {
    $ft = "$Third\freetype"
    $list = [regex]::Match((Get-Content "$ft\CMakeLists.txt" -Raw), '(?s)set\(BASE_SRCS(.*?)\)').Groups[1].Value -split '\s+' |
            Where-Object { $_ -like 'src/*.c' }
    $srcs = @($list) + @('builds/windows/ftsystem.c', 'builds/windows/ftdebug.c') | ForEach-Object { Join-Path $ft $_ }
    Build-Library $t 'freetype' $srcs @('-O2', '-DFT2_BUILD_LIBRARY', '-w', (Quote "-I$ft\include"))
}

# GLFW's Win32 + WGL build, the files its src/CMakeLists.txt names for Windows
function Build-Glfw($t) {
    $g = "$Third\glfw\src"
    $names = 'context init input monitor platform vulkan window egl_context osmesa_context null_init null_monitor ' +
             'null_window null_joystick win32_module win32_time win32_thread win32_init win32_joystick win32_monitor ' +
             'win32_window wgl_context'
    $srcs = $names -split ' ' | ForEach-Object { "$g\$_.c" }
    Build-Library $t 'glfw3' $srcs @('-O2', '-D_GLFW_WIN32', '-w', (Quote "-I$Third\glfw\include"))
}

# libusb's Windows backend (WinUSB and UsbDk), with the config.h its configure would write for MinGW
function Build-Libusb($t) {
    $u = "$Third\libusb\libusb"
    $cfgDir = "$($t.Out)\deps\libusb-config"
    New-Item -ItemType Directory -Force $cfgDir | Out-Null
    @(
        '#define DEFAULT_VISIBILITY __attribute__ ((visibility ("default")))',
        '#define ENABLE_LOGGING 1',
        '#define PLATFORM_WINDOWS 1',
        '#define PRINTF_FORMAT(a, b) __attribute__ ((__format__ (__printf__, a, b)))',
        '#define HAVE_STRUCT_TIMESPEC 1',
        '#define HAVE_CLOCK_GETTIME 1',
        '#define HAVE_SYS_TIME_H 1',
        '#define _WIN32_WINNT _WIN32_WINNT_VISTA'
    ) | Set-Content -Encoding ascii "$cfgDir\config.h"
    '#define LIBUSB_DESCRIBE "SynthLib ThirdParty"' | Set-Content -Encoding ascii "$cfgDir\version_describe.h"
    $names = 'core descriptor hotplug io strerror sync os\events_windows os\threads_windows os\windows_common ' +
             'os\windows_usbdk os\windows_winusb'
    $srcs = $names -split ' ' | ForEach-Object { "$u\$_.c" }
    Build-Library $t 'usb-1.0' $srcs @('-O2', '-w', (Quote "-I$cfgDir"), (Quote "-I$u"), (Quote "-I$u\os"))
}

# A zsh array from one of the Mac scripts: its quoted paths that start from a variable, the variables filled in
function Read-ZshArray($file, $name, $vars) {
    $text = Get-Content $file -Raw
    $body = [regex]::Match($text, '(?ms)^' + [regex]::Escape($name) + '=\(\s*$(.*?)^\)').Groups[1].Value
    foreach ($m in [regex]::Matches($body, '(?m)^\s*"(\$[^"]+)"')) {
        $p = $m.Groups[1].Value
        foreach ($k in $vars.Keys) { $p = $p.Replace("`$$k", $vars[$k]) }
        [IO.Path]::GetFullPath($p)
    }
}

function Show-Binary($t, $file) {
    $dump = & $t.Objdump -p $file
    $leaf = Split-Path -Leaf $file
    $needs = ($dump | Select-String 'DLL Name: (\S+)' | ForEach-Object { $_.Matches[0].Groups[1].Value } |
              Where-Object { $_ -ne $leaf }) -join ' '
    Write-Host ("built: $file ({0:N1} MB, $($t.Triple))" -f ((Get-Item $file).Length / 1MB))
    return [pscustomobject]@{ Dump = $dump; Needs = $needs }
}

function Get-VersionString {
    if ($env:G2_WIN_VERSION) { return $env:G2_WIN_VERSION }
    if (Get-Command git -ErrorAction SilentlyContinue) {
        $v = git -C $Root describe --tags --always --dirty 2>$null
        if ($LASTEXITCODE -eq 0 -and $v) { return $v }
    }
    return 'development build'
}
