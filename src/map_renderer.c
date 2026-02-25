#include "map_renderer.h"
#include <ncurses.h>
#include <string.h>

#define PALETTE_OFFSET 1
#define CONTROLS_OFFSET 2
#define STATUS_OFFSET 3

void map_render_tile(WINDOW *win, int y, int x, TileType t) {
    TileDef *def = &tile_defs[t];
    wattron(win, COLOR_PAIR(def->color_pair));
    mvwprintw(win, y + 1, x * 2 + 1, "%s", def->glyph);
    wattroff(win, COLOR_PAIR(def->color_pair));
}

void map_render_canvas(const World *w) {
    box(w->map_window, 0, 0);

    for (int y = 0; y < w->height; y++)
        for (int x = 0; x < w->width; x++)
            map_render_tile(w->map_window, y, x, world_get_tile(w, x, y));

    wnoutrefresh(w->map_window);
}

void map_render_cursor(WINDOW *win, int x, int y) {
    wattron(win, A_REVERSE | A_BOLD);
    mvwprintw(win, y + 1, x * 2 + 1, "[]");
    wattroff(win, A_REVERSE | A_BOLD);

    wnoutrefresh(win);
}

void map_render_palette(WINDOW *win, TileType cur_tile) {
    box(win, 0, 0);

    mvwprintw(win, PALETTE_OFFSET, 1, "Tiles: ");
    int col = strlen("Tiles: ") + 1;
    for (int i = 0; i < NUM_TILE_TYPES; i++) {
        TileDef *def = &tile_defs[i];
        if (def->color_pair) wattron(win, COLOR_PAIR(def->color_pair));
        if (i != (int)cur_tile) wattron(win, A_BOLD | A_REVERSE);

        mvwprintw(win, PALETTE_OFFSET, col, " %d:%-7s", i, def->name);

        if (i != (int)cur_tile) wattroff(win, A_BOLD | A_REVERSE);
        if (def->color_pair) wattroff(win, COLOR_PAIR(def->color_pair));

        col += 10;
    }

    wnoutrefresh(win);
}

void map_render_status(WINDOW *win, const char *status) {
    box(win, 0, 0);

    mvwprintw(win, CONTROLS_OFFSET, 1, "hjkl:move  Space:paint f:floodfill i:info S:save  O:load  Q:quit");
    mvwprintw(win, STATUS_OFFSET, 1, "Status: %s", status);

    // Clear prompt without erasing border (adding freezes everything)
    for (int i = 1 + strlen("Status: ") + strlen(status); i < getmaxx(win) - 1; i++)
       mvwaddch(win, STATUS_OFFSET, i, ' ');

    wmove(win, STATUS_OFFSET, 1 + strlen("Status: "));  // move cursor after prompt

    wrefresh(win);

    wnoutrefresh(win);
}

// Temerary info overlay window
void map_render_info(int x, int y) {
    WINDOW *info_win = newwin(10, 30, y, x * 2 + 2);

    box(info_win, 0, 0);
    mvwprintw(info_win, 1, 2, "INFORMATION");
    wrefresh(info_win);
    wgetch(info_win);
    delwin(info_win);
}
