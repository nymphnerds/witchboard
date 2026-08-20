# Archived Switchboard User Guide

## What Switchboard Does

Switchboard is one disting NT algorithm for routing up to 12 mono or stereo
channels. Each channel passes through two insert selectors in sequence:

```text
Sound
-> Insert 1: None, Route A, Route B, or Route C
-> selected Route return
-> Insert 2: None, Route A, Route B, or Route C
-> selected Route return
-> Main or Bypass
-> optional stereo FX Send 1 and FX Send 2 copies
```

`None` skips that insert and lets the sound continue. Route A, B, and C are three
user-configured external send/return paths. Only one choice is active at each
insert.

Switchboard performs routing with post-insert send levels. It contains no channel
fader, compressor, sidechain, filter, or effect.

## One Algorithm

The `Switchboard.o` file adds exactly one algorithm named `Switchboard`.

Add it once. The same Channel page contains both `Insert 1` and `Insert 2`.
Route Setup, Final Outputs, FX Sends, and MIDI Profile also belong to that one
algorithm instance.

## Page Layout

1. `Route Setup`: Route A/B/C sends, returns, Width, and Add/Replace modes.
2. `Final Outputs`: Main and Bypass destinations.
3. `FX Sends`: both send destinations and every Channel's two send levels.
4. `Button Setup`: maps the four button states to None or Route A/B/C.
5. `MIDI Profile`: shared CC offsets, values, and tolerance.
6. `Channel 1-12`: audio input, output path, Inserts, repeat protection, and both MIDI button assignments.

## Recommended Setup Order

1. Add `Switchboard` and choose the number of Channels.
2. Open `Route Setup` and configure any external Routes you use.
3. Open `Final Outputs` and choose the Main and Bypass buses.
4. Open `FX Sends` and configure both sends and each Channel's send levels.
5. Configure each Channel's input and two Insert choices.
6. Configure the two MIDI buttons for each Channel.

Routes A, B, and C begin completely unassigned. Nothing is preselected for their
Outputs or Returns.

## Four-State Buttons

The global `Button Setup` page maps the four button states. Defaults are:

| Button state | Choice |
|---|---|
| State 1 | None |
| State 2 | Route A |
| State 3 | Route B |
| State 4 | Route C |

Both Insert buttons use this mapping. Each Channel can use 2, 3, or 4 states for
each Insert.

## Complete Parameter List

### When Adding Switchboard

| Parameter | Options | Default | What it does |
|---|---|---|---|
| `Channels` | 1-12 | 4 | Number of independent mono/stereo sounds handled by this Switchboard. |

Choose `Channels = 1` for the first test. The NT lets you respecify the Channel
count later from the algorithm's specification controls.

### Route Setup

| Parameter | Options | Default | What it does |
|---|---|---|---|
| `Switch fade` | 0-100 ms | 2 ms | Smooths Insert and FX Send level changes to reduce clicks. |
| `Route A output L` | None or any NT bus | None | Route A left or mono hardware send. |
| `Route A output R` | None or any NT bus | None | Optional Route A stereo right send. |
| `Route A return L` | None or any NT bus | None | Route A left or mono hardware return. |
| `Route A return R` | None or any NT bus | None | Optional Route A stereo right return. |
| `Route A send width` | Mono, Stereo | Mono | Chooses a mono or stereo send to Route A. |
| `Route A return width` | Mono, Stereo | Mono | Chooses a mono or stereo return from Route A. |
| `Route A output mode` | Add, Replace | Add | Adds to or replaces audio already on Route A's output. |
| `Route B output L` | None or any NT bus | None | Route B left or mono hardware send. |
| `Route B output R` | None or any NT bus | None | Optional Route B stereo right send. |
| `Route B return L` | None or any NT bus | None | Route B left or mono hardware return. |
| `Route B return R` | None or any NT bus | None | Optional Route B stereo right return. |
| `Route B send width` | Mono, Stereo | Mono | Chooses a mono or stereo send to Route B. |
| `Route B return width` | Mono, Stereo | Mono | Chooses a mono or stereo return from Route B. |
| `Route B output mode` | Add, Replace | Add | Adds to or replaces audio already on Route B's output. |
| `Route C output L` | None or any NT bus | None | Route C left or mono hardware send. |
| `Route C output R` | None or any NT bus | None | Optional Route C stereo right send. |
| `Route C return L` | None or any NT bus | None | Route C left or mono hardware return. |
| `Route C return R` | None or any NT bus | None | Optional Route C stereo right return. |
| `Route C send width` | Mono, Stereo | Mono | Chooses a mono or stereo send to Route C. |
| `Route C return width` | Mono, Stereo | Mono | Chooses a mono or stereo return from Route C. |
| `Route C output mode` | Add, Replace | Add | Adds to or replaces audio already on Route C's output. |

