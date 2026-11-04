# do-windows-plugin.ps1 - builds the G2Alike VST3 ON WINDOWS, with llvm-mingw. The native twin of
# tools/do-windows-plugin (which cross-builds on the Mac); see Docs/build.md.
#
#   powershell -ExecutionPolicy Bypass -File tools\do-windows-plugin.ps1 [-Arch arm64|x64] [-Install]
#
# Result: build-win-<arch>\G2Alike.vst3, one self-contained DLL. -Install copies it into
# C:\Program Files\Common Files\VST3 (needs an elevated shell).
#
# THE SOURCE LISTS ARE READ FROM ../do-plugin AND tools/do-windows-plugin, so the three cannot drift.
#
# Environment: LLVM_MINGW (default ~\Developer\llvm-mingw*), VST3_SDK (default ~\Documents\vst3sdk),
# G2_WIN_VERSION (the About box).

param([string]$Arch = $(if ($env:PROCESSOR_ARCHITECTURE -eq 'ARM64') { 'arm64' } else { 'x64' }),
      [switch]$Install)
. "$PSScriptRoot\windows-common.ps1"

$t = Get-Toolchain $Arch
$Sdk = if ($env:VST3_SDK) { $env:VST3_SDK } else { "$env:USERPROFILE\Documents\vst3sdk" }
if (-not (Test-Path "$Sdk\pluginterfaces")) { throw "VST3 SDK not found at $Sdk - set VST3_SDK (Docs/build.md)" }

$ftLib = Build-FreeType $t

$vars = [ordered]@{ SYNTHLIB_SRC = "$SynthLib\src"; HERE = $Root; ROOT = $Root; SDK = $Sdk }
$cSources = @(Read-ZshArray "$Root\do-plugin" 'SOURCES' $vars) +
            @(Read-ZshArray "$Root\tools\do-windows-plugin" 'SOURCES+' $vars)
$cxxSources = @(Read-ZshArray "$Root\tools\do-windows-plugin" 'CXX_SOURCES+' $vars) +
              @(Read-ZshArray "$Root\tools\do-windows-plugin" 'VST3_CXX' $vars)
foreach ($s in $cSources + $cxxSources) { if (-not (Test-Path $s)) { throw "missing source: $s" } }

$defs = @((Quote "-DSYNTHLIB_VERSION_STRING=`"$(Get-VersionString)`""), '-DSYNTHLIB_PLUGIN_BUILD=1',
          '-D_USE_MATH_DEFINES', '-D_POSIX_THREAD_SAFE_FUNCTIONS=200112L')
$incs = @("$Root\src", "$SynthLib\src", "$SynthLib\plugin", "$Root\plugin",
          "$Third\glfw\include", "$Third\freetype\include") | ForEach-Object { Quote "-I$_" }
$incs += '-isystem', (Quote $Sdk)
$flags = @('-O2', '-include', (Quote "$Root\platform\windows\winCompat.h"))

$obj = "$($t.Out)\plugin-obj"
if (Test-Path $obj) { Remove-Item -Recurse -Force $obj }
New-Item -ItemType Directory -Force $obj | Out-Null

Write-Host "==> G2Alike VST3 for $($t.Triple) ($($cSources.Count) C, $($cxxSources.Count) C++)"
$jobs = @()
foreach ($s in $cSources) {
    $jobs += New-CompileJob $t.CC $s (Join-Path $obj (([IO.Path]::GetFileNameWithoutExtension($s)) + '.o')) `
             (@('-std=gnu11') + $flags + $defs + $incs)
}
foreach ($s in $cxxSources) {
    $quiet = if ($s.StartsWith($Sdk)) { @('-w') } else { @() }   # Steinberg's own files, theirs to warn about
    $jobs += New-CompileJob $t.CXX $s (Join-Path $obj (([IO.Path]::GetFileNameWithoutExtension($s)) + '.oxx')) `
             (@('-std=gnu++17') + $flags + $quiet + @('-Wno-pragma-pack') + $defs + $incs)
}
Invoke-Compile $jobs

$dll = "$($t.Out)\G2Alike.vst3"
if (Test-Path $dll) { Remove-Item -Recurse -Force $dll }    # also a bundle folder an older build left
# One self-contained DLL: the C++ runtime and winpthreads linked in
& $t.CXX -shared -static -o $dll @($jobs | ForEach-Object Obj) $ftLib `
    -lopengl32 -lgdi32 -luser32 -lole32 -lwinmm -lpthread '-Wl,--exclude-all-symbols'
if ($LASTEXITCODE) { throw 'link failed' }

Write-Host ""
$info = Show-Binary $t $dll
$exports = ($info.Dump | Select-String '^\s+\d+\s+0x[0-9a-f]+\s+(\S+)$' | ForEach-Object { $_.Matches[0].Groups[1].Value }) -join ' '
Write-Host "exports: $exports"
Write-Host "needs: $($info.Needs)"

if ($Install) {
    $dest = "$env:CommonProgramFiles\VST3"
    New-Item -ItemType Directory -Force $dest | Out-Null
    Copy-Item $dll $dest -Force
    Write-Host "installed: $dest\G2Alike.vst3"
}
