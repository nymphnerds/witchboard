# Assets

Reference images for my Witchboard example patch.

These assets document how I use Witchboard in my current hardware setup. They
are examples, not requirements. Witchboard itself is a generic NT plugin: the
inputs, outputs, route names, and MIDI mappings can be changed by hand for any
patch.

| File | Purpose |
|---|---|
| `babyjaws.jpg` | Photo of my hardware setup for this preset. |
| `midiController.png` | My Michigan Synth Works XVI-M MIDI controller setup for this preset. |
| `current-patch-radiant-mixer-target.png` | My Disting NT patch context around the mixer/router section Witchboard replaces or augments. |

`midiController.png` shows my XVI-M setup:

- strips 1-16 on `TRS 1`
- MIDI channels 1-16
- faders sending `CC9`
- switches set to `Toggle 4P`
- switches sending `CC80`
- switch values `0 / 42 / 85 / 127`

The `Witchboard Example` preset uses strips 1-8 for my four Witchboard channels.
Strips 9-16 follow the same pattern and are ready for future channels or manual
mapping.
