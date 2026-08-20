# Whole Patch Understanding — Current Final State

**Updated:** 2026-08-12, corrected sends pre-fade

---

## Summary

This is a Disting NT hub patch built around one **RADIANT MIXER**, a sidechain compressor, and a post-compressor kick add-back.

```text
Non-kick sources + Percall/Radiant returns -> RADIANT MIXER -> SIDECHAIN COMP -> Output1/2
Kick Aux21 -> compressor key
Kick Aux21 -> KICK ADD-BACK after compressor -> Output1/2 Add
```

The old bigger architecture with Perc sampler, Kirbinator, and Return Mixer was removed due limits.

---

## Why the kick is special

Kick is not in the mixer.

Reason:

```text
If kick is in mixer, compressor ducks the kick too.
If kick is added after compressor, kick stays clean but still keys ducking.
```

Kick path:

```text
Kick Sample Player -> Aux21
Aux21 -> SIDECHAIN COMP sidechain
Aux21 -> KICK ADD-BACK -> Output1/2 Add
```

Kick is MIDI-triggered:

```text
MIDI channel 1
MIDI note 48
Trigger input None
Loop Off
```

Loop Off fixed the kick-path hum.

---

## Slot layout

```text
0   KICK SAMPLE PLAYER       Aux21
1   SNARE SAMPLE PLAYER      Aux22
2   HAT SAMPLE PLAYER        Aux23
3   SLICED SAMPLE PLAYER     Aux24
4   POLY RESONATOR           Aux25/Aux26 stereo
5   NT-303                   Aux27
6   POLY LOFI                Aux28
7   USB audio from host      present but not core patch focus
8   RADIANT MIXER
9   SIDECHAIN COMP
10  KICK ADD-BACK
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

Unused old buses:

```text
Aux29, Aux31, Aux32, Aux33, Aux34
```

---

## Inputs/outputs

Inputs:

```text
1 Radio
2 Chord
3 Pico
4 Pony
5 Percall return
7 Radiant return L
8 Radiant return R
9 PolyRes gate/trigger input
10 Turing CV, verified working
```

Outputs:

```text
1/2 final stereo to Messor
4 Percall send
7/8 Radiant send
```

Turing Machine pulse/gate had a hardware issue earlier; user made a temporary chip-swap fix and gate works again for now.

---

## RADIANT MIXER

Channels:

```text
1  Snare              Aux22
2  Hat                Aux23
3  Sliced             Aux24
4  PolyRes            Aux25/Aux26 stereo
5  NT-303             Aux27
6  PolyLofi           Aux28
7  Radio              Input 1
8  Chord              Input 2
9  Pico               Input 3
10 Pony               Input 4
11 Percall Return     Input 5
12 Radiant Return     Input 7/8 stereo
```

Outputs/sends:

```text
Main  -> Output1/2 Add
Send1 -> Output4 Pre-fade Mono Add
Send2 -> Output7/8 Pre-fade Stereo Add
```

Both sends must stay **Pre-fade** so channel Gain can control dry level without killing sends.

Current gain posture:

```text
Ch1-10 gains = 0 dB
Ch11/12 gains = -12 dB
All send gains = -70 dB/off
```

---

## Poly Resonator

Poly Resonator is stereo:

```text
Odd output  -> Aux25
Even output -> Aux26
Mixer Ch4   -> Aux25/Aux26 stereo
```

Control:

```text
Gate input 1    -> Input9
Gate 1 CV count -> 1, so Input10 is paired CV
Harmony Enable  -> Off
Sustain         -> Off
Audio input     -> None
```

---

## Sidechain Comp

```text
Ch1 input      Output1
Ch1 sidechain  Aux21
Ch2 input      Output2
Ch2 sidechain  Aux21
```

This compresses the non-kick mix using the kick as key.

---

## Kick Add-back

```text
Lane1 Aux21 -> Output1 Add
Lane2 Aux21 -> Output2 Add
```

This adds kick after the compressor.

---

## Performance notes

Use channel **Gain**, not **Mute**, for controller dry-kill buttons. Mute kills sends; Gain allows pure-wet Radiant send behaviour.

---

## Current finish status

Patch is internally finished and usable.
