# MySetup2 — Routing Matrix Direction

Planning doc for the `_Radiant` preset update.

## Core routing decision

The per-source 4-state routing button is:

```text
1. Dry
2. Pico MMF
3. MS-22
4. Percall ch4
```

**Radiant is not one of the 4 button states.**

Radiant is a separate send available from:

```text
Main/source mixer -> Radiant send
Return mixer      -> Radiant send
```

So the matrix chooses the main path/processor, and Radiant can be added separately.

---

## Percall role

Percall channels 1–3 are source performance control. Percall ch4 is the Disting shared send/return processor.

```text
Percall ch1 = Radio Station control
Percall ch2 = Chord Organ control
Percall ch3 = Pico/Pony combined 2-osc bass control
Percall ch4 = Disting shared Percall send/return
```

Disting Percall loop:

```text
Disting OUT4 -> Percall ch4 -> Disting IN5
```

No stereo Percall send. No OUT3 Percall use.

---

## Pico/Pony bass concept

The Pico VCO is consolidated with the Pony as one stronger bass instrument, not treated as a separate independent OXI lead voice.

```text
Pico VCO = second oscillator / FM source / modulation source
Pony VCO = main bass oscillator
Percall ch3 = shapes/attenuates the combined Pico/Pony bass voice
```

The Pico can FM the Pony, with Percall ch3 used to control/attenuate that relationship.

---

## OXI / Turing layout

Current OXI One Mk1 track use:

```text
Track 1 = Radio Station
Track 2 = Chord Organ
Track 3 = Pico/Pony 2-osc bass
Track 4 = multitrack drums on Disting
```

Planned OXI consolidation:

```text
Track 1 = multitrack Radio Station vocal glitching + Chord Organ
Track 2 = internal Disting voice, likely NT-303
Track 3 = Pico/Pony 2-osc bass
Track 4 = multitrack drums on Disting
```

Turing Machine controls the Poly Resonator:

```text
Turing pulse/gate -> Disting IN9  -> Poly Resonator gate/trigger
Turing CV/pitch   -> Disting IN10 -> Poly Resonator pitch/CV
```

---

## Physical Disting inputs

```text
IN1   Radio Station path after Percall ch1
IN2   Chord Organ path after Percall ch2
IN3   Pico/Pony bass path / Pico side after Percall ch3
IN4   Pony/Pico bass path / Pony side
IN5   Percall ch4 return from Disting OUT4
IN6   Free
IN7   Radiant return L
IN8   Radiant return R
IN9   Turing Machine pulse/gate for Poly Resonator
IN10  Turing Machine CV/pitch for Poly Resonator
IN11  Pico MMF return
IN12  MS-22 return
```

---

## Physical Disting outputs

```text
OUT1/2  Final stereo to Messor -> Befaco Output Bus
OUT3    Free
OUT4    Percall ch4 send
OUT5    Pico MMF send
OUT6    MS-22 send
OUT7/8  Radiant send L/R
```

---

## Internal voice buses

```text
Aux21  Kick
Aux22  Snare
Aux23  Hat
Aux24  Sliced / Perc
Aux25  Poly Resonator L/Odd
Aux26  Poly Resonator R/Even
Aux27  NT-303
Aux28  Poly Lofi
```

---

## Source mixer / routing matrix

Slot 8 remains the main source/routing mixer.

Important sends:

```text
Send1 -> OUT4    Percall ch4
Send2 -> OUT7/8  Radiant
```

For the full 4-state matrix, the source mixer/switchboard needs paths for:

```text
Dry       -> main mix
Pico MMF  -> OUT5 -> Pico MMF -> IN11
MS-22     -> OUT6 -> MS-22 -> IN12
Percall4  -> OUT4 -> Percall ch4 -> IN5
```

Radiant remains separate:

```text
Source mixer Radiant send -> OUT7/8 -> Radiant -> IN7/8
```

---

## Return Mixer

Slot 10: **RETURN MIXER**, currently useful as 4 channels / 1 send.

Target channels:

```text
Ch1  Aux21  Kick add-back
Ch2  IN5    Percall ch4 return
Ch3  IN11   Pico MMF return
Ch4  IN12   MS-22 return
```

Return Mixer main:

```text
Main L/R -> OUT1/2 Add
```

Return Mixer Radiant send:

```text
Send1 -> OUT7/8 Add
```

Do **not** put Radiant return IN7/8 into the Return Mixer.

---

## 4-state source button concept

Per source, the routing button selects one of four paths:

```text
Dry      = source to main mix
Pico MMF = source to OUT5, returns on IN11
MS-22    = source to OUT6, returns on IN12
Percall4 = source to OUT4, returns on IN5
```

Radiant is independent/separate from this 4-state choice.

---

## Safety defaults

Keep these off unless deliberately doing feedback:

```text
Percall ch4 return -> Percall ch4 send
Pico MMF return -> Pico MMF send
MS-22 return -> MS-22 send
Radiant return -> Radiant send
```

Use **Add** output mode for routing/mixer outputs. Compressor insert on OUT1/2 is the intentional exception.
