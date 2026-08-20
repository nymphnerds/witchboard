# Switchboard Mixer Replacement Concept

Switchboard is intended as a purpose-built C++ routing mixer for the Expert Sleepers disting NT.

The goal is to replace the current patch's generic mixer stack, starting with `#9 RADIANT MIXER`, and potentially absorbing the useful parts of `#11 RETURN MIXER` later.

## Why Replace The Current Mixers

The current patch relies on built-in mixer algorithms to combine sources, feed FX, handle returns, and manage ducking/bypass paths.

Problems:

- the main mixer is at the 12-channel limit
- adding more mixers costs CPU and algorithm slots
- generic mixer MIDI mapping cannot handle 4-state route buttons cleanly
- routing logic is spread across multiple algorithms
- feedback safety depends on many separate mixer send settings

Switchboard should combine the routing-specific work into one focused plugin.

## What Switchboard Should Do

Switchboard should act as a low-resource routing mixer / serial patchbay.

Core responsibilities:

- collect source channels
- route sources through selectable hardware loops
- support multi-state MIDI button switching
- provide a stereo FX Send
- provide a ducked output for the compressor path
- provide a direct main output for sources that bypass the ducker
- optionally handle hardware loop returns currently handled by the Return Mixer

Switchboard should not duplicate a full traditional mixer unless needed.

Source gains are expected to be controlled at the source algorithms or external instruments.

## Current Patch Replacement Target

Current patch structure:

```text
sources + returns -> #9 RADIANT MIXER -> #10 SIDECHAIN COMP -> main outs
kick -> main outs bypassing ducker
returns / kick add-back -> #11 RETURN MIXER
```

Target idea:

```text
sources -> Switchboard -> ducked output -> sidechain comp -> main outs
kick/direct sources -> Switchboard direct output -> main outs
hardware loop returns -> Switchboard or Return Mixer, depending on final design
FX Send -> external FX -> FX return
```

The compressor/ducker algorithm remains separate.

The compressor sidechain/key input remains handled by the compressor algorithm, not Switchboard.

## Outputs Switchboard Should Expose

At minimum:

```text
Ducked Out L
Ducked Out R
Direct Out L
Direct Out R
FX Send L
FX Send R
```

For hardware loops:

```text
Loop A Send
Loop A Return
Loop B Send
Loop B Return
Loop C Send
Loop C Return
```

These should be bus-configurable, not hardcoded.

## Channel Model

A likely channel model:

```text
Channel enable
Source mode: Mono / Stereo
Input L / mono
Input R
Channel role: Normal / Direct / Return / Disabled
Route Stage 1 state
Route Stage 2 state
FX Send enable/amount or state
```

Possible channel roles:

```text
Normal  -> routed to ducked output path
Direct  -> routed to direct main output, bypassing ducker
Return  -> loop/FX return handling, with feedback safety
Disabled
```

For the current patch:

```text
Kick = Direct
Most voices = Normal
Hardware returns = Return or handled externally
```

## Routing Model

Switchboard routes through configurable route slots.

Generic route slots:

```text
Main
Loop A
Loop B
Loop C
```

Current patch mapping:

```text
Main   = dry/main path
Loop A = Pico MMF loop
Loop B = MS-22 loop
Loop C = Percall ch4 loop
FX Send = Radiant-style stereo FX send
```

Hardware example:

```text
Loop A = OUT5 -> Pico MMF -> IN11
Loop B = OUT6 -> MS-22    -> IN12
Loop C = OUT4 -> Percall  -> IN5
FX Send = OUT7/8 -> FX -> IN7/8
```

## MIDI Button Switching

Switchboard should support 2, 3, or 4-state MIDI buttons.

Each state maps to a route state.

Example:

```text
CC 80 value 0   = Main
CC 80 value 42  = Loop A
CC 80 value 85  = Loop B
CC 80 value 127 = Loop C
```

When a button state is received:

```text
selected route opens
other routes for that stage close
```

This is the core feature missing from normal Disting MIDI mapping.

## Mono / Stereo Rules

Switchboard must support mono and stereo sources.

Rules:

- external hardware voices are often mono
- Disting instruments may be stereo
- Poly Resonator is stereo
- FX Send is stereo
- mono sources copied to L/R for stereo destinations
- stereo sources preserve L/R for stereo destinations
- stereo sources summed to mono for mono hardware loops

## Feedback Safety

Feedback is musically useful but dangerous.

Safe defaults should prevent:

```text
Loop A return -> Loop A send
Loop B return -> Loop B send
Loop C return -> Loop C send
FX return -> FX send
```

Optional advanced mode can allow deliberate feedback later.

## Major Design Caveat

External loop returns are shared buses.

If multiple sources feed the same hardware loop, the return is a combined signal:

```text
source 1 + source 2 -> Loop A send -> hardware -> Loop A return
```

Switchboard cannot separate that return back into individual sources.

This means serial routing after a hardware loop is bus/group serial, not perfectly isolated per-source serial, unless only one source uses that loop at a time.

## Suggested Development Path

1. Build the MIDI route-state engine.
2. Add simple audio routing to Main / Loop A / Loop B / Loop C.
3. Add ducked and direct outputs.
4. Add FX Send.
5. Add return handling and feedback safety.
6. Decide whether it fully replaces Return Mixer.

## Success Criteria

Switchboard is successful if it:

- reduces the need for multiple built-in mixer algorithms
- avoids the 12-channel mixer bottleneck where possible
- decodes multi-state MIDI buttons directly
- switches routes cleanly with one-active-route logic
- supports mono and stereo sources correctly
- provides clean ducked and direct output paths
- keeps FX send and hardware loop routing understandable
