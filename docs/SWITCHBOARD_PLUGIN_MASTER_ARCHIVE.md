# Switchboard Plugin Master Doc

> Historical planning document. It is retained for design background but does not
> describe the current implementation. See `GUIDE.md` for the authoritative user
> guide and complete current parameter list, and `README.md` for build status.

Switchboard is a configurable C++ audio routing mixer / serial patchbay plugin for the Expert Sleepers disting NT.

It is designed for patches where audio sources need to be routed between dry paths, hardware loops, FX sends, and compressor/ducker paths under direct MIDI control.

Project directory:

```text
M:\DistingNT\SwitchboardPlugin
```

## Core Idea

Switchboard combines three things in one plugin:

```text
multi-source audio routing
+ serial route stages
+ multi-state MIDI button control
```

It is not mainly a gain mixer. Source levels can be handled at the instruments, samplers, external gear, or controller faders.

Switchboard focuses on routing decisions.

## Main Features

- configurable source count, up to the disting NT's 12 input buses
- mono or stereo mode per source
- configurable input bus assignment per source
- configurable output buses for main, direct, loop, and FX paths
- two serial routing stages per source or route group
- route states such as Main, Loop A, Loop B, Loop C, with user-definable labels
- stereo FX send path
- direct output path for sources that should bypass a compressor/ducker
- ducked output path for sources that should feed a compressor/ducker
- optional internal handling of loop/FX returns
- MIDI control from 2-state, 3-state, or 4-state buttons
- one-active-route switching per routing stage
- feedback safety rules for returns and sends
- defaults that work for one user's patch, but configurable enough for other controllers and systems

## What Problem It Solves

Standard mixer algorithms are good at mixing, but less good at discrete route logic.

Switchboard is for cases where a MIDI button should select exactly one route from a small set:

```text
button state 1 -> Main
button state 2 -> Loop A
button state 3 -> Loop B
button state 4 -> Loop C
```

When one state is selected, the other route states for that stage are closed.

This makes multi-state MIDI buttons usable as patchbay selectors instead of simple on/off controls.

## MIDI Button Model

Switchboard should support generic MIDI controllers, not only one device.

A useful default model is:

```text
same CC number
same state values
MIDI channel selects the source or route group
```

Example:

```text
CC 80 value 0   -> Main
CC 80 value 42  -> Loop A
CC 80 value 85  -> Loop B
CC 80 value 127 -> Loop C
```

In that setup, MIDI channel 1 can control source 1, MIDI channel 2 can control source 2, and so on.

The plugin should also allow custom mappings:

- different CC per source
- different MIDI channel per source
- custom state values
- 2-state, 3-state, or 4-state operation
- configurable route order
- optional disabled route states

Route changes are discrete state changes, not fader movements.

## Serial Routing

Switchboard should support two routing stages.

Conceptually:

```text
source
  -> stage 1 route
  -> stage 1 result/return
  -> stage 2 route
  -> stage 2 result/return
  -> final output path
```

This allows routing again after an earlier route.

Example uses:

```text
source -> Loop A -> Main
source -> Loop A -> FX Send -> Ducked Out
source -> Main   -> FX Send -> Ducked Out
source -> Loop B -> Direct Out
```

Important limitation: if multiple sources are sent to the same external hardware loop at the same time, the shared loop return is mixed audio. The plugin cannot separate that return back into individual source identities.

## Audio Paths

Switchboard should expose configurable audio paths for:

```text
Main / ducked output L/R
Direct output L/R
FX send L/R
FX return L/R, if handled internally
Loop A send/return
Loop B send/return
Loop C send/return
```

The names Main, Loop A, Loop B, and Loop C are default labels only. Users should be able to rename route slots to match their patch.

## Ducker / Compressor Integration

Switchboard does not replace a compressor algorithm.

It simply provides two useful output paths:

```text
Ducked output -> compressor input
Direct output -> main outs, bypassing compressor
```

The compressor sidechain/key remains patched and configured in the compressor algorithm.

This lets a source such as a kick bypass the ducker audio path while still being used as the compressor key elsewhere in the patch.

## Mono and Stereo Sources

Each source should be configurable as mono or stereo.

Mono source behaviour:

```text
one input bus
copied to L/R for stereo destinations
sent as mono to mono hardware loops
```

Stereo source behaviour:

```text
two input buses
preserved as L/R for stereo destinations and stereo FX sends
summed or left/right-selected for mono hardware loops, depending on configuration
```

Stereo FX sends should preserve stereo where possible.

## Source Gain

Switchboard should not require per-channel gain for the main use case.

Expected gain locations:

- external instrument output level
- source algorithm level
- MIDI controller faders mapped to source parameters
- downstream effects or output stage

