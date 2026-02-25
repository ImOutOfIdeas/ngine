#include "editor.h"
#include "map_renderer.h"

#include <ncurses.h>
#include <stdio.h>
#include <string.h>

void editor_init(EditorState *e) {
    e->cur_x      = 0;
    e->cur_y      = 0;
    e->cur_tile   = TILE_WALL;
    e->cur_entity = ENTITY_NONE;
    e->mode       = PAINT_TILE;
    strncpy(e->status, "ready", sizeof(e->status) - 1);
    e->status[sizeof(e->status) - 1] = '\0';
}

void editor_render(const EditorState *e, const World *w) {
    map_render_canvas(w);
    map_render_cursor(w->map_window, e->cur_x, e->cur_y);
    map_render_palette(w->status_window, e->cur_tile, e->cur_entity, e->mode);
    map_render_status(w->status_window, e->status);
}

void prompt_filename(WINDOW *win, const char *prompt, char *out, int maxlen) {
    int row        = 3;
    int prompt_len = strlen(prompt);
    int width      = getmaxx(win);

    echo();
    curs_set(1);
    mvwprintw(win, row, 1, "%s", prompt);

    for (int i = 1 + prompt_len; i < width - 1; i++)
        mvwaddch(win, row, i, ' ');

    wrefresh(win);
    wmove(win, row, 1 + prompt_len);
    wgetnstr(win, out, maxlen - 1);
    noecho();
    curs_set(0);
}

// Paint at (x,y) according to the current mode.
static void editor_paint(EditorState *e, World *w, int x, int y) {
    if (e->mode == PAINT_TILE) {
        world_set_tile(w, x, y, e->cur_tile);
    } else {
        const char *err = world_place_entity(w, x, y, e->cur_entity);
        if (err) snprintf(e->status, sizeof(e->status), "%s", err);
    }
}

int editor_handle_input(EditorState *e, World *w, int ch) {
    e->status[0] = '\0';

    switch (ch) {
        //  hjkl - Cursor movement
        case KEY_UP:   case 'k': if (e->cur_y > 0)              e->cur_y--; break;
        case KEY_DOWN: case 'j': if (e->cur_y < MAP_HEIGHT - 1) e->cur_y++; break;
        case KEY_LEFT: case 'h': if (e->cur_x > 0)              e->cur_x--; break;
        case KEY_RIGHT:case 'l': if (e->cur_x < MAP_WIDTH - 1)  e->cur_x++; break;

        //  Shift + hjkl - paint while moving
        case 'K':
            if (e->cur_y > 0)
                { editor_paint(e, w, e->cur_x, e->cur_y); e->cur_y--; } break;
        case 'J':
            if (e->cur_y < MAP_HEIGHT - 1)
                { editor_paint(e, w, e->cur_x, e->cur_y); e->cur_y++; } break;
        case 'H':
            if (e->cur_x > 0)
                { editor_paint(e, w, e->cur_x, e->cur_y); e->cur_x--; } break;
        case 'L':
            if (e->cur_x < MAP_WIDTH - 1)
                { editor_paint(e, w, e->cur_x, e->cur_y); e->cur_x++; } break;

        // Space - paint current tile or entity
        case ' ':
            editor_paint(e, w, e->cur_x, e->cur_y);
            break;

        // f/F - fill current tile (entity fill does nothing)
        case 'f': case 'F':
            if (e->mode == PAINT_TILE)
                world_flood_fill(w, e->cur_x, e->cur_y, e->cur_tile,
                                 world_get_tile(w, e->cur_x, e->cur_y));
            break;

        //  Tile selection
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9': {
            int t = ch - '0';
            if (t < NUM_TILE_TYPES) {
                e->cur_tile = (TileType)t;
                e->mode     = PAINT_TILE;
            }
            break;
        }
        case '[': e->cur_tile = (e->cur_tile + NUM_TILE_TYPES - 1) % NUM_TILE_TYPES;
                  e->mode = PAINT_TILE; break;
        case ']': e->cur_tile = (e->cur_tile + 1) % NUM_TILE_TYPES;
                  e->mode = PAINT_TILE; break;

        // Entity selection
        case ')': case '!': case '@': case '#':
        case '$': case '%': case '^': case '&':
        case '*': case '(': {
            static const char shift_map[] = ")!@#$%^&*(";
            for (int i = 0; i < NUM_ENTITY_TYPES; i++) {
                if (ch == shift_map[i]) {
                    e->cur_entity = (EntityType)i;
                    e->mode       = PAINT_ENTITY;
                    break;
                }
            }
            break;
        }

        //  Info overlay
        case 'i':
            map_render_info(w, e->cur_x, e->cur_y);
            break;

        //  Save / load
        case 's': case 'S': {
            char path[128];
            prompt_filename(w->status_window, "Save to: ", path, sizeof(path));
            if (world_save(w, path))
                snprintf(e->status, sizeof(e->status), "saved to %s", path);
            else
                snprintf(e->status, sizeof(e->status), "save failed!");
            break;
        }
        case 'o': case 'O': {
            char path[128];
            prompt_filename(w->status_window, "Open file: ", path, sizeof(path));
            if (world_load(w, path))
                snprintf(e->status, sizeof(e->status), "loaded %s", path);
            else
                snprintf(e->status, sizeof(e->status), "load failed!");
            break;
        }

        //  Quit
        case 'q': case 'Q':
            return 0;

        //  Terminal resize
        case KEY_RESIZE:
            resize_term(0, 0);
            clear();
            refresh();
            redrawwin(w->map_window);
            redrawwin(w->status_window);
            box(w->map_window, 0, 0);
            box(w->status_window, 0, 0);
            snprintf(e->status, sizeof(e->status), "terminal resized");
            break;
    }

    return 1;
}
