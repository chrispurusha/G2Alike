# do-windows.ps1 - builds the G2Alike editor ON WINDOWS, with llvm-mingw. The native twin of tools/do-windows
# (which cross-builds on the Mac through CMake); see Docs/build.md.
#
#   powershell -ExecutionPolicy Bypass -File tools\do-windows.ps1 [-Arch arm64|x64] [-Debug] [-Clean]
#
# Result: build-win-<arch>\G2Alike.exe, one self-contained file, with "Read Me First.txt" beside it.
# Release (the default) is a GUI program at -Os; -Debug keeps the console window for the log, at -O1.
# GLFW, FreeType and libusb are compiled from SynthLib\ThirdParty once per architecture - no CMake.
#
# THE SOURCE LIST IS THE GENERATED CMakeLists.txt's, with cmake/platform.cmake's Windows swaps applied,
# both read here, so this cannot drift from the CMake build.
#
# Environment: LLVM_MINGW, G2_WIN_VERSION (the About box), G2_WERROR=ON.

param([string]$Arch = $(if ($env:PROCESSOR_ARCHITECTURE -eq 'ARM64') { 'arm64' } else { 'x64' }),
      [switch]$Debug, [switch]$Clean)
. "$PSScriptRoot\windows-common.ps1"

$t = Get-Toolchain $Arch
if ($Clean) { Remove-Item -Recurse -Force $t.Out; $t = Get-Toolchain $Arch }

$libs = @((Build-Glfw $t), (Build-FreeType $t), (Build-Libusb $t))

# --- the sources, from CMakeLists.txt and cmake/platform.cmake ----------------------------------
$cmake = Get-Content "$Root\CMakeLists.txt" -Raw
$platform = Get-Content "$Root\cmake\platform.cmake" -Raw

function Expand-CMakePath($p) {
    $p = $p.Trim('"').Replace('${SYNTHLIB_DIR}', $SynthLib)
    if (-not [IO.Path]::IsPathRooted($p)) { $p = Join-Path $Root $p }
    [IO.Path]::GetFullPath($p)
}
$sources = [regex]::Match($cmake, '(?s)set\(G2_SOURCES(.*?)\)').Groups[1].Value -split '\s+' |
           Where-Object { $_ } | ForEach-Object { Expand-CMakePath $_ }
$swaps = [regex]::Match($platform, '(?s)set\(G2_WINDOWS_SWAPS(.*?)\)').Groups[1].Value -split '\s+' |
         Where-Object { $_ } | ForEach-Object { Expand-CMakePath $_ }
for ($i = 0; $i -lt $swaps.Count; $i += 2) {
    $sources = @($sources | Where-Object { $_ -ne $swaps[$i] }) + $swaps[$i + 1]
}
$sources += "$Root\platform\windows\miscWin.c"
$quietSources = @("$Root\platform\windows\miniaudioImpl.c")    # miniaudio, without our warnings
foreach ($s in $sources + $quietSources) { if (-not (Test-Path $s)) { throw "missing source: $s" } }

# The warning switches the Xcode project turns on, as CMakeLists.txt lists them
$warnings = [regex]::Matches($cmake, '(?m)^\s+(-W[\w=-]+)\s*$') | ForEach-Object { $_.Groups[1].Value } |
            Where-Object { $_ -ne '-Werror' } | Select-Object -Unique
if ($env:G2_WERROR -eq 'ON') { $warnings += '-Werror' }

$config = @(if ($Debug) { '-O1', '-g', '-DDEBUG=1', '-DENABLE_LOG_DEBUG=1' } else { '-Os' })    # @(): one item stays an array
$version = Get-VersionString
$defs = @('-DGL_SILENCE_DEPRECATION=1', '-D_USE_MATH_DEFINES', '-D_POSIX_THREAD_SAFE_FUNCTIONS=200112L')
if ($version -ne 'development build') { $defs += Quote "-DSYNTHLIB_VERSION_STRING=`"$version`"" }
$incs = @("$SynthLib\src", "$Root\src") | ForEach-Object { Quote "-I$_" }
# the third-party headers as system headers, as CMakeLists.txt has them - their warnings are not ours
$incs += @("$Third\glfw\include", "$Third\freetype\include", "$Third\libusb\libusb") | ForEach-Object { '-isystem', (Quote $_) }
$flags = $config + $defs + $incs + @('-include', (Quote "$Root\platform\windows\winCompat.h"))

$obj = "$($t.Out)\app-obj"
if (Test-Path $obj) { Remove-Item -Recurse -Force $obj }
New-Item -ItemType Directory -Force $obj | Out-Null

Write-Host "==> G2Alike for $($t.Triple), $(if ($Debug) { 'Debug' } else { 'Release' }) ($($sources.Count + $quietSources.Count) files)"
$jobs = @()
foreach ($s in $sources + $quietSources) {
    $isCxx = $s.EndsWith('.cpp')
    $o = Join-Path $obj (([IO.Path]::GetFileNameWithoutExtension($s)) + $(if ($isCxx) { '.oxx' } else { '.o' }))
    $w = if ($quietSources -contains $s) { @('-w') } else { $warnings }
    $jobs += New-CompileJob $(if ($isCxx) { $t.CXX } else { $t.CC }) $s $o `
             (@($(if ($isCxx) { '-std=gnu++20' } else { '-std=gnu17' })) + $flags + $w)
}
Invoke-Compile $jobs

$exe = "$($t.Out)\G2Alike.exe"
$gui = @(if (-not $Debug) { '-mwindows' })    # a Release .exe opens no console window
& $t.CXX -static -o $exe @($jobs | ForEach-Object Obj) $libs `
    -lopengl32 -lgdi32 -luser32 -lshell32 -lole32 -lavrt -lwinmm -lsetupapi -lpthread @gui
if ($LASTEXITCODE) { throw 'link failed' }
Copy-Item "$Root\platform\windows\Read Me First.txt" $t.Out -Force

Write-Host ""
$info = Show-Binary $t $exe
Write-Host "needs: $($info.Needs)"