`Mono` Send Width downmixes the signal to Output L. `Stereo` uses Output L and R.
`Mono` Return Width reads Return L and continues as dual mono. `Stereo` reads both
Return L and R. Send and Return Width are independent, so mono-send/stereo-return
processors are supported directly.

### Final Outputs

| Parameter | Options | Default | What it does |
|---|---|---|---|
| `Main L` | None or any NT bus | Output 1 | Main left or mono destination after both Inserts. |
| `Main R` | None or any NT bus | Output 2 | Main right destination. None makes Main mono. |
| `Main output mode` | Add, Replace | Add | Adds to or replaces audio already on Main. |
| `Bypass L` | None or any NT bus | A31 | Alternate left or mono destination. |
| `Bypass R` | None or any NT bus | A32 | Alternate right destination. None makes Bypass mono. |
| `Bypass output mode` | Add, Replace | Add | Adds to or replaces audio already on Bypass. |

Begin with every output mode set to `Add`.

### FX Sends

This page contains both FX Send destinations followed by the two send levels for
every audio Channel.

| Parameter | Options | Default | What it does |
|---|---|---|---|
| `FX Send 1 L` | None or any NT bus | Output 7 | First post-insert send's left or mono destination. |
| `FX Send 1 R` | None or any NT bus | Output 8 | First post-insert send's right destination. None makes it mono. |
| `FX Send 1 width` | Mono, Stereo | Stereo | Chooses a mono or stereo FX Send 1. |
| `FX Send 1 output mode` | Add, Replace | Add | Adds to or replaces audio already on FX Send 1. |
| `FX Send 2 L` | None or any NT bus | None | Second post-insert send's left or mono destination. |
| `FX Send 2 R` | None or any NT bus | None | Second post-insert send's right destination. None makes it mono. |
| `FX Send 2 width` | Mono, Stereo | Stereo | Chooses a mono or stereo FX Send 2. |
| `FX Send 2 output mode` | Add, Replace | Add | Adds to or replaces audio already on FX Send 2. |
| `FX Send 1 level` | -inf to 0 dB | -inf | This Channel's level to FX Send 1. |
| `FX Send 2 level` | -inf to 0 dB | -inf | This Channel's level to FX Send 2. |

The Channel number shown beside each level identifies the audio Channel it affects.
Both sends work whether that Channel's Output path is Main or Bypass. `-inf` turns
that send off without changing the Main/Bypass signal. Mono Width downmixes to the
L destination; Stereo Width uses both L and R.

### Button Setup

These four settings are shared by every Channel and both Inserts.

| Parameter | Options | Default | What it does |
|---|---|---|---|
| `State 1 choice` | None, Route A, Route B, Route C | None | Choice selected by button state 1. |
| `State 2 choice` | None, Route A, Route B, Route C | Route A | Choice selected by button state 2. |
| `State 3 choice` | None, Route A, Route B, Route C | Route B | Choice selected by button state 3. |
| `State 4 choice` | None, Route A, Route B, Route C | Route C | Choice selected by button state 4. |

### Each Channel

| Parameter | Options | Default | What it does |
|---|---|---|---|
| `Enable` | Off, On | On | Turns this complete Channel off or on. |
| `Input/Left` | None or any NT bus | Matching Input number | Mono input or the left side of a stereo sound. |
| `Right Input` | None or any NT bus | None | Stereo right input. None tells Switchboard the sound is mono. |
| `Output path` | Main, Bypass | Main | Chooses the final destination after Insert 2. |
| `Repeat protection` | Off, On | On | Stops this Channel's Insert 2 from selecting the same Route as Insert 1. |
| `Insert 1` | State 1-4 | State 1 / None | Live selector for the first insert. The screen shows its assigned choice. |
| `Insert 1 states` | 2, 3, 4 | 4 | Number of states recognised for the first button. |
| `Insert 1 MIDI Ch` | 0-16 | 0 | MIDI channel for the first button. 0 disables it. |
| `Insert 1 Base CC` | 0-127 | 80 | CC used by the first button. Leave at 80 for the XVI-M setup below. |
| `Insert 2` | State 1-4 | State 1 / None | Live selector for the second insert. |
| `Insert 2 states` | 2, 3, 4 | 4 | Number of states recognised for the second button. |
| `Insert 2 MIDI Ch` | 0-16 | 0 | MIDI channel for the second button. 0 disables it. |
| `Insert 2 Base CC` | 0-127 | 80 | CC used by the second button. Leave at 80 for the XVI-M setup below. |

