#include "map_renderer.h"

#include <ncurses.h>

#define SELECTION_ROW (MAP_HEIGHT + 1)
#define HELP_ROW      (MAP_HEIGHT + 2)
#define STATUS_ROW    (MAP_HEIGHT + 3)

void map_render_canvas(const World *w) {
    for (int y = 0; y < w->height; y++)
        for (int x = 0; x < w->width; x++)
            world_draw_tile(y, x, world_get_tile(w, x, y));
}

void map_render_cursor(int x, int y) {
    attron(A_REVERSE | A_BOLD);
    mvprintw(y, x * 2, "[]");
    attroff(A_REVERSE | A_BOLD);
}

void map_render_palette(TileType cur_tile) {
    mvprintw(SELECTION_ROW, 0, "Tile: ");
    int col = 6;
    for (int i = 0; i < NUM_TILE_TYPES; i++) {
        TileDef *def = &tile_defs[i];
        if (def->color_pair) attron(COLOR_PAIR(def->color_pair));
        if (i != (int)cur_tile) attron(A_BOLD | A_REVERSE);

        mvprintw(SELECTION_ROW, col, " %d:%-7s", i, def->name);

        if (i != (int)cur_tile) attroff(A_BOLD | A_REVERSE);
        if (def->color_pair) attroff(COLOR_PAIR(def->color_pair));

        col += 10;
    }
}

void map_render_help(const char *status) {
    mvprintw(HELP_ROW,   0, "Arrows/hjkl:move  Space:paint  S:save  O:load  Q:quit");
    mvprintw(STATUS_ROW, 0, "Status: %s", status);
    clrtoeol();
}
