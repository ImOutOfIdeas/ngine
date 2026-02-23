# NCurses Map Editor + Game — Project Roadmap
---

## Project Overview

| File | Purpose |
|------|---------|
| `world.h` | Tile definitions, `World` struct, shared types |
| `world.c` | Tile drawing, color setup, save/load implementation |
| `sim.h/.c` | Game simulation: player movement, collision, entity logic |
| `render.h/.c` | All ncurses drawing for the game view |
| `editor.h/.c` | Editor state, overlay rendering, input handling |
| `input.h/.c` | Raw keypress reading and routing based on current mode |
| `main.c` | Entry point, main loop, mode switching (play ↔ editor) |


## Phase 1 — Refactor Editor
- [x] Define `TileType` enum and `TileDef` struct in a new `world.h`
- [x] Give each tile a glyph (e.g. `##`, `  `, `||`)
- [x] Move all color/tile setup into `world.c`
- [x] Rework the palette UI to show tile names and glyphs, not just colors
- [x] Support selecting tiles by number key and cycling with `[` / `]`
---

## Phase 2 — Save & Load
- [x] Design a map file format (header + tile grid + entity block)
- [x] Implement `world_save(World *w, const char *path)`
- [x] Implement `world_load(World *w, const char *path)`
- [x] Bind `s` to save and `o` to open/load in the editor
- [x] Show a status message on save/load success or failure
- - [x] fix status buffer size cant hold full length of path
- [x] Test: save a map, reload it, verify it matches
---

## Phase 3 — Editor Quality of Life
- [x] Add flood fill with `f` key
- [x] Add auto-paint mode (hold-to-draw as cursor moves)
- [ ] Show tile name and type info for the tile under the cursor
- [ ] Add an undo stack (circular buffer of last N world states or diff operations)
- [ ] Show canvas grid coordinates somewhere in the UI
---

## Phase 4 — Entities & Metadata
- [ ] Define `Entity` struct with position, type, and a `param` field
- [ ] Store entities in `World` separately from the tile grid
- [ ] Render entity glyphs on top of tiles in the editor
- [ ] Implement entity inspector sub-mode (`e` key on an entity tile)
- [ ] Implement link mode (`l` key) to wire buttons to doors by cursor selection
- [ ] Include entities in save/load
---

## Phase 5 — Game Simulation
- [ ] Create `sim.h` / `sim.c` with `SimState` struct (player pos, velocity, etc.)
- [ ] Implement `sim_init(SimState *s, World *w)`
- [ ] Implement `sim_step(SimState *s, World *w, InputFrame *input, float dt)`
- [ ] Add tile-based collision detection (walls block movement)
- [ ] Add gravity and basic platformer or top-down movement (pick your genre)
- [ ] Implement entity logic: buttons toggle doors, goal tile triggers win, etc.
- [ ] Add player spawn: sim reads `TILE_SPAWN` from the world on init
---

## Phase 6 — Renderer Split
- [ ] Create `render.h` / `render.c`
- [ ] Move all ncurses drawing out of `main.c` into `render_game(SimState, World)`
- [ ] Write `render_editor_overlay(EditorState, World)` as a second pass
- [ ] Ensure overlay only writes to cells it owns so game view shows through
- [ ] Validate that two-pass rendering produces a clean combined frame
---

## Phase 7 — Live Overlay
- [ ] Create `input.h` / `input.c` with `InputFrame` struct
- [ ] Route raw keypresses to either `sim_step` or `editor_step` based on mode
- [ ] Add TAB to toggle between `MODE_PLAY` and `MODE_EDITOR`
- [ ] In editor mode, keep calling `sim_step` with zeroed input so world stays live
- [ ] Confirm player and entities remain visible and animated under the editor overlay
- [ ] Add a visible mode indicator (e.g. `[EDITOR]` / `[PLAY]` in corner)
---

## Phase 8 — Polish & Extras
- [ ] Add a "test from here" shortcut that spawns the player at the cursor position
- [ ] Add camera/viewport scrolling for maps larger than the terminal
- [ ] Add a map metadata header (map name, author, version)
- [ ] Support multiple maps and a level select screen
- [ ] Package a few demo maps with the project
- [ ] Write a README covering controls, file format, and build instructions
---

## Ongoing
- [ ] Keep `sim.c` and `editor.c` from ever calling each other directly
- [ ] Keep `world.h` as the single shared definition both sides depend on
- [ ] Test save/load after every new feature that touches world data
- [ ] Playtest each map in the live overlay to catch editor/sim disagreements early
