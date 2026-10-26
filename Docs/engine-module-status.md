# Sound engine: module status

What the local sound engine does with each module type the palette offers, by palette group, under
the names the palette itself shows - and, for each, what it has been checked against. Section numbers
are `sound-engine-reference.md`'s, where each module's model is described.
The first table below defines the states and counts the modules in each.

Modelled is not the same as checked. A model that has never been put beside a G2 has not been
checked, however complete it looks; the comparison is what found the DrumSynth click, the OscDual
modulation depth and the FltMulti FreqM.

The tables are GENERATED - `tools/do-modulestatus` builds and runs `tools/modulestatus`, which asks
`sound_engine_models_module()` about every type the palette offers and takes each modelled module's
evidence from its `kEvidence` table. **Regenerate rather than edit**: change a module's row in
`kEvidence` when its evidence changes. The run fails if a modelled module has no row, or a row names a
module that is not offered or not modelled.

The **Fixed point** column says what arithmetic each model runs in, against 24-bit
words. The G2's DSP is fixed point throughout, so where a model is in doubles its rounding is finer than
the instrument's: a tail that reaches exact zero on the G2 decays on into denormals here, a loop that
truncates its way to silence keeps ringing, and a value compared against a threshold can land a step
either side. Every model saturates at full scale where its value leaves the module (sound-engine-notes
§196), so the column is about what happens inside.

| Fixed point | meaning |
|---|---|
| **Integer words** | the signal path in 24-bit integer words: its truncation, rounding and saturation |
| **Integer control** | counters, phases, steps or coefficients in its words; the audio path in doubles |
| **Saturating doubles** | doubles on the instrument's scaling, saturated where it saturates inside the module, but never rounded to 24 bits |
| **Doubles** | doubles throughout. Whether rounding to 24 bits would be audible is unchecked |
| **Not affected** | logic, switching, copies and MIDI: thresholds and whole numbers that rounding does not move |

Doubles is not a defect by itself: for a mixer or a gain the difference is below the G2's own
noise floor. It is where to look first when a module disagrees with a G2 at low levels, in a feedback
loop, or at a threshold. The classification comes from reading each model and is editorial, in
`kPrecision` beside `kEvidence`; the run fails if a module has no row.

| state | meaning | modules |
|---|---|---|
| **Confirmed on the G2** | compared with a G2 (a capture, its meters, or a stage patch played against it) and it agrees | 163 |
| **Modelled** | a full model in the reference, not yet compared with a G2 | 3 |
| **No sound** | nothing to render (a name, or a module whose output leaves by MIDI) | 4 |
| **Total** | every module type the palette offers | 170 |

| Group | Confirmed on the G2 | Modelled | No sound |
|---|---|---|---|
| **Oscillators** | Osc A, Osc B, Osc C, Osc D, Osc Phase Mod, Osc Shape A, Osc Shape B, Osc Dual, Noise Osc, Noise, Metallic Noise, Osc Percussion, Drum Synth, Osc String, FM Operator, DX Router, Driver, Resonator, Osc Master | - | - |
| **Filters** | LP Filter, HP Filter, Nord Filter, Classic Filter, Multi Filter, Phase Filter, Comb Filter, Static Filter, FltVoice, WahWah, Vocoder, Eq 2-band, Eq 3-band, Eq Peak | - | - |
| **Envelopes** | Envelope ADSR, Envelope AHD, Envelope ADR, Envelop ADDSR, Envelope H, Envelope D, Envelope Multi, Envelope Mod AHD, Envelope Mod ADSR | - | - |
| **LFOs** | LFO A, LFO B, LFO C, LFO Shp A, Clock Generator | - | - |
| **Mixers** | Mixer 1-1 A, Mixer 1-1 S, Mixer 2-1 A, Mixer 4-1 A, Mixer 4-1 B, Mixer 4-1 C, Mixer 4-1 S, Mixer 2-1 B, Mixer 8-1 A, Mixer 8-1 B, MixFader, MixStereo, Fade 1-2, Fade 2-1, X-Fade, Pan | - | - |
| **Level** | Constant, ConstSwM, ConstSwT, CompLev, CompSig, LevAdd, LevAmp, LevConv, LevMod, LevMult, MinMax, ModAmt, NoiseGate, EnvFollow, Red2Blue, Blue2Red | - | - |
| **Shapers** | Saturate, Clip, OverDrive, ShpExp, WaveWrap, ShpStatic, Rect | - | - |
| **Delays** | Delay Single A, Delay Single B, Delay Dual, Delay Quad, Delay A, Delay B, Delay Stereo, Delay Clock, Delay Eight, DlyShiftReg | - | - |
| **Effects** | Compressor, Digitizer, FreqShift, Flanger, Chorus, Phaser, PShift, Reverb, Scratch | - | - |
| **In/Out** | 2 Outputs, 4 Outputs, 2 Inputs, 4 Inputs, FX Input, Keyboard, Monophonic Keyboard, Status, Note Detector | Device | Name Bar |
| **Switches** | SwOnOffM, SwOnOffT, Sw2-1, Sw2-1M, Sw4-1, Sw8-1, Sw1-2, Sw1-2M, Sw1-4, Sw1-8, ValSw2-1, ValSw1-2, Mux8-1, Mux1-8, Mux8-1X, S&H, T&H, WindSw | - | - |
| **Logic** | Invert, Pulse, Delay, Gate, FlipFlop, ClkDiv, 8Counter, BinCounter, ADConv, DAConv | - | - |
| **Sequencers** | Sequencer Event, Sequencer Values, Sequencer Level, Sequencer Note, Sequencer Controlled | - | - |
| **Random** | Random A, Random B, Rnd Clock A, Rnd Clock B, Rnd Trig, Rnd Pattern | - | - |
| **Note** | Note Quantiser, Key Quantiser, Partial Quantiser, Note Scaler, Glide, Pitch Tracker, Zero Crossing Counter, Level Scaler | - | - |
| **MIDI** | CtrlSend, NoteSend | CtrlRcv, NoteRcv | PCSend, NoteZone, Automate |
| **Total 170** | **163** | **3** | **4** |

