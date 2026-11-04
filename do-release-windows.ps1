# do-release-windows.ps1 - the Windows release, built ON WINDOWS. The native twin of ./do-release-windows
# (which cross-builds on the Mac), with the same arguments, the same checks and the same .zips:
#
#   powershell -ExecutionPolicy Bypass -File do-release-windows.ps1 beta      V0.5.0-beta.6 -> V0.5.0-beta.7
#   ... patch | minor | major | final                                          as ./do-release
#   ... 0.9.0-rc1                                                              an explicit version
#   ... (nothing)                                                              the newest tag as-is
#   ... beta -Dist C:\somewhere                                                default: the Desktop
#
# The bump keywords and the no-argument form read the git tags, so they need git; an explicit version
# does not. Never creates or pushes a tag.
#
# Writes G2Alike-<version>-windows-x64.zip and -arm64.zip, each holding G2Alike.exe, G2Alike.vst3,
# "Read Me First.txt" and LICENSE. G2_RELEASE_SKIP_PLUGINS=1 releases the editor alone. -Arch builds one.

param([string]$Bump = '', [string]$Dist = [Environment]::GetFolderPath('Desktop'), [string[]]$Arch = @('x64', 'arm64'))
. "$PSScriptRoot\tools\windows-common.ps1"

# --- version (as ./do-release) ------------------------------------------------------------------
$haveGit = [bool](Get-Command git -ErrorAction SilentlyContinue)
$latest = if ($haveGit) { git -C $Root tag --list --sort=-v:refname 'V*' 2>$null | Select-Object -First 1 } else { $null }
$newTag = $null

if ($Bump -in 'major', 'minor', 'patch', 'beta', 'final') {
    if (-not $haveGit) { throw "'$Bump' reads the git tags and git is not installed - give an explicit version" }
    if (-not $latest) { throw "no existing V* tag to bump from - give an explicit version" }
    if ($latest -notmatch '^[Vv](\d+)\.(\d+)\.(\d+)(-beta\.(\d+))?$') { throw "newest tag '$latest' cannot be bumped" }
    $ma = [int]$Matches[1]; $mi = [int]$Matches[2]; $pa = [int]$Matches[3]; $be = $Matches[5]
    switch ($Bump) {
        'major' { $ma++; $mi = 0; $pa = 0; $be = 1 }
        'minor' { $mi++; $pa = 0; $be = 1 }
        'patch' { $pa++; $be = 1 }
        'beta'  { if (-not $be) { throw "'$latest' has no -beta.n to bump; use 'patch'" }; $be = [int]$be + 1 }
        'final' { $be = $null }
    }
    $newTag = if ($be) { "V$ma.$mi.$pa-beta.$be" } else { "V$ma.$mi.$pa" }
    git -C $Root rev-parse -q --verify "refs/tags/$newTag" *> $null
    if ($LASTEXITCODE -eq 0) { throw "tag $newTag already exists" }
    $version = $newTag
    Write-Host "==> version $version ($Bump bump from $latest)"
} elseif ($Bump -eq '') {
    if (-not $latest) { throw "no git tag found and no version given - try: do-release-windows.ps1 beta" }
    $version = $latest
    Write-Host "==> version $version (reusing newest tag)"
} else {
    if ($Bump -notmatch '^[Vv]?\d+\.') { throw "'$Bump' is not a bump keyword (major minor patch beta final) or a version" }
    $version = $Bump
    Write-Host "==> version $version (specified)"
}
if ($haveGit -and (git -C $Root status --porcelain 2>$null)) { Write-Host "    WARNING: working tree has uncommitted changes" }
$versionClean = $version -replace '^[Vv]', ''

# --- build --------------------------------------------------------------------------------------
$env:G2_WIN_VERSION = $version
$withPlugin = -not $env:G2_RELEASE_SKIP_PLUGINS
$stage = Join-Path ([IO.Path]::GetTempPath()) ("g2release-" + [guid]::NewGuid())
$zips = @()

try {
    foreach ($a in $Arch) {
        Write-Host "==> clean Release build, Windows $a"
        & "$Root\tools\do-windows.ps1" -Arch $a | Out-Null    # the app from scratch; the libraries are kept
        if ($withPlugin) { & "$Root\tools\do-windows-plugin.ps1" -Arch $a | Out-Null }

        $t = Get-Toolchain $a
        $exe = "$($t.Out)\G2Alike.exe"

        # GUI subsystem, or the user gets a console window behind the editor
        $subsystem = (& $t.Objdump -p $exe | Select-String '^Subsystem\s+(\S+)').Matches[0].Groups[1].Value
        if ($subsystem -ne '00000002') { throw "$exe is not a GUI program (subsystem $subsystem)" }

        # The version is in the binary, or the About box would say "development build"
        if (-not (Select-String -Path $exe -Pattern $version -SimpleMatch -Quiet)) {
            throw "$version is not in $exe"
        }

        $name = "G2Alike-$versionClean-windows-$a"
        $dir = Join-Path $stage $name
        New-Item -ItemType Directory -Force $dir | Out-Null
        Copy-Item $exe, "$Root\platform\windows\Read Me First.txt", "$Root\LICENSE" $dir
        if ($withPlugin) { Copy-Item "$($t.Out)\G2Alike.vst3" $dir }

        $zip = Join-Path $Dist "$name.zip"
        Remove-Item $zip -ErrorAction SilentlyContinue
        Compress-Archive -Path $dir -DestinationPath $zip
        $zips += $zip
    }
} finally {
    Remove-Item -Recurse -Force $stage -ErrorAction SilentlyContinue
}

Write-Host ""
foreach ($z in $zips) { Write-Host ("done: $z ({0:N1} MB)" -f ((Get-Item $z).Length / 1MB)) }
if ($withPlugin) { Write-Host "contents: G2Alike.exe, G2Alike.vst3, Read Me First.txt, LICENSE" }
else { Write-Host "contents: G2Alike.exe, Read Me First.txt, LICENSE - NO plug-in (G2_RELEASE_SKIP_PLUGINS was set)" }
Write-Host "signature: none - SmartScreen warns on first run (More info > Run anyway)"

if ($newTag) {
    Write-Host ""
    Write-Host "$newTag does not exist yet. Once you are happy with the .zips (and the Mac .dmg):"
    Write-Host ""
    Write-Host "    git tag -a $newTag -m `"$newTag`""
    Write-Host "    git push origin $newTag"
}
