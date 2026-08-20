# Witchboard Whole-Patch Handoff

This is the current handoff for building the full patch around the clean Witchboard plugin and the `Witchboard Example` preset. It contains only the current intended setup.

## Current Files

| Item | Path |
|---|---|
| GitHub repository | `https://github.com/nymphnerds/witchboard` |
| Plugin source | `M:\DistingNT\WitchboardPlugin\src\WitchboardClean.cpp` |
| Built plugin | `M:\DistingNT\WitchboardPlugin\plugins\Witchboard.o` |
| Installed plugin | `M:\DistingNT\programs\plug-ins\Witchboard.o` |
| Backup/card-copy plugin | `M:\DistingNT\Babyjaws NT\programs\plug-ins\Witchboard.o` |
| Example preset | `M:\DistingNT\WitchboardPlugin\presets\Witchboard Example.json` |
| Hardware setup image | `M:\DistingNT\WitchboardPlugin\assets\babyjaws.jpg` |
| MIDI controller setup image | `M:\DistingNT\WitchboardPlugin\assets\midiController.png` |
| Current NT patch image | `M:\DistingNT\WitchboardPlugin\assets\current-patch-radiant-mixer-target.png` |
| Preset validator | `M:\DistingNT\WitchboardPlugin\tests\validate_preset.py` |
| Source isolation test | `M:\DistingNT\WitchboardPlugin\tests\WitchboardCleanTest.cpp` |

## Reference Images

| Asset | Purpose |
|---|---|
| `assets\babyjaws.jpg` | Photo of my hardware setup for this preset. |
| `assets\midiController.png` | My known-correct MiSW XVI-M setup for this preset: TRS 1, channels 1-16, fader CC9, Toggle 4P button CC80, values 0/42/85/127. |
| `assets\current-patch-radiant-mixer-target.png` | My current NT patch context around the mixer/router area Witchboard replaces or simplifies. |

These are example/reference images for my rig. They are not requirements for
Witchboard. The plugin itself stays generic: mappings, routes, sources, outputs,
and names can all be changed by hand.

## Plugin Identity

| Field | Value |
|---|---|
| Algorithm name | `Witchboard` |
| GUID | `WtC1` |
| API | disting NT plugin API v13 |
| Source used by Makefile | `src/WitchboardClean.cpp` |
| MIDI callbacks | None; native NT MIDI Mapping owns MIDI |
| Serialisation | Used only for route/FX names via `witchboardNames` |

The GUID is intentionally `WtC1`. It separates the clean plugin from older incompatible Witchboard builds. The visible algorithm name should simply be `Witchboard`.

## Core Purpose

Witchboard is a routing mixer and serial patchbay. Each channel can pass through two selectable hardware inserts, then crossfade into stereo FX sends, then leave by either a Main path or a Bypass path.

```text
source
  -> Gain
  -> Insert 1: Dry / Route A / Route B / Route C
  -> Insert 2: Dry / Route A / Route B / Route C
  -> FX Send 1 dry/wet crossfade
  -> FX Send 2 dry/wet crossfade
  -> Main or Bypass output pair
```

Witchboard does not contain the compressor. The compressor/output routing is part of the larger NT patch.

In this patch, Witchboard replaces a much heavier stock-mixer routing stack. The
purpose-built plugin is roughly an order of magnitude lighter on CPU than
building the same routing behaviour from multiple mixers and routers.

## Parameter Shape

For a four-channel instance, Witchboard has 84 plugin parameters. Preset JSON has 85 values because the NT host common parameter comes first.

Global plugin parameters:

| Range | Purpose |
|---|---|
| `0` | Switch fade |
| `1..21` | Route A/B/C sends, returns, widths, modes |
| `22..27` | Main and Bypass output pairs/modes |
| `28..43` | FX Send 1 and FX Send 2 setup |

Per-channel parameters begin at plugin index 44, with 10 parameters per channel:

| Offset | Parameter |
|---:|---|
| 0 | Enable |
| 1 | Input/Left |
| 2 | Right Input |
| 3 | Gain |
| 4 | Insert 1 |
| 5 | Radiant mix / FX Send 1 mix |
| 6 | Insert 2 |
| 7 | Output path |
| 8 | Repeat protection |
| 9 | FX Send 2 mix |

## Example Preset

Preset name: `Witchboard Example`

Preset file:

```text
M:\DistingNT\WitchboardPlugin\presets\Witchboard Example.json
```

The example is the current four-channel hardware setup.

| Witchboard channel | Source | Input | Output path | MIDI strips |
|---:|---|---|---|---|
| 1 | Radio Station | I1 | Main | 1 + 2 |
| 2 | Chord Organ | I2 | Main | 3 + 4 |
| 3 | Bass / Pony VCO voice | I3 | Main | 5 + 6 |
| 4 | Kick Sample Player | Aux 21 | Bypass | 7 + 8 |

## Hardware Routes

| Route | Hardware | Send | Return |
|---|---|---|---|
| Route A | Threetom MS22 | O3 / bus 15 | I12 |
| Route B | Pico MMF | O4 / bus 16 | I11 |
| Route C | Percall 4 | O5 / bus 17 | I4 |
| FX Send 1 | Radiant stereo | O7/O8 / buses 19/20 | I7/I8 |
| FX Send 2 | iPad/spare | None by default | None by default |

The preset includes:

```json
"witchboardNames": {
  "routes": ["Threetom MS22", "Pico MMF", "Percall 4"],
  "fx": ["Radiant", "iPad"]
}
```

## Main, Bypass, And Why Sources May Be Silent