### Oscillators

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| Osc A | Confirmed on the G2 | §6.3 | captures (G2Captures/osca); OscC/OscD level against it 09-12 | Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only | Doubles: phase accumulator in doubles |
| Osc B | Confirmed on the G2 | §6.3, §6.6-§6.8 | 5 waves x 8 pitches, harmonics within 0.1 dB (09-17); DualSaw below zero 0.1 dB (10-08) | FM and Sync not compared on the G2; Shape 127's one-sample click not modelled; Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only | Doubles: phase accumulator in doubles |
| Osc C | Confirmed on the G2 | §6.3, §6.1a | Saw, Sqr50/25/10, Tri at 440-3520 Hz (09-17); Freq mode tracks the key: AdditiveG2 within 1.4 dB per octave (10-16) | FM not compared on the G2 | Doubles: phase accumulator in doubles |
| Osc D | Confirmed on the G2 | §6.3, §6.1a | level and harmonics against OscA (09-12) | Pitch Type: Freq tracking heard on Osc C (10-16), not on Osc D | Doubles: phase accumulator in doubles |
| Osc Phase Mod | Confirmed on the G2 | §53 | Sine and Tri at 330 Hz phase-modulated by a 262 Hz sine, PhM 32-127: every sideband within 0.37 dB, level on the rig offset (10-25); Bassic and Basslines FM4 (10-16) | Sync not compared; Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only | Doubles: phase and Phase M in doubles; the sine is the module's polynomial |
| Osc Shape A | Confirmed on the G2 | §27, §6.7 | all six waves at Shape 0/64/127 at 330 Hz: harmonics within 0.08 dB, level on the rig offset; Sine3 all harmonics and Sine4 odd, as OscShpB's (10-25) | Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only | Doubles: phase accumulator in doubles |
| Osc Shape B | Confirmed on the G2 | §27, §6.7 | every wave, Shape Mod -1..+1, harmonics within 0.16 dB (10-02); Sine3/4 within 0.001 (09-17) | Pulse at exactly +-1: click -33 dB against the G2's -41; Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only | Doubles: phase accumulator in doubles |
| Osc Dual | Confirmed on the G2 | §12.4, §12.5 | levels, duty, phase (09-12); PW and Phase mod (10-03) | Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only | Doubles: phase accumulator in doubles |
| Noise Osc | Confirmed on the G2 | §8.2-§8.5 | 09-12 capture agrees 110 Hz-1 kHz (the old model's fit) | level above 1 kHz not re-captured since the §8 model; Freq mode and Partial below 33 now track Kbt (§6.1a) - heard on Osc C only | Saturating doubles: 24-bit LFSR; the band-passes saturate at the word's +-1 |
| Noise | Confirmed on the G2 | §7.2a | 09-12 capture reproduced through the desk's shelf to a constant | - | Doubles: xorshift draw, not the instrument's generator |
| Metallic Noise | Confirmed on the G2 | §66 | Colour 0/64/127 x Freq 0/64/127: third-octave balance within 0.63 dB (mean 0.28 at most), level on the rig offset (10-25) | the FreqMod and ColourMod inputs not compared | Integer words: squares, Freq word and one-poles |
| Osc Percussion | Confirmed on the G2 | §40 | 8 takes: levels 0.1 dB, decay 2 ms, pitch exact (09-25) | - | Saturating doubles: DSP words carried in doubles, saturated |
| Drum Synth | Confirmed on the G2 | §39 | Master Freq 0.2% (09-21); noise, click, oscillator energies within 0.8 dB (09-25) | level curve set by §39.3, not fitted to the 09-20 sweeps | Saturating doubles: the integer words carried in doubles |
| Osc String | Confirmed on the G2 | §70.4 | noise through it, Decay 64-120, Damp 0-64: resonances within 3.7 cents and 0.6 dB at Decay 64; pitch at Tune 64/100/112 within 1.2 cents, level within 0.4 dB (10-25) | peak heights at Decay 100-120 narrower than the measurement resolves | Doubles |
| FM Operator | Confirmed on the G2 | §14.2-§14.6 | envelope, level, FM depth, feedback, level and rate scaling, velocity (10-03); an idle voice starts from L4: Backtalk DX repeated notes as the G2 within 0.7 dB (10-16) | AMod and Pitch inputs (§14.6) not yet compared (to-test); Gate/Note/Vel jacks read from the voice | Integer control: envelope, level and rate offsets in words; the operator in doubles |
| DX Router | Confirmed on the G2 | §14.1, §14.5 | Main level 8.8 dB against 8.45 (10-03); Backtalk DX held tone within 1-2 dB per octave (10-16) | Out1-Out6 not used outside the node | Integer control: envelope, level and rate offsets in words; the operators in doubles |
| Driver | Confirmed on the G2 | §70.5, §70.5a | Reed and Bow, Stiffness and Embouchure 32/96, In2 0 and +32 units, a sine on In1: 16 settings within 0.06 dB per harmonic and 0.03 dB in level (10-25) | -Lip- and -Mallet- share the Reed and were not played | Doubles |
| Resonator | Confirmed on the G2 | §70.4a | noise through all five Algs at Decay 100, Damp 32, Pos 64: resonances within 4 cents (Tube3 one at 14), peaks within 1.7 dB mean, level within 0.3 dB; Tune 100 to 0.7 cents (10-25) | Pos and the pitch inputs not swept | Doubles |
| Osc Master | Confirmed on the G2 | §51 | through an OscB's Pitch input: Coarse 40-76, Cent 0-127, Pitch M 0-127 with a Constant on PitchVar - within 0.74 cents (10-25) | - | Doubles |

### Filters

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| LP Filter | Confirmed on the G2 | §22 | noise through it, Freq 50/85/110, all six slopes: mean within 0.22 dB, worst 0.87 at 252 Hz (10-25); FM at 2 semitones a unit (notes §160) | Kbt and the Mod input not compared | Doubles |
| HP Filter | Confirmed on the G2 | §22 | noise through it, Freq 50/85, slopes 6/12/24/36: mean within 0.09 dB, worst 0.55 (10-25) | Kbt and the Mod input not compared | Doubles |
| Nord Filter | Confirmed on the G2 | §23 | noise through LP/BP/HP/BR, 12 and 24 dB, Freq 50 and 85, Res 0 and 100: within 1.44 dB, mean 0.06-0.35 (10-11) | Pitch, FM and Res mod not compared | Doubles |
| Classic Filter | Confirmed on the G2 | §21 | 04 Chris' Pad within 0.5 dB 100 Hz-12 kHz; a high-Res test patch's peak within 0.04 semitone (09-28) | Env and the dB/Oct tap not compared since the 09-14 law | Doubles |
| Multi Filter | Confirmed on the G2 | §10 | 54 noise responses within 0.5-0.6 dB (09-12); FreqM within 1% (10-08) | GComp off not captured | Doubles |
| Phase Filter | Confirmed on the G2 | §67 | noise through it, all three types, 2 and 6 notches, FB 20/64/100: mean within 0.11 dB (10-25) | the Pitch, Spread and FM inputs not compared | Integer words: the six allpass stages in the accumulator |
| Comb Filter | Confirmed on the G2 | §13.4 | noise through all three Types (09-12); Pitch attenuator (10-08) | - | Saturating doubles: every word saturated |
| Static Filter | Confirmed on the G2 | §10.4, §10.4a | noise through LP, BP and HP at Freq 50/85, Res 0/64/110, GC off and on: 21 settings, mean within 0.28 dB, worst 1.13 on the Res 110 peak (10-25) | - | Doubles |
| FltVoice | Confirmed on the G2 | §56, §56.1 | noise through it, vowels A/E/I, Freq 64/100, Res 0/64/127: formants within 18 cents, peaks within 0.8 dB (10-25); the Freq law from 63 to 66 to 2 cents | Res 127 peak 3 dB under; between the formants 1.3-2.8 dB | Integer control: vowel table and resonance word; the resonators in doubles |
| WahWah | Confirmed on the G2 | §69.9 | noise through it at Sweep 0/64/127: mean within 0.24 dB, worst 1.4 in the lowest band at Sweep 0 (10-25) | the Sweep input not compared | Integer words: sweep and filter words |
| Vocoder | Confirmed on the G2 | §70.8 | a 110 Hz saw carrier and a sine modulator at 300 Hz, 1 kHz and 3 kHz, Emphasis off and on: the harmonics that pass within 0.15 dB, level within 0.08 (10-25) | BandSel routing other than straight, and Monitor, not compared | Saturating doubles: the converters' and bands' words, saturated |
| Eq 2-band | Confirmed on the G2 | §11.2 | 43 noise settings across the EQs, 0.53 dB mean (09-12) | high shelf changed 10-09 (0.3-0.7 dB near 3 kHz), not re-captured | Doubles |
| Eq 3-band | Confirmed on the G2 | §11 | 0.66 dB mean, 1.42 worst (09-12) | the 1.42 dB setting (mid -13.5 dB at 8 kHz) not explained by the model | Doubles |
| Eq Peak | Confirmed on the G2 | §11.3, §11.5 | 0.57 dB mean (09-12) | - | Doubles |

### Envelopes

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| Envelope ADSR | Confirmed on the G2 | §17.1-§17.8 | time law and curves agree with the 08-24 and 09-07 captures (not kept); release at LogExp R14 and LinExp R20 within one 50 ms window (10-16) | - | Integer words: level and segments in words; the VCA multiply in doubles |
| Envelope AHD | Confirmed on the G2 | §17.11a | Hold 16-80, within 1% (10-08) | - | Integer words: level and segments in words; the VCA multiply in doubles |
| Envelope ADR | Confirmed on the G2 | §17 | an OscB through it on a held key, Decay and Release modes and the Exp shape: the curve within 0.1 dB on average through the hold and the release (10-25) | - | Integer words: level and segments in words; the VCA multiply in doubles |
| Envelop ADDSR | Confirmed on the G2 | §17 | an OscB through it on a held key: within 0.5 dB on average, the release within 0.4 (10-25) | - | Integer words: level and segments in words; the VCA multiply in doubles |
| Envelope H | Confirmed on the G2 | §17.9a | Hold 12-64 within 0.1 ms (10-08) | - | Integer words: level and segments in words; the VCA multiply in doubles |
| Envelope D | Confirmed on the G2 | §17 | Decay 60 and 90, triggered by the Keyboard Gate: within 0.1 dB on average (10-25) | - | Integer words: level and segments in words; the VCA multiply in doubles |
| Envelope Multi | Confirmed on the G2 | §17.11 | its defaults, a fall from full to nothing and a rise to full: within 0.2 dB on average; its In is input 1 - our face had In and AM swapped, fixed (10-25) | - | Integer words: its tick in words |
| Envelope Mod AHD | Confirmed on the G2 | §17.9a, §17.10 | Hold within 0.3% (10-08); every mod jack at +-32 units and at partial amounts on the G2, the Sustain jack after a fix (10-26) | - | Integer words: level and segments in words; the VCA multiply in doubles |
| Envelope Mod ADSR | Confirmed on the G2 | §17.10 | Decay mod at -32..+32 units (09-20); every mod jack at +-32 units and at partial amounts on the G2, the Sustain jack after a fix (10-26) | - | Integer words: level and segments in words; the VCA multiply in doubles |

### LFOs

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| LFO A | Confirmed on the G2 | §28 | Hi rate measured; Sine/Tri/Saw/Sqr at Hi 112 correlate 0.999-1.000, harmonics within 0.01 dB; Sub, Lo and BPM within 0.5%, the Rate input and RateVar through Rate M within 0.2% (10-26) | RndSt/Rnd, Kbt and Mode not compared | Doubles: phase in doubles |
| LFO B | Confirmed on the G2 | §28, §28.4a | Hi range, all four waves and Phase 0-96 against its Snc output: to 0.002 cycle; rate 0.001%, harmonics 0.5 dB, level 0.07 dB (10-25); Lo, Sub, BPM and Clk within 0.7%, the Rate input and RateVar through Rate M 0-127 within 0.2% (10-25) | the Phase M input not compared | Doubles: phase in doubles |
| LFO C | Confirmed on the G2 | §28, §28.7 | Hi 64-127 and Lo 100/127 rate within 0.09%; Sine/Tri/Saw/Sqr shape and harmonics within 0.7 dB; every Output Type, level within 0.2 dB (10-25); Lo and Sub through an OscB's pitch within 0.7%, BPM within 0.3% at 120 BPM, the Rate input +-12/+24 units within 0.2% (10-25) | Mono not compared | Doubles: phase in doubles |
| LFO Shp A | Confirmed on the G2 | §28.6 | Rate Sub, Lo, Hi within 0.013% (09-07); all six waves at Shape 1/32/64/96/127 against its Snc: correlation 0.96-1.00, harmonics within 0.6 dB, phase within 0.014 cycle; Phase 32-96, Shape M, Phase M and Dir (+-32, +16 units) likewise; Sub, Lo, BPM, Clk within 0.5%, Rate and RateVar within 0.2% (10-26) | Rst not compared | Doubles: phase in doubles |
| Clock Generator | Confirmed on the G2 | §59 | Master clock within 7-17 ppm; engine within 1-4 ms over 66 s (10-09) | - | Integer words: phase and tempo words |

### Mixers

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| Mixer 1-1 A | Confirmed on the G2 | §3 | 106 configurations of the eleven mixers within 0.07 dB (09-12) | - | Doubles |
| Mixer 1-1 S | Confirmed on the G2 | §3 | as Mixer 1-1 A; On off mutes to -93 dBFS at Lev 40/78/127, Lin and Exp (10-16) | - | Doubles |
| Mixer 2-1 A | Confirmed on the G2 | §3 | as Mixer 1-1 A | - | Doubles |
| Mixer 4-1 A | Confirmed on the G2 | §3 | as Mixer 1-1 A | - | Doubles |
| Mixer 4-1 B | Confirmed on the G2 | §3 | as Mixer 1-1 A | - | Doubles |
| Mixer 4-1 C | Confirmed on the G2 | §3 | 218 steps 0.01 dB; Pad 0/-6/-12 (09-07, 09-12) | - | Doubles |
| Mixer 4-1 S | Confirmed on the G2 | §3 | as Mixer 1-1 A | - | Doubles |
| Mixer 2-1 B | Confirmed on the G2 | §3 | as Mixer 1-1 A; Inv cancels | - | Doubles |
| Mixer 8-1 A | Confirmed on the G2 | §3 | as Mixer 1-1 A | - | Doubles |
| Mixer 8-1 B | Confirmed on the G2 | §3 | as Mixer 1-1 A | - | Doubles |
| MixFader | Confirmed on the G2 | §3 | as Mixer 1-1 A | - | Doubles |
| MixStereo | Confirmed on the G2 | §5 | 19 settings each, Log and Lin (09-12) | - | Doubles |
| Fade 1-2 | Confirmed on the G2 | §4.2 | 19 settings, every point within 0.001 (09-12) | mod-input depth not captured, still 4 x the input | Doubles |
| Fade 2-1 | Confirmed on the G2 | §4.2 | as Fade 1-2 | mod-input depth not captured, still 4 x the input | Doubles |
| X-Fade | Confirmed on the G2 | §4.2, §4.3 | as Fade 1-2 | mod-input depth (Pan's law since 10-10) not captured | Doubles |
| Pan | Confirmed on the G2 | §4.2, §4.3 | as Fade 1-2; mod depth, an LFO at 31: +-6.3 dB (10-10) | - | Doubles |

### Level

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| Constant | Confirmed on the G2 | §16.1 | the known input of most G2 checks since 09-13 (OscShpB, OscDual, DrumSynth, FltMulti) | - | Doubles |
| ConstSwM | Confirmed on the G2 | §68.1 | as ConstSwT (10-25) | - | Doubles |
| ConstSwT | Confirmed on the G2 | §44 | Value 20/96/127, Bip and Uni, on and off: on the G2's value within its 0.3% (10-25) | - | Doubles |
| CompLev | Confirmed on the G2 | §48 | six A/level pairs either side of the threshold: high at A >= level, as on the G2 (10-25) | - | Integer words: compared as words |
| CompSig | Confirmed on the G2 | §69.2 | five A/B pairs: high at A >= B, as on the G2 (10-25) | - | Integer words: compared as words |
| LevAdd | Confirmed on the G2 | §32 | five In and Constant pairs, Bip and Uni, past 64 units unsaturated on both (10-25) | - | Doubles |
| LevAmp | Confirmed on the G2 | paramCurves notes §29 | 33 dial positions (08-30); 127 as 0x7FFFFF settled BCHydro_DZLW (10-09) | - | Doubles |
| LevConv | Confirmed on the G2 | §31 | all 18 In/Out type pairs at -32 and +32 units; over-range inputs run on to +-128 units - the engine clamped at 64, fixed (10-25) | - | Doubles |
| LevMod | Confirmed on the G2 | §69.3 | six In/Mod/Depth/Type settings with the ModDepth input: within 0.1 unit of the G2 (10-25) | - | Integer words |
| LevMult | Confirmed on the G2 | §74 | an OscB sine times a Constant at 8 values, -64 to +64 units: gain within 0.04 dB and the sign right (10-25) | - | Doubles |
| MinMax | Confirmed on the G2 | §43 | both outputs at two input pairs (10-25) | - | Doubles |
| ModAmt | Confirmed on the G2 | §29 | an OscB times a Constant through it, Exp and Lin, m and 1-m, Depth 32/64/100/127, Mod -64 to +64, Enable off: 66 settings within 0.04 dB, or at the rig's noise floor (10-25) | - | Doubles |
| NoiseGate | Confirmed on the G2 | §70.6 | a sine at five levels and thresholds: opens above Threshold (v/128), shuts below three quarters of it, Env at full open, as on the G2 (10-25) | attack and release times not timed on the G2 | Integer words: follower and gate in words |
| EnvFollow | Confirmed on the G2 | §69.4 | a sine at three levels and three Attack/Release settings: the settled output within 0.3% of the G2 (10-25) | attack and release times not timed on the G2 | Integer words: peak word and follower |
| Red2Blue | Confirmed on the G2 | §68.8, notes §203 | passes a Constant unchanged (10-25) | - | Not affected: a copy, held at 24 kHz |
| Blue2Red | Confirmed on the G2 | §68.8 | passes a Constant unchanged (10-25) | - | Not affected: a copy |

### Shapers

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| Saturate | Confirmed on the G2 | paramCurves notes §33 | a 330 Hz sine through it at two drives (the input on the other channel), Amount 64/127, curves 1 and 4: every harmonic within 0.38 dB, level within 0.03 (10-25) | the Mod input not compared | Doubles |
| Clip | Confirmed on the G2 | paramCurves notes §36 | a 330 Hz sine through it at two drives (the input on the other channel), Clip level 32/96, Asym and Sym: every harmonic within 0.38 dB, level within 0.03 (10-25) | the Mod input not compared | Doubles |
| OverDrive | Confirmed on the G2 | §71 | energy above 6 kHz in 14 CS80project72: -41.5 against -41.6 dB (10-04) | - | Saturating doubles: drive, feedback and blend saturated as words |
| ShpExp | Confirmed on the G2 | paramCurves notes §32 | a 330 Hz sine through it at two drives (the input on the other channel), Amount 127, all four curves: every harmonic within 0.38 dB, level within 0.03 (10-25) | the Mod input not compared | Doubles |
| WaveWrap | Confirmed on the G2 | §73, paramCurves notes §34 | a 330 Hz sine through it at two drives (the input on the other channel), Amount 32/96: every harmonic within 0.38 dB, level within 0.03 (10-25) | the Mod input not compared | Doubles |
| ShpStatic | Confirmed on the G2 | paramCurves notes §31 | a 330 Hz sine through it at two drives (the input on the other channel), all four modes: every harmonic within 0.38 dB, level within 0.03 (10-25) | the Mod input not compared | Doubles |
| Rect | Confirmed on the G2 | paramCurves notes §30 | a 330 Hz sine through it at two drives (the input on the other channel), all four modes: every harmonic within 0.38 dB, level within 0.03 (10-25) | the Mod input not compared | Doubles |

### Delays

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| Delay Single A | Confirmed on the G2 | §52 | noise through it at Ranges 5 ms to 1 s, Time 0-127: the tap on the G2's own sample, unity gain (10-26) | - | Doubles: a 4-point read |
| Delay Single B | Confirmed on the G2 | §52.1 | the same at three Ranges, and Time M 64-127 with +-16/+32 units on the Mod jack: on the G2's sample (10-26) | - | Doubles: a 4-point read |
| Delay Dual | Confirmed on the G2 | §70.1, §52.1 | noise through it, Ranges 5/25/500 ms, Time 0-127: each tap on the G2's sample, unity gain (10-25) | Time Mod inputs not compared; Range 2.7 s silences the patch on the G2 (the budget) | Doubles: a 4-point read |
| Delay Quad | Confirmed on the G2 | §70.1, §52.1 | noise through it at 25 and 100 ms: the four taps and Main on the G2's sample, unity gain (10-25) | Time Mod inputs and Clk mode not compared | Doubles: a 4-point read |
| Delay A | Confirmed on the G2 | §24 | FB, LP, dry/wet, Time and Clk (09; audio not kept) | - | Integer words: the tap, LP, HP and feedback on half-scale words |
| Delay B | Confirmed on the G2 | §24 | as Delay A; HP at four settings | - | Integer words: the tap, LP, HP and feedback on half-scale words |
| Delay Stereo | Confirmed on the G2 | §65 | both taps on the G2's sample; FB 40/96/127 and X-FB 100, each dial alone, within 3% per pass and flat to 16 kHz; Dry/Wet 0-127 within 0.01 (10-26) | LP and HP dials not swept; FB + X-FB at a loop gain of 1 grows to saturation on the G2 only | Integer words: Delay B's tap on both lines |
| Delay Clock | Confirmed on the G2 | §69.7 | Time 0, 2 and 3 over five clocks: the value written N clocks before, as on the G2 (10-26) | - | Not affected: a copy on each clock |
| Delay Eight | Confirmed on the G2 | §70.1, §52.1 | noise through it at 25 and 100 ms, Time 64-127: taps 1/3/4/8 on the G2's sample (10-25) | - | Doubles: a 4-point read |
| DlyShiftReg | Confirmed on the G2 | §69.6 | nine values clocked in, Outs 1, 2, 3, 5 and 8: In k-1 clocks ago, as on the G2 (10-26) | - | Not affected: copies |

### Effects

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| Compressor | Confirmed on the G2 | §25 | a steady sine at 8 levels, -47 to -19 dB, 04 Chris' Pad's settings: gain equal to the G2's within 0.01 dB (10-11) | attack/release timing and SideChain not compared on the G2 | Integer words: detector, gain table and make-up |
| Digitizer | Confirmed on the G2 | §69.8 | a 330 Hz sine at Rate 32/96, Bits 3/11: every line within 0.8 dB (10-25) | the Rate input not compared | Integer words: rate word and phase |
| FreqShift | Confirmed on the G2 | §57 | a 330 Hz sine, Shift 32/80/127 in each Range: Up and Down within 0.1 Hz and 0.3 dB, the other sideband within 0.2 dB at -64; In is input 0 - ours had In and Mod swapped, fixed (10-25) | the Mod input not compared | Saturating doubles: allpass chains saturated at the word's +-1 |
| Flanger | Confirmed on the G2 | §70.2 | noise through it, Range 32/96, FB 0/64/127, averaged over its sweep: within 0.4 dB mean; the sweep from a sine's FM at Rate 64/127 within 0.06%, its depth within 0.2% (10-25) | its random start phase not compared | Integer control: sweep phase word; the ring read in doubles |
| Chorus | Confirmed on the G2 | §19 | tap spans, rate, mix (09-07); unity at Amount 0 (09-14); no tail once its input stops, as the G2 (10-16) | - | Integer control: LFO phase word; the taps in doubles |
| Phaser | Confirmed on the G2 | §55 | noise through it, both types, FB 0/64/127, averaged over its sweep: within 0.56 dB mean; sweep rate within 0.1% (10-25) | - | Saturating doubles: the module's fractions, saturated |
| PShift | Confirmed on the G2 | §70.3 | a 330 Hz sine at Semi 52/76/100: the shifted line exact in frequency and within 0.2 dB, its neighbours too; In is input 0 - ours had In and PitchVar swapped, fixed (10-25) | the PitchVar input and Fine not compared | Integer control: phase word; the taps in doubles |
| Reverb | Confirmed on the G2 | §20 | 19 captures: onsets, stereo, Time law (09); Dry/Wet 96/64/48/24 within 1 dB of the cubed law (10-16) | captured decays 6-12% long (their fits) | Integer words: every stored value rounded down to the 24-bit grid and clamped |
| Scratch | Confirmed on the G2 | §70.3 | a sine at Ratio 10-127 and both Delays: the pitch within 0.02% of the G2, level within 0.1 dB; Ratio M at 64 and 127 with +-1 to +-16 units, after a fix (10-26) | the level at a speed of exactly 1 depends on where the taps stopped | Integer control: ratio and phase words; the taps in doubles |

### In/Out

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| 2 Outputs | Confirmed on the G2 | §63, notes §167, §198 | Pad +6 dB (09-07); an unpatched socket is silent (10-10); the path every G2 check goes through; Out to Bus 1/2 silent on its own, as the G2 (10-16) | - | Doubles: saturated at twice full scale |
| 4 Outputs | Confirmed on the G2 | - | Pad +6 dB (09-07) | - | Doubles: saturated at twice full scale |
| 2 Inputs | Confirmed on the G2 | §37 | full scale and Pad off the G2's own meter (10-08) | the application has no input device | Doubles |
| 4 Inputs | Confirmed on the G2 | §37, §69.12 | a tone into each G2 input at both Pads: its own output within 0.03 dB of 2-In, the others silent (10-26) | From Bus not compared | Doubles |
| FX Input | Confirmed on the G2 | capture-inventory | Pad +6/0/-6/-12 dB (09-07) | - | Doubles |
| Keyboard | Confirmed on the G2 | §26 | velocity through the Operators within 0.03 dB (10-03) | - | Not affected |
| Monophonic Keyboard | Confirmed on the G2 | §35 | held-note return on 01 Mini Emulator (CT, 09-19) | Poly use is a guess | Not affected |
| Device | Modelled | §70.13, §70.13a | - | the filtering and scaling not compared on the G2: move the wheel and the pedal on both | Not affected |
| Status | Confirmed on the G2 | §70.13 | Patch Active is low, off the G2 (10-04) | - | Not affected |
| Note Detector | Confirmed on the G2 | §69.11 | Gate, Vel and RVel of the selected key on the G2's own keyboard, a matching key and not: Vel and RVel 0.992 for 127, held after release (10-26) | - | Not affected |
| Name Bar | No sound | - | - | - | Not affected |

### Switches

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| SwOnOffM | Confirmed on the G2 | §68.1 | passes held and blocks released on the G2 (10-25) | - | Not affected |
| SwOnOffT | Confirmed on the G2 | §30 | passes On and blocks Off on the G2 (10-25) | - | Not affected |
| Sw2-1 | Confirmed on the G2 | §33 | every Sel position routes the right input, eight OscBs told apart by pitch (10-25) | the Ctrl output not compared | Not affected |
| Sw2-1M | Confirmed on the G2 | §68.1 | both positions (10-25) | the Ctrl output not compared | Not affected |
| Sw4-1 | Confirmed on the G2 | §68.1 | every Sel position (10-25) | the Ctrl output not compared | Not affected |
| Sw8-1 | Confirmed on the G2 | §33 | every Sel position (10-25) | the Ctrl output not compared | Not affected |
| Sw1-2 | Confirmed on the G2 | §68.1 | both positions; Sel 0 feeds output 1 on the G2 - the engine fed output 0, fixed, and the output rows swapped to match (10-25) | the Ctrl output not compared | Not affected |
| Sw1-2M | Confirmed on the G2 | §68.1 | both positions; as Sw1-2, Sel off feeds output 1 - fixed (10-25) | the Ctrl output not compared | Not affected |
| Sw1-4 | Confirmed on the G2 | §68.1 | every Sel position, the next output silent (10-25) | the Ctrl output not compared | Not affected |
| Sw1-8 | Confirmed on the G2 | §45 | every Sel position, the next output silent (10-25) | the Ctrl output not compared | Not affected |
| ValSw2-1 | Confirmed on the G2 | §34 | a Ctrl sweep at value 32: the On input only at 32 units, as on the G2 (10-25) | - | Not affected |
| ValSw1-2 | Confirmed on the G2 | §68.2 | a Ctrl sweep from 0 to 63 units at value 32: output 0 at 32 units alone, output 1 otherwise; the engine had the outputs the other way round, fixed (10-25) | - | Not affected |
| Mux8-1 | Confirmed on the G2 | §68.3 | Ctrl from 0 to 127 units: four units a step, clamped at both ends (10-25) | - | Integer control: Ctrl's word >> 17 picks the input |
| Mux1-8 | Confirmed on the G2 | §68.3 | Ctrl steps 0-7 each on its own output (10-25) | - | Integer control: Ctrl's word >> 17 picks the output |
| Mux8-1X | Confirmed on the G2 | §70.11 | the crossfade between neighbours at six Ctrl levels within 0.005 of the G2's (10-25) | - | Doubles |
| S&H | Confirmed on the G2 | §38.5 | In taken on Ctrl's rise only, held with Ctrl high and low, at three values: as on the G2 (10-26) | - | Not affected: a copy |
| T&H | Confirmed on the G2 | §68.4 | tracks In while Ctrl is high, holds while low, at three values: as on the G2 (10-26) | - | Not affected: a copy |
| WindSw | Confirmed on the G2 | §68.5 | Ctrl from 0 to 46 units, window 20-30: passes inclusive at both ends, as on the G2 (10-25) | - | Not affected |

### Logic

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| Invert | Confirmed on the G2 | §38.1 | both inverters at all four input pairs: on the G2's levels to 0.1 dB (10-25) | - | Not affected |
| Pulse | Confirmed on the G2 | §18 | 17 widths within two samples (09) | Plus/Minus edge (10-09) in to-test | Not affected |
| Delay | Confirmed on the G2 | §46 | Pos, Neg and Cycle against a logic reference on the G2, Sub/Lo/Hi at four Times from 95 ms to 985 ms: each delayed edge within 1.2 ms of the law (0.7%), the other immediate (10-25) | the Time Mod input not compared | Not affected |
| Gate | Confirmed on the G2 | §38.2 | all six types on both gates, set differently, at all four input pairs (10-25); the Types are modes, which the engine read as missing parameters - every Gate was an AND until this date | - | Not affected |
| FlipFlop | Confirmed on the G2 | §38.3 | D and RS walked step by step on the G2 by its LEDs: the input order (Clk, In, Rst), D on the clock's rise, RS toggle, S and Rst (10-25) | - | Not affected |
| ClkDiv | Confirmed on the G2 | §38.4 | 14 pattern seq steps once a bar, as on the G2 (10-07) | - | Not affected |
| 8Counter | Confirmed on the G2 | §68.6 | reset and ten clock edges walked by its LEDs on the G2: the one-hot output steps and wraps after the eighth; Rst holds it at the first through an edge (10-25) | - | Not affected |
| BinCounter | Confirmed on the G2 | §68.6 | reset and ten clock edges walked by its LEDs on the G2: the outputs are the count's bits; Rst clears and holds (10-25) | - | Not affected |
| ADConv | Confirmed on the G2 | §68.7 | eleven Constants from -64 to +64 units walked by its LEDs on the G2: every bit as the law has it, the top saturating at 127 (10-25) | - | Integer words: the input's word, bit by bit |
| DAConv | Confirmed on the G2 | §68.7 | ten codes from -128 to +127 through a LevMult: code/128 on both, within the rig's 0.3% (10-25) | - | Integer words: the bits as a word |

### Sequencers

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| Sequencer Event | Confirmed on the G2 | §58 | step timing at 192 kHz (09-26); 1:2 BCHydro_DZLW within 0.2 dB (10-09) | - | Integer words: the module's own arithmetic |
| Sequencer Values | Confirmed on the G2 | §58 | as Sequencer Event | - | Integer words: the module's own arithmetic |
| Sequencer Level | Confirmed on the G2 | §58, §69.1 | Length/Cycle restart in BCHydro_DZLW (10-09) | - | Integer words: the module's own arithmetic |
| Sequencer Note | Confirmed on the G2 | §58, §58.1 | 18 Unreal Dreams' pitches (09-28) | the record stage never compared | Integer words: the module's own arithmetic |
| Sequencer Controlled | Confirmed on the G2 | §70.10 | Ctrl through steps 1-16 and out of range, all three XFades mid-step and bipolar Pol: within 0.2 unit of the G2 (10-26) | Trig output and the Val/Trig inputs not compared | Integer words: Ctrl's word picks the step |

### Random

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| Random A | Confirmed on the G2 | §47 | Mono at Step 100% and 25%: 31 values found in the engine's sequence from load, each within 0.2 unit of the G2 (10-26) | Edge glides and the Pitch input not compared; Pitch not read | Integer control: step words; the one-pole in doubles |
| Random B | Confirmed on the G2 | §69.10 | Mono at Step dial 127 and 40: 30-32 values found in the engine's sequence from load, within 0.2 unit (10-26) | Edge glides, Rate inputs and Kbt not compared | Integer control: step words; the one-pole in doubles |
| Rnd Clock A | Confirmed on the G2 | §64 | from a reset at two Seeds and two Steps: the same 16 values in order as the G2, within 0.2 unit (10-26) | - | Integer control: seed and step words |
| Rnd Clock B | Confirmed on the G2 | §70.9 | Rnd1 and Rnd2 from a reset at two Seeds and Steps, and the Step M jack: the same 15-16 values as the G2; Rnd2 from Seed 0 only after a fix (10-26) | - | Integer control: seed, step and Step M words |
| Rnd Trig | Confirmed on the G2 | §64 | from a reset at three Seeds and Densities: the same pulses passed, 18-38 gaps identical (10-26) | the Prob input not compared | Integer control: the draw against the threshold |
| Rnd Pattern | Confirmed on the G2 | §70.9 | from a reset at four PatA/PatB/Step/Loop settings: the same 9-16 values as the G2, loops included (10-26) | State wave and the A/B/Step inputs not compared | Integer words: generator and counter |

### Note

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| Note Quantiser | Confirmed on the G2 | §49 | ten In/Range/Notes settings, Off to 12 semitones, within 0.1 unit of the G2 (10-26) | - | Integer words: the module's own arithmetic |
| Key Quantiser | Confirmed on the G2 | §41 | a seven-key scale, Closest and Evenly, ten inputs across two octaves: the same keys as the G2 (10-26) | - | Integer words: the reference model's integer arithmetic |
| Partial Quantiser | Confirmed on the G2 | §69.5 | partials 1-31 and negative input at three Ranges within 0.2 unit (10-26) | - | Integer words |
| Note Scaler | Confirmed on the G2 | §60 | four In/Range pairs within 0.1 unit (10-26) | - | Saturating doubles: saturated as a word |
| Glide | Confirmed on the G2 | §36, §36.1 | Log at Time 32/64/96 within 0.3%, Lin at 32/64/80/96 within 0.02%, a square through it into an OscB's pitch (10-25) | the Glide On input and the button off not compared | Doubles |
| Pitch Tracker | Confirmed on the G2 | §70.7 | a sine at 41 Hz to 2.6 kHz: Pitch within 0.2 unit of the G2 and Gate high, after moving 0 units to E4 (10-26) | Threshold and Period not compared | Doubles |
| Zero Crossing Counter | Confirmed on the G2 | §70.7a | a sine at 41 Hz to 2.6 kHz: within 0.1 unit of the G2, after moving 0 units to E4 (10-26) | - | Doubles |
| Level Scaler | Confirmed on the G2 | §70.12 | Level and Out at nine Note/L/BP/R settings, both slopes to +-8 dB/oct, within 0.3% (10-26) | - | Saturating doubles: gain and output saturated |

### MIDI

| Module | State | Reference | On the G2 | Open | Fixed point |
|---|---|---|---|---|---|
| CtrlSend | Confirmed on the G2 | §62.3 | 1:2 BCHydro_DZLW's variation switching, within 1-4 ms (10-09) | This and channels 1-16 leave by MIDI, which the engine has no output for (§62.2) | Saturating doubles: the signal saturated as a word, then the controller value |
| PCSend | No sound | - | - | nothing leaves by MIDI | Not affected |
| NoteSend | Confirmed on the G2 | §62, §62.2 | 18 Unreal Dreams' pitches and band balance (09-28); Slot A plays, This and channels 3/5 do not (10-25) | This and channels 1-16 leave by MIDI, which the engine has no output for | Not affected |
| CtrlRcv | Modelled | §70.13 | - | in the plug-in only notes arrive | Not affected |
| NoteRcv | Modelled | §70.13 | - | - | Not affected |
| NoteZone | No sound | - | - | nothing leaves by MIDI | Not affected |
| Automate | No sound | - | - | nothing leaves by MIDI | Not affected |

### Fixed point

| Arithmetic | Modules |
|---|---|
| Integer words | 35 |
| Integer control | 14 |
| Saturating doubles | 11 |
| Doubles | 71 |
| Not affected | 39 |

## The G2 check queue

What would move the most modules to Confirmed on the G2, cheapest first. `to-test.md` carries the
individual checks; `capture-inventory.md` the rigs and what is already on disk.

| Check | Modules | Why first |
|---|---|---|
| One slow full-scale ramp through each mode | Saturate, Clip, ShpExp, WaveWrap, ShpStatic, Rect | memoryless, so one ramp is the whole transfer function; all six are Approximate |
| Noise through each filter, as FltMulti was (§10.3) | LP, HP, Nord, Classic, Static Filter | the filters most stage patches use; no response ever captured |
| A gate train through each envelope | Envelope ADR, ADDSR, D, Multi; Mod ADSR's Attack and Sustain mod | their stage structure has never been heard against a G2 |
| Rate at a few dials, and BPM/Clk against the master clock | LFO B, and BPM/Clk on LFO A, LFO C and LFO Shp A | LFO C's Hi and Lo done 10-25 (reference §28.7); its Sub and BPM are left |
| A Constant into each jack | FM Operator's AMod and Pitch (§14.6) | added 2026-10-09; nothing on file exercises them |
| Model fully, then capture | Delay Dual/Quad/Eight, FlipFlop, Device, Eq 3-band's mid width | Approximate or Partial |


## Notes

A patch of nothing but **Not implemented** modules reports "Nothing is patched into it". One that
mixes the two plays its modelled part, which is why an unfinished patch sounds thin rather than
broken.

`2 Inputs` and `4 Inputs` play whatever the caller feeds the engine (§37): G2Alike's side-chain as
In 1/2. The application has no input device yet, so there they are silent.

The palette offers 170 of the 210 types in `types.h`; the rest the G2 does not offer from its own
menus. `Operator` and `Name` sit outside `module_kind()` - an Operator plays only as part of the
DXRouter that owns it (§14), and a Name module is text.
