/*
 * modulestatus — which module types the sound engine plays, by palette group, and on what evidence.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
// Notes: Docs/code-notes/modulestatus.c.md - "// notes §k" refers there.

// Docs/engine-module-status.md used to be kept by hand and was wrong about eight modules within a
// day of their being added. This asks the engine itself — sound_engine_models_module() is the same
// test the canvas uses to grey a module out — and prints the status tables ready to paste. The
// EVIDENCE for each modelled module is editorial, so it is kept here in kEvidence; the run fails if
// a modelled module has no row, or a row names a module that is not offered or not modelled.

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "moduleResourcesAccess.h"
#include "soundEngine.h"

typedef enum {
    eConfirmed = 0,    // compared with the G2, and it agrees
    eModelled,         // a full model in the reference, not yet compared with the G2
    eApproximate,      // follows the manual's description; a full model is still to come
    ePartial,          // something about it is known to be missing
    eNoSound,          // nothing to render
    STATE_COUNT
} tState;

typedef struct {
    tPaletteGroup group;
    const char *  name;
} tGroupRow;

typedef struct {
    const char * name;
    tState       state;
    const char * refs;     // where the reference describes it
    const char * g2;       // what was compared on the G2, and when
    const char * open;     // what is still open
} tEvidence;

static const char * kStateName[STATE_COUNT] = {"Confirmed on the G2", "Modelled", "Approximate", "Partial", "No sound"};

static const tGroupRow kGroups[] = {
    {palGroupOsc,    "Oscillators"},
    {palGroupFilter, "Filters"    },
    {palGroupEnv,    "Envelopes"  },
    {palGroupLfo,    "LFOs"       },
    {palGroupMixer,  "Mixers"     },
    {palGroupLevel,  "Level"      },
    {palGroupShaper, "Shapers"    },
    {palGroupDelay,  "Delays"     },
    {palGroupFx,     "Effects"    },
    {palGroupIo,     "In/Out"     },
    {palGroupSwitch, "Switches"   },
    {palGroupLogic,  "Logic"      },
    {palGroupSeq,    "Sequencers" },
    {palGroupRnd,    "Random"     },
    {palGroupNote,   "Note"       },
    {palGroupMidi,   "MIDI"       },
};

static const tEvidence kEvidence[] = {
    // Oscillators
    {"Osc A",                 eConfirmed,   "§6.3",                      "captures (G2Captures/osca); OscC/OscD level against it 09-12", "Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only"},
    {"Osc B",                 eConfirmed,   "§6.3, §6.6-§6.8",           "5 waves x 8 pitches, harmonics within 0.1 dB (09-17); DualSaw below zero 0.1 dB (10-08)", "FM and Sync not compared on the G2; Shape 127's one-sample click not modelled; Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only"},
    {"Osc C",                 eConfirmed,   "§6.3, §6.1a",               "Saw, Sqr50/25/10, Tri at 440-3520 Hz (09-17); Freq mode tracks the key: AdditiveG2 within 1.4 dB per octave (10-16)", "FM not compared on the G2"},
    {"Osc D",                 eConfirmed,   "§6.3, §6.1a",               "level and harmonics against OscA (09-12)", "Pitch Type: Freq tracking heard on Osc C (10-16), not on Osc D"},
    {"Osc Phase Mod",         eConfirmed,   "§53",                       "Sine and Tri at 330 Hz phase-modulated by a 262 Hz sine, PhM 32-127: every sideband within 0.37 dB, level on the rig offset (10-25); Bassic and Basslines FM4 (10-16)", "Sync not compared; Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only"},
    {"Osc Shape A",           eConfirmed,   "§27, §6.7",                 "all six waves at Shape 0/64/127 at 330 Hz: harmonics within 0.08 dB, level on the rig offset; Sine3 all harmonics and Sine4 odd, as OscShpB's (10-25)", "Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only"},
    {"Osc Shape B",           eConfirmed,   "§27, §6.7",                 "every wave, Shape Mod -1..+1, harmonics within 0.16 dB (10-02); Sine3/4 within 0.001 (09-17)", "Pulse at exactly +-1: click -33 dB against the G2's -41; Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only"},
    {"Osc Dual",              eConfirmed,   "§12.4, §12.5",              "levels, duty, phase (09-12); PW and Phase mod (10-03)", "Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only"},
    {"Noise Osc",             eConfirmed,   "§8.2-§8.5",                 "09-12 capture agrees 110 Hz-1 kHz (the old model's fit)", "level above 1 kHz not re-captured since the §8 model; Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only"},
    {"Noise",                 eConfirmed,   "§7.2a",                     "09-12 capture reproduced through the desk's shelf to a constant", "-"},
    {"Sampler",               ePartial,     "notes §213",                "engine only - the G2 has no such module", "velocity layers untested (no layered file); E-mu banks: first preset only, no layered voices; .npno pianos not read"},
    {"Metallic Noise",        eConfirmed,   "§66",                       "Colour 0/64/127 x Freq 0/64/127: third-octave balance within 0.63 dB (mean 0.28 at most), level on the rig offset (10-25)", "the FreqMod and ColourMod inputs not compared"},
    {"Osc Percussion",        eConfirmed,   "§40",                       "8 takes: levels 0.1 dB, decay 2 ms, pitch exact (09-25)", "-"},
    {"Drum Synth",            eConfirmed,   "§39",                       "Master Freq 0.2% (09-21); noise, click, oscillator energies within 0.8 dB (09-25)", "level curve set by §39.3, not fitted to the 09-20 sweeps"},
    {"Osc String",            eConfirmed,   "§70.4",                     "noise through it, Decay 64-120, Damp 0-64: resonances within 3.7 cents and 0.6 dB at Decay 64; pitch at Tune 64/100/112 within 1.2 cents, level within 0.4 dB (10-25)", "peak heights at Decay 100-120 narrower than the measurement resolves"},
    {"FM Operator",           eConfirmed,   "§14.2-§14.6",               "envelope, level, FM depth, feedback, level and rate scaling, velocity (10-03); an idle voice starts from L4: Backtalk DX repeated notes as the G2 within 0.7 dB (10-16)", "AMod and Pitch inputs (§14.6) not yet compared (to-test); Gate/Note/Vel jacks read from the voice"},
    {"DX Router",             eConfirmed,   "§14.1, §14.5",              "Main level 8.8 dB against 8.45 (10-03); Backtalk DX held tone within 1-2 dB per octave (10-16)", "Out1-Out6 not used outside the node"},
    {"Driver",                eConfirmed,   "§70.5, §70.5a",             "Reed and Bow, Stiffness and Embouchure 32/96, In2 0 and +32 units, a sine on In1: 16 settings within 0.06 dB per harmonic and 0.03 dB in level (10-25)", "-Lip- and -Mallet- share the Reed and were not played"},
    {"Resonator",             eConfirmed,   "§70.4a",                    "noise through all five Algs at Decay 100, Damp 32, Pos 64: resonances within 4 cents (Tube3 one at 14), peaks within 1.7 dB mean, level within 0.3 dB; Tune 100 to 0.7 cents (10-25)", "Pos and the pitch inputs not swept"},
    {"Osc Master",            eConfirmed,   "§51",                       "through an OscB's Pitch input: Coarse 40-76, Cent 0-127, Pitch M 0-127 with a Constant on PitchVar - within 0.74 cents (10-25)", "-"},
    // Filters
    {"LP Filter",             eConfirmed,   "§22",                       "noise through it, Freq 50/85/110, all six slopes: mean within 0.22 dB, worst 0.87 at 252 Hz (10-25); FM at 2 semitones a unit (notes §160)", "Kbt and the Mod input not compared"},
    {"HP Filter",             eConfirmed,   "§22",                       "noise through it, Freq 50/85, slopes 6/12/24/36: mean within 0.09 dB, worst 0.55 (10-25)", "Kbt and the Mod input not compared"},
    {"Nord Filter",           eConfirmed,   "§23",                       "noise through LP/BP/HP/BR, 12 and 24 dB, Freq 50 and 85, Res 0 and 100: within 1.44 dB, mean 0.06-0.35 (10-11)", "Pitch, FM and Res mod not compared"},
    {"Classic Filter",        eConfirmed,   "§21",                       "04 Chris' Pad within 0.5 dB 100 Hz-12 kHz; a high-Res test patch's peak within 0.04 semitone (09-28)", "Env and the dB/Oct tap not compared since the 09-14 law"},
    {"Multi Filter",          eConfirmed,   "§10",                       "54 noise responses within 0.5-0.6 dB (09-12); FreqM within 1% (10-08)", "GComp off not captured"},
    {"Phase Filter",          eConfirmed,   "§67",                       "noise through it, all three types, 2 and 6 notches, FB 20/64/100: mean within 0.11 dB (10-25)", "the Pitch, Spread and FM inputs not compared"},
    {"Comb Filter",           eConfirmed,   "§13.4",                     "noise through all three Types (09-12); Pitch attenuator (10-08)", "-"},
    {"Static Filter",         eConfirmed,   "§10.4, §10.4a",             "noise through LP, BP and HP at Freq 50/85, Res 0/64/110, GC off and on: 21 settings, mean within 0.28 dB, worst 1.13 on the Res 110 peak (10-25)", "-"},
    {"FltVoice",              eConfirmed,   "§56, §56.1",                "noise through it, vowels A/E/I, Freq 64/100, Res 0/64/127: formants within 18 cents, peaks within 0.8 dB (10-25); the Freq law from 63 to 66 to 2 cents", "Res 127 peak 3 dB under; between the formants 1.3-2.8 dB"},
    {"WahWah",                eConfirmed,   "§69.9",                     "noise through it at Sweep 0/64/127: mean within 0.24 dB, worst 1.4 in the lowest band at Sweep 0 (10-25)", "the Sweep input not compared"},
    {"Vocoder",               eConfirmed,   "§70.8",                     "a 110 Hz saw carrier and a sine modulator at 300 Hz, 1 kHz and 3 kHz, Emphasis off and on: the harmonics that pass within 0.15 dB, level within 0.08 (10-25)", "BandSel routing other than straight, and Monitor, not compared"},
    {"Eq 2-band",             eConfirmed,   "§11.2",                     "43 noise settings across the EQs, 0.53 dB mean (09-12)", "high shelf changed 10-09 (0.3-0.7 dB near 3 kHz), not re-captured"},
    {"Eq 3-band",             eConfirmed,   "§11",                       "0.66 dB mean, 1.42 worst (09-12)", "the 1.42 dB setting (mid -13.5 dB at 8 kHz) not explained by the model"},
    {"Eq Peak",               eConfirmed,   "§11.3, §11.5",              "0.57 dB mean (09-12)", "-"},
    // Envelopes
    {"Envelope ADSR",         eConfirmed,   "§17.1-§17.8",               "time law and curves agree with the 08-24 and 09-07 captures (not kept); release at LogExp R14 and LinExp R20 within one 50 ms window (10-16)", "-"},
    {"Envelope AHD",          eConfirmed,   "§17.11a",                   "Hold 16-80, within 1% (10-08)", "-"},
    {"Envelope ADR",          eConfirmed,   "§17",                       "an OscB through it on a held key, Decay and Release modes and the Exp shape: the curve within 0.1 dB on average through the hold and the release (10-25)", "-"},
    {"Envelop ADDSR",         eConfirmed,   "§17",                       "an OscB through it on a held key: within 0.5 dB on average, the release within 0.4 (10-25)", "-"},
    {"Envelope H",            eConfirmed,   "§17.9a",                    "Hold 12-64 within 0.1 ms (10-08)", "-"},
    {"Envelope D",            eConfirmed,   "§17",                       "Decay 60 and 90, triggered by the Keyboard Gate: within 0.1 dB on average (10-25)", "-"},
    {"Envelope Multi",        eConfirmed,   "§17.11",                    "its defaults, a fall from full to nothing and a rise to full: within 0.2 dB on average; its In is input 1 - our face had In and AM swapped, fixed (10-25)", "-"},
    {"Envelope Mod AHD",      eConfirmed,   "§17.9a, §17.10",            "Hold within 0.3% (10-08); every mod jack at +-32 units and at partial amounts on the G2, the Sustain jack after a fix (10-26)", "-"},
    {"Envelope Mod ADSR",     eConfirmed,   "§17.10",                    "Decay mod at -32..+32 units (09-20); every mod jack at +-32 units and at partial amounts on the G2, the Sustain jack after a fix (10-26)", "-"},
    // LFOs
    {"LFO A",                 eConfirmed,   "§28",                       "Hi rate measured; Sine/Tri/Saw/Sqr at Hi 112 correlate 0.999-1.000, harmonics within 0.01 dB; Sub, Lo and BPM within 0.5%, the Rate input and RateVar through Rate M within 0.2% (10-26)", "RndSt/Rnd, Kbt and Mode not compared"},
    {"LFO B",                 eConfirmed,   "§28, §28.4a",               "Hi range, all four waves and Phase 0-96 against its Snc output: to 0.002 cycle; rate 0.001%, harmonics 0.5 dB, level 0.07 dB (10-25); Lo, Sub, BPM and Clk within 0.7%, the Rate input and RateVar through Rate M 0-127 within 0.2% (10-25)", "the Phase M input not compared"},
    {"LFO C",                 eConfirmed,   "§28, §28.7",                "Hi 64-127 and Lo 100/127 rate within 0.09%; Sine/Tri/Saw/Sqr shape and harmonics within 0.7 dB; every Output Type, level within 0.2 dB (10-25); Lo and Sub through an OscB's pitch within 0.7%, BPM within 0.3% at 120 BPM, the Rate input +-12/+24 units within 0.2% (10-25)", "Mono not compared"},
    {"LFO Shp A",             eConfirmed,   "§28.6",                     "Rate Sub, Lo, Hi within 0.013% (09-07); all six waves at Shape 1/32/64/96/127 against its Snc: correlation 0.96-1.00, harmonics within 0.6 dB, phase within 0.014 cycle; Phase 32-96, Shape M, Phase M and Dir (+-32, +16 units) likewise; Sub, Lo, BPM, Clk within 0.5%, Rate and RateVar within 0.2% (10-26)", "Rst not compared"},
    {"Clock Generator",       eConfirmed,   "§59",                       "Master clock within 7-17 ppm; engine within 1-4 ms over 66 s (10-09)", "-"},
    // Mixers
    {"Mixer 1-1 A",           eConfirmed,   "§3",                        "106 configurations of the eleven mixers within 0.07 dB (09-12)", "-"},
    {"Mixer 1-1 S",           eConfirmed,   "§3",                        "as Mixer 1-1 A; On off mutes to -93 dBFS at Lev 40/78/127, Lin and Exp (10-16)", "-"},
    {"Mixer 2-1 A",           eConfirmed,   "§3",                        "as Mixer 1-1 A", "-"},
    {"Mixer 4-1 A",           eConfirmed,   "§3",                        "as Mixer 1-1 A", "-"},
    {"Mixer 4-1 B",           eConfirmed,   "§3",                        "as Mixer 1-1 A", "-"},
    {"Mixer 4-1 C",           eConfirmed,   "§3",                        "218 steps 0.01 dB; Pad 0/-6/-12 (09-07, 09-12)", "-"},
    {"Mixer 4-1 S",           eConfirmed,   "§3",                        "as Mixer 1-1 A", "-"},
    {"Mixer 2-1 B",           eConfirmed,   "§3",                        "as Mixer 1-1 A; Inv cancels", "-"},
    {"Mixer 8-1 A",           eConfirmed,   "§3",                        "as Mixer 1-1 A", "-"},
    {"Mixer 8-1 B",           eConfirmed,   "§3",                        "as Mixer 1-1 A", "-"},
    {"MixFader",              eConfirmed,   "§3",                        "as Mixer 1-1 A", "-"},
    {"MixStereo",             eConfirmed,   "§5",                        "19 settings each, Log and Lin (09-12)", "-"},
    {"Fade 1-2",              eConfirmed,   "§4.2",                      "19 settings, every point within 0.001 (09-12)", "mod-input depth not captured, still 4 x the input"},
    {"Fade 2-1",              eConfirmed,   "§4.2",                      "as Fade 1-2", "mod-input depth not captured, still 4 x the input"},
    {"X-Fade",                eConfirmed,   "§4.2, §4.3",                "as Fade 1-2", "mod-input depth (Pan's law since 10-10) not captured"},
    {"Pan",                   eConfirmed,   "§4.2, §4.3",                "as Fade 1-2; mod depth, an LFO at 31: +-6.3 dB (10-10)", "-"},
    // Level
    {"Constant",              eConfirmed,   "§16.1",                     "the known input of most G2 checks since 09-13 (OscShpB, OscDual, DrumSynth, FltMulti)", "-"},
    {"ConstSwM",              eConfirmed,   "§68.1",                     "as ConstSwT (10-25)", "-"},
    {"ConstSwT",              eConfirmed,   "§44",                       "Value 20/96/127, Bip and Uni, on and off: on the G2's value within its 0.3% (10-25)", "-"},
    {"CompLev",               eConfirmed,   "§48",                       "six A/level pairs either side of the threshold: high at A >= level, as on the G2 (10-25)", "-"},
    {"CompSig",               eConfirmed,   "§69.2",                     "five A/B pairs: high at A >= B, as on the G2 (10-25)", "-"},
    {"LevAdd",                eConfirmed,   "§32",                       "five In and Constant pairs, Bip and Uni, past 64 units unsaturated on both (10-25)", "-"},
    {"LevAmp",                eConfirmed,   "paramCurves notes §29",     "33 dial positions (08-30); 127 as 0x7FFFFF settled BCHydro_DZLW (10-09)", "-"},
    {"LevConv",               eConfirmed,   "§31",                       "all 18 In/Out type pairs at -32 and +32 units; over-range inputs run on to +-128 units - the engine clamped at 64, fixed (10-25)", "-"},
    {"LevMod",                eConfirmed,   "§69.3",                     "six In/Mod/Depth/Type settings with the ModDepth input: within 0.1 unit of the G2 (10-25)", "-"},
    {"LevMult",               eConfirmed,   "§74",                       "an OscB sine times a Constant at 8 values, -64 to +64 units: gain within 0.04 dB and the sign right (10-25)", "-"},
    {"MinMax",                eConfirmed,   "§43",                       "both outputs at two input pairs (10-25)", "-"},
    {"ModAmt",                eConfirmed,   "§29",                       "an OscB times a Constant through it, Exp and Lin, m and 1-m, Depth 32/64/100/127, Mod -64 to +64, Enable off: 66 settings within 0.04 dB, or at the rig's noise floor (10-25)", "-"},
    {"NoiseGate",             eConfirmed,   "§70.6",                     "a sine at five levels and thresholds: opens above Threshold (v/128), shuts below three quarters of it, Env at full open, as on the G2 (10-25)", "attack and release times not timed on the G2"},
    {"EnvFollow",             eConfirmed,   "§69.4",                     "a sine at three levels and three Attack/Release settings: the settled output within 0.3% of the G2 (10-25)", "attack and release times not timed on the G2"},
    {"Red2Blue",              eConfirmed,   "§68.8, notes §203",         "passes a Constant unchanged (10-25)", "-"},
    {"Blue2Red",              eConfirmed,   "§68.8",                     "passes a Constant unchanged (10-25)", "-"},
    // Shapers
    {"Saturate",              eConfirmed,   "paramCurves notes §33",     "a 330 Hz sine through it at two drives (the input on the other channel), Amount 64/127, curves 1 and 4: every harmonic within 0.38 dB, level within 0.03 (10-25)", "the Mod input not compared"},
    {"Clip",                  eConfirmed,   "paramCurves notes §36",     "a 330 Hz sine through it at two drives (the input on the other channel), Clip level 32/96, Asym and Sym: every harmonic within 0.38 dB, level within 0.03 (10-25)", "the Mod input not compared"},
    {"OverDrive",             eConfirmed,   "§71",                       "energy above 6 kHz in 14 CS80project72: -41.5 against -41.6 dB (10-04)", "-"},
    {"ShpExp",                eConfirmed,   "paramCurves notes §32",     "a 330 Hz sine through it at two drives (the input on the other channel), Amount 127, all four curves: every harmonic within 0.38 dB, level within 0.03 (10-25)", "the Mod input not compared"},
    {"WaveWrap",              eConfirmed,   "§73, paramCurves notes §34", "a 330 Hz sine through it at two drives (the input on the other channel), Amount 32/96: every harmonic within 0.38 dB, level within 0.03 (10-25)", "the Mod input not compared"},
    {"ShpStatic",             eConfirmed,   "paramCurves notes §31",     "a 330 Hz sine through it at two drives (the input on the other channel), all four modes: every harmonic within 0.38 dB, level within 0.03 (10-25)", "the Mod input not compared"},
    {"Rect",                  eConfirmed,   "paramCurves notes §30",     "a 330 Hz sine through it at two drives (the input on the other channel), all four modes: every harmonic within 0.38 dB, level within 0.03 (10-25)", "the Mod input not compared"},
    // Delays
    {"Delay Single A",        eConfirmed,   "§52",                       "noise through it at Ranges 5 ms to 1 s, Time 0-127: the tap on the G2's own sample, unity gain (10-26)", "-"},
    {"Delay Single B",        eConfirmed,   "§52.1",                     "the same at three Ranges, and Time M 64-127 with +-16/+32 units on the Mod jack: on the G2's sample (10-26)", "-"},
    {"Delay Dual",            eConfirmed,   "§70.1, §52.1",              "noise through it, Ranges 5/25/500 ms, Time 0-127: each tap on the G2's sample, unity gain (10-25)", "Time Mod inputs not compared; Range 2.7 s silences the patch on the G2 (the budget)"},
    {"Delay Quad",            eConfirmed,   "§70.1, §52.1",              "noise through it at 25 and 100 ms: the four taps and Main on the G2's sample, unity gain (10-25)", "Time Mod inputs and Clk mode not compared"},
    {"Delay A",               eConfirmed,   "§24",                       "FB, LP, dry/wet, Time and Clk (09; audio not kept)", "-"},
    {"Delay B",               eConfirmed,   "§24",                       "as Delay A; HP at four settings", "-"},
    {"Delay Stereo",          eConfirmed,   "§65",                       "both taps on the G2's sample; FB 40/96/127 and X-FB 100, each dial alone, within 3% per pass and flat to 16 kHz; Dry/Wet 0-127 within 0.01 (10-26)", "LP and HP dials not swept; FB + X-FB at a loop gain of 1 grows to saturation on the G2 only"},
    {"Delay Clock",           eConfirmed,   "§69.7",                     "Time 0, 2 and 3 over five clocks: the value written N clocks before, as on the G2 (10-26)", "-"},
    {"Delay Eight",           eConfirmed,   "§70.1, §52.1",              "noise through it at 25 and 100 ms, Time 64-127: taps 1/3/4/8 on the G2's sample (10-25)", "-"},
    {"DlyShiftReg",           eConfirmed,   "§69.6",                     "nine values clocked in, Outs 1, 2, 3, 5 and 8: In k-1 clocks ago, as on the G2 (10-26)", "-"},
    // Effects
    {"Compressor",            eConfirmed,   "§25",                       "a steady sine at 8 levels, -47 to -19 dB, 04 Chris' Pad's settings: gain equal to the G2's within 0.01 dB (10-11)", "attack/release timing and SideChain not compared on the G2"},
    {"Digitizer",             eConfirmed,   "§69.8",                     "a 330 Hz sine at Rate 32/96, Bits 3/11: every line within 0.8 dB (10-25)", "the Rate input not compared"},
    {"FreqShift",             eConfirmed,   "§57",                       "a 330 Hz sine, Shift 32/80/127 in each Range: Up and Down within 0.1 Hz and 0.3 dB, the other sideband within 0.2 dB at -64; In is input 0 - ours had In and Mod swapped, fixed (10-25)", "the Mod input not compared"},
    {"Flanger",               eConfirmed,   "§70.2",                     "noise through it, Range 32/96, FB 0/64/127, averaged over its sweep: within 0.4 dB mean; the sweep from a sine's FM at Rate 64/127 within 0.06%, its depth within 0.2% (10-25)", "its random start phase not compared"},
    {"Chorus",                eConfirmed,   "§19",                       "tap spans, rate, mix (09-07); unity at Amount 0 (09-14); no tail once its input stops, as the G2 (10-16)", "-"},
    {"Phaser",                eConfirmed,   "§55",                       "noise through it, both types, FB 0/64/127, averaged over its sweep: within 0.56 dB mean; sweep rate within 0.1% (10-25)", "-"},
    {"PShift",                eConfirmed,   "§70.3",                     "a 330 Hz sine at Semi 52/76/100: the shifted line exact in frequency and within 0.2 dB, its neighbours too; In is input 0 - ours had In and PitchVar swapped, fixed (10-25)", "the PitchVar input and Fine not compared"},
    {"Reverb",                eConfirmed,   "§20",                       "19 captures: onsets, stereo, Time law (09); Dry/Wet 96/64/48/24 within 1 dB of the cubed law (10-16)", "captured decays 6-12% long (their fits)"},
    {"Scratch",               eConfirmed,   "§70.3",                     "a sine at Ratio 10-127 and both Delays: the pitch within 0.02% of the G2, level within 0.1 dB; Ratio M at 64 and 127 with +-1 to +-16 units, after a fix (10-26)", "the level at a speed of exactly 1 depends on where the taps stopped"},
    // In/Out
    {"2 Outputs",             eConfirmed,   "§63, notes §167, §198",     "Pad +6 dB (09-07); an unpatched socket is silent (10-10); the path every G2 check goes through; Out to Bus 1/2 silent on its own, as the G2 (10-16)", "-"},
    {"4 Outputs",             eConfirmed,   "-",                         "Pad +6 dB (09-07)", "-"},
    {"2 Inputs",              eConfirmed,   "§37",                       "full scale and Pad off the G2's own meter (10-08)", "the application has no input device"},
    {"4 Inputs",              eConfirmed,   "§37, §69.12",               "a tone into each G2 input at both Pads: its own output within 0.03 dB of 2-In, the others silent (10-26)", "From Bus not compared"},
    {"FX Input",              eConfirmed,   "capture-inventory",         "Pad +6/0/-6/-12 dB (09-07)", "-"},
    {"Keyboard",              eConfirmed,   "§26",                       "velocity through the Operators within 0.03 dB (10-03)", "-"},
    {"Monophonic Keyboard",   eConfirmed,   "§35",                       "held-note return on 01 Mini Emulator (CT, 09-19)", "Poly use is a guess"},
    {"Device",                eModelled,    "§70.13, §70.13a",           "-", "the filtering and scaling not compared on the G2: move the wheel and the pedal on both"},
    {"Status",                eConfirmed,   "§70.13",                    "Patch Active is low, off the G2 (10-04)", "-"},
    {"Note Detector",         eConfirmed,   "§69.11",                    "Gate, Vel and RVel of the selected key on the G2's own keyboard, a matching key and not: Vel and RVel 0.992 for 127, held after release (10-26)", "-"},
    {"Name Bar",              eNoSound,     "-",                         "-", "-"},
    // Switches
    {"SwOnOffM",              eConfirmed,   "§68.1",                     "passes held and blocks released on the G2 (10-25)", "-"},
    {"SwOnOffT",              eConfirmed,   "§30",                       "passes On and blocks Off on the G2 (10-25)", "-"},
    {"Sw2-1",                 eConfirmed,   "§33",                       "every Sel position routes the right input, eight OscBs told apart by pitch (10-25)", "the Ctrl output not compared"},
    {"Sw2-1M",                eConfirmed,   "§68.1",                     "both positions (10-25)", "the Ctrl output not compared"},
    {"Sw4-1",                 eConfirmed,   "§68.1",                     "every Sel position (10-25)", "the Ctrl output not compared"},
    {"Sw8-1",                 eConfirmed,   "§33",                       "every Sel position (10-25)", "the Ctrl output not compared"},
    {"Sw1-2",                 eConfirmed,   "§68.1",                     "both positions; Sel 0 feeds output 1 on the G2 - the engine fed output 0, fixed, and the output rows swapped to match (10-25)", "the Ctrl output not compared"},
    {"Sw1-2M",                eConfirmed,   "§68.1",                     "both positions; as Sw1-2, Sel off feeds output 1 - fixed (10-25)", "the Ctrl output not compared"},
    {"Sw1-4",                 eConfirmed,   "§68.1",                     "every Sel position, the next output silent (10-25)", "the Ctrl output not compared"},
    {"Sw1-8",                 eConfirmed,   "§45",                       "every Sel position, the next output silent (10-25)", "the Ctrl output not compared"},
    {"ValSw2-1",              eConfirmed,   "§34",                       "a Ctrl sweep at value 32: the On input only at 32 units, as on the G2 (10-25)", "-"},
    {"ValSw1-2",              eConfirmed,   "§68.2",                     "a Ctrl sweep from 0 to 63 units at value 32: output 0 at 32 units alone, output 1 otherwise; the engine had the outputs the other way round, fixed (10-25)", "-"},
    {"Mux8-1",                eConfirmed,   "§68.3",                     "Ctrl from 0 to 127 units: four units a step, clamped at both ends (10-25)", "-"},
    {"Mux1-8",                eConfirmed,   "§68.3",                     "Ctrl steps 0-7 each on its own output (10-25)", "-"},
    {"Mux8-1X",               eConfirmed,   "§70.11",                    "the crossfade between neighbours at six Ctrl levels within 0.005 of the G2's (10-25)", "-"},
    {"S&H",                   eConfirmed,   "§38.5",                     "In taken on Ctrl's rise only, held with Ctrl high and low, at three values: as on the G2 (10-26)", "-"},
    {"T&H",                   eConfirmed,   "§68.4",                     "tracks In while Ctrl is high, holds while low, at three values: as on the G2 (10-26)", "-"},
    {"WindSw",                eConfirmed,   "§68.5",                     "Ctrl from 0 to 46 units, window 20-30: passes inclusive at both ends, as on the G2 (10-25)", "-"},
    // Logic
    {"Invert",                eConfirmed,   "§38.1",                     "both inverters at all four input pairs: on the G2's levels to 0.1 dB (10-25)", "-"},
    {"Pulse",                 eConfirmed,   "§18",                       "17 widths within two samples (09)", "Plus/Minus edge (10-09) in to-test"},
    {"Delay",                 eConfirmed,   "§46",                       "Pos, Neg and Cycle against a logic reference on the G2, Sub/Lo/Hi at four Times from 95 ms to 985 ms: each delayed edge within 1.2 ms of the law (0.7%), the other immediate (10-25)", "the Time Mod input not compared"},
    {"Gate",                  eConfirmed,   "§38.2",                     "all six types on both gates, set differently, at all four input pairs (10-25); the Types are modes, which the engine read as missing parameters - every Gate was an AND until this date", "-"},
    {"FlipFlop",              eConfirmed,   "§38.3",                     "D and RS walked step by step on the G2 by its LEDs: the input order (Clk, In, Rst), D on the clock's rise, RS toggle, S and Rst (10-25)", "-"},
    {"ClkDiv",                eConfirmed,   "§38.4",                     "14 pattern seq steps once a bar, as on the G2 (10-07)", "-"},
    {"8Counter",              eConfirmed,   "§68.6",                     "reset and ten clock edges walked by its LEDs on the G2: the one-hot output steps and wraps after the eighth; Rst holds it at the first through an edge (10-25)", "-"},
    {"BinCounter",            eConfirmed,   "§68.6",                     "reset and ten clock edges walked by its LEDs on the G2: the outputs are the count's bits; Rst clears and holds (10-25)", "-"},
    {"ADConv",                eConfirmed,   "§68.7",                     "eleven Constants from -64 to +64 units walked by its LEDs on the G2: every bit as the law has it, the top saturating at 127 (10-25)", "-"},
    {"DAConv",                eConfirmed,   "§68.7",                     "ten codes from -128 to +127 through a LevMult: code/128 on both, within the rig's 0.3% (10-25)", "-"},
    // Sequencers
    {"Sequencer Event",       eConfirmed,   "§58",                       "step timing at 192 kHz (09-26); 1:2 BCHydro_DZLW within 0.2 dB (10-09)", "-"},
    {"Sequencer Values",      eConfirmed,   "§58",                       "as Sequencer Event", "-"},
    {"Sequencer Level",       eConfirmed,   "§58, §69.1",                "Length/Cycle restart in BCHydro_DZLW (10-09)", "-"},
    {"Sequencer Note",        eConfirmed,   "§58, §58.1",                "18 Unreal Dreams' pitches (09-28)", "the record stage never compared"},
    {"Sequencer Controlled",  eConfirmed,   "§70.10",                    "Ctrl through steps 1-16 and out of range, all three XFades mid-step and bipolar Pol: within 0.2 unit of the G2 (10-26)", "Trig output and the Val/Trig inputs not compared"},
    // Random
    {"Random A",              eConfirmed,   "§47",                       "Mono at Step 100% and 25%: 31 values found in the engine's sequence from load, each within 0.2 unit of the G2 (10-26)", "Edge glides and the Pitch input not compared; Pitch not read"},
    {"Random B",              eConfirmed,   "§69.10",                    "Mono at Step dial 127 and 40: 30-32 values found in the engine's sequence from load, within 0.2 unit (10-26)", "Edge glides, Rate inputs and Kbt not compared"},
    {"Rnd Clock A",           eConfirmed,   "§64",                       "from a reset at two Seeds and two Steps: the same 16 values in order as the G2, within 0.2 unit (10-26)", "-"},
    {"Rnd Clock B",           eConfirmed,   "§70.9",                     "Rnd1 and Rnd2 from a reset at two Seeds and Steps, and the Step M jack: the same 15-16 values as the G2; Rnd2 from Seed 0 only after a fix (10-26)", "-"},
    {"Rnd Trig",              eConfirmed,   "§64",                       "from a reset at three Seeds and Densities: the same pulses passed, 18-38 gaps identical (10-26)", "the Prob input not compared"},
    {"Rnd Pattern",           eConfirmed,   "§70.9",                     "from a reset at four PatA/PatB/Step/Loop settings: the same 9-16 values as the G2, loops included (10-26)", "State wave and the A/B/Step inputs not compared"},
    // Note
    {"Note Quantiser",        eConfirmed,   "§49",                       "ten In/Range/Notes settings, Off to 12 semitones, within 0.1 unit of the G2 (10-26)", "-"},
    {"Key Quantiser",         eConfirmed,   "§41",                       "a seven-key scale, Closest and Evenly, ten inputs across two octaves: the same keys as the G2 (10-26)", "-"},
    {"Partial Quantiser",     eConfirmed,   "§69.5",                     "partials 1-31 and negative input at three Ranges within 0.2 unit (10-26)", "-"},
    {"Note Scaler",           eConfirmed,   "§60",                       "four In/Range pairs within 0.1 unit (10-26)", "-"},
    {"Glide",                 eConfirmed,   "§36, §36.1",                "Log at Time 32/64/96 within 0.3%, Lin at 32/64/80/96 within 0.02%, a square through it into an OscB's pitch (10-25)", "the Glide On input and the button off not compared"},
    {"Pitch Tracker",         eConfirmed,   "§70.7",                     "a sine at 41 Hz to 2.6 kHz: Pitch within 0.2 unit of the G2 and Gate high, after moving 0 units to E4 (10-26)", "Threshold and Period not compared"},
    {"Zero Crossing Counter", eConfirmed,   "§70.7a",                    "a sine at 41 Hz to 2.6 kHz: within 0.1 unit of the G2, after moving 0 units to E4 (10-26)", "-"},
    {"Level Scaler",          eConfirmed,   "§70.12",                    "Level and Out at nine Note/L/BP/R settings, both slopes to +-8 dB/oct, within 0.3% (10-26)", "-"},
    // MIDI
    {"CtrlSend",              eConfirmed,   "§62.3",                     "1:2 BCHydro_DZLW's variation switching, within 1-4 ms (10-09)", "This and channels 1-16 leave by MIDI, which the engine has no output for (§62.2)"},
    {"PCSend",                eNoSound,     "-",                         "-", "nothing leaves by MIDI"},
    {"CtrlRcv",               eModelled,    "§70.13",                    "-", "in the plug-in only notes arrive"},
    {"NoteRcv",               eModelled,    "§70.13",                    "-", "-"},
    {"NoteZone",              eNoSound,     "-",                         "-", "nothing leaves by MIDI"},
    {"Automate",              eNoSound,     "-",                         "-", "nothing leaves by MIDI"},
    {"NoteSend",              eConfirmed,   "§62, §62.2",                "18 Unreal Dreams' pitches and band balance (09-28); Slot A plays, This and channels 3/5 do not (10-25)", "This and channels 1-16 leave by MIDI, which the engine has no output for"},
};

// notes §2 - the arithmetic each module's model runs in, against 24-bit words
typedef enum {
    ePrecWords = 0,    // the signal path in 24-bit integer words: its truncation, rounding and saturation
    ePrecControl,      // counters, phases, steps or coefficients in its words; the audio path in doubles
    ePrecSaturating,   // doubles on the instrument's scaling, saturated where it saturates; never rounded to 24 bits
    ePrecDoubles,      // doubles throughout, saturated only where the value leaves the module
    ePrecNone,         // logic, switching, copies, MIDI: thresholds and whole numbers, which no rounding moves
    PREC_COUNT
} tPrecision;

static const char * kPrecisionName[PREC_COUNT] = {"Integer words", "Integer control", "Saturating doubles", "Doubles", "Not affected"};

typedef struct {
    const char * name;
    tPrecision   precision;
    const char * note;    // what is in words, or "-"
} tPrecisionRow;

static const tPrecisionRow kPrecision[] = {
    // Oscillators
    {"Osc A",                 ePrecDoubles,    "phase accumulator in doubles"},
    {"Osc B",                 ePrecDoubles,    "phase accumulator in doubles"},
    {"Osc C",                 ePrecDoubles,    "phase accumulator in doubles"},
    {"Osc D",                 ePrecDoubles,    "phase accumulator in doubles"},
    {"Osc Phase Mod",         ePrecDoubles,    "phase and Phase M in doubles; the sine is the module's polynomial"},
    {"Osc Shape A",           ePrecDoubles,    "phase accumulator in doubles"},
    {"Osc Shape B",           ePrecDoubles,    "phase accumulator in doubles"},
    {"Osc Dual",              ePrecDoubles,    "phase accumulator in doubles"},
    {"Noise Osc",             ePrecSaturating, "24-bit LFSR; the band-passes saturate at the word's +-1"},
    {"Noise",                 ePrecDoubles,    "xorshift draw, not the instrument's generator"},
    {"Metallic Noise",        ePrecWords,      "squares, Freq word and one-poles"},
    {"Osc Percussion",        ePrecSaturating, "DSP words carried in doubles, saturated"},
    {"Drum Synth",            ePrecSaturating, "the integer words carried in doubles"},
    {"Osc String",            ePrecDoubles,    "-"},
    {"FM Operator",           ePrecControl,    "envelope, level and rate offsets in words; the operator in doubles"},
    {"DX Router",             ePrecControl,    "envelope, level and rate offsets in words; the operators in doubles"},
    {"Driver",                ePrecDoubles,    "-"},
    {"Resonator",             ePrecDoubles,    "-"},
    {"Osc Master",            ePrecDoubles,    "-"},
    // Filters
    {"LP Filter",             ePrecDoubles,    "-"},
    {"HP Filter",             ePrecDoubles,    "-"},
    {"Nord Filter",           ePrecDoubles,    "-"},
    {"Classic Filter",        ePrecDoubles,    "-"},
    {"Multi Filter",          ePrecDoubles,    "-"},
    {"Phase Filter",          ePrecWords,      "the six allpass stages in the accumulator"},
    {"Comb Filter",           ePrecSaturating, "every word saturated"},
    {"Static Filter",         ePrecDoubles,    "-"},
    {"FltVoice",              ePrecControl,    "vowel table and resonance word; the resonators in doubles"},
    {"WahWah",                ePrecWords,      "sweep and filter words"},
    {"Vocoder",               ePrecSaturating, "the converters' and bands' words, saturated"},
    {"Eq 2-band",             ePrecDoubles,    "-"},
    {"Eq 3-band",             ePrecDoubles,    "-"},
    {"Eq Peak",               ePrecDoubles,    "-"},
    // Envelopes
    {"Envelope ADSR",         ePrecWords,      "level and segments in words; the VCA multiply in doubles"},
    {"Envelope AHD",          ePrecWords,      "level and segments in words; the VCA multiply in doubles"},
    {"Envelope ADR",          ePrecWords,      "level and segments in words; the VCA multiply in doubles"},
    {"Envelop ADDSR",         ePrecWords,      "level and segments in words; the VCA multiply in doubles"},
    {"Envelope H",            ePrecWords,      "level and segments in words; the VCA multiply in doubles"},
    {"Envelope D",            ePrecWords,      "level and segments in words; the VCA multiply in doubles"},
    {"Envelope Multi",        ePrecWords,      "its tick in words"},
    {"Envelope Mod AHD",      ePrecWords,      "level and segments in words; the VCA multiply in doubles"},
    {"Envelope Mod ADSR",     ePrecWords,      "level and segments in words; the VCA multiply in doubles"},
    // LFOs
    {"LFO A",                 ePrecDoubles,    "phase in doubles"},
    {"LFO B",                 ePrecDoubles,    "phase in doubles"},
    {"LFO C",                 ePrecDoubles,    "phase in doubles"},
    {"LFO Shp A",             ePrecDoubles,    "phase in doubles"},
    {"Clock Generator",       ePrecWords,      "phase and tempo words"},
    // Mixers
    {"Mixer 1-1 A",           ePrecDoubles,    "-"},
    {"Mixer 1-1 S",           ePrecDoubles,    "-"},
    {"Mixer 2-1 A",           ePrecDoubles,    "-"},
    {"Mixer 4-1 A",           ePrecDoubles,    "-"},
    {"Mixer 4-1 B",           ePrecDoubles,    "-"},
    {"Mixer 4-1 C",           ePrecDoubles,    "-"},
    {"Mixer 4-1 S",           ePrecDoubles,    "-"},
    {"Mixer 2-1 B",           ePrecDoubles,    "-"},
    {"Mixer 8-1 A",           ePrecDoubles,    "-"},
    {"Mixer 8-1 B",           ePrecDoubles,    "-"},
    {"MixFader",              ePrecDoubles,    "-"},
    {"MixStereo",             ePrecDoubles,    "-"},
    {"Fade 1-2",              ePrecDoubles,    "-"},
    {"Fade 2-1",              ePrecDoubles,    "-"},
    {"X-Fade",                ePrecDoubles,    "-"},
    {"Pan",                   ePrecDoubles,    "-"},
    // Level
    {"Constant",              ePrecDoubles,    "-"},
    {"ConstSwM",              ePrecDoubles,    "-"},
    {"ConstSwT",              ePrecDoubles,    "-"},
    {"CompLev",               ePrecWords,      "compared as words"},
    {"CompSig",               ePrecWords,      "compared as words"},
    {"LevAdd",                ePrecDoubles,    "-"},
    {"LevAmp",                ePrecDoubles,    "-"},
    {"LevConv",               ePrecDoubles,    "-"},
    {"LevMod",                ePrecWords,      "-"},
    {"LevMult",               ePrecDoubles,    "-"},
    {"MinMax",                ePrecDoubles,    "-"},
    {"ModAmt",                ePrecDoubles,    "-"},
    {"NoiseGate",             ePrecWords,      "follower and gate in words"},
    {"EnvFollow",             ePrecWords,      "peak word and follower"},
    {"Red2Blue",              ePrecNone,       "a copy, held at 24 kHz"},
    {"Blue2Red",              ePrecNone,       "a copy"},
    // Shapers
    {"Saturate",              ePrecDoubles,    "-"},
    {"Clip",                  ePrecDoubles,    "-"},
    {"OverDrive",             ePrecSaturating, "drive, feedback and blend saturated as words"},
    {"ShpExp",                ePrecDoubles,    "-"},
    {"WaveWrap",              ePrecDoubles,    "-"},
    {"ShpStatic",             ePrecDoubles,    "-"},
    {"Rect",                  ePrecDoubles,    "-"},
    // Delays
    {"Delay Single A",        ePrecDoubles,    "a 4-point read"},
    {"Delay Single B",        ePrecDoubles,    "a 4-point read"},
    {"Delay Dual",            ePrecDoubles,    "a 4-point read"},
    {"Delay Quad",            ePrecDoubles,    "a 4-point read"},
    {"Delay A",               ePrecWords,      "the tap, LP, HP and feedback on half-scale words"},
    {"Delay B",               ePrecWords,      "the tap, LP, HP and feedback on half-scale words"},
    {"Delay Stereo",          ePrecWords,      "Delay B's tap on both lines"},
    {"Delay Clock",           ePrecNone,       "a copy on each clock"},
    {"Delay Eight",           ePrecDoubles,    "a 4-point read"},
    {"DlyShiftReg",           ePrecNone,       "copies"},
    // Effects
    {"Compressor",            ePrecWords,      "detector, gain table and make-up"},
    {"Digitizer",             ePrecWords,      "rate word and phase"},
    {"FreqShift",             ePrecSaturating, "allpass chains saturated at the word's +-1"},
    {"Flanger",               ePrecControl,    "sweep phase word; the ring read in doubles"},
    {"Chorus",                ePrecControl,    "LFO phase word; the taps in doubles"},
    {"Phaser",                ePrecSaturating, "the module's fractions, saturated"},
    {"PShift",                ePrecControl,    "phase word; the taps in doubles"},
    {"Reverb",                ePrecWords,      "every stored value rounded down to the 24-bit grid and clamped"},
    {"Scratch",               ePrecControl,    "ratio and phase words; the taps in doubles"},
    // In/Out
    {"2 Outputs",             ePrecDoubles,    "saturated at twice full scale"},
    {"4 Outputs",             ePrecDoubles,    "saturated at twice full scale"},
    {"2 Inputs",              ePrecDoubles,    "-"},
    {"4 Inputs",              ePrecDoubles,    "-"},
    {"FX Input",              ePrecDoubles,    "-"},
    {"Keyboard",              ePrecNone,       "-"},
    {"Monophonic Keyboard",   ePrecNone,       "-"},
    {"Device",                ePrecNone,       "-"},
    {"Status",                ePrecNone,       "-"},
    {"Note Detector",         ePrecNone,       "-"},
    {"Name Bar",              ePrecNone,       "-"},
    // Switches
    {"SwOnOffM",              ePrecNone,       "-"},
    {"SwOnOffT",              ePrecNone,       "-"},
    {"Sw2-1",                 ePrecNone,       "-"},
    {"Sw2-1M",                ePrecNone,       "-"},
    {"Sw4-1",                 ePrecNone,       "-"},
    {"Sw8-1",                 ePrecNone,       "-"},
    {"Sw1-2",                 ePrecNone,       "-"},
    {"Sw1-2M",                ePrecNone,       "-"},
    {"Sw1-4",                 ePrecNone,       "-"},
    {"Sw1-8",                 ePrecNone,       "-"},
    {"ValSw2-1",              ePrecNone,       "-"},
    {"ValSw1-2",              ePrecNone,       "-"},
    {"Mux8-1",                ePrecControl,    "Ctrl's word >> 17 picks the input"},
    {"Mux1-8",                ePrecControl,    "Ctrl's word >> 17 picks the output"},
    {"Mux8-1X",               ePrecDoubles,    "-"},
    {"S&H",                   ePrecNone,       "a copy"},
    {"T&H",                   ePrecNone,       "a copy"},
    {"WindSw",                ePrecNone,       "-"},
    // Logic
    {"Invert",                ePrecNone,       "-"},
    {"Pulse",                 ePrecNone,       "-"},
    {"Delay",                 ePrecNone,       "-"},
    {"Gate",                  ePrecNone,       "-"},
    {"FlipFlop",              ePrecNone,       "-"},
    {"ClkDiv",                ePrecNone,       "-"},
    {"8Counter",              ePrecNone,       "-"},
    {"BinCounter",            ePrecNone,       "-"},
    {"ADConv",                ePrecWords,      "the input's word, bit by bit"},
    {"DAConv",                ePrecWords,      "the bits as a word"},
    // Sequencers
    {"Sequencer Event",       ePrecWords,      "the module's own arithmetic"},
    {"Sequencer Values",      ePrecWords,      "the module's own arithmetic"},
    {"Sequencer Level",       ePrecWords,      "the module's own arithmetic"},
    {"Sequencer Note",        ePrecWords,      "the module's own arithmetic"},
    {"Sequencer Controlled",  ePrecWords,      "Ctrl's word picks the step"},
    // Random
    {"Random A",              ePrecControl,    "step words; the one-pole in doubles"},
    {"Random B",              ePrecControl,    "step words; the one-pole in doubles"},
    {"Rnd Clock A",           ePrecControl,    "seed and step words"},
    {"Rnd Clock B",           ePrecControl,    "seed, step and Step M words"},
    {"Rnd Trig",              ePrecControl,    "the draw against the threshold"},
    {"Rnd Pattern",           ePrecWords,      "generator and counter"},
    // Note
    {"Note Quantiser",        ePrecWords,      "the module's own arithmetic"},
    {"Key Quantiser",         ePrecWords,      "the reference model's integer arithmetic"},
    {"Partial Quantiser",     ePrecWords,      "-"},
    {"Note Scaler",           ePrecSaturating, "saturated as a word"},
    {"Glide",                 ePrecDoubles,    "-"},
    {"Pitch Tracker",         ePrecDoubles,    "-"},
    {"Zero Crossing Counter", ePrecDoubles,    "-"},
    {"Level Scaler",          ePrecSaturating, "gain and output saturated"},
    // MIDI
    {"CtrlSend",              ePrecSaturating, "the signal saturated as a word, then the controller value"},
    {"PCSend",                ePrecNone,       "-"},
    {"CtrlRcv",               ePrecNone,       "-"},
    {"NoteRcv",               ePrecNone,       "-"},
    {"NoteZone",              ePrecNone,       "-"},
    {"Automate",              ePrecNone,       "-"},
    {"NoteSend",              ePrecNone,       "-"},
};

#define PRECISION_COUNT (sizeof(kPrecision) / sizeof(kPrecision[0]))

#define EVIDENCE_COUNT (sizeof(kEvidence) / sizeof(kEvidence[0]))
#define GROUP_COUNT    (sizeof(kGroups) / sizeof(kGroups[0]))
#define LINE_BYTES     (4096)

static const char * module_name(tModuleType type) {
    const char * name = "?";

    for (uint32_t e = 0; e < array_size_palette_list(); e++) {
        if (gPaletteList[e].moduleType == type) {
            name = gPaletteList[e].menuLabel;
            break;
        }
    }
    return name;
}

static int evidence_index(const char * name) {
    int index = -1;

    for (uint32_t e = 0; e < EVIDENCE_COUNT; e++) {
        if (strcmp(kEvidence[e].name, name) == 0) {
            index = (int)e;
            break;
        }
    }
    return index;
}

static int precision_index(const char * name) {
    for (uint32_t r = 0; r < PRECISION_COUNT; r++) {
        if (strcmp(kPrecision[r].name, name) == 0) {
            return (int)r;
        }
    }
    return -1;
}

static void append_name(char * line, uint32_t * n, const char * name) {
    snprintf(line + strlen(line), LINE_BYTES - strlen(line), "%s%s", (*n > 0) ? ", " : "", name);
    (*n)++;
}

static const char * cell(const char * line) {
    return (line[0] != '\0') ? line : "-";
}

// notes §1 - the aspects a module has, read off the module tables (Docs/module-aspect-audit-design.md)
typedef enum {
    eAspectCore = 0,
    eAspectTiming,
    eAspectDials,
    eAspectInputs,
    eAspectLevel,
    eAspectModes,
    eAspectMeters,
    eAspectLeds,
    ASPECT_COUNT
} tAspect;

static const char * kAspectName[ASPECT_COUNT] = {"Core law", "Timing", "Dials", "Inputs", "Level / gain", "Modes / On-Off", "Meters", "LEDs"};

#define MAX_NAMED_TYPES  (256)
#define MAX_NAMED_PARAMS (MAX_NUM_PARAMETERS)
#define PARAM_NAME_BYTES (24)

// Parameter names by module type and index, from Docs/param-validation.md - the table rows carry few labels
static char sParamName[MAX_NAMED_TYPES][MAX_NAMED_PARAMS][PARAM_NAME_BYTES];

static void load_param_names(const char * path) {
    FILE *   file = fopen(path, "r");
    char     text[512];
    int      type = -1;

    if (file == NULL) {
        fprintf(stderr, "cannot read %s - dials will be numbered, not named\n", path);
        return;
    }

    while (fgets(text, sizeof(text), file) != NULL) {
        const char * header = strstr(text, "[module type ");
        unsigned     index  = 0;
        int          used   = 0;

        if (header != NULL) {
            type = ((sscanf(header, "[module type %d]", &type) == 1) && (type < MAX_NAMED_TYPES)) ? type : -1;
        } else if ((type >= 0) && (sscanf(text, " %u. %n", &index, &used) == 1) && (used > 0) && (index < MAX_NAMED_PARAMS)) {
            // The name runs up to the first run of two spaces: "Pitch Type  Menu"
            const char * name = text + used;
            const char * end  = strstr(name, "  ");
            size_t       len  = (end != NULL) ? (size_t)(end - name) : strcspn(name, "\n");

            if ((len > 0) && (strncmp(name, "(unnamed)", 9) != 0)) {
                snprintf(sParamName[type][index], PARAM_NAME_BYTES, "%.*s", (int)len, name);
            }
        }
    }
    fclose(file);
}

static bool is_switch_param(tParamType type) {
    switch (type) {
        case paramTypeBypass:
        case paramTypeEnable:
        case paramTypeToggle:
        case paramTypePush:
        case paramTypeMenu:
        case paramTypeStrMap:
        case paramTypeRadioEdit:
        case paramTypeOscWave:
        case paramTypeCustomData:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

static bool is_time_param(tParamType type) {
    switch (type) {
        case paramTypeLFORate:
        case paramTypeADRTime:
        case paramTypePulseTime:
        case paramTypeTime:
        case paramTypeTimeClk:
        case paramTypeFlangerRate:
        case paramTypePhaserRate:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

static void append_item(char * line, uint32_t * n, const char * label, uint32_t index, const char * fallback) {
    char name[64];

    if ((label != NULL) && (label[0] != '\0')) {
        snprintf(name, sizeof(name), "%s", label);
    } else {
        snprintf(name, sizeof(name), "%s %u", fallback, (unsigned)(index + 1u));
    }
    append_name(line, n, name);
}

// One module's aspects: items[a] lists what the aspect covers (its dials, its jacks), count[a] how many.
static bool group_has_level(tPaletteGroup group) {
    switch (group) {
        case palGroupOsc:
        case palGroupFilter:
        case palGroupEnv:
        case palGroupMixer:
        case palGroupLevel:
        case palGroupShaper:
        case palGroupDelay:
        case palGroupFx:
        case palGroupIo:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

static void module_aspects(tModuleType type, tPaletteGroup group, char items[ASPECT_COUNT][LINE_BYTES], uint32_t count[ASPECT_COUNT],
                           bool has[ASPECT_COUNT]) {
    uint32_t paramIndex  = 0;
    bool     anyOut      = false;
    uint32_t inIndex     = 0;

    memset(items, 0, sizeof(char) * ASPECT_COUNT * LINE_BYTES);
    memset(count, 0, sizeof(uint32_t) * ASPECT_COUNT);
    memset(has, 0, sizeof(bool) * ASPECT_COUNT);

    for (uint32_t r = 0; r < array_size_param_location_list(); r++) {
        const tParamLocation * row = &paramLocationList[r];

        if (row->moduleType != type) {
            continue;
        }

        const char * label = ((type < MAX_NAMED_TYPES) && (paramIndex < MAX_NAMED_PARAMS) && (sParamName[type][paramIndex][0] != '\0'))
                             ? sParamName[type][paramIndex] : row->label;

        if (is_switch_param(row->type)) {
            append_item(items[eAspectModes], &count[eAspectModes], label, paramIndex, "param");
        } else {
            append_item(items[eAspectDials], &count[eAspectDials], label, paramIndex, "dial");

            if (is_time_param(row->type)) {
                append_item(items[eAspectTiming], &count[eAspectTiming], label, paramIndex, "dial");
            }
        }
        paramIndex++;
    }

    for (uint32_t r = 0, modeIndex = 0; r < array_size_mode_location_list(); r++) {
        if (modeLocationList[r].moduleType == type) {
            append_item(items[eAspectModes], &count[eAspectModes], modeLocationList[r].label, modeIndex++, "mode");
        }
    }

    for (uint32_t r = 0; r < array_size_connector_location_list(); r++) {
        const tConnectorLocation * jack = &connectorLocationList[r];

        if (jack->moduleType != type) {
            continue;
        }

        if (jack->direction != connectorDirIn) {
            anyOut = true;
            continue;
        }

        // Every input, signal or modulation: what an unpatched one reads is part of the aspect
        {
            static const char * kColour[] = {"red", "blue", "yellow"};
            char                 fallback[24];
            bool                 named    = (jack->label != NULL) && (jack->label[0] != '\0') && (strcmp(jack->label, "-") != 0)
                                            && (strcmp(jack->label, "--") != 0);

            snprintf(fallback, sizeof(fallback), "in %u (%s)", (unsigned)(inIndex + 1u),
                     ((uint32_t)jack->type < 3u) ? kColour[jack->type] : "?");
            append_name(items[eAspectInputs], &count[eAspectInputs], named ? jack->label : fallback);
        }
        inIndex++;
    }

    has[eAspectCore]      = anyOut || (group == palGroupIo);
    has[eAspectLevel]     = (anyOut || (group == palGroupIo)) && group_has_level(group);
    has[eAspectDials]     = (count[eAspectDials] > 0);
    has[eAspectTiming]    = (count[eAspectTiming] > 0);
    has[eAspectInputs] = (count[eAspectInputs] > 0);
    has[eAspectModes]     = (count[eAspectModes] > 0);
    // Meters: the level indications. LEDs: the lamps, and the sequencers' step position, which arrives as a
    // "volume" but is a position, not a level
    for (uint32_t r = 0; r < array_size_volume_location_list(); r++) {
        const tVolumeLocation * meter = &volumeLocationList[r];

        if (meter->moduleType != type) {
            continue;
        }

        switch (meter->volumeType) {
            case volumeTypeMono:      append_name(items[eAspectMeters], &count[eAspectMeters], "mono meter");
                break;
            case volumeTypeStereo:    append_name(items[eAspectMeters], &count[eAspectMeters], "stereo meter");
                break;
            case volumeTypeQuad:      append_name(items[eAspectMeters], &count[eAspectMeters], "quad meter");
                break;
            case volumeTypeCompress:  append_name(items[eAspectMeters], &count[eAspectMeters], "gain-reduction LEDs");
                break;
            case volumeTypeSequencer: append_name(items[eAspectLeds], &count[eAspectLeds], "step position");
                break;
            case volumeTypeNone:
                break;
        }
    }
    {
        uint32_t lamps = 0;
        uint32_t multi = 0;
        char     text[48];

        for (uint32_t r = 0; r < array_size_led_location_list(); r++) {
            if (ledLocationList[r].moduleType == type) {
                lamps += (ledLocationList[r].ledType == ledTypeYes) ? 1u : 0u;
                multi += (ledLocationList[r].ledType == ledTypeMultiBit) ? 1u : 0u;
            }
        }

        if (lamps > 0) {
            snprintf(text, sizeof(text), (lamps == 1) ? "%u lamp" : "%u lamps", (unsigned)lamps);
            append_name(items[eAspectLeds], &count[eAspectLeds], text);
        }

        if (multi > 0) {
            snprintf(text, sizeof(text), "a %u-LED group on one value", (unsigned)multi);
            append_name(items[eAspectLeds], &count[eAspectLeds], text);
        }
    }
    has[eAspectMeters] = (count[eAspectMeters] > 0);
    has[eAspectLeds]   = (count[eAspectLeds] > 0);
}

typedef struct {
    const char * module;
    tAspect      aspect;
    tState       state;
    const char * refs;
    const char * g2;
    const char * open;
} tAspectEvidence;

// notes §1 - the evidence one aspect at a time; an aspect with no row reads Unknown
static const tAspectEvidence kAspectEvidence[] = {
    {"Mixer 4-1 C", eAspectCore,      eConfirmed, "§3",              "Exp taper 0.99x^3 + 0.01x, 218 steps within 0.01 dB (09-12)", "-"},
    {"Mixer 4-1 C", eAspectDials,     eConfirmed, "§3",              "as Core law; Pad 0/-6/-12 (09-07)", "-"},
    {"Mixer 4-1 C", eAspectMeters,    eConfirmed, "§1.1, notes §191", "law on 82 steps (09-12); a chord meters one voice, 7-9 on both (10-10); clip held 1 s", "-"},
    {"Mixer 4-1 S", eAspectMeters,    eModelled,  "§1.1",            "FX area under a chord: 7-9 on the engine, 6-7 on the G2 (10-10)", "is 04 Chris' Pad's FX side hot? (todo)"},
    {"Pan",         eAspectCore,      eConfirmed, "§4.2",            "19 settings, Log and Lin, every point within 0.001 (09-12)", "-"},
    {"Pan",         eAspectInputs,    eConfirmed, "§4.3",            "LFO at PanMod 31: +-6.3 dB on both (10-10)", "the signal input's level not compared"},
    {"X-Fade",      eAspectCore,      eConfirmed, "§4.2",            "as Pan", "-"},
    {"X-Fade",      eAspectInputs,    eModelled,  "§4.3",            "-", "mod depth: Pan's law, not captured"},
    {"Fade 1-2",    eAspectInputs,    eApproximate, "§4.3",          "-", "mod depth still 4 x the input; its dial scales differently from Pan's"},
    {"Fade 2-1",    eAspectInputs,    eApproximate, "§4.3",          "-", "as Fade 1-2"},
    {"2 Outputs",   eAspectInputs,    eConfirmed, "notes §167",      "an unpatched socket is silent, outputs 1/2 and the FX bus (10-10)", "-"},
    {"2 Outputs",   eAspectLevel,     eConfirmed, "§63",             "Pad +6 dB (09-07)", "-"},
    {"4 Outputs",   eAspectLevel,     eConfirmed, "-",               "Pad +6 dB (09-07)", "-"},
    {"FX Input",    eAspectLevel,     eConfirmed, "capture-inventory", "Pad +6/0/-6/-12 dB (09-07)", "-"},
    {"FX Input",    eAspectCore,      eConfirmed, "notes §167",      "a side left unpatched at the 2-Out arrives silent (10-10)", "-"},
    {"LFO A",       eAspectCore,      eConfirmed, "§28, notes §63",  "never restarted by a note; three notes start at three phases on both (10-10)", "waves not captured"},
    // Meters and LEDs (2026-10-10): what the engine drives; a module it leaves dark shows only a connected G2's
    {"Mixer 4-1 B", eAspectMeters, eModelled, "§1.1, notes §191", "the law as Mixer 4-1 C", "-"},
    {"Mixer 8-1 A", eAspectMeters, eModelled, "§1.1, notes §191", "the law as Mixer 4-1 C", "-"},
    {"Mixer 8-1 B", eAspectMeters, eModelled, "§1.1, notes §191", "the law as Mixer 4-1 C", "-"},
    {"MixFader", eAspectMeters, eModelled, "§1.1, notes §191", "the law as Mixer 4-1 C", "-"},
    {"MixStereo", eAspectMeters, eModelled, "§1.1, notes §191", "the law as Mixer 4-1 C", "-"},
    {"2 Outputs", eAspectMeters, eModelled, "§1.1, notes §191", "the law as Mixer 4-1 C", "-"},
    {"4 Outputs", eAspectMeters, eModelled, "§1.1, notes §191", "the law as Mixer 4-1 C", "-"},
    {"2 Inputs", eAspectMeters, eModelled, "§1.1, notes §191", "the law as Mixer 4-1 C", "-"},
    {"4 Inputs", eAspectMeters, eModelled, "§1.1, notes §191", "the law as Mixer 4-1 C", "-"},
    {"FX Input", eAspectMeters, eModelled, "§1.1, notes §191", "the law as Mixer 4-1 C", "-"},
    {"Compressor", eAspectMeters, eModelled, "§25, notes §122", "-", "gain-reduction lamps not compared"},
    {"Phase Filter", eAspectMeters, eModelled, "§1.1", "-", "the output's peak since 10-10, as Comb Filter and Eq Peak; not compared"},
    {"Comb Filter", eAspectMeters, eConfirmed, "§1.1", "its output's peak: 7, now and then 9, on both (10-10)", "-"},
    {"FltVoice", eAspectMeters, eModelled, "§1.1", "-", "the output's peak since 10-10, as Comb Filter and Eq Peak; not compared"},
    {"Eq 2-band", eAspectMeters, eModelled, "§1.1", "-", "the output's peak since 10-10, as Comb Filter and Eq Peak; not compared"},
    {"Eq 3-band", eAspectMeters, eModelled, "§1.1", "-", "the output's peak since 10-10, as Comb Filter and Eq Peak; not compared"},
    {"Eq Peak", eAspectMeters, eConfirmed, "§1.1", "its output's peak: 7, now and then 9, on both (10-10)", "-"},
    {"LFO A", eAspectLeds, eConfirmed, "notes §194", "half of each cycle, same rate and duty as the G2's (10-10)", "-"},
    {"LFO B", eAspectLeds, eModelled, "notes §194", "-", "lit on the positive half; not compared"},
    {"LFO C", eAspectLeds, eModelled, "notes §194", "-", "lit on the positive half; not compared"},
    {"LFO Shp A", eAspectLeds, eModelled, "notes §194", "-", "lit on the positive half; not compared"},
    {"Drum Synth", eAspectLeds, eModelled, "§39.5, notes §194", "-", "the master envelope; not compared"},
    {"FM Operator", eAspectLeds, ePartial, "notes §194", "-", "the engine lights no lamp here"},
    {"Envelope ADSR", eAspectLeds, eConfirmed, "notes §194", "the gate: lit at note-on, dark at key-up through a long release, on both (10-10)", "-"},
    {"Envelope AHD", eAspectLeds, eModelled, "notes §194", "-", "the gate, as Envelope ADSR; not compared"},
    {"Envelope ADR", eAspectLeds, eModelled, "notes §194", "-", "the gate, as Envelope ADSR; not compared"},
    {"Envelop ADDSR", eAspectLeds, eModelled, "notes §194", "-", "the gate, as Envelope ADSR; not compared"},
    {"Envelope H", eAspectLeds, eModelled, "notes §194", "-", "the gate, as Envelope ADSR; not compared"},
    {"Envelope D", eAspectLeds, eModelled, "notes §194", "-", "the gate, as Envelope ADSR; not compared"},
    {"Envelope Multi", eAspectLeds, eModelled, "notes §194", "-", "the gate, as Envelope ADSR; not compared"},
    {"Envelope Mod AHD", eAspectLeds, eModelled, "notes §194", "-", "the gate, as Envelope ADSR; not compared"},
    {"Envelope Mod ADSR", eAspectLeds, eModelled, "notes §194", "-", "the gate, as Envelope ADSR; not compared"},
    {"NoiseGate", eAspectLeds, ePartial, "notes §194", "-", "the engine lights no lamp here"},
    {"Note Detector", eAspectLeds, eModelled, "notes §194", "the output; dark or brief on both in the 10-10 test, never compared lit", "-"},
    {"ValSw2-1", eAspectLeds, eModelled, "notes §194", "-", "lit while Ctrl matches; not compared"},
    {"ValSw1-2", eAspectLeds, eModelled, "notes §194", "-", "lit while Ctrl matches; not compared"},
    {"Mux8-1", eAspectLeds, eModelled, "notes §194", "the input through, mostly the same LEDs (10-10)", "a negative Ctrl shows no LED on the G2, the first LED here"},
    {"Mux1-8", eAspectLeds, eModelled, "notes §194", "-", "built 10-10; not compared"},
    {"Mux8-1X", eAspectLeds, ePartial, "notes §194", "-", "the engine lights no lamp here"},
    {"WindSw", eAspectLeds, eModelled, "notes §194", "the output; dark or brief on both in the 10-10 test, never compared lit", "-"},
    {"Invert", eAspectLeds, eConfirmed, "notes §194", "the output, toggling with the G2's at the same duty (10-10)", "-"},
    {"Pulse", eAspectLeds, eModelled, "notes §194", "the output; dark or brief on both in the 10-10 test, never compared lit", "-"},
    {"Delay", eAspectLeds, eConfirmed, "notes §194", "the output, toggling with the G2's at the same duty (10-10)", "-"},
    {"Gate", eAspectLeds, eModelled, "notes §194", "the output; dark or brief on both in the 10-10 test, never compared lit", "-"},
    {"FlipFlop", eAspectLeds, ePartial, "notes §194", "-", "the engine lights no lamp here"},
    {"8Counter", eAspectLeds, eConfirmed, "notes §194", "the count, one LED stepping up once a clock on both (10-10)", "-"},
    {"BinCounter", eAspectLeds, eModelled, "notes §194", "-", "built 10-10; not compared"},
    {"ADConv", eAspectLeds, eModelled, "notes §194", "-", "built 10-10; not compared"},
    {"Random A", eAspectLeds, eConfirmed, "notes §194", "the rate clock, half of each cycle, at the G2's rate (10-10)", "-"},
    {"Random B", eAspectLeds, eModelled, "notes §194", "-", "the rate clock, as Random A; not compared"},
    {"Pitch Tracker", eAspectLeds, ePartial, "notes §194", "-", "the engine lights no lamp here"},
    {"Sequencer Event", eAspectLeds, ePartial, "-", "-", "the engine sends no step position"},
    {"Sequencer Values", eAspectLeds, ePartial, "-", "-", "the engine sends no step position"},
    {"Sequencer Level", eAspectLeds, ePartial, "-", "-", "the engine sends no step position"},
    {"Sequencer Note", eAspectLeds, ePartial, "-", "-", "the engine sends no step position"},
    {"Sequencer Controlled", eAspectLeds, ePartial, "-", "-", "the engine sends no step position"},
    // Oscillators (2026-10-11) - Confirmed only where the whole aspect was compared on the G2; a part
    // compared is named in the evidence and the rest is open
    {"Osc A", eAspectCore, eConfirmed, "§6.2, §6.3", "captured (G2Captures/osca); its waves as OscC's (09-17)", "-"},
    {"Osc A", eAspectDials, eModelled, "§6.1a, §2", "Semi Tune: pitch exact in every capture", "Freq, Factor and Partial Tune, Cent and Pitch not compared"},
    {"Osc A", eAspectInputs, eModelled, "§6.4", "-", "Pitch and PitchVar depths not compared on the G2"},
    {"Osc A", eAspectLevel, eConfirmed, "§6.3", "the level reference OscC and OscD were measured against (09-12)", "-"},
    {"Osc A", eAspectModes, eModelled, "§6.2, §6.1a", "Waveform: every wave (09-17)", "Pitch Type, Kbt and Bypass not compared"},
    {"Osc B", eAspectCore, eConfirmed, "§6.3", "5 waves x 8 pitches, harmonics within 0.1 dB (09-17)", "Shape 127's one-sample click not modelled"},
    {"Osc B", eAspectDials, eModelled, "§6.3, §6.7, §6.8", "Shape 0-127 on Sqr and DualSaw (09-17); Shape M through a Constant (10-08)", "Tune other than Semi, Cent, Pitch and FM not compared"},
    {"Osc B", eAspectInputs, eModelled, "§6.4, §6.6-§6.8", "Shape Mod: DualSaw at +-0.5 and +-0.75 within 0.1 dB (10-08)", "Pitch, PitchVar, Sync and FM jacks not compared on the G2"},
    {"Osc B", eAspectLevel, eConfirmed, "§6.3", "harmonic levels within 0.1 dB at 8 pitches (09-17); a bare OscB is the rig's calibration (10-11)", "-"},
    {"Osc B", eAspectModes, eModelled, "§6.2, §6.1a, §6.8", "Waveform: all five (09-17)", "Pitch Type, Kbt, FM Lin/Trk and Bypass not compared"},
    {"Osc C", eAspectCore, eConfirmed, "§6.3", "Saw, Sqr50/25/10 and Tri at four pitches, harmonics within 0.1 dB (09-17)", "its Sine not captured"},
    {"Osc C", eAspectDials, eModelled, "§6.1a, §6.8", "Tune in Freq mode at 64 (10-16)", "Tune in Factor and Partial, Cent, Pitch and FM not compared"},
    {"Osc C", eAspectInputs, eModelled, "§6.4, §6.6, §6.8", "-", "Pitch, Sync and FM jacks not compared on the G2"},
    {"Osc C", eAspectLevel, eConfirmed, "§6.3", "against OscA (09-12)", "-"},
    {"Osc C", eAspectModes, eModelled, "§6.1a, §6.2, §6.8", "Wave: Saw, Sqr50/25/10, Tri (09-17); Tune Mode Freq with Kbt on tracks the key, AdditiveG2 (10-16)", "Tune Modes Factor and Partial, FM Type and Bypass not compared"},
    {"Osc D", eAspectCore, eConfirmed, "§6.3", "level and harmonics against OscA (09-12)", "-"},
    {"Osc D", eAspectDials, eModelled, "§6.1a", "-", "Tune Mode other than Semi (read since 09-28) not heard on the G2"},
    {"Osc D", eAspectInputs, eModelled, "§6.4", "-", "its Pitch jack not compared"},
    {"Osc D", eAspectLevel, eConfirmed, "§6.3", "against OscA (09-12)", "-"},
    {"Osc D", eAspectModes, eModelled, "§6.2", "Wave: against OscA (09-12)", "Kbt and Bypass not compared"},
    {"Osc Phase Mod", eAspectCore, eModelled, "§53", "-", "no capture"},
    {"Osc Shape A", eAspectCore, eModelled, "§27", "through OscShpB's captures only", "no OscShpA capture of its own"},
    {"Metallic Noise", eAspectCore, eModelled, "§66", "-", "no capture"},
    {"Osc String", eAspectCore, eModelled, "§70.4", "-", "no capture"},
    {"Driver", eAspectCore, eModelled, "§70.5", "-", "no capture"},
    {"Resonator", eAspectCore, eModelled, "§70.4a", "-", "no capture"},
    {"Osc Master", eAspectCore, eModelled, "§51", "-", "no capture"},
    {"Osc Phase Mod", eAspectDials, eModelled, "§53", "-", "-"},
    {"Osc Shape A", eAspectDials, eModelled, "§27, §6.1a, §6.6-§6.8", "-", "-"},
    {"Metallic Noise", eAspectDials, eModelled, "§66", "-", "-"},
    {"Osc String", eAspectDials, eModelled, "§70.4", "-", "-"},
    {"Driver", eAspectDials, eModelled, "§70.5", "-", "-"},
    {"Resonator", eAspectDials, eModelled, "§70.4a", "-", "-"},
    {"Osc Master", eAspectDials, eModelled, "§51", "-", "-"},
    {"Osc Phase Mod", eAspectInputs, eModelled, "§53", "-", "-"},
    {"Osc Shape A", eAspectInputs, eModelled, "§27, §6.1a, §6.6-§6.8", "-", "-"},
    {"Metallic Noise", eAspectInputs, eModelled, "§66", "-", "-"},
    {"Osc String", eAspectInputs, eModelled, "§70.4", "-", "-"},
    {"Driver", eAspectInputs, eModelled, "§70.5", "-", "-"},
    {"Resonator", eAspectInputs, eModelled, "§70.4a", "-", "-"},
    {"Osc Master", eAspectInputs, eModelled, "§51", "-", "-"},
    {"Osc Phase Mod", eAspectLevel, eModelled, "§53", "-", "-"},
    {"Osc Shape A", eAspectLevel, eModelled, "§27", "-", "-"},
    {"Metallic Noise", eAspectLevel, eModelled, "§66", "-", "-"},
    {"Osc String", eAspectLevel, eModelled, "§70.4", "-", "-"},
    {"Driver", eAspectLevel, eModelled, "§70.5", "-", "-"},
    {"Resonator", eAspectLevel, eModelled, "§70.4a", "-", "-"},
    {"Osc Master", eAspectLevel, eModelled, "§51", "-", "-"},
    {"Osc Phase Mod", eAspectModes, eModelled, "§53", "-", "-"},
    {"Osc Shape A", eAspectModes, eModelled, "§27, §6.1a, §6.6-§6.8", "-", "-"},
    {"Metallic Noise", eAspectModes, eModelled, "§66", "-", "-"},
    {"Osc String", eAspectModes, eModelled, "§70.4", "-", "-"},
    {"Driver", eAspectModes, eModelled, "§70.5", "-", "-"},
    {"Resonator", eAspectModes, eModelled, "§70.4a", "-", "-"},
    {"Osc Master", eAspectModes, eModelled, "§51", "-", "-"},
    {"Osc Shape B", eAspectCore, eConfirmed, "§27, §6.7", "every wave at Shape -1..+1, harmonics within 0.16 dB (10-02); Sine3/4 within 0.001 (09-17)", "Pulse at exactly +-1: click -33 dB against the G2's -41"},
    {"Osc Shape B", eAspectDials, eModelled, "§6.7, §6.8, §6.1a", "Shape and ShpCtrl through a Constant at every wave (10-02)", "Tune other than Semi, Cent, Pitch and FM not compared"},
    {"Osc Shape B", eAspectInputs, eModelled, "§6.4, §6.6-§6.8", "Shape Mod -1..+1 at every wave (10-02)", "Pitch, PitchVar, Sync and FM jacks not compared"},
    {"Osc Shape B", eAspectLevel, eConfirmed, "§27", "harmonics within 0.16 dB (10-02)", "-"},
    {"Osc Shape B", eAspectModes, eModelled, "§27, §6.1a", "Wave: all eight (10-02)", "Pitch Type (read since 10-07), Kbt, FM Type and Bypass not compared"},
    {"Osc Dual", eAspectCore, eConfirmed, "§12.2-§12.5", "square, saw and sub against an OscA sine; Soft at three pitches (09-12)", "-"},
    {"Osc Dual", eAspectDials, eModelled, "§12.2, §12.4", "SqrLvl, SawLvl, SubLvl, PW and Phase (09-12); Sqr M and Saw M at full (10-03)", "Tune other than Semi, Cent and Pitch not compared"},
    {"Osc Dual", eAspectInputs, eModelled, "§12.4, §6.6", "PW and Phase jacks: duty within 0.01%, harmonics within 0.15 dB (10-03)", "Pitch, PitchVar and Sync not compared"},
    {"Osc Dual", eAspectLevel, eConfirmed, "§12.5", "against an OscA sine (09-12)", "meters one value above the G2 at 9 of 10 settings (band-limited edges)"},
    {"Osc Dual", eAspectModes, eModelled, "§12.3, §12.5", "Soft on and off (09-12)", "Tune M, Kbt and Bypass not compared"},
    {"Noise Osc", eAspectCore, eModelled, "§8.2", "the 09-12 capture agrees 110 Hz-1 kHz", "above 1 kHz not re-captured since the §8 model"},
    {"Noise Osc", eAspectDials, eModelled, "§8.3, §8.4", "-", "-"},
    {"Noise Osc", eAspectInputs, eModelled, "§8.4", "-", "-"},
    {"Noise Osc", eAspectLevel, eModelled, "§8.5", "-", "-"},
    {"Noise Osc", eAspectModes, eModelled, "§8.1", "-", "-"},
    {"Noise", eAspectCore, eConfirmed, "§7.2a", "the 09-12 capture reproduced through the desk's shelf to a constant", "-"},
    {"Noise", eAspectDials, eConfirmed, "§7.2a, §7.4", "Color across its range in the 09-12 capture; meters agree at 6 of 9 settings", "the brightest settings meter one value higher"},
    {"Noise", eAspectLevel, eConfirmed, "§7.3, §7.4", "meters agree at 6 of 9 settings (09-12)", "the brightest one value higher"},
    {"Noise", eAspectModes, eModelled, "-", "-", "Bypass not compared"},
    {"Osc Percussion", eAspectCore, eConfirmed, "§40.3", "eight takes at E4: levels within 0.1 dB, pitch exact (09-25)", "-"},
    {"Osc Percussion", eAspectTiming, eConfirmed, "§40.3", "Decay 32/64/100 within 2 ms (09-25)", "-"},
    {"Osc Percussion", eAspectDials, eModelled, "§40", "Decay and Click in the takes (09-25)", "Tune other than Semi, Cent and Pitch not compared"},
    {"Osc Percussion", eAspectInputs, eModelled, "§40", "Trig through the takes (09-25)", "the Pitch jacks not compared"},
    {"Osc Percussion", eAspectLevel, eConfirmed, "§40.3", "3.36 dB over a full-scale sine against 3.45 (09-25)", "-"},
    {"Osc Percussion", eAspectModes, eModelled, "§40", "-", "Tune Mode, Kbt, Punch and Bypass not compared"},
    {"Drum Synth", eAspectCore, eConfirmed, "§39", "noise, click and oscillator energies within 0.8 dB of one constant (09-25); Kick 1-5 band balance within 0.5 dB (10-11)", "-"},
    {"Drum Synth", eAspectTiming, eModelled, "§39", "-", "decay times not compared with the 09-20 sweeps (§39.3)"},
    {"Drum Synth", eAspectDials, eModelled, "§39", "Master Freq at five dials within 0.2% (09-21); the five Kick presets, every band within 0.5 dB (10-11)", "Snare, Tom, Cymbal and Perc presets; dials swept one at a time"},
    {"Drum Synth", eAspectInputs, eModelled, "§39", "Vel and Pitch read from the inputs the G2 uses (a Constant, 09-21)", "their depths not compared"},
    {"Drum Synth", eAspectLevel, eConfirmed, "§39", "the master alone against the G2 (09-25); the five Kick presets within 0.3 dB (10-11)", "-"},
    {"Drum Synth", eAspectModes, eModelled, "§39", "-", "Noise filter type and On not compared"},
    {"FM Operator", eAspectCore, eConfirmed, "§14.2-§14.5, §14.2a", "carrier harmonics 1-10 within 0.2 dB, 0.4 with feedback (10-03); a note on an idle voice starts from L4, Backtalk DX within 0.7 dB note to note (10-16)", "-"},
    {"FM Operator", eAspectDials, eModelled, "§14.2-§14.4", "rates and levels within one 50 ms window; BrPt with L/R-Depth within 0.12 dB (10-03)", "Coarse and Fine readouts against the G2 panel (todo)"},
    {"FM Operator", eAspectInputs, eModelled, "§14.6", "-", "AMod and Pitch not yet compared (to-test); Gate, Note and Vel are read from the voice"},
    {"FM Operator", eAspectLevel, eConfirmed, "§14.5", "8.8 dB below an OscDual square against 8.45 (10-03)", "-"},
    {"FM Operator", eAspectModes, eModelled, "§14", "RateScale 7, L/R-Depth Exp and Lin, Vel 7 (10-03)", "Kbt, Sync, RatioFixed, Detune, AMod, KBEnv and Bypass not compared"},
    {"DX Router", eAspectCore, eConfirmed, "§14.1", "algorithms 1 and 2 through two operators (10-03)", "-"},
    {"DX Router", eAspectInputs, eModelled, "§14.5", "-", "the six inputs from outside the node not compared"},
    {"DX Router", eAspectLevel, eConfirmed, "§14.5", "Main 8.8 dB below an OscDual square against 8.45 (10-03)", "-"},
    {"DX Router", eAspectModes, eModelled, "§14.1", "algorithm 1 and 2, Feedback 3, 5 and 7 (10-03)", "the other algorithms not compared"},
    // Filters (2026-10-11)
    {"LP Filter", eAspectCore, eModelled, "§22", "-", "no response captured"},
    {"LP Filter", eAspectDials, eModelled, "§22", "-", "-"},
    {"LP Filter", eAspectInputs, eModelled, "§22", "-", "-"},
    {"LP Filter", eAspectLevel, eModelled, "§22", "-", "-"},
    {"LP Filter", eAspectModes, eModelled, "§22", "-", "-"},
    {"HP Filter", eAspectCore, eModelled, "§22", "-", "no response captured"},
    {"HP Filter", eAspectDials, eModelled, "§22", "-", "-"},
    {"HP Filter", eAspectInputs, eModelled, "§22", "-", "-"},
    {"HP Filter", eAspectLevel, eModelled, "§22", "-", "-"},
    {"HP Filter", eAspectModes, eModelled, "§22", "-", "-"},
    {"Nord Filter", eAspectCore, eConfirmed, "§23", "36 settings of type, slope, Freq, Res and GC within 1.44 dB, mean under 0.4 (10-11)", "-"},
    {"Nord Filter", eAspectDials, eModelled, "§23", "Freq 50 and 85, Res 0 and 100 (10-11)", "Pitch, Fm Lin and Res M not compared"},
    {"Nord Filter", eAspectInputs, eModelled, "§23.5", "the signal (10-11)", "Pitch, PitchVar, FM and Res jacks not compared"},
    {"Nord Filter", eAspectLevel, eConfirmed, "§23", "the passband in the same responses (10-11)", "-"},
    {"Nord Filter", eAspectModes, eModelled, "§23", "FilterType all four, dB/Oct both, GC on and off (10-11)", "Kbt and Bypass not compared"},
    {"Phase Filter", eAspectCore, eModelled, "§67", "-", "no capture"},
    {"Phase Filter", eAspectDials, eModelled, "§67", "-", "-"},
    {"Phase Filter", eAspectInputs, eModelled, "§67", "-", "-"},
    {"Phase Filter", eAspectLevel, eModelled, "§67", "-", "-"},
    {"Phase Filter", eAspectModes, eModelled, "§67", "-", "-"},
    {"FltVoice", eAspectCore, eModelled, "§56", "-", "no capture"},
    {"FltVoice", eAspectDials, eModelled, "§56", "-", "-"},
    {"FltVoice", eAspectInputs, eModelled, "§56", "-", "-"},
    {"FltVoice", eAspectLevel, eModelled, "§56", "-", "-"},
    {"FltVoice", eAspectModes, eModelled, "§56", "-", "-"},
    {"WahWah", eAspectCore, eModelled, "§69.9", "-", "no capture"},
    {"WahWah", eAspectDials, eModelled, "§69.9", "-", "-"},
    {"WahWah", eAspectInputs, eModelled, "§69.9", "-", "-"},
    {"WahWah", eAspectLevel, eModelled, "§69.9", "-", "-"},
    {"WahWah", eAspectModes, eModelled, "§69.9", "-", "-"},
    {"Vocoder", eAspectCore, eModelled, "§70.8", "-", "no capture"},
    {"Vocoder", eAspectInputs, eModelled, "§70.8", "-", "-"},
    {"Vocoder", eAspectLevel, eModelled, "§70.8", "-", "-"},
    {"Vocoder", eAspectModes, eModelled, "§70.8", "-", "-"},
    {"Classic Filter", eAspectCore, eConfirmed, "§21", "04 Chris' Pad within 0.5 dB 100 Hz-12 kHz; a high-Res test patch's peak within 0.04 semitone (09-28)", "-"},
    {"Classic Filter", eAspectDials, eModelled, "§21, §26.2", "Freq and Res in the 09-28 tests", "Env not compared"},
    {"Classic Filter", eAspectInputs, eModelled, "§21", "the signal through the 09-28 tests", "the Env and Pitch jacks not compared"},
    {"Classic Filter", eAspectLevel, eModelled, "§21", "-", "absolute level not compared on its own"},
    {"Classic Filter", eAspectModes, eModelled, "§21", "-", "the dB/Oct tap, Kbt and Bypass not compared since the 09-14 law"},
    {"Multi Filter", eAspectCore, eConfirmed, "§10", "54 noise responses within 0.5-0.6 dB mean, 1.1 dB worst (09-12)", "-"},
    {"Multi Filter", eAspectDials, eConfirmed, "§10", "Freq and Res in the 54 responses (09-12); Pitch at +8 units within 1% (10-08)", "-"},
    {"Multi Filter", eAspectInputs, eModelled, "§10", "the signal and FreqM at +8 units within 1% (10-08)", "the direct Pitch jack not compared"},
    {"Multi Filter", eAspectLevel, eConfirmed, "§10", "within the 54 responses, the 6 dB BP included (09-12)", "-"},
    {"Multi Filter", eAspectModes, eModelled, "§10", "LP, BP and HP, 12 and 24 dB in the responses (09-12)", "GC off not captured; Kbt and Bypass not compared"},
    {"Comb Filter", eAspectCore, eConfirmed, "§13.4", "noise through all three Types (09-12)", "-"},
    {"Comb Filter", eAspectDials, eModelled, "§13", "Freq at four settings to 0.01 samples; Pitch at +63 units (10-08)", "FB, FB Mod and Level not compared on their own"},
    {"Comb Filter", eAspectInputs, eModelled, "§13", "PitchVar at +63 units (10-08)", "the other jacks not compared"},
    {"Comb Filter", eAspectLevel, eModelled, "§13.4", "-", "absolute level not compared on its own"},
    {"Comb Filter", eAspectModes, eModelled, "§13", "Type: all three (09-12)", "Kbt and Bypass not compared"},
    {"Static Filter", eAspectCore, eModelled, "§10.4", "peak heights only (2026-08)", "HP tap term not modelled; a noise capture per type"},
    {"Static Filter", eAspectDials, eModelled, "§10.4", "-", "-"},
    {"Static Filter", eAspectInputs, eModelled, "§10.4", "-", "-"},
    {"Static Filter", eAspectLevel, eModelled, "§10.4", "-", "-"},
    {"Static Filter", eAspectModes, eModelled, "§10.4", "-", "-"},
    {"Eq 2-band", eAspectCore, eModelled, "§11.2", "43 noise settings across the EQs, 0.53 dB mean (09-12)", "the high shelf changed 10-09 (0.3-0.7 dB near 3 kHz), not re-captured"},
    {"Eq 2-band", eAspectDials, eModelled, "§11.2", "Lo and Hi in the 09-12 settings", "re-capture after the 10-09 shelf change; Level not compared"},
    {"Eq 2-band", eAspectInputs, eModelled, "§11", "the signal through the captures", "unpatched behaviour not compared"},
    {"Eq 2-band", eAspectLevel, eModelled, "§11.2", "-", "not compared on its own"},
    {"Eq 2-band", eAspectModes, eModelled, "§11.2", "Lo Freq and Hi Freq in the 09-12 settings", "Bypass not compared"},
    {"Eq 3-band", eAspectCore, eConfirmed, "§11", "0.66 dB mean, 1.42 worst (09-12)", "the 1.42 dB setting (mid -13.5 dB at 8 kHz) not explained"},
    {"Eq 3-band", eAspectDials, eModelled, "§11", "Lo, MidGn, Freq and Hi in the 09-12 settings", "Level not compared"},
    {"Eq 3-band", eAspectInputs, eModelled, "§11", "the signal through the captures", "unpatched behaviour not compared"},
    {"Eq 3-band", eAspectLevel, eModelled, "§11", "-", "not compared on its own"},
    {"Eq 3-band", eAspectModes, eModelled, "§11", "Lo Freq and Hi Freq in the 09-12 settings", "Bypass not compared"},
    {"Eq Peak", eAspectCore, eConfirmed, "§11.3, §11.5", "0.57 dB mean (09-12)", "-"},
    {"Eq Peak", eAspectDials, eModelled, "§11.3, §11.5", "Freq, Gain and BW in the 09-12 settings", "Level not compared"},
    {"Eq Peak", eAspectInputs, eModelled, "§11", "the signal through the captures", "unpatched behaviour not compared"},
    {"Eq Peak", eAspectLevel, eModelled, "§11", "-", "not compared on its own"},
    {"Eq Peak", eAspectModes, eModelled, "§11", "-", "Bypass not compared"},
    // Envelopes (2026-10-11)
    {"Envelope ADSR", eAspectCore, eConfirmed, "§17.1-§17.3", "time law and curves against the 08-24 and 09-07 captures", "-"},
    {"Envelope ADSR", eAspectTiming, eConfirmed, "§17.1", "Attack, Decay and Rel against the same captures", "-"},
    {"Envelope ADSR", eAspectDials, eConfirmed, "§17.1, §17.6", "Attack, Decay, Rel; Sustain 32/64/100/127 within 0.04 of full, signed through a LevMult against the G2 (10-26)", "-"},
    {"Envelope ADSR", eAspectInputs, eConfirmed, "§17.4, §17.5", "Gate, and AM at +32 units, signed through a LevMult against the G2 (10-26)", "-"},
    {"Envelope ADSR", eAspectLevel, eConfirmed, "§17.6", "every Output Type within 0.03 of full, signed through a LevMult against the G2 (10-26)", "-"},
    {"Envelope ADSR", eAspectModes, eConfirmed, "§17.2, §17.6, §17.7", "all four Env Shapes and six Output Types, signed through a LevMult against the G2 (10-26)", "KB and Reset not compared"},
    {"Envelope AHD", eAspectCore, eConfirmed, "§17.11a", "its segments from 10 ms to 3.35 s within 1% (10-08)", "-"},
    {"Envelope AHD", eAspectTiming, eConfirmed, "§17.11a", "as Core law (10-08)", "-"},
    {"Envelope AHD", eAspectDials, eConfirmed, "§17.11a", "Attack, Hold, Release, signed through a LevMult against the G2 (10-26)", "-"},
    {"Envelope AHD", eAspectInputs, eConfirmed, "§17.11a", "Trig, signed through a LevMult against the G2 (10-26)", "AM not compared"},
    {"Envelope AHD", eAspectLevel, eConfirmed, "§17.11a", "four OutTypes, signed through a LevMult against the G2 (10-26)", "-"},
    {"Envelope AHD", eAspectModes, eConfirmed, "§17.11a", "all four Shapes and OutTypes, signed through a LevMult against the G2 (10-26)", "KB and Reset not compared"},
    {"Envelope ADR", eAspectCore, eConfirmed, "§17.9", "against the G2 on a held key (10-25)", "-"},
    {"Envelop ADDSR", eAspectCore, eModelled, "§17.9, §17.9a", "-", "stages never heard against the G2; release fixed 10-09"},
    {"Envelope Multi", eAspectCore, eConfirmed, "§17.11", "against the G2 on a held key (10-25)", "-"},
    {"Envelope ADR", eAspectTiming, eConfirmed, "§17.9", "against the G2 on a held key (10-25)", "-"},
    {"Envelop ADDSR", eAspectTiming, eModelled, "§17.9, §17.9a", "-", "-"},
    {"Envelope Multi", eAspectTiming, eConfirmed, "§17.11", "against the G2 on a held key (10-25)", "-"},
    {"Envelope ADR", eAspectDials, eConfirmed, "§17.9", "against the G2 on a held key (10-25)", "-"},
    {"Envelop ADDSR", eAspectDials, eModelled, "§17.9, §17.9a", "-", "-"},
    {"Envelope Multi", eAspectDials, eConfirmed, "§17.11", "against the G2 on a held key (10-25)", "-"},
    {"Envelope ADR", eAspectInputs, eConfirmed, "§17.9", "against the G2 on a held key (10-25)", "-"},
    {"Envelop ADDSR", eAspectInputs, eModelled, "§17.9, §17.9a", "-", "-"},
    {"Envelope Multi", eAspectInputs, eConfirmed, "§17.11", "against the G2 on a held key (10-25)", "-"},
    {"Envelope ADR", eAspectLevel, eModelled, "§17.9", "-", "-"},
    {"Envelop ADDSR", eAspectLevel, eModelled, "§17.9, §17.9a", "-", "-"},
    {"Envelope Multi", eAspectLevel, eModelled, "§17.11", "-", "-"},
    {"Envelope ADR", eAspectModes, eModelled, "§17.9", "-", "-"},
    {"Envelop ADDSR", eAspectModes, eModelled, "§17.9, §17.9a", "-", "-"},
    {"Envelope Multi", eAspectModes, eModelled, "§17.11", "-", "-"},
    {"Envelope H", eAspectCore, eConfirmed, "§17.9a", "Hold 12-64 within 0.1 ms (10-08)", "-"},
    {"Envelope H", eAspectTiming, eConfirmed, "§17.9a", "as Core law (10-08)", "-"},
    {"Envelope H", eAspectDials, eConfirmed, "§17.9a", "Hold 12-64 within 0.1 ms (10-08)", "-"},
    {"Envelope H", eAspectInputs, eConfirmed, "§17.9a", "Trig, and AM at +32 units, signed through a LevMult against the G2 (10-26)", "-"},
    {"Envelope H", eAspectLevel, eConfirmed, "§17.9a", "four OutTypes, steps within 1 ms, signed through a LevMult against the G2 (10-26)", "-"},
    {"Envelope H", eAspectModes, eConfirmed, "§17.9a", "all four OutTypes, signed through a LevMult against the G2 (10-26)", "-"},
    {"Envelope D", eAspectCore, eConfirmed, "§17.9", "against the G2 on a held key (10-25)", "-"},
    {"Envelope D", eAspectTiming, eConfirmed, "§17.9", "against the G2 on a held key (10-25)", "-"},
    {"Envelope D", eAspectDials, eConfirmed, "§17.9", "against the G2 on a held key (10-25)", "-"},
    {"Envelope D", eAspectInputs, eConfirmed, "§17.9", "against the G2 on a held key (10-25)", "-"},
    {"Envelope D", eAspectLevel, eModelled, "§17.9", "-", "-"},
    {"Envelope D", eAspectModes, eModelled, "§17.9", "-", "-"},
    {"Envelope Mod AHD", eAspectCore, eConfirmed, "§17.9a, §17.10", "each stage and its mod jack against the G2 at +-32 units (10-26)", "-"},
    {"Envelope Mod AHD", eAspectTiming, eConfirmed, "§17.9a", "each stage and its mod jack against the G2 at +-32 units (10-26)", "-"},
    {"Envelope Mod AHD", eAspectDials, eConfirmed, "§17.10", "each stage and its mod jack against the G2 at +-32 units (10-26)", "-"},
    {"Envelope Mod AHD", eAspectInputs, eConfirmed, "§17.10", "each stage and its mod jack against the G2 at +-32 units (10-26)", "-"},
    {"Envelope Mod AHD", eAspectLevel, eModelled, "§17.10", "-", "-"},
    {"Envelope Mod AHD", eAspectModes, eModelled, "§17.10", "-", "-"},
    {"Envelope Mod ADSR", eAspectCore, eConfirmed, "§17.9, §17.10", "each stage and its mod jack against the G2 at +-32 units (10-26)", "-"},
    {"Envelope Mod ADSR", eAspectTiming, eConfirmed, "§17.10", "each stage and its mod jack against the G2 at +-32 units (10-26)", "-"},
    {"Envelope Mod ADSR", eAspectDials, eConfirmed, "§17.10", "each stage and its mod jack against the G2 at +-32 units (10-26)", "-"},
    {"Envelope Mod ADSR", eAspectInputs, eConfirmed, "§17.10", "each stage and its mod jack against the G2 at +-32 units (10-26)", "-"},
    {"Envelope Mod ADSR", eAspectLevel, eModelled, "§17.10", "-", "-"},
    {"Envelope Mod ADSR", eAspectModes, eModelled, "§17.10", "-", "-"},
    // LFOs (2026-10-11)
    {"LFO A", eAspectTiming, eConfirmed, "§28", "Sub, Lo, Hi and BPM on the G2 (10-26)", "-"},
    {"LFO A", eAspectDials, eConfirmed, "§28, §50", "Rate and Rate M (10-26)", "Kbt not compared"},
    {"LFO A", eAspectInputs, eConfirmed, "§50", "Rate and RateVar (10-26)", "-"},
    {"LFO A", eAspectModes, eConfirmed, "§28", "the four waves at Hi 112 (10-26)", "RndSt/Rnd, OutType, Mode not compared"},
    {"LFO B", eAspectCore, eConfirmed, "§28, §28.4, §54", "four waves and Phase against Snc to 0.002 cycle (10-25)", "-"},
    {"LFO C", eAspectCore, eConfirmed, "§28", "Sine/Tri/Saw/Sqr at 165 Hz: correlation 0.99-1.00, harmonics within 0.7 dB, the saw falling on both (10-25)", "-"},
    {"LFO B", eAspectTiming, eConfirmed, "§28, §28.4, §54", "Hi, Lo, Sub, BPM, Clk (10-25)", "-"},
    {"LFO C", eAspectTiming, eConfirmed, "§28", "rate at Hi, Lo, Sub and BPM within 0.7% (10-25)", "-"},
    {"LFO B", eAspectDials, eConfirmed, "§28, §28.4, §54", "Rate, Rate M, Phase (10-25)", "-"},
    {"LFO C", eAspectDials, eConfirmed, "§28", "Rate and Range (10-25)", "-"},
    {"LFO B", eAspectInputs, eConfirmed, "§28, §28.4, §54", "Rate and RateVar (10-25)", "Phase M not compared"},
    {"LFO C", eAspectInputs, eConfirmed, "§28", "the Rate input at +-12/+24 units within 0.2% (10-25)", "-"},
    {"LFO B", eAspectModes, eConfirmed, "§28, §28.4, §54", "Wave and Range (10-25)", "-"},
    {"LFO C", eAspectModes, eConfirmed, "§28, §28.7", "all six waves and all six Output Types; Neg/NegInv corrected (10-25)", "Mono not compared"},
    {"LFO Shp A", eAspectCore, eConfirmed, "§28.6", "six waves at five Shapes against Snc (10-26)", "-"},
    {"LFO Shp A", eAspectTiming, eConfirmed, "§28.6", "Sub, Lo, Hi, BPM, Clk (10-26)", "-"},
    {"LFO Shp A", eAspectDials, eConfirmed, "§28.4, §28.6", "Rate, Rate M, Shape, Phase, Phase M, Shape M (10-26)", "Kbt not compared"},
    {"LFO Shp A", eAspectInputs, eConfirmed, "§28.4", "Rate, RateVar, Shape M, Phase M, Dir (10-26)", "Rst not compared"},
    {"LFO Shp A", eAspectModes, eConfirmed, "§28.6", "Wave and Range (10-26)", "OutType, Mode not compared"},
    {"Clock Generator", eAspectCore, eConfirmed, "§59", "master clock within 7-17 ppm; the engine within 1-4 ms over 66 s (10-09)", "-"},
    {"Clock Generator", eAspectDials, eModelled, "§59", "Tempo in the 10-09 test", "Swing not compared"},
    {"Clock Generator", eAspectInputs, eModelled, "§59", "-", "Reset not compared"},
    {"Clock Generator", eAspectModes, eModelled, "§59", "Source: Master (10-09)", "Internal source, Sync every and Bypass not compared"},
    // Mixers (2026-10-11)
    {"Mixer 1-1 A", eAspectCore, eConfirmed, "§3.2, §3.6", "all eleven mixers, 106 configurations, within 0.07 dB (09-12)", "-"},
    {"Mixer 1-1 A", eAspectDials, eConfirmed, "§3.2, §3.3", "the levels in the 106 configurations (09-12)", "-"},
    {"Mixer 1-1 A", eAspectInputs, eModelled, "§3.5", "the channels in the 106 configurations (09-12)", "Chain and an unpatched channel not compared on their own"},
    {"Mixer 1-1 A", eAspectLevel, eConfirmed, "§3.6", "within 0.07 dB (09-12)", "-"},
    {"Mixer 1-1 A", eAspectModes, eModelled, "§3.2-§3.4", "curves and Pad in the configurations (09-12)", "the On buttons not compared"},
    {"Mixer 1-1 S", eAspectCore, eConfirmed, "§3.2, §3.6", "all eleven mixers, 106 configurations, within 0.07 dB (09-12)", "-"},
    {"Mixer 1-1 S", eAspectDials, eConfirmed, "§3.2, §3.3", "the levels in the 106 configurations (09-12)", "-"},
    {"Mixer 1-1 S", eAspectInputs, eModelled, "§3.5", "the channels in the 106 configurations (09-12)", "Chain and an unpatched channel not compared on their own"},
    {"Mixer 1-1 S", eAspectLevel, eConfirmed, "§3.6", "within 0.07 dB (09-12)", "-"},
    {"Mixer 1-1 S", eAspectModes, eConfirmed, "§3.2-§3.4", "curves and Pad in the configurations (09-12); On off mutes to -93 dBFS, Exp at Lev 78/40 -12.6/-29.4 dB and Lin -4.3/-10.1 dB against 127, both laws exact (10-16)", "-"},
    {"Mixer 2-1 A", eAspectCore, eConfirmed, "§3.2, §3.6", "all eleven mixers, 106 configurations, within 0.07 dB (09-12)", "-"},
    {"Mixer 2-1 A", eAspectDials, eConfirmed, "§3.2, §3.3", "the levels in the 106 configurations (09-12)", "-"},
    {"Mixer 2-1 A", eAspectInputs, eModelled, "§3.5", "the channels in the 106 configurations (09-12)", "Chain and an unpatched channel not compared on their own"},
    {"Mixer 2-1 A", eAspectLevel, eConfirmed, "§3.6", "within 0.07 dB (09-12)", "-"},
    {"Mixer 2-1 A", eAspectModes, eModelled, "§3.2-§3.4", "curves and Pad in the configurations (09-12)", "the On buttons not compared"},
    {"Mixer 4-1 A", eAspectCore, eConfirmed, "§3.2, §3.6", "all eleven mixers, 106 configurations, within 0.07 dB (09-12)", "-"},
    {"Mixer 4-1 A", eAspectInputs, eModelled, "§3.5", "the channels in the 106 configurations (09-12)", "Chain and an unpatched channel not compared on their own"},
    {"Mixer 4-1 A", eAspectLevel, eConfirmed, "§3.6", "within 0.07 dB (09-12)", "-"},
    {"Mixer 4-1 B", eAspectCore, eConfirmed, "§3.2, §3.6", "all eleven mixers, 106 configurations, within 0.07 dB (09-12)", "-"},
    {"Mixer 4-1 B", eAspectDials, eConfirmed, "§3.2, §3.3", "the levels in the 106 configurations (09-12)", "-"},
    {"Mixer 4-1 B", eAspectInputs, eModelled, "§3.5", "the channels in the 106 configurations (09-12)", "Chain and an unpatched channel not compared on their own"},
    {"Mixer 4-1 B", eAspectLevel, eConfirmed, "§3.6", "within 0.07 dB (09-12)", "-"},
    {"Mixer 4-1 B", eAspectModes, eModelled, "§3.2-§3.4", "curves and Pad in the configurations (09-12)", "the On buttons not compared"},
    {"Mixer 4-1 C", eAspectInputs, eModelled, "§3.5", "the channels in the 106 configurations (09-12)", "Chain and an unpatched channel not compared on their own"},
    {"Mixer 4-1 C", eAspectLevel, eConfirmed, "§3.6", "within 0.07 dB (09-12)", "-"},
    {"Mixer 4-1 C", eAspectModes, eModelled, "§3.2-§3.4", "curves and Pad in the configurations (09-12)", "the On buttons not compared"},
    {"Mixer 4-1 S", eAspectCore, eConfirmed, "§3.2, §3.6", "all eleven mixers, 106 configurations, within 0.07 dB (09-12)", "-"},
    {"Mixer 4-1 S", eAspectDials, eConfirmed, "§3.2, §3.3", "the levels in the 106 configurations (09-12)", "-"},
    {"Mixer 4-1 S", eAspectInputs, eModelled, "§3.5", "the channels in the 106 configurations (09-12)", "Chain and an unpatched channel not compared on their own"},
    {"Mixer 4-1 S", eAspectLevel, eConfirmed, "§3.6", "within 0.07 dB (09-12)", "-"},
    {"Mixer 4-1 S", eAspectModes, eModelled, "§3.2-§3.4", "curves and Pad in the configurations (09-12)", "the On buttons not compared"},
    {"Mixer 2-1 B", eAspectCore, eConfirmed, "§3.2, §3.6", "all eleven mixers, 106 configurations, within 0.07 dB (09-12)", "-"},
    {"Mixer 2-1 B", eAspectDials, eConfirmed, "§3.2, §3.3", "the levels in the 106 configurations (09-12)", "-"},
    {"Mixer 2-1 B", eAspectInputs, eModelled, "§3.5", "the channels in the 106 configurations (09-12)", "Chain and an unpatched channel not compared on their own"},
    {"Mixer 2-1 B", eAspectLevel, eConfirmed, "§3.6", "within 0.07 dB (09-12)", "-"},
    {"Mixer 2-1 B", eAspectModes, eModelled, "§3.2-§3.4", "curves and Pad in the configurations (09-12); Inv cancels", "the On buttons not compared"},
    {"Mixer 8-1 A", eAspectCore, eConfirmed, "§3.2, §3.6", "all eleven mixers, 106 configurations, within 0.07 dB (09-12)", "-"},
    {"Mixer 8-1 A", eAspectInputs, eModelled, "§3.5", "the channels in the 106 configurations (09-12)", "Chain and an unpatched channel not compared on their own"},
    {"Mixer 8-1 A", eAspectLevel, eConfirmed, "§3.6", "within 0.07 dB (09-12)", "-"},
    {"Mixer 8-1 A", eAspectModes, eModelled, "§3.2-§3.4", "curves and Pad in the configurations (09-12)", "the On buttons not compared"},
    {"Mixer 8-1 B", eAspectCore, eConfirmed, "§3.2, §3.6", "all eleven mixers, 106 configurations, within 0.07 dB (09-12)", "-"},
    {"Mixer 8-1 B", eAspectDials, eConfirmed, "§3.2, §3.3", "the levels in the 106 configurations (09-12)", "-"},
    {"Mixer 8-1 B", eAspectInputs, eModelled, "§3.5", "the channels in the 106 configurations (09-12)", "Chain and an unpatched channel not compared on their own"},
    {"Mixer 8-1 B", eAspectLevel, eConfirmed, "§3.6", "within 0.07 dB (09-12)", "-"},
    {"Mixer 8-1 B", eAspectModes, eModelled, "§3.2-§3.4", "curves and Pad in the configurations (09-12)", "the On buttons not compared"},
    {"MixFader", eAspectCore, eConfirmed, "§3.2, §3.6", "all eleven mixers, 106 configurations, within 0.07 dB (09-12)", "-"},
    {"MixFader", eAspectDials, eConfirmed, "§3.2, §3.3", "the levels in the 106 configurations (09-12)", "-"},
    {"MixFader", eAspectInputs, eModelled, "§3.5", "the channels in the 106 configurations (09-12)", "Chain and an unpatched channel not compared on their own"},
    {"MixFader", eAspectLevel, eConfirmed, "§3.6", "within 0.07 dB (09-12)", "-"},
    {"MixFader", eAspectModes, eModelled, "§3.2-§3.4", "curves and Pad in the configurations (09-12)", "the On buttons not compared"},
    {"Fade 1-2", eAspectCore, eConfirmed, "§4.2", "19 settings, every point within 0.001 (09-12)", "-"},
    {"Fade 1-2", eAspectDials, eModelled, "§4.2, §4.3", "Mix at 19 settings, every point within 0.001 (09-12)", "Mix M not compared"},
    {"Fade 1-2", eAspectLevel, eConfirmed, "§4.2", "19 settings, every point within 0.001 (09-12)", "-"},
    {"Fade 2-1", eAspectCore, eConfirmed, "§4.2", "as Fade 1-2 (09-12)", "-"},
    {"Fade 2-1", eAspectDials, eModelled, "§4.2, §4.3", "as Fade 1-2 (09-12)", "Mix M not compared"},
    {"Fade 2-1", eAspectLevel, eConfirmed, "§4.2", "as Fade 1-2 (09-12)", "-"},
    {"X-Fade", eAspectDials, eModelled, "§4.2, §4.3", "as Fade 1-2 (09-12)", "Mix M not compared"},
    {"X-Fade", eAspectLevel, eConfirmed, "§4.2", "as Fade 1-2 (09-12)", "-"},
    {"X-Fade", eAspectModes, eModelled, "§4.2", "-", "not compared"},
    {"Pan", eAspectDials, eConfirmed, "§4.2, §4.3", "Pan at 19 settings (09-12); PanMod 31 against an LFO, +-6.3 dB on both (10-10)", "-"},
    {"Pan", eAspectLevel, eConfirmed, "§4.2", "19 settings, every point within 0.001 (09-12)", "-"},
    {"Pan", eAspectModes, eConfirmed, "§4.2", "Log and Lin, 19 settings each (09-12)", "-"},
    {"MixStereo", eAspectCore, eConfirmed, "§5", "19 settings each, Log and Lin (09-12)", "-"},
    {"MixStereo", eAspectDials, eConfirmed, "§5.1, §5.2", "Lvl and Pan in the 19 settings (09-12)", "-"},
    {"MixStereo", eAspectInputs, eModelled, "§5.3", "the channels in the 09-12 settings", "an unpatched channel not compared"},
    {"MixStereo", eAspectLevel, eConfirmed, "§5.2", "0.14 dB below unity at the extremes, as measured (09-12)", "-"},
};

#define ASPECT_EVIDENCE_COUNT (sizeof(kAspectEvidence) / sizeof(kAspectEvidence[0]))

static int aspect_evidence_index(const char * module, tAspect aspect) {
    for (uint32_t e = 0; e < ASPECT_EVIDENCE_COUNT; e++) {
        if ((kAspectEvidence[e].aspect == aspect) && (strcmp(kAspectEvidence[e].module, module) == 0)) {
            return (int)e;
        }
    }
    return -1;
}

// --aspects: the audit - every modelled module's aspects, with their evidence where there is some
static int print_aspects(const char * paramNames) {
    uint32_t modules                  = 0;
    uint32_t aspectTotal[ASPECT_COUNT] = {0};
    uint32_t aspectState[ASPECT_COUNT][STATE_COUNT + 1] = {{0}};   // the last column is Unknown
    uint32_t all                      = 0;
    uint32_t evidenced                = 0;
    bool     seen[ASPECT_EVIDENCE_COUNT] = {false};
    int      result                   = 0;

    load_param_names(paramNames);

    for (uint32_t g = 0; g < GROUP_COUNT; g++) {
        tModuleType types[256];
        uint32_t    count = palette_group_modules(kGroups[g].group, types, 256);

        printf("\n### %s\n\n", kGroups[g].name);
        printf("| Module | Aspect | Covers | State | Evidence |\n");
        printf("|---|---|---|---|---|\n");

        for (uint32_t i = 0; i < count; i++) {
            tModule      module = {0};
            const char * name   = module_name(types[i]);
            static char  items[ASPECT_COUNT][LINE_BYTES];
            uint32_t     n[ASPECT_COUNT];
            bool         has[ASPECT_COUNT];

            module.type = types[i];

            if (sound_engine_models_module(&module) == false) {
                continue;
            }
            module_aspects(types[i], kGroups[g].group, items, n, has);
            modules++;

            for (uint32_t a = 0; a < ASPECT_COUNT; a++) {
                int e = aspect_evidence_index(name, (tAspect)a);

                if (has[a] == false) {
                    if (e >= 0) {
                        fprintf(stderr, "kAspectEvidence names %s's %s, which it does not have\n", name, kAspectName[a]);
                        result = 1;
                    }
                    continue;
                }

                if (e >= 0) {
                    const tAspectEvidence * row = &kAspectEvidence[e];

                    seen[e] = true;
                    printf("| %s | %s | %s | %s | %s: %s; open: %s |\n", name, kAspectName[a], cell(items[a]),
                           kStateName[row->state], row->refs, row->g2, row->open);
                    evidenced++;
                    aspectState[a][row->state]++;
                } else {
                    printf("| %s | %s | %s | Unknown | - |\n", name, kAspectName[a], cell(items[a]));
                    aspectState[a][STATE_COUNT]++;
                }
                aspectTotal[a]++;
                all++;
            }
        }
    }
    // notes §1 - by state as well, which is where the gaps show
    uint32_t stateTotal[STATE_COUNT + 1] = {0};

    for (uint32_t a = 0; a < ASPECT_COUNT; a++) {
        for (uint32_t st = 0; st <= STATE_COUNT; st++) {
            stateTotal[st] += aspectState[a][st];
        }
    }
    printf("\n| Aspect | Modules |");

    for (uint32_t st = 0; st <= STATE_COUNT; st++) {
        if (stateTotal[st] > 0u) {
            printf(" %s |", (st < STATE_COUNT) ? kStateName[st] : "Unknown");   // empty states left out
        }
    }
    printf("\n|---|---|");

    for (uint32_t st = 0; st <= STATE_COUNT; st++) {
        if (stateTotal[st] > 0u) {
            printf("---|");
        }
    }
    printf("\n");

    for (uint32_t a = 0; a < ASPECT_COUNT; a++) {
        printf("| %s | %u |", kAspectName[a], (unsigned)aspectTotal[a]);

        for (uint32_t st = 0; st <= STATE_COUNT; st++) {
            if (stateTotal[st] > 0u) {
                printf(" %u |", (unsigned)aspectState[a][st]);
            }
        }
        printf("\n");
    }
    printf("| **%u aspects over %u modules, %u with evidence** | |", (unsigned)all, (unsigned)modules, (unsigned)evidenced);

    for (uint32_t st = 0; st <= STATE_COUNT; st++) {
        if (stateTotal[st] > 0u) {
            printf(" **%u** |", (unsigned)stateTotal[st]);
        }
    }
    printf("\n");

    for (uint32_t e = 0; e < ASPECT_EVIDENCE_COUNT; e++) {
        if (seen[e] == false) {
            fprintf(stderr, "kAspectEvidence names '%s', which is not a modelled module\n", kAspectEvidence[e].module);
            result = 1;
        }
    }
    return result;
}

static const char * state_name(uint32_t s) {
    return (s < STATE_COUNT) ? kStateName[s] : "Not implemented";
}

// The doc's state table, with how many modules are in each state; the last row is Not implemented
static const char * kStateMeaning[STATE_COUNT + 1] = {
    "compared with a G2 (a capture, its meters, or a stage patch played against it) and it agrees",
    "a full model in the reference, not yet compared with a G2",
    "follows the manual's description; a full model is still to come",
    "audible, but something about it is known to be missing - the Open column says what",
    "nothing to render (a name, or a module whose output leaves by MIDI)",
    "not modelled: it contributes nothing and is pruned from the chain",
};

// A state no module is in is left out of both tables, so the columns stay the ones in use
static void print_state_table(uint32_t * n) {
    uint32_t all = 0;

    for (uint32_t g = 0; g < GROUP_COUNT; g++) {
        tModuleType types[256];
        uint32_t    count = palette_group_modules(kGroups[g].group, types, 256);

        for (uint32_t i = 0; i < count; i++) {
            tModule module = {0};
            int     e      = evidence_index(module_name(types[i]));

            module.type = types[i];

            if (sound_engine_models_module(&module) == false) {
                n[STATE_COUNT]++;
            } else if (e >= 0) {
                n[kEvidence[e].state]++;
            }
            all++;
        }
    }
    printf("| state | meaning | modules |\n");
    printf("|---|---|---|\n");

    for (uint32_t s = 0; s <= STATE_COUNT; s++) {
        if (n[s] > 0u) {
            printf("| **%s** | %s | %u |\n", state_name(s), kStateMeaning[s], (unsigned)n[s]);
        }
    }
    printf("| **Total** | every module type the palette offers | %u |\n\n", (unsigned)all);
}

int main(int argc, char ** argv) {
    uint32_t total[STATE_COUNT + 1]       = {0};    // the last is Not implemented
    bool     evidenceSeen[EVIDENCE_COUNT] = {false};
    bool     precisionSeen[PRECISION_COUNT] = {false};
    uint32_t precisionTotal[PREC_COUNT]     = {0};
    int      result                       = 0;

    init_module_resource_cache();

    if ((argc > 1) && (strcmp(argv[1], "--aspects") == 0)) {
        return print_aspects((argc > 2) ? argv[2] : "Docs/param-validation.md");
    }
    uint32_t inState[STATE_COUNT + 1] = {0};

    print_state_table(inState);
    printf("| Group |");

    for (uint32_t s = 0; s <= STATE_COUNT; s++) {
        if (inState[s] > 0u) {
            printf(" %s |", state_name(s));
        }
    }
    printf("\n|---|");

    for (uint32_t s = 0; s <= STATE_COUNT; s++) {
        if (inState[s] > 0u) {
            printf("---|");
        }
    }
    printf("\n");

    for (uint32_t g = 0; g < GROUP_COUNT; g++) {
        tModuleType types[256];
        uint32_t    count                      = palette_group_modules(kGroups[g].group, types, 256);
        static char line[STATE_COUNT + 1][LINE_BYTES];
        uint32_t    n[STATE_COUNT + 1]         = {0};

        memset(line, 0, sizeof(line));

        for (uint32_t i = 0; i < count; i++) {
            tModule      module = {0};
            const char * name   = module_name(types[i]);
            int          e      = evidence_index(name);

            module.type = types[i];

            if (sound_engine_models_module(&module) == false) {
                append_name(line[STATE_COUNT], &n[STATE_COUNT], name);

                if (e >= 0) {
                    fprintf(stderr, "kEvidence has a row for '%s', which the engine does not model\n", name);
                    result = 1;
                }
            } else if (e < 0) {
                fprintf(stderr, "'%s' is modelled but has no row in kEvidence\n", name);
                result = 1;
            } else {
                evidenceSeen[e] = true;
                append_name(line[kEvidence[e].state], &n[kEvidence[e].state], name);
            }
        }
        printf("| **%s** |", kGroups[g].name);

        for (uint32_t s = 0; s <= STATE_COUNT; s++) {
            if (inState[s] > 0u) {
                printf(" %s |", cell(line[s]));
            }
            total[s] += n[s];
        }
        printf("\n");
    }
    {
        uint32_t all = 0;

        for (uint32_t s = 0; s <= STATE_COUNT; s++) {
            all += total[s];
        }
        printf("| **Total %u** |", (unsigned)all);

        for (uint32_t s = 0; s <= STATE_COUNT; s++) {
            if (inState[s] > 0u) {
                printf(" **%u** |", (unsigned)total[s]);
            }
        }
        printf("\n");
    }

    for (uint32_t g = 0; g < GROUP_COUNT; g++) {
        tModuleType types[256];
        uint32_t    count = palette_group_modules(kGroups[g].group, types, 256);

        printf("\n### %s\n\n", kGroups[g].name);
        printf("| Module | State | Reference | On the G2 | Open | Fixed point |\n");
        printf("|---|---|---|---|---|---|\n");

        for (uint32_t i = 0; i < count; i++) {
            int e = evidence_index(module_name(types[i]));

            if ((e >= 0) && (evidenceSeen[e] == true)) {
                const tEvidence * row = &kEvidence[e];

                int               p   = precision_index(row->name);

                if (p < 0) {
                    fprintf(stderr, "'%s' has no row in kPrecision\n", row->name);
                    result = 1;
                    continue;
                }
                precisionSeen[p] = true;
                precisionTotal[kPrecision[p].precision]++;

                if (strcmp(kPrecision[p].note, "-") == 0) {
                    printf("| %s | %s | %s | %s | %s | %s |\n", row->name, kStateName[row->state], row->refs, row->g2, row->open,
                           kPrecisionName[kPrecision[p].precision]);
                } else {
                    printf("| %s | %s | %s | %s | %s | %s: %s |\n", row->name, kStateName[row->state], row->refs, row->g2, row->open,
                           kPrecisionName[kPrecision[p].precision], kPrecision[p].note);
                }
            }
        }
    }

    printf("\n### Fixed point\n\n| Arithmetic | Modules |\n|---|---|\n");

    for (uint32_t k = 0; k < PREC_COUNT; k++) {
        printf("| %s | %u |\n", kPrecisionName[k], (unsigned)precisionTotal[k]);
    }

    for (uint32_t r = 0; r < PRECISION_COUNT; r++) {
        if (precisionSeen[r] == false) {
            fprintf(stderr, "kPrecision names '%s', which has no status row\n", kPrecision[r].name);
            result = 1;
        }
    }

    for (uint32_t e = 0; e < EVIDENCE_COUNT; e++) {
        if (evidenceSeen[e] == false) {
            fprintf(stderr, "kEvidence names '%s', which the palette does not offer or the engine does not model\n",
                    kEvidence[e].name);
            result = 1;
        }
    }
    return result;
}
