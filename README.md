# CouchDoom

Four instances of Doom running side by side — as if on four networked
computers — fake-networked into a local **split-screen deathmatch**. Built as a
JUCE standalone app on top of the per-instance `Gin_Doom` engine.

## How it works

Each of the 4 players is a completely independent Doom simulation (its own
`data_t`), running on its own thread — literally "four computers in one
process". Instead of real sockets, a local **arbiter** exchanges input every
tic so all four simulations stay in perfect lockstep. This is exactly Doom's
own peer-to-peer deathmatch model, with the transport replaced by a function
call.

### The tic exchange (the core mechanic)

Doom advances in fixed 35 Hz *tics*. Each tic, every instance produces one
local `ticcmd_t` (its player's input). For a synchronized deathmatch, all four
instances must run the identical set of four commands each tic.

- Each instance builds its own local command: `G_BuildTiccmd` →
  `data->ticdata[maketic].cmds[localplayer]` (`d_loop.c`).
- The arbiter reads all four local commands, assembles a
  `ticcmd_t cmds[NET_MAXPLAYERS]` + `players_mask`, and calls
  **`D_ReceiveTic(data, cmds, mask)`** on *every* instance — the network
  injection point. Each instance keeps its own slot and takes the other three
  from the arbiter.
- `recvtic` / `maketic` gate advancement (`GetLowTic`/`TryRunTics`), so the
  arbiter holds everyone to the slowest instance — true lockstep.

`ticdata` was moved into `data_t` (Gin_Doom `101fd24`) so the four threads no
longer share one global tic buffer — this made CouchDoom possible.

### Per-instance deathmatch setup

For each instance *i* (0..3), before `D_DoomMain`:
`netgame = true`, `deathmatch = 1` (or 2), `playeringame[0..3] = true`,
`localplayer = consoleplayer = displayplayer = i`, and (process-global, set
once) `net_client_connected = true`. The map needs ≥4 deathmatch starts.

### Input — controllers

4 gamepads via `gin::GameControllerManager`. Each controller maps to one
instance's input. Cleanest path: write the pad state into that instance's
per-instance input (`data->joyxmove/joyymove/joybuttons`, or post
`ev_joystick`/key events via `Doom::addEvent`) so `G_BuildTiccmd` picks it up
as that player's local command.

### Audio

Each instance has its own `DoomAudioEngine::processBlock`. CouchDoom's
`SoundEngine` owns the `juce::AudioDeviceManager` and, in its audio callback,
pulls and **sums the four engines** into one output.
- **Instance 0**: music + SFX.
- **Instances 1–3**: SFX only (started with `-nomusic`).

### Video

4 framebuffers (`Doom::getScreen()`, 640×400 each) drawn into a 2×2 grid.

## Layout (mirrors the Shunt project)

```
CouchDoom/
├── CMakeLists.txt          # juce_add_gui_app; gin + gin_doom modules
├── CMakePresets.json       # toolchains from the gin submodule
├── VERSION, tag.sh, Changelist.txt
├── Assets/DOOM1.WAD        # shareware IWAD, baked into BinaryData
├── modules/                # submodules: juce, gin, Gin_Doom
└── Source/
    ├── Main.cpp            # JUCEApplication bootstrap
    ├── MainComponent.*     # 60 Hz driver; owns the subsystems below
    ├── game/               # DoomHost (4 instances) + NetArbiter (tic exchange)
    ├── audio/              # SoundEngine (device manager + 4-engine mixer)
    └── view/               # 2x2 grid renderer / HUD
```

## Status / plan

- [x] **Phase 0** — Project scaffold (build, submodules, app shell).
- [x] Blocker resolved upstream: `ticdata` moved into `data_t` (`Gin_Doom`).
- [ ] **Phase 1 — Gin_Doom**: extend `gin::Doom` to accept per-instance config
      (player index, deathmatch) and expose the tic-exchange seam
      (`D_ReceiveTic` + read-local-cmd) so the arbiter can drive it.
- [ ] **Phase 2 — DoomHost + NetArbiter**: create 4 instances, run the lockstep
      tic exchange, render the 2×2 grid.
- [ ] **Phase 3 — Controllers**: 4 gamepads → per-instance input.
- [ ] **Phase 4 — Audio**: mix the 4 engines; instance 0 music, 1–3 SFX only.

## Build

```
cmake --preset xcode      # or: ninja-gcc (Linux), vs (Windows)
cmake --build build
```