The example preset is designed for the full patch, not as a standalone direct-output mixer.

| Path | Witchboard output |
|---|---|
| Main | Aux 31/32 |
| Bypass | O1/O2 |

Channels 1-3 are set to `Output path = Main`, so they go to Aux 31/32. They will not be heard at O1/O2 unless the downstream compressor/output-router part of the patch is loaded and routing Aux 31/32 onward.

Channel 4 is set to `Output path = Bypass`, so the kick goes directly to O1/O2.

For a standalone audio test without the downstream compressor chain, temporarily do one of these:

1. Set `Main L = O1` and `Main R = O2` in Witchboard.
2. Or set channels 1-3 `Output path = Bypass`.

For the full patch, keep Main on Aux 31/32 and add the downstream compressor/output router.

## Full Patch After Witchboard

Required downstream chain for the intended setup:

```text
Witchboard Main Aux 31/32
  -> Sidechain compressor / ducker in place
  -> small output router or mixer
  -> O1/O2
  -> Messor
```

Bypass channels skip the compressor:

```text
Witchboard Bypass O1/O2
  -> Messor
```

The compressor sidechain/key source is outside Witchboard. The kick source on Aux 21 is used as the bypassed audio channel and can also be used as the compressor key elsewhere in the patch.

## MIDI Controller Setup

Controller I use for this example: Michigan Synth Works XVI-M.

Reference image:

```text
M:\DistingNT\WitchboardPlugin\assets\midiController.png
```

All faders are standard 7-bit CC, not 14-bit.

My full controller setup is:

| XVI-M strips | Port | MIDI channels | Faders | Buttons |
|---:|---|---:|---|---|
| 1-16 | TRS 1 | 1-16 | CC9 | Toggle 4P, CC80 |

The `Witchboard Example` preset uses strips 1-8 for my four Witchboard channels.
Strips 9-16 follow the same pattern and can be used for future channels or manual mapping.

| XVI-M strip | Port | MIDI channel | Fader | Button |
|---:|---|---:|---|---|
| 1 | TRS 1 | 1 | CC9 -> Ch 1 Gain | CC80 -> Ch 1 Insert 1 |
| 2 | TRS 1 | 2 | CC9 -> Ch 1 Radiant mix | CC80 -> Ch 1 Insert 2 |
| 3 | TRS 1 | 3 | CC9 -> Ch 2 Gain | CC80 -> Ch 2 Insert 1 |
| 4 | TRS 1 | 4 | CC9 -> Ch 2 Radiant mix | CC80 -> Ch 2 Insert 2 |
| 5 | TRS 1 | 5 | CC9 -> Ch 3 Gain | CC80 -> Ch 3 Insert 1 |
| 6 | TRS 1 | 6 | CC9 -> Ch 3 Radiant mix | CC80 -> Ch 3 Insert 2 |
| 7 | TRS 1 | 7 | CC9 -> Ch 4 Gain | CC80 -> Ch 4 Insert 1 |
| 8 | TRS 1 | 8 | CC9 -> Ch 4 Radiant mix | CC80 -> Ch 4 Insert 2 |

Each button is `Toggle 4P`, CC80, with values:

| Button value | Witchboard state | Example route |
|---:|---:|---|
| 0 | 0 | Dry |
| 42 | 1 | Threetom MS22 |
| 85 | 2 | Pico MMF |
| 127 | 3 | Percall 4 |

Important preset detail: the native NT MIDI mappings for the CC80 buttons use `Min = 0` and `Max = 4`. This is intentional. It makes `0 / 42 / 85 / 127` land in four clean buckets, then Witchboard clamps to valid insert states `0..3`.

## Native MIDI Mapping Rule

Witchboard does not parse MIDI directly.

The plugin exposes parameters. The disting NT native MIDI Mapping system maps controller CCs to those parameters. Any parameter can be remapped by hand in the NT UI.

The plugin factory has:

```text
midiMessage = NULL
midiRealtime = NULL
midiSysEx = NULL
```

## Build And Install

Build from the plugin directory:

```sh
make NT_API_PATH=/tmp/distingNT_API-official
```

If the M: mount refuses to overwrite directly, compile to `/tmp/Witchboard.o` and copy it over:

```sh
arm-none-eabi-c++ -std=c++11 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -fno-rtti -fno-exceptions -Os -fPIC -Wall -I/tmp/distingNT_API-official/include -c -o /tmp/Witchboard.o src/WitchboardClean.cpp
cp -a /tmp/Witchboard.o plugins/Witchboard.o
cp -a /tmp/Witchboard.o /mnt/m/DistingNT/programs/plug-ins/Witchboard.o
```

After copying the plugin, rescan plugins or restart the disting NT.

## Validation

Run:

```sh
python3 tests/validate_preset.py
g++ -std=c++11 -I/tmp/distingNT_API-official/include tests/WitchboardCleanTest.cpp -o /tmp/WitchboardCleanTest
/tmp/WitchboardCleanTest
```

Expected output:

```text
PASS: Witchboard hardware preset maps 0/42/85/127 to Dry/MS22/MMF/Percall
PASS: clean Witchboard has isolated per-channel Insert 1, Radiant, and Insert 2 parameters.
```

## Current Known-Good Behaviour

- Direct controller MIDI works when the OXI is not collapsing/filtering channels incorrectly.
- Gain faders map independently per channel.
- Insert 1 and Insert 2 are independent per channel.
- Radiant mix is independent from gain and insert controls.
- The red button value 42 selects Route A / Threetom MS22.
- Route names are loaded from the preset.
- Main sources are silent unless Aux 31/32 is routed onward by the rest of the patch.
