# SynthLib as one shared checkout

Started 2026-10-30. Goal: `GitHub/SynthLib` beside the siblings, used by all of them,
instead of a submodule inside each.

## State

- `GitHub/SynthLib` is cloned at 3a683e8 with ThirdParty submodules. freetype, glfw and libusb are
  built there (libusb is used by G2Alike and GenBridge only).
- Steps 1-3 below are done, uncommitted: all build paths in all eight siblings now point at
  `../SynthLib`. `$SYNTHLIB_DIR` overrides it in every script and in CMake.
- Uncommitted in `GitHub/SynthLib` itself: all the library changes below, including two new headers.
- The submodules are still in place but no longer used by any build. Removing them is step 4.
- No CI exists: the `.github` folders hold only FUNDING.yml.
- Verified 2026-10-30 with every embedded `SynthLib` renamed away: `do-plugin all --no-install` for
  G2Alike, GenBridge, MidiSeqTool, MidiSyncTool, Pulse2Alike, TiAlike, and Xcode Debug for EmuUtility,
  G2Alike, GenBridge (both schemes), SynthEdit. Compile and link only; nothing run or loaded.
- Not exercised: `do-release`, CMake and the Windows scripts, `tools/do-*`, `do-poc`.

## Findings

- Eight siblings carry a copy: EmuUtility, G2Alike, GenBridge, MidiSeqTool, MidiSyncTool,
  Pulse2Alike, SynthEdit, TiAlike. All are at 3a683e8 except TiAlike at f91443b, one commit behind.
- Each checkout builds ThirdParty for itself, 34-41 MB each. A shared one needs this once.
- Build systems: Xcode for EmuUtility, G2Alike, GenBridge, SynthEdit; `do-plugin` scripts for
  GenBridge, MidiSeqTool, MidiSyncTool, Pulse2Alike, TiAlike; G2Alike also has CMake (Windows).
- `$SYNTHLIB_DIR` is already honoured by the `do-plugin` of GenBridge, MidiSeqTool, MidiSyncTool,
  Pulse2Alike, TiAlike. G2Alike's is hard-wired to `$HERE/SynthLib` (29 references).
- Xcode: EmuUtility, G2Alike and SynthEdit use a synchronised folder (`path = SynthLib/src`),
  one line to change. GenBridge lists every file individually (about 35 references).
- Other hard-wired paths: `G2Alike/CMakeLists.txt` (36), `G2Alike/tools/*`,
  `G2Alike/cmake/platform.cmake`, `MidiSyncTool/tools/do-driver`, `GenBridge/do-poc`,
  `GenBridge/tools/do-vst3host`, some `do-release`, READMEs and THIRD_PARTY.md files. Also
  `SynthEdit/src/graphics.c`, `GenBridge/vst3/gbDraw.c`, `G2Alike/src/misc.h` - not yet checked
  whether these are code or comments.

## One unified SynthLib build

Owner's rule (2026-10-30): SynthLib is common and generic, with 1 unified build. There are no
application-named settings and no per-application `#ifdef`s; what differs per application is passed in
through the library initialisation call. This replaces an earlier recommendation to keep compiling
SynthLib into each product.

### Done: palette and layout

- `G2ALIKE`, then the short-lived `SYNTHLIB_THEME_LIGHT` / `SYNTHLIB_LAYOUT_TOP_BAR`, are gone.
  `src/synthlibDefs.h` has one set of defaults with no switches.
- SynthLib's own sources only ever compiled the default branch, so removing it changes nothing in
  the library. G2Alike's values for the shared names now live in `G2Alike/src/defs.h`, which
  includes `synthlibDefs.h` first, so include order no longer matters (the trap in
  plugin-gui-notes is gone).
- Checked: `clang -dM` over `defs.h` + `synthlibDefs.h`, 745 macros, old against new. Only differences:
  `G2ALIKE` removed; `DRAW_BUTTON_MARGIN` and `RGB_GREY` now visible to G2Alike. Both include orders
  give the same result.
