#include <ncurses.h>
#include "world.h"

#define SELECTION_ROW (MAP_HEIGHT + 1)
#define HELP_ROW   (MAP_HEIGHT + 2)
#define STATUS_ROW (MAP_HEIGHT + 3)

typedef struct {
    int      cur_x, cur_y;
    TileType cur_tile;
    char     status[256]; // Must store path [128]
} EditorState;

//  Draw the full canvas
static void draw_canvas(const World *w) {
    for (int y = 0; y < w->height; y++)
        for (int x = 0; x < w->width; x++)
            world_draw_tile(y, x, world_get_tile(w, x, y));
}

//  Draw the cursor on top of the canvas
static void draw_cursor(int x, int y) {
    attron(A_REVERSE | A_BOLD);
    mvprintw(y, x * 2, "[]");
    attroff(A_REVERSE | A_BOLD);
}

//  Draw the tile palette bar
static void draw_palette(TileType cur_tile) {
    mvprintw(SELECTION_ROW, 0, "Tile: ");
    int column_offset = 6;
    for (int i = 0; i < NUM_TILE_TYPES; i++) {
        TileDef *def = &tile_defs[i];
        if (def->color_pair) attron(COLOR_PAIR(def->color_pair));
        if (i != (int)cur_tile) attron(A_BOLD | A_REVERSE);

        mvprintw(SELECTION_ROW, column_offset, " %d:%-7s", i, def->name);

        if (i != (int)cur_tile) attroff(A_BOLD | A_REVERSE);
        if (def->color_pair) attroff(COLOR_PAIR(def->color_pair));

        column_offset += 10;
    }
}

//  Draw the help bar and status message
static void draw_help(const EditorState *e) {
    mvprintw(HELP_ROW, 0, "hjkl:move Shift+hjl:brush Space:paint f:fill S:save  O:load  Q:quit");
    mvprintw(STATUS_ROW, 0, "Status: %s", e->status);
    clrtoeol();
}

//  Prompt the user for a filename at the bottom
static void prompt_filename(const char *prompt, char *out, int maxlen) {
    echo();
    curs_set(1);
    mvprintw(STATUS_ROW, 0, "%s", prompt);
    clrtoeol();
    getnstr(out, maxlen - 1);
    noecho();
    curs_set(0);
}

//  Entry point
int main(void) {
    initscr();
    start_color();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);

    world_setup_colors();

    World world;

    EditorState editor = {
        .cur_x    = 0,
        .cur_y    = 0,
        .cur_tile = TILE_WALL,
        .status   = "ready",
    };

    world_init(&world);

    while (1) {
        draw_canvas(&world);
        draw_cursor(editor.cur_x, editor.cur_y);
        draw_palette(editor.cur_tile);
        draw_help(&editor);
        refresh();

        int ch = getch();
        editor.status[0] = '\0'; // clear status each frame

        switch (ch) {
            // Basic movement
            case 'k':
                if (editor.cur_y > 0) editor.cur_y--;
                break;
            case 'j':
                if (editor.cur_y < MAP_HEIGHT - 1) editor.cur_y++;
                break;
            case 'h':
                if (editor.cur_x > 0) editor.cur_x--;
                break;
            case 'l':
                if (editor.cur_x < MAP_WIDTH - 1) editor.cur_x++;
                break;

            // Shift movement paints tile under cursor
            case 'K':
                if (editor.cur_y > 0) {
                    world_set_tile(&world, editor.cur_x, editor.cur_y, editor.cur_tile);
                    editor.cur_y--;
                }
                break;
            case 'J':
                if (editor.cur_y < MAP_HEIGHT - 1) {
                    world_set_tile(&world, editor.cur_x, editor.cur_y, editor.cur_tile);
                    editor.cur_y++;
                }
                break;
            case 'H':
                if (editor.cur_x > 0) {
                    world_set_tile(&world, editor.cur_x, editor.cur_y, editor.cur_tile);
                    editor.cur_x--;
                }
                break;
            case 'L':
                if (editor.cur_x < MAP_WIDTH - 1) {
                    world_set_tile(&world, editor.cur_x, editor.cur_y, editor.cur_tile);
                    editor.cur_x++;
                }
                break;

            case ' ':
                world_set_tile(&world, editor.cur_x, editor.cur_y, editor.cur_tile);
                break;

            // number keys select tile type
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
            case '8': case '9': {
                int t = ch - '0';
                if (t < NUM_TILE_TYPES) {
                    editor.cur_tile = (TileType)t;
                }

                break;
            }

            // cycle tile with [ and ]
            case '[':
                editor.cur_tile = (editor.cur_tile + NUM_TILE_TYPES - 1) % NUM_TILE_TYPES;
                break;
            case ']':
                editor.cur_tile = (editor.cur_tile + 1) % NUM_TILE_TYPES;
                break;

            case 's': case 'S': {
                char path[128];
                prompt_filename("Save to: ", path, sizeof(path));
                if (world_save(&world, path))
                    snprintf(editor.status, sizeof(editor.status), "saved to %s", path);
                else
                    snprintf(editor.status, sizeof(editor.status), "save failed!");
                break;
            }

            case 'o': case 'O': {
                char path[128];
                prompt_filename("Open file: ", path, sizeof(path));
                if (world_load(&world, path))
                    snprintf(editor.status, sizeof(editor.status), "loaded %s", path);
                else
                    snprintf(editor.status, sizeof(editor.status), "load failed!");
                break;
            }

            case 'f': case 'F': {
                world_flood_fill(&world,
                    editor.cur_x, editor.cur_y, editor.cur_tile,
                    world_get_tile(&world, editor.cur_x, editor.cur_y));
                break;
            }

            case 'q': case 'Q':
                endwin();
                return 0;
        }
    }
}