Optional trim gains may be useful later, but the core plugin is a routing tool first.

## Channel Model

A source/channel should expose parameters like:

```text
Enable
Label/name
Source mode: Mono / Stereo
Input L / mono bus
Input R bus
Role
Stage 1 route state
Stage 2 route state
FX send state/enable
MIDI channel
MIDI CC
MIDI state values
```

Possible roles:

```text
Normal   -> routed to ducked/main processing path
Direct   -> routed to direct output, bypassing ducker
Return   -> return handling with feedback safety
Disabled
```

The exact parameter layout can change during implementation, but the important point is that source behaviour is configurable.

## Feedback Safety

The plugin should make unsafe routings hard to create.

Useful safety rules:

- a loop return should not feed its own loop send
- an FX return should not feed the same FX send by default
- direct sources should not accidentally enter the ducked path unless configured
- disabled route states should produce silence for that route
- invalid bus assignments should fail safely

Feedback behaviour should be configurable, but safe defaults matter.

## Disting NT API Notes

Relevant API facts from the official Expert Sleepers SDK:

- API repository: `https://github.com/expertsleepersltd/distingNT_API`
- local SDK path: `M:\DistingNT\distingNT_API`
- plugin API header: `include/distingnt/api.h`
- inspected API version: `kNT_apiVersion13`
- the disting NT exposes 12 input buses and 8 output buses
- aux buses are also available for internal routing
- plugins process audio in `step()` using contiguous bus frame buffers
- plugins can read from and write to arbitrary buses
- `midiMessage()` can receive MIDI CC messages
- dynamic parameter specs/pages are supported
- `NT_setParameterFromUi()` can update plugin parameters from MIDI/UI logic

Useful official examples:

```text
gain.cpp
gainMultichannel.cpp
midiLFO.cpp
gainCustomUI.cpp
parameterPageGroupExample.cpp
```

`gainMultichannel.cpp` is especially relevant because it demonstrates dynamic channel-style parameter pages.

## Built-In Mixer Reference

The built-in Disting NT mixer algorithms are useful references, but their source code is not available.

Manual behaviour observed:

- Mixer Mono supports up to 12 mono channels
- Mixer Stereo supports up to 12 mono/stereo channels
- built-in mixers support aux sends, mute/solo, gain, pan, and channel names
- built-in mixer MIDI mapping is not designed for this plugin's multi-state one-active-route switching model

Switchboard should borrow the useful shape of a configurable mixer, but specialize it for routing.

## CPU and Resource Direction

The intended implementation should be lightweight:

- no convolution
- no FFT
- no resampling
- no modulation engine required
- mostly bus reads, bus clears, copies, sums, and simple gain/trim if added
- route decisions handled as state variables, not heavy DSP

This should be realistic as a clean C++ disting NT plugin if bus routing and parameter layout are kept disciplined.

## Implementation Target

The first serious `.o` should aim at the real plugin shape, not a tiny demo.

Minimum useful target:

- configurable source count
- mono/stereo source mode
- configurable source input buses
- configurable ducked output buses
- configurable direct output buses
- at least two configurable loop slots
- stereo FX send path
- two route stages
- 2/3/4-state MIDI button decoding
- one-active-route switching
- basic feedback safety
- clear parameter names/pages

The design should remain generic enough for community use while still being able to cover demanding personal patches through configuration.

## Example Use Case

One example use is replacing a patch that currently needs separate main and return mixer algorithms.

In that patch:

- the main mixer gathers instruments and sends
- the return mixer brings external loops back to the main outputs
- some sources feed a compressor/ducker path
- some sources need to bypass the ducker
- a stereo FX send is needed
- multi-state MIDI buttons choose routes

Switchboard should be configurable enough to cover that setup without being limited to it.

Reference screenshot:

```text
assets/current-patch-radiant-mixer-target.png
```

## Related Reference Plugin

A useful community reference is:

```text
https://github.com/kuttor/Disting-NT-Plugin--SwitchingMixer
```

That project proves a Disting NT C++ plugin can perform configurable mixer-style routing and MIDI-controlled switching.

Switchboard differs by focusing on:

- 2-stage serial routing
- explicit multi-state button decoding
- mono/stereo source handling
- ducked/direct output paths
- generic route labels
- community-configurable patchbay behaviour

## References

Official sources:

```text
https://github.com/expertsleepersltd/distingNT_API
https://www.expert-sleepers.co.uk/distingNT.html
https://www.expert-sleepers.co.uk/distingNTusermanual.html
```

Local docs/resources:

```text
M:\DistingNT\DOCS
M:\DistingNT\distingNT_API
M:\DistingNT\SwitchboardPlugin\assets\current-patch-radiant-mixer-target.png
```
