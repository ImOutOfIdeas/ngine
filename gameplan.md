# NCurses Map Editor + Game — Project Roadmap
---

## Project Overview

| File (Planned) | Purpose |
|------|---------|
| `world.h / world.c`         | Tile definitions, `World` struct, save/load, shared types |
| `editor.h / editor.c`       | Editor state, input handling, rendering delegation |
| `map_render.h / map_render.c` | ncurses tile-grid drawing (canvas, cursor, palette, help bar) |
| `ray_render.h / ray_render.c` | Raycaster renderer (first-person game view) |
| `sim.h / sim.c`             | Game simulation: player movement, collision, entity logic |
| `input.h / input.c`         | Raw keypress reading and routing based on current mode |
| `main.c`                    | Entry point, main loop, mode switching (play ↔ editor) |

---

## Phase 1 — Editor Foundation
- [x] Define `TileType` enum and `TileDef` struct in `world.h`
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
- [x] Fix status buffer — must be large enough to hold full path
- [x] Test: save a map, reload it, verify it matches
---

## Phase 3 — Code Structure Refactor
- [x] Extract `EditorState` and all input handling into `editor.h` / `editor.c`
- [x] Extract all ncurses drawing into `map_render.h` / `map_render.c`
- [x] Thin `main.c` down to init + loop only (~25 lines)
- [x] `editor_handle_input` returns 0 on quit
- [x] Add `hjkl` vim-style movement bindings
- [x] Ensure `map_render.c` has no dependency on `editor.h` (clean layering)
---

## Phase 4 — Editor Quality of Life
- [x] Add flood fill with `f` key
- [x] Add auto-paint mode (hold-to-draw as cursor moves)
- [x] implement the map view and status bar inside there own windows
- [ ] Add an undo stack (circular buffer of last N world states or diff operations)
- [ ] Show canvas grid coordinates in the UI
---

## Phase 5 — Entities & Metadata
- [ ] `Entity` struct is defined in `world.h` — wire it up in the editor
- [ ] Render entity glyphs on top of tiles in `map_render.c`
- [ ] Implement entity inspector sub-mode (`e` key when cursor is on an entity tile)
- [ ] Implement link mode (`l` key) to wire buttons to doors by cursor selection
- [ ] Confirm entities are included in save/load (skeleton already exists in `world_save` / `world_load`)
---

## Phase 6 — Game Simulation
- [ ] Create `sim.h` / `sim.c` with `SimState` struct (player pos, velocity, etc.)
- [ ] Implement `sim_init(SimState *s, World *w)` — reads `TILE_SPAWN` for start
    - [ ] Ensure only one spawn is on the map (furthest from goal if many?)
- [ ] Implement `sim_step(SimState *s, World *w, InputFrame *input, float dt)`
- [ ] Add tile-based collision detection (walls block movement)
- [ ] Implement movement model (FPS)
- [ ] Implement entity logic: buttons toggle doors, goal tile triggers win
- [ ] Create `input.h` / `input.c` with `InputFrame` struct
- [ ] Route raw keypresses to either `sim_step` or `editor_handle_input` based on mode
- [ ] Add TAB to toggle between `MODE_PLAY` and `MODE_EDITOR`
- [ ] Add a visible mode indicator (e.g. `[EDITOR]` / `[PLAY]` in corner)
---

## Phase 7 — Raycaster Renderer
- [ ] Create `ray_render.h` / `ray_render.c`
- [ ] Implement basic DDA raycasting using the tile grid as the world geometry
- [ ] Treat `solid = 1` tiles as walls, `solid = 0` as open space
- [ ] Render column-by-column into a fixed terminal viewport
- [ ] Use different ncurses characters / shading for wall distance (e.g. `█ ▓ ▒ ░`)
- [ ] Differentiate wall types by tile color pair
- [ ] Add floor and ceiling rows (flat shaded is fine to start)
- [ ] Hook `ray_render_frame(SimState *s, World *w)` into the play-mode render path
- [ ] Confirm the map editor and raycaster share the same `World` data with no copies
---

## Phase 8 — Live Editor Overlay
- [ ] In editor mode, keep calling `sim_step` with zeroed input so world stays live
- [ ] Render the game view as the base layer, editor overlay on top
- [ ] Ensure the overlay only writes to cells it owns so the game view shows through
- [ ] Validate that two-pass rendering produces a clean combined frame
- [ ] Add a "test from here" shortcut that spawns the player at the cursor position
---

## Phase 9 — Polish & Extras
- [ ] Camera / viewport scrolling for maps larger than the terminal
- [ ] Map metadata header (map name, author, version)
- [ ] Support multiple maps and a level select screen
- [ ] Package a few demo maps with the project
- [ ] Write a README covering controls, file format, and build instructions
---

## Ongoing
- [ ] Keep `sim.c` and `editor.c` from ever calling each other directly
- [ ] Keep `world.h` as the single shared definition both sides depend on
- [ ] `map_render.c` must never depend on `editor.h` — rendering is not editor logic
- [ ] Test save/load after every new feature that touches world data
- [ ] Playtest each map in the live overlay to catch editor/sim disagreements early
