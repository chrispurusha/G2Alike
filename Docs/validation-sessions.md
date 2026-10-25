# Validation sessions - checking engine modules against the G2

How to run a session that compares an engine module with the G2 itself, what to validate next, and the traps that
have already cost time. Written 2026-10-11 after the DrumSynth Kick presets and the Nord Filter were validated in
one afternoon with the method below. Read `capture-inventory.md` for what is already on disk.

**A capture CHECKS the engine; it never sets it.** No law is fitted to a capture. Where the two disagree, the
module's law is settled first, then checked again; a measurement downstream of more than the thing in question is
not evidence (`drumsynth-capture-recipe.md`, top).

## 1. The rig

| | |
|---|---|
| G2 outputs 1-4 | Fireface UC inputs 5-8 (zero-based 4-7), flat. The old desk inputs (a low shelf on 5/6) are history |
| Fireface outputs 0-3 | the G2's In 1-4, for driving its inputs (`tools/tone`) |
| MIDI | TM-1 to the G2; the G2 receives on **channel 3, slot A only** - other slots have no MIDI channel |
| Recording | `tools/capture --device Fireface --out f.wav --seconds N` (8 ch, 96 kHz, 32-bit) |
| Editor | the Debug build, `G2ALIKE_BACKDOOR=1`, ONLINE; `tools/g2bd 'CMD' ...` sends backdoor commands |

