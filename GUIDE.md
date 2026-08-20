# Witchboard User Guide

## What It Does

Witchboard routes each sound through two selectable inserts in series, then to
either a normal Main path or a compressor-bypass path.

```text
Input
-> Channel Gain
-> Insert 1: Dry, MS22, Pico MMF, or Percall 4
-> Insert 2: Dry, MS22, Pico MMF, or Percall 4
-> FX Send 1 and FX Send 2 dry/wet crossfades
-> Main or Bypass
```

Mono insert returns are copied to left and right. Stereo sources remain stereo
until sent through a mono insert.

## Personal Four-Channel Defaults

| Channel | Input | Path | Control 1 | Control 2 |
|---|---|---|---|---|
| 1 | I1 Radio Station | Main | MIDI Ch 1 | MIDI Ch 2 |
| 2 | I2 Chord Organ | Main | MIDI Ch 3 | MIDI Ch 4 |
| 3 | I3 Bass | Main | MIDI Ch 5 | MIDI Ch 6 |
| 4 | A21 Kick | Bypass | MIDI Ch 7 | MIDI Ch 8 |

The supplied hardware preset maps Control 1's CC9 fader to Gain and its
CC80 button to Insert 1. Control 2's CC9 fader maps only to the Radiant dry/wet
crossfade and its CC80 button maps to Insert 2. FX Send 2 is unmapped and Off.

The XVI-M faders must be configured as standard **7-bit CC** controls. Do not use
14-bit mode. Keep `CC9` and the MIDI channels shown above.

## Button States

Both Insert buttons use these XVI-M values:

| State | Value | Selection |
|---|---:|---|
| Off | 0 | Dry / no insert |
| Red | 42 | MS22 |
| Green | 85 | Pico MMF |
| Orange | 127 | Percall 4 |

## FX Crossfades

| Fader value | Result |
|---:|---|
| 0 | Full dry, no FX send |
| Midpoint | Dry/wet blend |
| 127 | No dry, full FX send |

Both crossfades are linear and smoothed using `Switch fade`. The shared stereo
returns are each mixed once, not once per source. Radiant's default return path
is Main, so its wet audio is ducked by the separate compressor. Select Bypass to
leave the return unducked.

## Parameters

### Specification

| Parameter | Options | Default |
|---|---|---|
| `Channels` | 1-12 | 4 |

### Route Setup

| Parameter group | Options | Personal default |
|---|---|---|
| `Switch fade` | 0-100 ms | 2 ms |
| Route A send/return | Any NT buses; Mono/Stereo; Add/Replace | O3 -> MS22 -> I12, Mono, Add |
| Route B send/return | Any NT buses; Mono/Stereo; Add/Replace | O4 -> Pico MMF -> I11, Mono, Add |
| Route C send/return | Any NT buses; Mono/Stereo; Add/Replace | O5 -> Percall 4 -> I4, Mono, Add |

### Final Outputs

| Parameter | Options | Default |
|---|---|---|
| `Main L/R` | Any NT output buses | A31/A32 |
| `Main output mode` | Add, Replace | Add |
| `Bypass L/R` | Any NT output buses | O1/O2 |
| `Bypass output mode` | Add, Replace | Add |

The separate Sidechain Compressor should process A31/A32 in place and use A21 as
its key. A small Mixer Stereo after it routes A31/A32 to O1/O2 in Add mode. This
lets the kick use Bypass without being ducked.

### FX Sends

| Parameter | Options | Default |
|---|---|---|
| `Radiant send L/R` | Any NT output buses | O7/O8 |
| `FX Send 1 width` | Mono, Stereo | Stereo |
| `FX Send 1 output mode` | Add, Replace | Add |
| `Radiant return L/R` | Any NT input buses | I7/I8 |
| `FX 1 return width` | Mono, Stereo | Stereo |
| `FX 1 return path` | Main, Bypass | Main |
| `FX Send 2 L/R` | Any NT output buses | None |
| `FX Send 2 width` | Mono, Stereo | Stereo |
| `FX Send 2 output mode` | Add, Replace | Add |
| `FX 2 return L/R` | Any NT input buses | None |
| `FX 2 return width` | Mono, Stereo | Stereo |
| `FX 2 return path` | Main, Bypass | Main |
| `FX Send 2 mix` | 0-100%, per channel | 0% |

### Each Channel

| Parameter | Options | Default behaviour |
|---|---|---|
| `Enable` | Off, On | On |
| `Input/Left` | None or any NT bus | See four-channel table |
| `Right Input` | None or any NT bus | None / mono |
| `Gain` | -inf to 0 dB | 0 dB |
| `Output path` | Main, Bypass | Kick Bypass; others Main |
| `Repeat protection` | Off, On | On |
| `Insert 1` | State 1-4 | Dry |
| `Insert 1 states` | 2, 3, 4 | 4 |
| `Insert 2` | State 1-4 | Dry |
| `Insert 2 states` | 2, 3, 4 | 4 |
| `Radiant mix` | 0-100% | 0% |
| `FX Send 2 mix` | 0-100% | 0% |

### Native NT MIDI Mappings

Witchboard has no private MIDI decoder. All MIDI control uses the disting NT's
normal MIDI Mappings system and is stored in the preset JSON.

| Control strip | Native mapping |
|---|---|
| First in pair | CC9 -> Gain; CC80 -> Insert 1 (Min 0, Max 3) |
| Second in pair | CC9 -> Radiant mix; CC80 -> Insert 2 (Min 0, Max 3) |

The supplied `presets/Witchboard 4ch Hardware.json` sets the complete four-channel
hardware routing and channel pairs 1/2,
3/4, 5/6, and 7/8. The button values 0, 42, 85, and 127 are scaled by the
disting NT native MIDI mapping system across Insert parameter values 0-3:
Dry, Threetom MS22, Pico MMF, and Percall 4. Preset JSON stores MIDI channels
as the displayed channel numbers 1-16.

## First Test

1. Install `plugins/Witchboard.o` and remove the old `Switchboard.o`.
2. Load `presets/Witchboard 4ch Hardware.json` for the complete four-channel test.
3. Monitor the Messor/output chain at a low level.
4. Leave both Inserts dry and both FX mixes at 0%.
5. Test Radio, Chord, Bass, then Kick one at a time.
6. Test one insert on one channel at a time.
7. Test Radiant last.

The first hardware test should verify audio, MIDI, routing and CPU before adding
more source channels.