- The run-time route already exists: `configure_synthlib_theme(tSynthLibTheme)` carries
  `topBarHeight` and four colours, set by G2Alike and GenBridge.

Open visual decision: only `utilsGraphics.c` reads the theme. `fileBrowser`, `bankBrowser`,
`midiPortDialog` and others use the default dark colours directly, including inside G2Alike's light
UI. Moving them onto the theme, or extending it, is the proper end state, but changes what G2Alike
looks like, so it should be done on its own and checked by eye. The owner has offered to run G2Alike
and check the dialogs at that point.

### Done: version, log level, Obj-C classes (2026-10-30)

- Version: `synthlib_set_app_info(version, built)`, reached through `SYNTHLIB_APP_INIT()`, a macro
  expanded in the application's own source. Apps call it in `main()`; the G2Alike, Pulse2Alike and
  TiAlike plug-ins call it from About. Pulse2Alike and TiAlike now show their real version instead of
  "development build". "Compiled" is the date of the file holding the call, so it can lag on an
  incremental build.
- Log level: `gSynthLibLogLevel`, set by `SYNTHLIB_LOG_INIT()` from the application's own
  `ENABLE_LOG_DEBUG` / `ENABLE_LOG_MODULE_DATA`, so Debug and Release print as before.
  `LOG_DEBUG` is now a run-time check rather than compiled out: one load and branch per call, also
  on audio threads in Release. USB logging is `gSynthLibLogSink`, set in G2Alike's `main()`.
- Panel view and AU container are run-time classes (`plugin/synthlibRuntimeClass.h`), named
  automatically from the image's address, so `SYNTHLIB_PANEL_VIEW_CLASS` is gone.
- AU view factory is the one exception, and it is forced: hosts that use `-[NSBundle classNamed:]`
  get nil for a run-time class (tested). It lives in the product, not SynthLib:
  `SYNTHLIB_AU_VIEW_FACTORY(name)`, generated by each `do-plugin`, forwarding to
  `synthlib_au_view_create()`.
- The images are pinned with `RTLD_NODELETE`. Static classes kept them loaded; without any, a host
  unloading a plug-in would be a new code path.

Tested: the real `synthlibPanelView.m` in two images in one process gave distinct classes, no
duplicate-class warning, and events, timer and resize reached the callbacks; the image stayed loaded
after `dlclose`. Four built AUs loaded together, each factory found by `classNamed:`. The panel view's
behaviour inside a real host, and the AU container, are not yet tested.

### Deployment target

macOS 12.0 everywhere (owner, 2026-10-30): current Xcode's minimum, so a lower target has no future.
The last 11.5 scripts and the ThirdParty builds moved up, which drops macOS 11 for GenBridge, MidiSyncTool
and G2Alike's plug-ins. The library builds at 12.0.

### Done: no application-supplied symbols (2026-10-30)

Owner's rule: SynthLib expects no symbols from applications; anything application-specific is
registered, as callbacks where it is behaviour. Checked by linking every SynthLib object together:
nothing is left unresolved except one VST3 SDK file left out of that link, and no two objects define
the same function.

- `SYNTHLIB_PLUGIN_REGISTER(fn, "logname")` in the project's source replaces `gSynthLibLogName` and a
  project-defined `synthlib_plugin_variants()`. It runs at load, because a host enters the wrappers
  before any project code. `SYNTHLIB_AU_VIEW_FACTORY` registers its class name the same way.
- The AU and VST3 wrappers both defined six `synthlib_plugin_*` functions. Now
  `plugin/synthlibPluginRegistry.c` defines them and forwards to a `tSynthLibPluginFormat` table,
  which each wrapper sets when a host enters it.