**Calibrate every session.** A bare OscB into a 2-Out, captured and rendered (`RENDERWAV`), gives the rig's
offset, engine minus G2: +5.5 dB on 2026-10-09, -0.5 dB on 2026-10-11 - something analogue moves between sessions
(the G2's volume, the interface's gain). An absolute level means nothing without that day's offset. A RATIO within
one capture (a filter's L/R, a band against the body) needs none, which is why the methods below use ratios.

## 2. A session, step by step

1. Start the editor online with the backdoor; `g2bd COMMS` says `online`.
2. `g2bd 'DEVSLOT B' NEWPATCH` - build in a slot whose edit buffer is free (edit buffers may be overwritten freely;
   NEVER store to a bank location without asking). DEVSLOT moves the keyboard focus too, in Patch mode.
3. `ADDMODULE <Name> <col> <row>`, `CABLE VA <from>:<out> <to>:<in>` - read the module indices back with `DUMP`
   (connector numbers are per direction, in `connectorLocationList` order).
4. Settings ON THE G2 with `DEVSET VA <index> <param> <value>` (plain `SET` is local only and measures nothing).
   `PARAMDUMP` before and after: a module switched off by accident once faked a 38 dB difference.
5. Calibration patch first (section 1), then the module.
6. The G2: `capture` while notes are played - `DEVNOTE <note> 127 on|off` (the virtual key: velocity 127 always,
   plays the keyboard-focus slot) or `tools/g2_note "Elektron TM-1" 3 <note> <vel>` (MIDI, any velocity, slot A).
7. The engine: `g2bd 'ENGINEOFFLINE on' 'RENDERWAV <path> <ms> [<note> <vel> [<holdMs>]]' 'ENGINEOFFLINE off'` -
   the same patch, rendered offline to a 48 kHz stereo WAV, while the editor stays online.
8. Analyse with the tools in section 3, record the result in `sound-engine-reference.md`, and update the module's rows in
   `tools/modulestatus.c` (`kEvidence`, `kAspectEvidence`), then regenerate `engine-module-status.md`.

`PUSH` sends the whole slot as one write, if ever the G2 and the editor disagree about a slot.

## 3. Methods and their tools

| What is checked | Rig | Tool |
|---|---|---|
| A filter's response | Noise > filter > 2-Out L, the same Noise > 2-Out R; Kbt and mods off | `filter_response.py --pairs DIR` (L/R per third-octave band, 63 Hz-16 kHz) |
| A percussive module's balance and level | Keyboard Gate > module > 2-Out; several hits by DEVNOTE, one render per hit | `band_balance.py --g2 ... --engine ... --rig <offset>` (bands relative to the body) |
| Harmonic content of a steady tone | module > 2-Out at a known pitch | `harmonics.py` |
| Levels and gains | the module between a known source and a 2-Out, against the calibration patch | RMS with that day's offset |
| A time or rate | a gate train or a clock, the output into something audible | onsets in the capture; align on a signal edge, never on the clock (captures start 0.06-0.6 s late) |
| Which modules matter | the stage patches read off the G2 | `stage_usage.py` (loads each into one slot's edit buffer) |

The outputs are AC-coupled (sound-engine-notes §198): a control signal does not survive to the Fireface as DC.
Measure a control module through something that turns it into sound - an OscA's pitch, a LevAmp's gain.

## 4. Traps (each has cost a session)

- **FM or PM at a 1:1 ratio depends on the two oscillators' phase.** Every sideband then lands on a harmonic,
  and its size depends on where carrier and modulator started - random, and different on the G2 and the
  engine: OscPM at PhM 64 read 14-19 dB out at 1:1 and 0.1 dB with the modulator four semitones down.
  Measure FM at an inharmonic ratio and read the sidebands at fc +- k fm.
- **An envelope needs the gate on the other channel.** DEVNOTE's hold is the shell's timing, 1.1-1.2 s for a
  requested 1.0, so a release compared by the clock is out by a tenth of a second. Put the Keyboard Gate on
  the second 2-Out input: the capture then carries its own gate edges, the engine is rendered with the hold
  they show, and both curves are aligned on gate-on. Mask to the G2's noise floor (-62 dBFS on 2026-10-25),
  since the engine falls to digital silence.
- **A capture at 192 kHz needs a longer FFT.** The Fireface ran at 192 kHz on 2026-10-25, where 16384 points
  give 11.7 Hz bins against the engine's 2.9 Hz: FltHP at Freq 50 read 1-3 dB out below 100 Hz, and 0.2 at
  `--fft 65536`. Keep the bin widths comparable - 65536 at 192 kHz.
- **FFT resolution at low frequencies.** At 4096 points a bin is 23 Hz at 96 kHz (the G2) and 12 Hz at 48 kHz
  (the engine), so third-octave bands below ~300 Hz hold one or two bins laid out differently on each side. It
  showed 2-4 dB on the Nord Filter at Freq 50 that were not there. 16384 points (the tool's default) cured it.
- **A tap before an envelope, in a poly patch,** sums every voice's oscillators, keyed or not (13 voices ~ +11 dB).
  Keep the envelope in the path.
- **Patch mode plays every active slot**, selected or not. A slot left holding a free-running patch sounds under
  the measurement - empty the others or check with a no-note capture.
- **Performance mode is another set of slots**: switching mode swaps the G2's four slots for the performance's.
- **The rig offset moves** between sessions (section 1).
- **Free-running patches vary run to run** (detuned oscillators, LFOs from random phases): two takes of 04 Chris'
  Pad differed by 2.1 dB. Compare averages over several takes, or patches without them.
- **DEVNOTE is velocity 127** and plays the keyboard-focus slot; an unpatched DrumSynth Vel is a fixed 64 units.
- **The G2's outputs roll off** above ~15 kHz (sound-engine-notes §199): compare below 16 kHz, or render with the
  G2 Output Filter on.
- **Check every 2-Out's destination** before a capture: a second module on the same outputs sums in.
- **ADDMODULE straight after NEWPATCH can be lost**: it answers OK, but the G2's reply to the new patch
  replaces the local copy and the module is gone. `DUMP` after building, before cabling.
- **A wave's direction needs the rig's polarity**: step a Constant 0 to +1 into the 2-Out and see which
  way the AC-coupled edge goes first (non-inverting on 2026-10-25).

## 5. What to validate next

From `stage_usage.py` over the 19 stage patches (2026-10-11): the modules they use most that have never been
compared with the G2. Settings in brackets are the first grid to try.

| Module | Stage patches | Rig and method |
|---|---|---|
| S&H | 6 | OscA (slow) into In, an LFO square as clock, the output into an OscA's pitch: held values and timing |
| KeyQuant, SwOnOffT | 4 each | through an OscA's pitch / a LevAmp |
| Phaser, LFO B, Envelope D | 3 each | Phaser by `filter_response.py`; LFO B as LFO C was (reference §28.7), Envelope D as Envelope ADR |

**One cheap sweep that covers many aspects:** every module's Bypass/On-Off. The audit (`./tools/do-modulestatus
--aspects`) shows almost no Modes / On-Off aspect compared on the G2. A session that toggles Bypass on each module
type in turn, with a source through it, and notes pass, silent or hold settles 134 aspects at once.

Done so far, for reference: DrumSynth Kick 1-5 (bands within 0.5 dB, level within 0.3 dB), Nord Filter (36
settings within 1.44 dB, mean under 0.4), Compressor (8 levels within 0.01 dB) - all 2026-10-11.
LFO C (2026-10-25, reference §28.7): straight into a 2-Out at Hi range, rate within 0.02%, every wave and
Output Type; it found Neg and NegInv swapped. Its Sub and BPM ranges are left - too slow for the AC-coupled
outputs, so through an OscA's pitch.
Glide (2026-10-25, reference §36.1a): an LFO C square through it into an OscB's PitchVar, Log and Lin at
several Times; it found the step's rounding. Glide On and the button off are left.
LevMult (2026-10-25, reference §74): an OscB times a Constant, the OscB on the other channel as the
reference; within 0.04 dB at 8 values. ModAmt the same way, 66 settings (reference §29.5).
**The noise floor** in a ratio rig like that one is -44 dB of the reference: a reading below about -34 dB
is the signal and the floor added in RMS, so compare sqrt(engine^2 + floor^2) there, not the engine alone.
