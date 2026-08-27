# Witchboard

Witchboard is a routing mixer and serial patchbay plugin for the Expert Sleepers
disting NT.

Use it as a compact routing matrix for hardware, external devices, and internal
paths. Patch gear, an iPad, a computer, or NT buses into Witchboard once, then
try different source-to-route combinations from NT parameters, MIDI mappings,
faders, buttons, knobs, or CV-mapped controls without constantly repatching
cables.

| Plug-in | GUID | Release file |
|---|---|---|
| Witchboard | `WtC1` | `Witchboard.o` |

## Signal Flow

Each channel runs:

```text
Input/Left + optional Right Input
-> Gain
-> Insert 1
-> Insert 2
-> FX Send 1 dry/wet crossfade
-> FX Send 2 dry/wet crossfade
-> Main or Bypass output path
```

Witchboard supports `1..12` channels. Unused channels can stay disabled or have
`Input/Left` set to `None`.

## Basic Setup

1. Set the `Channels` specification.
2. Set each channel's `Input/Left`.
3. For stereo sources, also set `Right Input`.
4. Set `Main L/R` to the normal destination.
5. Set `Bypass L/R` if some channels should skip a later processor.
6. Configure the insert routes you want to use.
7. Configure FX Send 1/2 only if you need shared send/return FX.

## Generic Example

Say you have four sources and three insert paths:

```text
Channel 1 -> drum voice
Channel 2 -> bass voice
Channel 3 -> chord voice
Channel 4 -> lead voice

Route A -> mono filter
Route B -> iPad or computer send/return
Route C -> stereo distortion
```

Patch each source into a Witchboard channel. Patch each processor or external
device as an insert route with a send output and return input. Then set each
channel's insert slots:

| Channel | Slot 1 | Slot 2 | Slot 3 |
|---:|---|---|---|
| 1 | Route A | Route C | Route B |
| 2 | Route A | Route B | Route C |
| 3 | Route B | Route A | Route C |
| 4 | Route C | Route A | Route B |

Now `Insert 1` and `Insert 2` work like quick selectors. The same button, fader,
or knob can move a channel from Dry to Slot 1, Slot 2, or Slot 3 without changing
patch cables.

## Insert Routes

Witchboard has five generic insert routes:

```text
Route A
Route B
Route C
Route D
Route E
```

Each route has a send output, return input, send width, and return width.

For mono hardware, set send width and return width to `Mono`. For stereo
hardware, set both widths to `Stereo` and set both L/R buses.

All Witchboard outputs are fixed to Add. There are no Add/Replace output-mode
parameters in the current layout.

## Insert Slots

Each channel has two live insert selectors:

```text
Insert 1
Insert 2
```

Each selector chooses:

| Value | State |
|---:|---|
| 0 | Dry |
| 1 | Slot 1 |
| 2 | Slot 2 |
| 3 | Slot 3 |
| 4 | Slot 3 |

The slots are per channel and per insert. A slot is not hardwired to one route;
the slot assignment parameters decide what route each slot uses:

```text
Insert 1 slot 1..3 -> Route A..E
Insert 2 slot 1..3 -> Route A..E
```

Default slot assignments are Slot 1 = Route A, Slot 2 = Route B, and Slot 3 =
Route C.

`Repeat protection` stops one channel from using the same route twice in series.
When it is On, a duplicate route choice in Insert 2 behaves as Dry. When it is
Off, both insert stages may use the same assigned route.

## Four-State Control

The insert selectors are normal NT parameters, so they can be mapped to a
4-state button, fader, knob, CV-mapped control, or any other NT-mappable source.

A common 4-state controller sends:

```text
0, 42, 85, 127
```

Map that controller to `Insert 1` or `Insert 2` with:

```text
Min = 0
Max = 4
```

That creates four useful zones:

| Incoming value | Result |
|---:|---|
| 0 | Dry |
| 42 | Slot 1 |
| 85 | Slot 2 |
| 127 | Slot 3 |

The extra top value, `4`, is handled as Slot 3. This keeps `0 / 42 / 85 / 127`
controllers landing on the intended four states.

For a fader or knob, think of the same control as four zones:

```text
low      -> Dry
low-mid  -> Slot 1
high-mid -> Slot 2
high     -> Slot 3
```

## FX Sends

Each channel has:

```text
FX Send 1 mix
FX Send 2 mix
```

Each mix is a shaped dry/wet crossfade from the channel path into the shared FX
send, smoothed by `Switch fade`.

At `0%`, the channel stays dry and sends nothing to that FX output. At `100%`,
the channel is fully sent to that FX output and removed from the dry Main/Bypass
path for that send stage.

Each FX return can be routed to Main or Bypass with `FX 1 return path` and
`FX 2 return path`. FX returns are shared and mixed once, not once per source
channel.

## Output Paths

Each channel can route to:

| Output path | Destination |
|---|---|
| Main | `Main L/R` |
| Bypass | `Bypass L/R` |

Use Bypass when a channel should skip a later processor.

## Naming

Witchboard can show preset-specific names for routes, FX sends, and insert slot
states. Names are preset data, not hardcoded plugin logic.

Preset serialization supports:

```text
witchboardNames.routes
witchboardNames.fx
witchboardNames.slots
```

## Parameter Count

Witchboard uses `49` global parameters and `16` parameters per channel.

| Channels | Plugin parameters |
|---:|---:|
| 1 | 65 |
| 4 | 113 |
| 8 | 177 |
| 12 | 241 |

The current layout is designed so a 12-channel instance fits under the NT
parameter limit.

## Installation

Copy the object to the disting NT MicroSD card:

```text
Witchboard.o -> /programs/plug-ins/Witchboard.o
```

Then rescan plugins or restart the module.

This plugin is built against the official disting NT plugin API v13.

## Building

```sh
make
```

By default the Makefile expects the official API at `../distingNT_API`. Override
it with `NT_API_PATH` if needed:

```sh
make NT_API_PATH=/path/to/distingNT_API
```

For a release-ready local check:

```sh
make package NT_API_PATH=/path/to/distingNT_API
```

That runs validation, the focused C++ host test, ARM build, object inspection,
and creates release artifacts.

## Quick Check

After installing a new object:

1. Add or load Witchboard.
2. Confirm `Channels` can be set to the count you need.
3. Set one channel input and leave both inserts Dry.
4. Confirm dry audio reaches Main or Bypass.
5. Configure one route send/return.
6. Assign that route to `Insert 1 slot 1`.
7. Change `Insert 1` from Dry to Slot 1.
8. Confirm audio switches route immediately.
9. Map a controller to `Insert 1` with `Min = 0`, `Max = 4`.
10. Confirm the control selects Dry / Slot 1 / Slot 2 / Slot 3.

## Repository Layout

```text
Makefile
plugins/Witchboard/Witchboard.cpp
scripts/
tests/
```

`plugins/Witchboard/Witchboard.cpp` is the current source used by the Makefile.
