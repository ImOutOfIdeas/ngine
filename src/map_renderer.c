#include "map_renderer.h"
#include <ncurses.h>
#include <string.h>

// Status window row layout (6 rows total, 0 and 5 are borders):
#define ROW_TILES     1   // tile palette
#define ROW_ENTITIES  2   // entity palette
#define ROW_CONTROLS  3   // key hints
#define ROW_STATUS    4   // status message

void map_render_tile(WINDOW *win, int y, int x, TileType t) {
    TileDef *def = &tile_defs[t];
    wattron(win, COLOR_PAIR(def->color_pair));
    mvwprintw(win, y + 1, x * 2 + 1, "  ");   // solid color block, no glyph
    wattroff(win, COLOR_PAIR(def->color_pair));
}

// Entity glyph rendered with entity fg + underlying tile's bg so the tile
// color is preserved exactly rather than falling back to terminal default.
void map_render_entity(WINDOW *win, int y, int x, EntityType et, TileType tt) {
    EntityDef *def = &entity_defs[et];
    int pair = world_entity_blend_pair(et, tt);
    wattron(win, COLOR_PAIR(pair) | A_BOLD);
    mvwprintw(win, y + 1, x * 2 + 1, "%s", def->glyph);
    wattroff(win, COLOR_PAIR(pair) | A_BOLD);
}

void map_render_canvas(const World *w) {
    box(w->map_window, 0, 0);

    for (int y = 0; y < w->height; y++)
        for (int x = 0; x < w->width; x++)
            map_render_tile(w->map_window, y, x, world_get_tile(w, x, y));

    for (int i = 0; i < w->entity_count; i++) {
        const Entity *e = &w->entities[i];
        if (!e->active) continue;
        if (e->x < 0 || e->x >= w->width)  continue;
        if (e->y < 0 || e->y >= w->height) continue;
        TileType tt = world_get_tile(w, e->x, e->y);
        map_render_entity(w->map_window, e->y, e->x, e->type, tt);
    }

    wnoutrefresh(w->map_window);
}

void map_render_cursor(WINDOW *win, int x, int y) {
    wattron(win, A_REVERSE | A_BOLD);
    mvwprintw(win, y + 1, x * 2 + 1, "[]");
    wattroff(win, A_REVERSE | A_BOLD);
    wnoutrefresh(win);
}

void map_render_palette(WINDOW *win, TileType cur_tile, EntityType cur_entity, PaintMode mode) {
    box(win, 0, 0);

    // Tile palette — dim when entity mode is active
    mvwprintw(win, ROW_TILES, 1, "Tiles: ");
    if (mode == PAINT_ENTITY) wattron(win, A_DIM);

    int col = strlen("Tiles: ") + 1;
    for (int i = 0; i < NUM_TILE_TYPES; i++) {
        TileDef *def = &tile_defs[i];
        int is_selected = (i == (int)cur_tile);

        if (def->color_pair && mode == PAINT_TILE) wattron(win, COLOR_PAIR(def->color_pair));
        if (!is_selected) wattron(win, A_REVERSE);

        mvwprintw(win, ROW_TILES, col, " %d:%-7s", i, def->name);

        if (!is_selected) wattroff(win, A_REVERSE);
        if (def->color_pair && mode == PAINT_TILE) wattroff(win, COLOR_PAIR(def->color_pair));

        col += 10;
    }
    if (mode == PAINT_ENTITY) wattroff(win, A_DIM);

    // Entity palette — dim when tile mode is active
    mvwprintw(win, ROW_ENTITIES, 1, "Ents:  ");
    if (mode == PAINT_TILE) wattron(win, A_DIM);
    col = strlen("Ents:  ") + 1;
    for (int i = 0; i < NUM_ENTITY_TYPES; i++) {
        EntityDef *def = &entity_defs[i];
        int is_selected = (i == (int)cur_entity);

        if (i == ENTITY_NONE) {
            if (is_selected) wattron(win, A_BOLD);
        } else {
            if (def->color_pair && mode == PAINT_ENTITY)
                wattron(win, COLOR_PAIR(def->color_pair) | A_BOLD);
        }
        if (!is_selected) wattron(win, A_REVERSE);

        mvwprintw(win, ROW_ENTITIES, col, " %d:%-7s", i, def->name);

        if (!is_selected) wattroff(win, A_REVERSE);
        if (i == ENTITY_NONE) {
            wattroff(win, A_BOLD);
        } else {
            if (def->color_pair && mode == PAINT_ENTITY)
                wattroff(win, COLOR_PAIR(def->color_pair) | A_BOLD);
        }
        col += 10;
    }
    if (mode == PAINT_TILE) wattroff(win, A_DIM);

    wnoutrefresh(win);
}

void map_render_status(WINDOW *win, const char *status) {
    box(win, 0, 0);

    mvwprintw(win, ROW_CONTROLS, 1,
        "hjkl:move  Spc:paint  f:fill  0-9:tile(shift for ent) i:info  S:save  O:load  Q:quit");
    mvwprintw(win, ROW_STATUS, 1, "Status: %s", status);

    for (int i = 1 + (int)strlen("Status: ") + (int)strlen(status);
         i < getmaxx(win) - 1; i++)
        mvwaddch(win, ROW_STATUS, i, ' ');

    wmove(win, ROW_STATUS, 1 + strlen("Status: "));
    wrefresh(win);
    wnoutrefresh(win);
}

void map_render_info(const World *w, int x, int y) {
    WINDOW *info_win = newwin(10, 32, y, x * 2 + 2);
    box(info_win, 0, 0);
    mvwprintw(info_win, 1, 2, "TILE INFO  (%d, %d)", x, y);

    TileType tt = world_get_tile(w, x, y);
    mvwprintw(info_win, 3, 2, "Tile  : %s", tile_defs[tt].name);

    Entity *e = world_find_entity((World *)w, x, y);
    if (e) {
        mvwprintw(info_win, 4, 2, "Entity: %s", entity_defs[e->type].name);
        mvwprintw(info_win, 5, 2, "Param : %d", e->param);
    } else {
        mvwprintw(info_win, 4, 2, "Entity: (none)");
    }

    mvwprintw(info_win, 8, 2, "[any key to close]");
    wrefresh(info_win);
    wgetch(info_win);
    delwin(info_win);
}
