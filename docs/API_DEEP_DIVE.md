# Switchboard Mixer API Deep Dive

Research notes from the Expert Sleepers disting NT manual, official `distingNT_API`, local setup docs, and related plugin examples.

## Bottom Line

A Switchboard-style mixer/routing plugin is feasible in C++.

The DSP work is simple:

- read source buses
- apply routing state
- add samples to output/send buses
- decode MIDI CC messages
- optionally smooth route changes

The hard parts are not CPU. They are routing design, feedback safety, parameter/UI scale, and the reality of shared external hardware returns.

## API Capabilities That Matter

From `include/distingnt/api.h`:

```text
12 input buses
8 output buses
44 aux buses
64 total buses
```

The plugin audio callback receives one contiguous `busFrames` block containing every bus. A plugin can read from any bus and add/replace into any bus by pointer arithmetic.

Relevant callbacks:

```text
calculateRequirements()
construct()
parameterChanged()
step()
midiMessage()
draw()
serialise() / deserialise()
parameterUiPrefix()
parameterString()
```

Useful API points:

- `step()` is where audio routing/mixing happens.
- `midiMessage()` receives MIDI CC/note/etc messages and can update internal route state or plugin parameters.
- `NT_setParameterFromUi()` can update plugin parameters from MIDI handling.
- `parameterUiPrefix()` is useful for channel-style pages such as `1:Input`, `2:Input`, etc.
- `parameterString()` can provide display strings for route names or custom value displays.
- Parameters can use audio input/output units, enums, dB units, percents, etc.
- Specs can configure algorithm size at creation time, e.g. number of channels/routes.

## Official Examples Inspected

### `gain.cpp`

Basic bus read/write pattern:

```text
input bus -> gain -> output bus
```

Shows Add vs Replace output mode and custom drawing.

### `gainMultichannel.cpp`

Most relevant official pattern for Switchboard.

Shows:

- max channel count
- specification-time channel count
- dynamic parameter arrays
- dynamic parameter pages
- per-channel cached gain values
- `parameterUiPrefix()` for channel prefixes

This is the best official pattern for a multi-channel plugin.

### `midiLFO.cpp`

Shows MIDI send functions, but the callback field is also what Switchboard would use for incoming CC handling.

### `gainCustomUI.cpp`

Shows:

- custom UI callbacks
- `NT_setParameterFromUi()`
- graying out params
- custom JSON serialisation/deserialisation

Useful if Switchboard later stores richer names/config beyond normal params.

## Built-In Mixer Algorithms

From manual v1.17:

### Mixer Mono, guid `mix1`

- 1-12 channels
- 0-4 aux sends
- mono input per channel
- mono output plus optional duplicate output
- per-channel gain/mute/solo/name
- per-send destination, pre/post, output mode
- per-channel send gain

### Mixer Stereo, guid `mix2`

- 1-12 channels
- 0-4 aux sends
- mono or stereo inputs
- stereo main output
- sends can be mono or stereo
- per-channel gain, pan, mute, solo/name
- per-channel send gain

The built-in mixer is powerful but generic. It cannot express multi-state button one-hot routing without many mapped parameters.

## Current Patch Context

Current MySetup1:

```text
RADIANT MIXER -> SIDECHAIN COMP -> main outputs
Kick -> sidechain key
Kick -> main outs bypassing ducker
```

Why kick is special:

- It should key the compressor.
- It should not be ducked by the compressor it triggers.
- This is why it sits outside the main mixer path.

MySetup2 idea:

```text
Dry/main
Pico loop      OUT5 -> IN11
MS-22 loop     OUT6 -> IN12
Percall loop   OUT4 -> IN5
FX send        OUT7/8 -> IN7/8
```

Use generic names in the plugin:

```text
Main
Loop A
Loop B
Loop C
FX Send
Ducked output
Direct output
```

## Proposed Switchboard Mixer Shape

Potential mixer replacement:

- 12 source channels
- configurable mono/stereo source bus per channel
- channel role: normal / direct / disabled / return maybe
- 2 routing stages per relevant channel or route group
- each stage controlled by a 2/3/4-state MIDI button
- route states map to Main or hardware loops
- configurable loop send and return buses
- configurable FX send bus
- ducked mix output for compressor input
- direct main output for sources that bypass the ducker

