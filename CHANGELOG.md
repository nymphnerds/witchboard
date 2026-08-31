# Changelog

## Unreleased

- Changed `FX Send 1 mix` and `FX Send 2 mix` to an overlap-style dry/wet
  control.
- Dry now stays at full level from `0..50%` while wet fades in.
- Wet now stays at full level from `50..100%` while dry fades out.
- This replaces the old normalized cubic crossfade, which could feel like a
  centre dip with real FX returns.
- Updated the README to explain why Witchboard exists as a performance routing
  matrix, and to document the overlap FX-send behaviour.

## 1.1.1

- Cleaned up per-instance UI page storage so differently sized Witchboard
  instances do not share mutable page layout data.
- Hardened the host test memory helper for zero-byte DRAM allocations.
- Added regression coverage for simultaneous 4-channel and 12-channel page
  layouts.

## 1.1.0

- Expanded Witchboard from fixed insert routes to five generic assignable
  routes.
- Added per-channel Insert 1 and Insert 2 slot assignments.
- Preserved optional repeat protection so one channel can deliberately use the
  same route in both insert stages when protection is Off.
- Made SRAM allocation scale with the configured channel count instead of
  reserving all 12 channels for every instance.
- Reused static channel parameter labels with the official NT UI-prefix callback.
- Removed the old output-mode parameters; Witchboard outputs now always Add.
- Reduced the 12-channel parameter count to 241 so 12-channel respec works on
  the NT.
- Renamed the compiled `Radiant mix` parameter to generic `FX Send 1 mix`.
- Kept insert button behaviour as four states: Dry / Slot 1 / Slot 2 / Slot 3,
  using NT MIDI Mapping `Min = 0`, `Max = 4`.
- Fixed native/MIDI-mapped insert route changes lagging behind until another
  channel parameter, such as Gain, was nudged.
- Added host audio tests for routing and duplicate-return prevention.
