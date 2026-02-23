#include "editor.h"
#include "map_renderer.h"

#include <ncurses.h>
#include <stdio.h>
#include <string.h>

void editor_init(EditorState *e) {
    e->cur_x    = 0;
    e->cur_y    = 0;
    e->cur_tile = TILE_WALL;
    strncpy(e->status, "ready", sizeof(e->status) - 1);
    e->status[sizeof(e->status) - 1] = '\0';
}

void editor_render(const EditorState *e, const World *w) {
    map_render_canvas(w);
    map_render_cursor(e->cur_x, e->cur_y);
    map_render_palette(e->cur_tile);
    map_render_help(e->status);
}

void prompt_filename(const char *prompt, char *out, int maxlen) {
    echo();
    curs_set(1);
    mvprintw(MAP_HEIGHT + 3, 0, "%s", prompt);
    clrtoeol();
    getnstr(out, maxlen - 1);
    noecho();
    curs_set(0);
}

int editor_handle_input(EditorState *e, World *w, int ch) {
    e->status[0] = '\0';

    switch (ch) {
        // Basic editor movement
        case KEY_UP:    case 'k':
            if (e->cur_y > 0) e->cur_y--;
            break;
        case KEY_DOWN:  case 'j':
            if (e->cur_y < MAP_HEIGHT - 1) e->cur_y++;
            break;
        case KEY_LEFT:  case 'h':
            if (e->cur_x > 0) e->cur_x--;
            break;
        case KEY_RIGHT: case 'l':
            if (e->cur_x < MAP_WIDTH - 1) e->cur_x++;
            break;

        // Shift movement paints tile under cursor
        case 'K':
            if (e->cur_y > 0) {
                world_set_tile(w, e->cur_x, e->cur_y, e->cur_tile);
                e->cur_y--;
            }
            break;
        case 'J':
            if (e->cur_y < MAP_HEIGHT - 1) {
                world_set_tile(w, e->cur_x, e->cur_y, e->cur_tile);
                e->cur_y++;
            }
            break;
        case 'H':
            if (e->cur_x > 0) {
                world_set_tile(w, e->cur_x, e->cur_y, e->cur_tile);
                e->cur_x--;
            }
            break;
        case 'L':
            if (e->cur_x < MAP_WIDTH - 1) {
                world_set_tile(w, e->cur_x, e->cur_y, e->cur_tile);
                e->cur_x++;
            }
            break;

        case ' ':
            world_set_tile(w, e->cur_x, e->cur_y, e->cur_tile);
            break;

        case 'f': case 'F': {
            world_flood_fill(w, e->cur_x, e->cur_y, e->cur_tile,
                                world_get_tile(w, e->cur_x, e->cur_y));
            break;
        }

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9': {
            int t = ch - '0';
            if (t < NUM_TILE_TYPES)
                e->cur_tile = (TileType)t;
            break;
        }

        case '[':
            e->cur_tile = (e->cur_tile + NUM_TILE_TYPES - 1) % NUM_TILE_TYPES;
            break;
        case ']':
            e->cur_tile = (e->cur_tile + 1) % NUM_TILE_TYPES;
            break;

        case 's': case 'S': {
            char path[128];
            prompt_filename("Save to: ", path, sizeof(path));
            if (world_save(w, path))
                snprintf(e->status, sizeof(e->status), "saved to %s", path);
            else
                snprintf(e->status, sizeof(e->status), "save failed!");
            break;
        }

        case 'o': case 'O': {
            char path[128];
            prompt_filename("Open file: ", path, sizeof(path));
            if (world_load(w, path))
                snprintf(e->status, sizeof(e->status), "loaded %s", path);
            else
                snprintf(e->status, sizeof(e->status), "load failed!");
            break;
        }

        case 'q': case 'Q':
            return 0;
    }

    return 1;
}
