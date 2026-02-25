#include "world.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  tile_defs[] — built from TILE_LIST at compile time
//  color_pair is filled in at runtime by world_setup_colors()
TileDef tile_defs[NUM_TILE_TYPES] = {
    #define X(id, name, glyph, fg, bg, solid, passable) \
    [id] = { name, glyph, 0, solid, passable },
    TILE_LIST
    #undef X
};

//  Color support detection
typedef enum {
    COLOR_SUPPORT_BASIC,  // 8 colors  — COLOR_* constants
    COLOR_SUPPORT_256,    // 256 colors — 0-255 indices
    COLOR_SUPPORT_TRUE,   // 24-bit RGB — init_extended_color
} ColorSupport;

static ColorSupport g_color_support = COLOR_SUPPORT_BASIC;

static ColorSupport detect_color_support(void) {
    if (!has_colors()) return COLOR_SUPPORT_BASIC;
    if (COLORS >= 256 && can_change_color()) {
        const char *ct = getenv("COLORTERM");
        if (ct && (strcmp(ct, "truecolor") == 0 || strcmp(ct, "24bit") == 0))
            return COLOR_SUPPORT_TRUE;
        return COLOR_SUPPORT_256;
    }
    return COLOR_SUPPORT_BASIC;
}

// Set up a single color pair using the best available method.
static void setup_color_pair(int pair_id,
                             int basic_fg,  int basic_bg,
                             int fg256,     int bg256,
                             int fr, int fg, int fb,
                             int br, int bg_r, int bb)
{
    if (g_color_support == COLOR_SUPPORT_TRUE) {
        int fg_id = 16 + pair_id * 2;
        int bg_id = 16 + pair_id * 2 + 1;
        init_extended_color(fg_id, fr * 1000/255, fg * 1000/255, fb * 1000/255);
        init_extended_color(bg_id, br * 1000/255, bg_r * 1000/255, bb * 1000/255);
        init_extended_pair(pair_id, fg_id, bg_id);
        return;
    }
    if (g_color_support == COLOR_SUPPORT_256) {
        init_pair(pair_id, fg256, bg256);
    } else {
        init_pair(pair_id, basic_fg, basic_bg);
    }
}

//  Color pair setup
//  256-color and true-color values are indexed by TileType

// 256-color palette
//                            EMPTY  WALL  FLOOR  BUTTON  DOOR  SPAWN  GOAL
static const int fg_256[] = { 59,   65,   110,  179,  176,  108,   73 };
static const int bg_256[] = { 235,  241,   24,   94,   55,   22,   23 };

// True color palette
static const int fg_rgb[][3] = {
    {  92,  99, 112 },  // EMPTY  — comment grey
    { 171, 178, 191 },  // FLOOR  — foreground
    { 130, 195, 255 },  // WALL   — blue, brightened
    { 229, 192, 123 },  // BUTTON — yellow
    { 198, 120, 221 },  // DOOR   — purple
    { 224, 108, 117 },  // SPAWN  — One Dark red
    {  86, 182, 194 },  // GOAL   — cyan
};
static const int bg_rgb[][3] = {
    {  0,    0,   0 },  // EMPTY  — background
    {  40,  44,  52 },  // FLOOR  — surface
    {  35,  65, 110 },  // WALL   — deep blue, brightened
    {  65,  48,  18 },  // BUTTON — deep amber
    {  52,  22,  65 },  // DOOR   — deep purple
    {  70,  22,  26 },  // SPAWN  — deep red derived from #e06c75
    {  16,  46,  50 },  // GOAL   — deep cyan
};

void world_setup_colors(void) {
    g_color_support = detect_color_support();

    // TILE_LIST provides basic color fallbacks.
    #define X(id, name, glyph, basic_fg, basic_bg, solid, passable)     \
        setup_color_pair(id + 1,                                         \
                         basic_fg,       basic_bg,                       \
                         fg_256[id],     bg_256[id],                     \
                         fg_rgb[id][0],  fg_rgb[id][1],  fg_rgb[id][2],  \
                         bg_rgb[id][0],  bg_rgb[id][1],  bg_rgb[id][2]); \
        tile_defs[id].color_pair = id + 1;
    TILE_LIST
    #undef X
}

