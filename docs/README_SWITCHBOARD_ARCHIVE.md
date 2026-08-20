# Archived Switchboard README

Switchboard is a community C++ plugin for the Expert Sleepers disting NT.

It is a configurable serial insert router for patches that need MIDI-controlled route selection, external processor routing, two post-insert stereo FX sends, and main/bypass output paths.

Core idea:

```text
multi-channel audio routing
+ two serial insert selections
+ 2/3/4-state MIDI button control
```

The plugin is intended to be configurable rather than patch-specific:

- up to 12 mono or stereo channels
- user-assignable input and output buses
- two serial insert selections
- choices: None, Route A, Route B, or Route C
- independent Mono/Stereo widths for every Route send and return
- one-active-choice MIDI switching
- two post-insert stereo FX sends with independent per-channel levels
- bypass buses for audio that must skip an external ducker/compressor
- main buses for audio that should feed an external processor
- independent repeat protection for every channel

The user guide and complete parameter reference are in:

```text
GUIDE.md
```

The current personal hardware map and XVI-M defaults are in:

```text
WIRING.md
```

Current status: the C++ implementation builds against disting NT API v13 and
loads successfully on the disting NT. The Channel count has been respecified from
one to eight on the module. After optimization, the initial eight-Channel test
uses approximately 13% CPU. Full audio-route and MIDI testing is in progress.

`GUIDE.md` is the authoritative description of the current plugin and its complete
parameter layout. Files under `docs/` are retained as background material and may
describe earlier design ideas.

## Algorithm

One `Switchboard.o` provides exactly one algorithm named `Switchboard`.

Each Channel contains both `Insert 1` and `Insert 2`. Start on `Route Setup`, where
each external Route places its Output and Return controls together. Route buses
begin unassigned.

The `FX Sends` page keeps both stereo output assignments and every Channel's two
post-insert send levels together. Sends remain active for Channels using either
Main or Bypass.

Switchboard contains no compressor or sidechain processing. The compressor remains a separate disting NT algorithm.

```text
audio source
-> Insert 1
-> selected route and return
-> Insert 2
-> selected route and return
-> main/bypass output and two optional FX Sends
```

A channel's `Output path` selects Main or Bypass after both inserts. This can route
audio around a separate compressor; Switchboard never handles the compressor
key/sidechain.

Switchboard supports up to 12 mono or stereo channels. Each Insert maps button
states to `None`, `Route A`, `Route B`, or `Route C`. `None` passes audio onward
without using an external route.

## MIDI Control

Each Channel page contains its own Insert 1 and Insert 2 MIDI Channel and Base CC
assignments. The separate MIDI Profile page contains only the shared editable
four-state message profile.

Each state is defined by a CC offset and value:

```text
expected CC = channel MIDI base CC + state CC offset
expected value = state value, within Value tolerance
```

This supports same-CC/different-value buttons, different-CC/same-value buttons, and
mixed formats. Every Channel has separate MIDI Channel and Base CC parameters for
Insert 1 and Insert 2. MIDI channel `0` disables that button's built-in decoder.

## Build

Run `make` in this directory. The output is `plugins/Switchboard.o`. The default API path is the sibling directory `..\distingNT_API`.

Copy the object to `/programs/plug-ins/` on the disting NT MicroSD card and rescan plug-ins. Firmware 1.17 or newer is required for API v13.

## References

- Expert Sleepers disting NT API: https://github.com/expertsleepersltd/distingNT_API
- Expert Sleepers firmware and matching manuals: https://www.expert-sleepers.co.uk/distingNTfirmwareupdates.html
- Michigan Synth Works XVI-M: https://michigansynthworks.com/products/xvi-m
