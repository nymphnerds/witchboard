# Personal Witchboard Wiring

This is the current hardware wiring reference for the personal Witchboard build.

## Physical Inputs

| NT input | Signal | Use |
|---|---|---|
| I1 | Radio Station through Percall 1 | Mono source |
| I2 | Chord Organ through Percall 2 | Mono source |
| I3 | Bass / Pony VCO | Mono source |
| I4 | Percall 4 output | Mono insert return |
| I5 | Turing Pulse | Control signal; not Witchboard audio |
| I6 | Turing CV | Control signal; not Witchboard audio |
| I7 | Radiant left output | Stereo FX return left |
| I8 | Radiant right output | Stereo FX return right |
| I9 | Free | - |
| I10 | Free | - |
| I11 | Pico MMF output | Mono insert return |
| I12 | Threetom MS22 output | Mono insert return |

## Physical Outputs

| NT output | Destination | Use |
|---|---|---|
| O1 | Messor left input | Final output left |
| O2 | Messor right input | Final output right |
| O3 | Threetom MS22 input | Mono insert send |
| O4 | Pico MMF input | Mono insert send |
| O5 | Percall 4 input | Mono insert send |
| O6 | Free | - |
| O7 | Radiant left input | Stereo FX send left |
| O8 | Radiant right input | Stereo FX send right |

The Messor is the final end-of-chain glue processor after the disting NT.

## Bass Voice

```text
Pico VCO -> Percall 3 -> Pony VCO FM input
Percall 3 envelope -> Pony VCO VCA input
Pony VCO audio -> I3
```

Only the Pony VCO output enters Witchboard, so the complete bass voice uses one
audio input and one Witchboard channel.

## Four-Channel Test

| Witchboard channel | Source | Output path | XVI-M strips |
|---|---|---|---|
| 1 | I1 Radio Station | Main / compressor | 1 + 2 |
| 2 | I2 Chord Organ | Main / compressor | 3 + 4 |
| 3 | I3 Bass | Main / compressor | 5 + 6 |
| 4 | A21 Kick Sample Player | Bypass / unducked | 7 + 8 |

Channels 5-8 are intentionally left for the later preset.

## Insert Routes

Both serial Insert buttons use the same four states:

| Button state | MIDI value | Route |
|---|---:|---|
| Off | 0 | No insert / dry |
| Red | 42 | Threetom MS22: O3 -> processor -> I12 |
| Green | 85 | Pico MMF: O4 -> processor -> I11 |
| Orange | 127 | Percall 4: O5 -> processor -> I4 |

All three inserts are physically mono. Witchboard copies each returned mono
signal to left and right before continuing through the stereo output path.

## Radiant FX

```text
Witchboard FX Send 1 L/R -> O7/O8 -> Radiant -> I7/I8
```

Radiant is a true stereo send and return. FX Send 2 remains available for a future
iPad send/return and has no physical default yet. Both sends use per-channel
dry/wet mix controls rather than ordinary additive send levels.

## Main And Compressor Bypass

```text
Main channels -> Witchboard Main A31/A32 -> separate NT sidechain compressor in place
A31/A32 -> small Mixer Stereo output router -> O1/O2 Add
Bypass channels -> Witchboard Bypass -> O1/O2 directly
O1/O2 -> Messor -> final output
```

Witchboard does not contain the compressor or its sidechain key. The compressor
remains a separate NT algorithm. Its processed A31/A32 buses need the small
post-compressor Mixer Stereo only as an output router. The Bypass path skips the
compressor but still passes through the Messor.

## XVI-M Native Mappings

Each Witchboard channel uses two adjacent XVI-M strips:

| Witchboard channel | XVI-M strips | MIDI channels |
|---|---|---|
| 1 | 1 + 2 | 1 + 2 |
| 2 | 3 + 4 | 3 + 4 |
| 3 | 5 + 6 | 5 + 6 |
| 4 | 7 + 8 | 7 + 8 |
| 5 | 9 + 10 | 9 + 10 |
| 6 | 11 + 12 | 11 + 12 |
| 7 | 13 + 14 | 13 + 14 |
| 8 | 15 + 16 | 15 + 16 |

The supplied hardware preset configures every pair as follows:

| Strip | Fader, CC 9 | Button, CC 80 |
|---|---|---|
| First | Channel gain | Insert 1 |
| Second | Radiant dry/wet crossfade | Insert 2 |

Configure all faders as standard **7-bit CC9**. Witchboard does not use the
XVI-M's 14-bit fader mode.

Radiant crossfade behaviour:

```text
CC 9 value 0   = fully dry, no Radiant send
CC 9 midpoint  = dry/Radiant blend
CC 9 value 127 = no dry, full Radiant send
```

Both buttons use CC 80 and the values `0`, `42`, `85`, and `127` shown in the
Insert Routes table.

FX Send 2 is unmapped and set to 0% in this preset. It remains available for a
future iPad send/return with a separately assigned native mapping.