//  World init
void world_init(World *w) {
    w->width        = MAP_WIDTH;
    w->height       = MAP_HEIGHT;
    w->entity_count = 0;
    memset(w->entities, 0, sizeof(w->entities));
    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            w->tiles[y][x] = TILE_EMPTY;

    w->map_window    = newwin(MAP_HEIGHT + 2, MAP_WIDTH * 2 + 2, 0, 0);
    w->status_window = newwin(5, MAP_WIDTH * 2 + 2, MAP_HEIGHT + 2, 0);

    keypad(w->map_window, TRUE);
    keypad(w->status_window, TRUE);

    box(w->map_window, 0, 0);
    box(w->status_window, 0, 0);
}

//  Tile accessor
TileType world_get_tile(const World *w, int x, int y) {
    if (x < 0 || x >= w->width || y < 0 || y >= w->height)
        return TILE_WALL;

    return w->tiles[y][x];
}

//  Tile mutator
void world_set_tile(World *w, int x, int y, TileType t) {
    if (x < 0 || x >= w->width || y < 0 || y >= w->height)
        return;

    w->tiles[y][x] = t;
}

//  Save / load  into plain text format
int world_save(const World *w, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return 0;

    fprintf(f, "%d %d\n", w->width, w->height);
    for (int y = 0; y < w->height; y++) {
        for (int x = 0; x < w->width; x++)
            fprintf(f, "%d%c", (int)w->tiles[y][x], x == w->width - 1 ? '\n' : ' ');
    }

    fprintf(f, "%d\n", w->entity_count);
    for (int i = 0; i < w->entity_count; i++) {
        const Entity *e = &w->entities[i];
        fprintf(f, "%d %d %d %d\n", e->x, e->y, (int)e->type, e->param);
    }

    fclose(f);
    return 1;
}

int world_load(World *w, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    if (fscanf(f, "%d %d\n", &w->width, &w->height) != 2) {
        fclose(f); return 0;
    }

    for (int y = 0; y < w->height; y++)
        for (int x = 0; x < w->width; x++) {
            int t;
            fscanf(f, "%d", &t);
            w->tiles[y][x] = (TileType)t;
        }

    fscanf(f, "%d\n", &w->entity_count);
    for (int i = 0; i < w->entity_count; i++) {
        Entity *e = &w->entities[i];
        int type;
        fscanf(f, "%d %d %d %d\n", &e->x, &e->y, &type, &e->param);
        e->type   = (TileType)type;
        e->active = 1;
    }

    fclose(f);
    return 1;
}

void world_flood_fill(World *w, int x, int y, TileType replace, TileType target) {
    if (replace == target) return;
    if (world_get_tile(w, x, y) != target) return;

    // Stack using a flat array — size is the whole map
    int stack[MAP_WIDTH * MAP_HEIGHT][2];
    int top = 0;

    stack[top][0] = x;
    stack[top][1] = y;
    top++;

    while (top > 0) {
        top--;
        int cx = stack[top][0];
        int cy = stack[top][1];

        if (world_get_tile(w, cx, cy) != target) continue;
        world_set_tile(w, cx, cy, replace);

        // Push neighbors onto the stack
        if (cx > 0)             { stack[top][0]=cx-1; stack[top][1]=cy;   top++; }
        if (cx < w->width - 1)  { stack[top][0]=cx+1; stack[top][1]=cy;   top++; }
        if (cy > 0)             { stack[top][0]=cx;   stack[top][1]=cy-1; top++; }
        if (cy < w->height - 1) { stack[top][0]=cx;   stack[top][1]=cy+1; top++; }
    }
}