- `plugin/pluginStubs.c` is gone. It replaced real functions by source-list choice.
  `synthlibGlobals.c` no longer needs GLFW: redraws go through `synthlib_set_redraw_hook()`, which
  `synthlibWindow.c` sets to `glfwPostEmptyEvent` and G2Alike's plug-in to its view.
  `synthlibPersistence.c` no longer needs GLFW either; its window loader moved to `synthlibWindow.c`.
- G2Alike's plug-in keeps its rotary default and its pref through `synthlib_set_dial_mode_hook()`.

Verified: `plugin/test/do-test` passes, all plug-ins and apps build, four AUs load together.

Not yet checked: Windows, whose list `do-windows-plugin` was updated by hand. Code-notes in
`SynthLib/Docs` still describe `pluginStubs.c` and the old symbols.

### Remaining before a real libSynthLib.a

| Item | Plan |
|---|---|
| `SYNTHLIB_PLUGIN_BUILD`, `SYNTHLIB_NO_GL_BACKEND` | no effect on macOS. Windows is a separate platform build (owner, 2026-10-30), so these can stay there |
| `plugin/` VST3 wrappers | build within the library, with the SDK as a build input |

### Building it

- `SynthLib/` gets its own build, producing `libSynthLib.a` in one known place, arm64 and x86_64.
  Products link it plus the ThirdParty libs, and stop listing SynthLib sources. That removes the
  manual lists in the `do-plugin` scripts, a known source of errors, and GenBridge's per-file entries.
- A static library pulls in only the object files a product uses, so most dead-code removal comes
  for free. Add `-Wl,-dead_strip` at the final link for the rest.
- Decide who rebuilds it: a script in SynthLib run by each product's build, so a stale library
  can't be linked by accident.

## What is lost: the pin

Today each sibling records the SynthLib commit it works with. With one checkout, every sibling
always gets whatever is checked out, so a SynthLib change can break a sibling you weren't working
on, and an old sibling commit no longer says which SynthLib it needs.

Decision: accept that, and have each `do-release` record the SynthLib commit hash, so a release
stays reproducible. A fresh clone of a sibling also stops being self-contained, so each README
needs the "clone SynthLib beside it" step.

## Migration order

1. Build libusb in the shared checkout; confirm which siblings use it.
2. In every script, make the default `SL="${SYNTHLIB_DIR:-$HERE/../SynthLib}"`, keeping the
   override. Bring G2Alike's `do-plugin` and tools onto `$SL`.
3. Point Xcode at `../SynthLib`, CMake likewise (define it once, `SYNTHLIB_DIR`).
4. One sibling at a time, TiAlike first, as it needs only its script: build, load in a host, then
   `git rm SynthLib`, drop `.gitmodules`, commit. G2Alike last, as it's the largest.
5. Add the build-everything-third-party steps as a script in SynthLib; update READMEs, the
   THIRD_PARTY.md files, `Third Party build notes.md`, and the submodule section of CLAUDE.md.
6. Record the SynthLib hash in `do-release`.

Check before step 4: anyone else (CT?) building from a fresh clone.

## LTO and dead-code removal

Unmeasured so far. What's known:

- Xcode targets already have `DEAD_CODE_STRIPPING = YES`. No `LLVM_LTO` setting anywhere.
- No `do-plugin`, `do-release` or CMake file passes `-dead_strip` or `-flto`. The plug-ins are where
  the unused SynthLib code is likely to be going unstripped.

Would they work? Yes, both are standard on macOS with clang, and neither needs the shared
checkout. Suggested order:

1. Add `-Wl,-dead_strip` to the plug-in links. It's cheap and low risk; check that the VST3/AU entry
   points are kept (they're exported, so they should be), then compare bundle sizes.
2. Try `-flto=thin` on a Release plug-in and measure size and DSP load. It also needs to be in
   the compile step, not just the link. Inlining across SynthLib and the engine could help, but the
   engines' hot loops mostly sit within one file already, so expect a small gain. Keep it only if
   measured. Link times rise, so Release builds only.

Measure each sibling's before and after, and note that `.a` ThirdParty libs aren't LTO-built.
