# Disting NT Quick I/O + Mixer Map

Quick live reference for the routing-matrix plan.

## 4-state source button

Per source:

```text
1. Dry
2. Pico MMF
3. MS-22
4. Percall ch4
```

Radiant is **separate**. It is a send on the main/source mixer and also on the Return Mixer.

---

## Physical loops

```text
OUT4 -> Percall ch4 -> IN5
OUT5 -> Pico MMF    -> IN11
OUT6 -> MS-22       -> IN12
OUT7/8 -> Radiant   -> IN7/8
```

---

## Disting Inputs

| Input | Signal |
|---:|---|
| IN1 | Radio after Percall ch1 |
| IN2 | Chord Organ after Percall ch2 |
| IN3 | Pico/Pony bass path / Pico side after Percall ch3 |
| IN4 | Pony/Pico bass path / Pony side |
| IN5 | Percall ch4 return |
| IN6 | Free |
| IN7 | Radiant return L |
| IN8 | Radiant return R |
| IN9 | Poly Resonator gate / Turing pulse |
| IN10 | Poly Resonator pitch / Turing CV |
| IN11 | Pico MMF return |
| IN12 | MS-22 return |

---

## Disting Outputs

| Output | Destination |
|---:|---|
| OUT1 | Final L to Messor / Output Bus |
| OUT2 | Final R to Messor / Output Bus |
| OUT3 | Free |
| OUT4 | Percall ch4 send |
| OUT5 | Pico MMF send |
| OUT6 | MS-22 send |
| OUT7 | Radiant input L |
| OUT8 | Radiant input R |

---

## Internal Voice Buses

| Bus | Source |
|---:|---|
| Aux21 | Kick |
| Aux22 | Snare |
| Aux23 | Hat |
| Aux24 | Sliced / Perc |
| Aux25/26 | Poly Resonator stereo |
| Aux27 | NT-303 |
| Aux28 | Poly Lofi |

---

## Source Mixer / Routing Matrix

Slot 8 is the main source/routing mixer.

Matrix paths:

| State | Destination |
|---|---|
| Dry | Main mix |
| Pico MMF | OUT5 -> Pico MMF -> IN11 |
| MS-22 | OUT6 -> MS-22 -> IN12 |
| Percall4 | OUT4 -> Percall ch4 -> IN5 |

Radiant send is separate:

| Send | Destination |
|---|---|
| Source Mixer Radiant Send | OUT7/8 -> Radiant -> IN7/8 |

---

## Return Mixer

Slot 10: **RETURN MIXER**.

| Ch | Source | Role |
|---:|---|---|
| 1 | Aux21 | Kick add-back |
| 2 | IN5 | Percall ch4 return |
| 3 | IN11 | Pico MMF return |
| 4 | IN12 | MS-22 return |

Return Mixer outputs:

| Path | Destination |
|---|---|
| Main | OUT1/2 Add |
| Send 1 | OUT7/8 Add — return-to-Radiant |

Do **not** put Radiant return IN7/8 into the Return Mixer.

---

## OXI / Turing Reminder

Current:

```text
OXI T1 = Radio
OXI T2 = Chord Organ
OXI T3 = Pico/Pony bass
OXI T4 = Disting drums multitrack
Turing = Poly Resonator via IN9/IN10
```

Planned:

```text
OXI T1 = Radio vocal glitching + Chord Organ multitrack
OXI T2 = internal Disting voice, likely NT-303
OXI T3 = Pico/Pony bass
OXI T4 = drums
```

---

## Safety

Keep off unless deliberately doing feedback:

- Percall ch4 return back to Percall ch4 send
- Pico MMF return back to Pico MMF send
- MS-22 return back to MS-22 send
- Radiant return back to Radiant send

Use **Add** mode for mixer/routing outputs.
