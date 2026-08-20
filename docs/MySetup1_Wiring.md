# Disting NT Wiring

**Updated:** 2026-08-13

This is the live patch wiring reference. It documents routing and safety-critical defaults only; current jam levels are intentionally not tracked.

---

## Core architecture

```text
Internal voices + external voices + returns
        -> RADIANT MIXER
        -> SIDECHAIN COMP
        -> Output 1/2 -> Messor -> Befaco Output Bus

Kick Aux21 -> compressor key
Kick Aux21 -> KICK ADD-BACK -> Output 1/2 after compressor
```

Kick is outside the main mixer so it can key the compressor without ducking itself.

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

Note: Turing pulse/gate had a hardware issue earlier; temporary chip-swap fix restored the gate for Poly Resonator.

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

## Physical effect loops

### Percall ch4 loop

```text
Mixer Send 1 -> Output 4 -> Percall ch4 -> Input 5 -> Mixer Ch11
```

### Radiant loop

```text
Mixer Send 2 -> Output 7/8 -> Radiant -> Input 7/8 -> Mixer Ch12 stereo
```

Both sends are pre-fade so a channel can be sent to Percall/Radiant even if its dry gain is pulled down.

---

## Aux bus map

```text
Aux21  Kick Sample Player / sidechain key / post-comp add-back
Aux22  Snare Sample Player
Aux23  Hat Sample Player
Aux24  Sliced Sample Player
Aux25  Poly Resonator Odd / Left
Aux26  Poly Resonator Even / Right
Aux27  NT-303
Aux28  Poly Lofi
```

Unused old-plan buses:

```text
Aux29    old Kirbinator send
Aux31/32 old dry mix
Aux33/34 old Kirbinator return
```

---

## RADIANT MIXER channels

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

## Sends

```text
Send1 -> Output 4,   Pre-fade, Mono,   Add   # Percall ch4
Send2 -> Output 7/8, Pre-fade, Stereo, Add   # Radiant
```

---

## Safety-critical defaults

- Do **not** send Percall Return Ch11 back to Send1 by default.
- Do **not** send Radiant Return Ch12 back to Send2 by default.
- Sampler loops should be Off for one-shot drum/sample behaviour.
- Use Add mode for mixer/routing outputs.
- Compressor is the intentional insert-style exception on Output 1/2.
- Use channel Gain, not Mute, for performance dry kills.