Switchboard should not handle the compressor sidechain key directly. The compressor algorithm already owns its key input. Switchboard only needs to provide audio paths that can be patched into or around the compressor.

For kick-like channels:

```text
channel -> direct main output
```

Outside Switchboard:

```text
kick bus -> compressor sidechain input
```

For normal channels:

```text
channel -> routing stages -> ducked output -> compressor input
```

## MIDI Model

For a controller like XVI-M, the simplest mapping is:

```text
same CC for route buttons
same state values everywhere
MIDI channel identifies strip/button/lane
```

Example:

```text
CC 80 value 0   = Main
CC 80 value 42  = Loop A
CC 80 value 85  = Loop B
CC 80 value 127 = Loop C
```

The plugin should still allow custom values.

Important: route changes are discrete state changes, not continuous fader movements.

## CPU and Memory Feasibility

This should be low CPU if implemented plainly.

Expected per-block work:

```text
channels * frames * a few adds/multiplies
```

No need for:

- heap allocation in audio
- large buffers
- FFT
- sample playback
- file I/O

The related community `SwitchingMixer` object is tiny:

```text
text+data ~3250 bytes
```

The scratch Switchboard prototype is also tiny:

```text
text+data ~3907 bytes
```

A fuller 12-channel Switchboard Mixer will be larger, mostly because of parameter definitions and state arrays, but the DSP should still be light.

## Major Gotchas

### 1. External Returns Are Shared Audio

A hardware loop is not a private per-channel object.

If channels 1 and 2 both send to Loop A:

```text
ch1 + ch2 -> OUT5 -> hardware -> IN11
```

then `IN11` is already the combined processed return.

The plugin cannot split that return back into separate channel identities.

So serial routing after a hardware loop is naturally **bus/group serial**, not truly isolated per-source serial, unless only one source uses that loop at a time.

### 2. Serial Hardware Routing Has Latency

A physical loop is:

```text
output DAC -> hardware -> input ADC -> next audio block
```

The return will not be available as an instantaneous same-sample result of the send.

That is normal for hardware effects, but the design must treat returns as live input buses, not immediate function calls.

### 3. Feedback Safety

Danger paths:

```text
Loop A return -> Loop A send
Loop B return -> Loop B send
FX return -> FX send
```

The plugin should probably have safety defaults:

- returns do not feed their own sends by default
- FX return cannot feed FX send unless explicitly enabled
- optional safe mode to block self-feedback

### 4. Parameter Count Can Explode

A full 12-channel mixer with 2 stages, MIDI config, sends, returns, names, and modes could become hard to use on the NT screen.

Use specs/pages carefully:

- global pages for loops/output/MIDI values
- per-channel pages for source/role/routes
- `parameterUiPrefix()` for channel prefixes
- avoid per-channel duplication of global MIDI values where possible

### 5. Naming Is Limited

Normal parameters can display enum strings and custom parameter strings. True free-text route labels may be awkward from the NT UI.

Best v1 approach:

- generic route names: Main, Loop A, Loop B, Loop C, FX Send
- optional name presets later
- custom JSON labels later if needed

## Recommended Development Path

### Phase 1: Route State Engine

No full mixer yet.

- 4-state CC decoding
- route enums
- one-active-route switching
- route state display
- test with no audio or simple bus output

### Phase 2: Simple Audio Router

- few source lanes
- route to Main/Loop A/B/C sends
- read loop returns as independent return buses
- direct and ducked outputs

### Phase 3: Mixer Replacement Candidate

- 12 source channels
- channel roles
- FX send
- direct output bypass path
- return handling
- safety options

### Phase 4: Polish

- route smoothing/click reduction
- custom display
- better labels
- presets/examples

## Assessment

Yes, C++ can handle this cleanly.

The plugin should be low-resource if it is written as a fixed-array routing mixer with no dynamic allocation in `step()`.

The key design decision is not CPU. It is whether Switchboard owns loop returns and serial routing, or whether it only replaces the source mixer while a separate return mixer continues to own returns.

For replacing both Radiant Mixer and Return Mixer, the plugin should explicitly model:

```text
source channels
hardware loop sends
hardware loop returns
FX send
ducked output
direct output
feedback safety
```