Mono sounds are copied to both sides of stereo destinations. Stereo sounds remain
stereo. A stereo signal is downmixed when a Route has only its left/mono output.

### XVI-M Four-State Setup

Switchboard is ready for the Michigan Synth Works XVI-M configuration below. In
the XVI-M web editor, set the button `Mode` to `4P`, select `CC`, then enter:

| XVI-M field | Off | 1P / red | 2P / green | 3P / orange |
|---|---:|---:|---:|---:|
| `CC` | 80 | 80 | 80 | 80 |
| `Value` | 0 | 42 | 85 | 127 |

Choose the required MIDI Port and set each XVI-M strip to its intended MIDI
Channel. The button follows its fader's MIDI Channel, but the fader's own CC can
remain separate; only the button messages above control Switchboard's Inserts.

In Switchboard, open the matching audio `Channel` page and set:

- `Insert 1 MIDI Ch` to the MIDI Channel of its first button.
- `Insert 2 MIDI Ch` to the MIDI Channel of its second button.
- Leave both Base CC settings at `80`.

No MIDI Profile editing is required. MIDI Channel `0` disables that Insert's MIDI
control.

For eight audio Channels, one straightforward layout is:

- XVI-M buttons 1-8 / MIDI Channels 1-8 control Insert 1.
- XVI-M buttons 9-16 / MIDI Channels 9-16 control Insert 2.

XVI-M reference: https://michigansynthworks.com/products/xvi-m

### MIDI Profile

The default profile already matches the XVI-M setup above. These parameters remain
editable so Switchboard can support other multistate MIDI controllers.

| Parameter | Options | Default | What it does |
|---|---|---|---|
| `State 1 CC offset` | -127 to +127 | 0 | Added to Base CC for state 1. |
| `State 1 value` | 0-127 | 0 | MIDI value selecting state 1. |
| `State 2 CC offset` | -127 to +127 | 0 | Added to Base CC for state 2. |
| `State 2 value` | 0-127 | 42 | MIDI value selecting state 2. |
| `State 3 CC offset` | -127 to +127 | 0 | Added to Base CC for state 3. |
| `State 3 value` | 0-127 | 85 | MIDI value selecting state 3. |
| `State 4 CC offset` | -127 to +127 | 0 | Added to Base CC for state 4. |
| `State 4 value` | 0-127 | 127 | MIDI value selecting state 4. |
| `Value Tolerance` | 0-16 | 2 | Allowed difference from the configured MIDI value. |

## Repeat Protection

Each Channel has its own Repeat protection setting. If its Insert 1 and Insert 2
select the same external Route, sending the return straight back to the same
hardware can create feedback. With protection On, Switchboard treats the second
identical choice as None. Set it Off only for Channels where repeating a Route is
intentional.

## First Test: One Mono Source

1. Copy `plugins/Switchboard.o` to `/programs/plug-ins/` on the NT MicroSD card.
2. Restart the NT or rescan plug-ins.
3. Create a blank preset.
4. Add `Switchboard` and choose `Channels = 1`.
5. Connect the mono sound to Input 1.
6. Leave `Input/Left = Input 1` and `Right Input = None`.
7. Leave both Inserts on State 1 / None, `Output path = Main`, and both FX Send levels at `-inf`.
8. Monitor Outputs 1 and 2 at a low level.

Expected result: the mono sound appears on both Outputs 1 and 2.

Routes remain silent until their Output and Return buses are assigned on Route
Setup. Test one external Route at a time before building the full two-insert chain.

## Current Hardware Test Status

- The plugin loads as one `Switchboard` algorithm.
- The Channel specification has been respecified from one to eight on the NT.
- The optimized eight-Channel build has shown approximately 13% CPU in the initial test.
- Parameter layout is confirmed; full Route, MIDI switching, mono/stereo, and FX Send tests are next.
