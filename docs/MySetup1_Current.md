# My Disting NT Setup

**Updated:** 2026-08-13

This is the concise full-system reference for the current live patch. It tracks roles, routing, wiring, buses, control ideas, and safety notes — not temporary jam levels.

---

## What the patch does

The Disting NT is the central hub for voices, routing, sends, and final dynamics.

```text
Internal voices + external voices + Percall/Radiant returns
        -> RADIANT MIXER
        -> SIDECHAIN COMP
        -> Output 1/2 -> Messor -> Output Bus

Kick Sample Player -> Aux21
Aux21 -> compressor sidechain key
Aux21 -> KICK ADD-BACK -> Output 1/2 after compressor
```

Kick is not in the main mixer, so it can duck the mix without ducking itself.

---

## Slot order

```text
0   KICK SAMPLE PLAYER       Aux21, MIDI ch1 note48, one-shot
1   SNARE SAMPLE PLAYER      Aux22, one-shot
2   HAT SAMPLE PLAYER        Aux23, one-shot
3   SLICED SAMPLE PLAYER     Aux24
4   POLY RESONATOR           Aux25/Aux26 stereo, Input9 gate, Input10 CV
5   NT-303                   Aux27
6   POLY LOFI                Aux28
7   USB audio from host      Optional; not required for standalone hardware patch
8   RADIANT MIXER            Main 12-channel mixer + Percall/Radiant sends
9   SIDECHAIN COMP           Output1/2 insert keyed from Aux21
10  KICK ADD-BACK            Aux21 to Output1/2 after compressor
```

---

## Drum sample-variant / velocity / round-robin format

Verified from `M:\DistingNT\DECENT_SAMPLER_TO_DISTING_NT_CONVERTER_HANDOFF.md`, which was based on the Disting NT manual MicroSD sample naming sections:

- Disting reads filename tags beginning with `_`.
- Root pitch is encoded as a note name, e.g. `_C3`.
- Disting octave convention: `C3 = MIDI 48`, `C4 = MIDI 60`, `A4 = MIDI 69`.
- Velocity layer order is encoded as `_V<number>`.
- Lowest-numbered `_V` is the lowest velocity layer.
- Round-robin variants are encoded as `_RR<number>`.
- If velocity and round robin are combined, velocity comes before round robin:

```text
Instrument_C3_V1_RR1.wav
Instrument_C3_V1_RR2.wav
Instrument_C3_V2_RR1.wav
Instrument_C3_V2_RR2.wav
```

For current one-shot drum players, use MIDI note 48 / `C3` unless deliberately changed.

Example kick naming for 8 playable variants with 2 RR each:

```text
KICK_808_C3_V1_RR1.wav
KICK_808_C3_V1_RR2.wav
KICK_808_C3_V2_RR1.wav
KICK_808_C3_V2_RR2.wav
KICK_808_C3_V3_RR1.wav
KICK_808_C3_V3_RR2.wav
KICK_808_C3_V4_RR1.wav
KICK_808_C3_V4_RR2.wav
```

Notes:

- Do **not** use invented lowercase tags like `v127` or `rr1` as functional Disting tags.
- Use `_V1`, `_V2`, etc. for ordered velocity layers, not exact MIDI velocity values.
- Use `_RR1`, `_RR2`, etc. for round robins.
- Keep `Loop = Off` for one-shot drum behaviour.

Performance idea:

```text
OXI mod lane CC -> Sample Player “Sample” parameter
0-127 / 8 buttons = 8 selectable drum variants
```

For the kick:

```text
OXI mod lane -> Slot 0 KICK SAMPLE PLAYER -> 1:Sample
```

---

## Disting NT inputs

```text
Input 1   Radio Station after Percall ch1
Input 2   Chord Organ after Steves MS-22
Input 3   Pico VCO after Percall ch3
Input 4   Pony VCO after Pico MMF
Input 5   Percall ch4 return
Input 6   Free
Input 7   Radiant return L
Input 8   Radiant return R
Input 9   Poly Resonator gate/trigger input
Input 10  Turing Machine CV/pitch
Input 11  Free
Input 12  Free
```

Turing pulse/gate had a hardware issue earlier; temporary chip-swap fix restored the gate for now.

---

## Disting NT outputs

```text
Output 1/2  Final stereo to Messor -> Befaco Output Bus
Output 3    Free
Output 4    Percall ch4 input
Output 5/6  Free
Output 7/8  Radiant input L/R
```

---

## Aux map

```text
Aux21  Kick
Aux22  Snare
Aux23  Hat
Aux24  Sliced Sample
Aux25  Poly Resonator Odd / Left
Aux26  Poly Resonator Even / Right
Aux27  NT-303
Aux28  Poly Lofi
```

---

## Main mixer channels

```text
Ch1   Snare              Aux22
Ch2   Hat                Aux23
Ch3   Sliced Sample      Aux24
Ch4   Poly Resonator     Aux25/Aux26 stereo
Ch5   NT-303             Aux27
Ch6   Poly Lofi          Aux28
Ch7   Radio Station      Input 1
Ch8   Chord Organ        Input 2
Ch9   Pico VCO           Input 3
Ch10  Pony VCO           Input 4
Ch11  Percall Return     Input 5
Ch12  Radiant Return     Input 7/Input 8 stereo
```

---

## Sends / external loops

All sends are pre-fade.

```text
Send1 -> Output 4,   Mono,   Add -> Percall ch4 -> Input 5
Send2 -> Output 7/8, Stereo, Add -> Radiant     -> Input 7/8
```

Pre-fade sends allow send-only playing: pull a channel dry gain down while still feeding Percall/Radiant.

---

## Safety notes

- Do **not** send Percall Return Ch11 back to Send1 by default.
- Do **not** send Radiant Return Ch12 back to Send2 by default.
- Sampler loops should be Off for one-shot drum/sample behaviour.
- Use Add mode for mixer/routing outputs.
- Compressor is the intentional insert-style exception on Output 1/2.
- Use channel Gain, not Mute, for performance dry kills.

---

## Removed / old-plan items

These are not part of the current working patch:

```text
Perc sampler voice
Kirbinator routing
Return Mixer
Aux31/32 dry bus
Aux29 and Aux33/34 Kirbinator buses
```
