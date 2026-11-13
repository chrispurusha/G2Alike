# Several G2s at once - design

Living design note, started 2026-11-13. Nothing built yet.

G2Alike drives one G2. This note is what it takes to drive several from one application: each with its own
patches, settings, banks and engine, switched between in the editor, with copy and paste between them.

## 0. The route: one G2 per running copy (CT, 2026-11-13)

Rather than one application driving several G2s, **several copies of G2Alike, each driving one G2**. Each process
keeps its own memory, USB thread, engine and audio, so none of §3's state has to be untangled. What it needs:

1. **A G2 not already taken.** Enumerate the attached G2s and claim the first that is free - a G2 held by another
   process refuses `libusb_claim_interface()` - then keep its bus, address and port path and follow them (§1).
2. **The name cache per G2**, its file named by the G2's Synth Name: each G2 has its own banks.
3. **The backdoor channel per process**: its `/tmp` paths carry the process id.
4. **Another copy from the menu**: File > New Window runs `open -n` on the application; the window title names its
   G2 (Synth Name).

What works as it is: one audio device serves several processes; every copy hears all MIDI and already listens on
its own chosen channel, which answers §5's two questions. What is lost against one window: copy and paste between
G2s (each process has its own clipboard - also putting a copied selection on the system pasteboard would mend it),
and the preferences file is shared, last writer winning, so anything per G2 keys by Synth Name.

The rest of this note - one application, several documents, a topbar switcher - stays as the way to go further if
working across G2s from one window is ever wanted.

## 1. Telling G2s apart

Nothing in a G2's USB descriptor tells two units apart: same vendor and product, no serial number. What does is
**where it is plugged in**:

- A G2 is opened by its **bus and address**, and its **port path** (`libusb_get_port_numbers()`) is recorded
  when it opens.
- After a G2 reboots, macOS gives it a **new address** on the same port (seen in another editor's long runs:
  0:4 became 0:5). A reconnect therefore looks for its own address first, and failing that for a G2 on the
  **same port path** at another address, and takes that.
- A G2 moved to another port is a new connection; nothing tries to match it to the old one.
- For the user, a G2 is named by its own **Synth Name** (Settings, read from the G2 at connect), with the port
  path as a fallback when two share a name. A name is what the user set; a port path is not something anyone
  recognises.

Today `usbComms.c` opens the first G2 found (`libusb_open_device_with_vid_pid()`), so with two attached it takes
whichever libusb lists first.

## 2. What is already per G2

Most of it. Since 2026-09-11 every patch, setting and the command queue live in a `tG2Document` (globalVars.h),
selected per thread, because a plug-in instance needed its own. The engine is banked per document: one engine
per slot, four per document. So a second G2 needs a second document, which the application can already make
(`g2_document_create()`); it has simply never had a reason to.

## 3. What is not

| What | Where | Change |
|---|---|---|
| The USB connection: device handle, thread, connection flags, bank upload/restore buffers, the name sweep's progress | ~34 file statics in `usbComms.c` | into a per-G2 connection record, one USB thread each; each thread selects its document at start, as the plug-in's threads do |
| Which G2 is opened | `libusb_open_device_with_vid_pid()` | enumerate, open by bus and address, remember the port path, follow it across a reboot (§1) |
| Bank name tables | `gPatchNameTable`, `gPerfNameTable`, globals | into the document - each G2 has its own banks |
| The name cache | `nameCache.c`, one file | one file per G2, keyed by its Synth Name |
| The clipboard | `gClipboard`, in the document | OUT of the document, shared - so copy on one G2 and paste on another works |
| The engine count | `SOUND_ENGINE_MAX_ENGINES` 4 in the application | 4 per G2 (the plug-in already has 128) |
| MIDI input | `midiInput.c` feeds the current document | each G2's engine on its own MIDI channels; see §5 |
| Preferences | per application | most stay so (dial mode, theme); per-G2 ones (last bank browsed, view state) keyed by Synth Name |

## 4. The editor

One window, as now, with **a G2 switcher in the topbar**: one button per connected G2, named as in §1. Switching
selects that G2's document on the UI thread and redraws; the others stay connected, their USB threads running.
Every G2 keeps its own selected slot, scroll position and dial focus, since those are document fields already.

Separate windows, one per G2, are possible later but cost more: GLFW and the render backend have one window
today, and the menu bar, popups and file browser all assume it.

New G2s are found by a periodic scan (every few seconds while the application runs), added to the switcher and
connected in the background; one that goes away keeps its document and greys its button until it returns on
its port path.

## 5. Open questions

- **Local sound engine with several G2s.** Each document has its engine, but the application has one audio device
  and one MIDI input. Mix all connected G2s' engines into the output, or only the selected one's? Route MIDI by
  channel, as the G2s' own slots do?
- **Windows.** WinUSB through Zadig is per device: a second G2 needs the driver installed for it too.
- **The backdoor channel and tools** assume one G2; they would name a G2 by index.

## 6. Order of work (one application; see §0 for the chosen route)

1. Move the USB statics into a connection record, still one G2 - no behaviour change, testable on its own.
2. Name tables into the document, the clipboard out of it, the name cache per G2.
3. Enumerate and open by bus/address with the port path kept and followed (§1), still one G2.
4. A second connection with its own thread and document; the topbar switcher.
5. The engine count and MIDI routing (§5), once the questions are answered.
6. The hot-plug scan.
