import json
from pathlib import Path


preset_path = Path(__file__).parent.parent / "presets" / "Witchboard Example.json"
with preset_path.open(encoding="utf-8") as handle:
    preset = json.load(handle)

slot = preset["slots"][0]
parameters = slot["parameters"]

assert slot["guid"] == "WtC1"
assert slot["specs"] == [4, 0, 0]
assert slot["name"] == "Witchboard"
assert preset["name"] == "Witchboard Example"
assert len(parameters) == 85
assert set(slot) == {"guid", "specs", "name", "witchboardNames", "parameters", "ui"}
assert slot["witchboardNames"] == {
    "routes": ["Threetom MS22", "Pico MMF", "Percall 4"],
    "fx": ["Radiant", "iPad"],
}

# One NT common parameter precedes Witchboard's 84 plugin parameters.
assert parameters[0] == 0
assert parameters[1] == 2
assert parameters[2:23] == [
    15, 0, 12, 0, 0, 0, 0,  # Route A / Threetom MS22: O3 -> I12
    16, 0, 11, 0, 0, 0, 0,  # Route B / Pico MMF: O4 -> I11
    17, 0, 4, 0, 0, 0, 0,   # Route C / Percall 4: O5 -> I4
]
assert parameters[23:29] == [31, 32, 0, 13, 14, 0]
assert parameters[29:37] == [19, 20, 1, 0, 7, 8, 1, 0]
assert parameters[37:45] == [0, 0, 1, 0, 0, 0, 1, 0]

expected_inputs = [1, 2, 3, 21]
expected_paths = [0, 0, 0, 1]
for channel in range(4):
    base = 45 + channel * 10
    assert parameters[base] == 1
    assert parameters[base + 1] == expected_inputs[channel]
    assert parameters[base + 2] == 0
    assert parameters[base + 3]["v"] == 0
    assert parameters[base + 4]["v"] == 0
    assert parameters[base + 5]["v"] == 0
    assert parameters[base + 6]["v"] == 0
    assert parameters[base + 7] == expected_paths[channel]
    assert parameters[base + 8] == 1
    assert parameters[base + 9] == 0

expected_mappings = {
    48: (1, 9, -60, 0, "Ch 1 Gain"),
    49: (1, 80, 0, 4, "Ch 1 Insert 1"),
    50: (2, 9, 0, 100, "Ch 1 Radiant mix"),
    51: (2, 80, 0, 4, "Ch 1 Insert 2"),
    58: (3, 9, -60, 0, "Ch 2 Gain"),
    59: (3, 80, 0, 4, "Ch 2 Insert 1"),
    60: (4, 9, 0, 100, "Ch 2 Radiant mix"),
    61: (4, 80, 0, 4, "Ch 2 Insert 2"),
    68: (5, 9, -60, 0, "Ch 3 Gain"),
    69: (5, 80, 0, 4, "Ch 3 Insert 1"),
    70: (6, 9, 0, 100, "Ch 3 Radiant mix"),
    71: (6, 80, 0, 4, "Ch 3 Insert 2"),
    78: (7, 9, -60, 0, "Ch 4 Gain"),
    79: (7, 80, 0, 4, "Ch 4 Insert 1"),
    80: (8, 9, 0, 100, "Ch 4 Radiant mix"),
    81: (8, 80, 0, 4, "Ch 4 Insert 2"),
}

actual_mappings = {}
for index, parameter in enumerate(parameters):
    if not isinstance(parameter, dict):
        continue
    midi = parameter["midi"]
    assert set(midi) == {
        "channel", "type", "cc", "enabled", "symmetric", "relative", "min", "max"
    }
    assert midi["type"] == 0
    assert midi["enabled"] == 1
    assert midi["symmetric"] == 0
    assert midi["relative"] == 0
    actual_mappings[index] = (
        midi["channel"], midi["cc"], midi["min"], midi["max"]
    )

assert actual_mappings == {
    index: values[:4] for index, values in expected_mappings.items()
}
assert len({(mapping[0], mapping[1]) for mapping in actual_mappings.values()}) == 16
assert all(1 <= mapping[0] <= 8 for mapping in actual_mappings.values())

def native_floor_bucket(cc_value, minimum, maximum):
    return minimum + (cc_value * (maximum - minimum)) // 127

assert [
    min(native_floor_bucket(value, 0, 4), 3)
    for value in [0, 42, 85, 127]
] == [0, 1, 2, 3]

def targets(physical_channel, cc):
    return {
        index
        for index, mapping in actual_mappings.items()
        if mapping[0] == physical_channel and mapping[1] == cc
    }

assert targets(1, 9) == {48}
assert targets(1, 80) == {49}
assert targets(2, 9) == {50}
assert targets(2, 80) == {51}
assert targets(3, 9) == {58}
assert targets(3, 80) == {59}
assert targets(4, 9) == {60}
assert targets(4, 80) == {61}
assert targets(5, 9) == {68}
assert targets(5, 80) == {69}
assert targets(6, 9) == {70}
assert targets(6, 80) == {71}
assert targets(7, 9) == {78}
assert targets(7, 80) == {79}
assert targets(8, 9) == {80}
assert targets(8, 80) == {81}

print("PASS: Witchboard hardware preset maps 0/42/85/127 to Dry/MS22/MMF/Percall")
