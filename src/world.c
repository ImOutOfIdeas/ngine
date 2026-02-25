#include "world.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
//  tile_defs[] — built from TILE_LIST at compile time
// ---------------------------------------------------------------------------
TileDef tile_defs[NUM_TILE_TYPES] = {
    #define X(id, name, fg, bg, solid, passable) \
    [id] = { name, 0, solid, passable },
    TILE_LIST
    #undef X
};

// ---------------------------------------------------------------------------
//  entity_defs[] — built from ENTITY_LIST at compile time
// ---------------------------------------------------------------------------
EntityDef entity_defs[NUM_ENTITY_TYPES] = {
    #define X(id, name, glyph, basic_fg, r, g, b) \
    [id] = { name, glyph, 0 },
    ENTITY_LIST
    #undef X
};

// ---------------------------------------------------------------------------
//  Color support detection
// ---------------------------------------------------------------------------
typedef enum {
    COLOR_SUPPORT_BASIC,
    COLOR_SUPPORT_256,
    COLOR_SUPPORT_TRUE,
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

// ---------------------------------------------------------------------------
//  Tile color tables  (indexed by TileType enum value)
//  Since tiles render as solid "  " blocks, bg is the visible tile color.
//  fg is used as the entity glyph foreground when an entity sits on this tile.
// ---------------------------------------------------------------------------

// 256-color approximations
static const int tile_fg_256[] = { 59,  244, 250, 179, 176, 108,  73 };
static const int tile_bg_256[] = { 234, 238, 236,  94,  55,  22,  23 };

// True-color tile palette  (One Dark–inspired)
//                              R    G    B
static const int tile_fg_rgb[][3] = {
    {  92,  99, 112 },  // EMPTY  — dim fg for entity contrast
    { 171, 178, 191 },  // WALL   — light fg
    { 171, 178, 191 },  // FLOOR  — light fg
    { 229, 192, 123 },  // BUTTON — amber fg
    { 198, 120, 221 },  // DOOR   — purple fg
    { 152, 195, 121 },  // SPAWN  — green fg
    {  86, 182, 194 },  // GOAL   — cyan fg
};
static const int tile_bg_rgb[][3] = {
    {  28,  30,  35 },  // EMPTY  — near-black void
    {  62,  68,  81 },  // WALL   — One Dark selection grey
    {  48,  52,  64 },  // FLOOR  — slightly lighter than empty
    {  90,  68,  18 },  // BUTTON — deep amber
    {  68,  32,  88 },  // DOOR   — deep purple
    {  32,  72,  40 },  // SPAWN  — deep green
    {  18,  62,  70 },  // GOAL   — deep cyan
};

// ---------------------------------------------------------------------------
//  Entity foreground colors  (indexed by EntityType enum value)
// ---------------------------------------------------------------------------

static const int entity_fg_256[] = {
    // NONE   PLAYER  ENEMY  ITEM  TRIGGER  NPC
       240,    46,    196,   220,   51,     183
};

static const int entity_fg_rgb[][3] = {
    { 100, 100, 100 },  // NONE    — dim, eraser
    { 106, 255, 140 },  // PLAYER  — bright green
    { 255, 100,  90 },  // ENEMY   — red
    { 255, 215,   0 },  // ITEM    — gold
    { 100, 220, 255 },  // TRIGGER — cyan
    { 200, 180, 255 },  // NPC     — lavender
};

//  Extended color IDs (true-color):
//  TILE_COLOR_BASE + t*2     = tile t fg
//  TILE_COLOR_BASE + t*2 + 1 = tile t bg
//  ENT_COLOR_BASE  + e       = entity e fg
#define TILE_COLOR_BASE  16
#define ENT_COLOR_BASE   (TILE_COLOR_BASE + NUM_TILE_TYPES * 2)
#define BLEND_BASE       (NUM_TILE_TYPES + NUM_ENTITY_TYPES + 1)

static int s_blend_pair[NUM_ENTITY_TYPES][NUM_TILE_TYPES];

int world_entity_blend_pair(EntityType e, TileType t) {
    return s_blend_pair[e][t];
}

static void setup_tile_colors(void) {
    for (int t = 0; t < NUM_TILE_TYPES; t++) {
        int pair_id = t + 1;
        if (g_color_support == COLOR_SUPPORT_TRUE) {
            int fg_id = TILE_COLOR_BASE + t * 2;
            int bg_id = TILE_COLOR_BASE + t * 2 + 1;
            init_extended_color(fg_id,
                tile_fg_rgb[t][0] * 1000/255,
                tile_fg_rgb[t][1] * 1000/255,
                tile_fg_rgb[t][2] * 1000/255);
            init_extended_color(bg_id,
                tile_bg_rgb[t][0] * 1000/255,
                tile_bg_rgb[t][1] * 1000/255,
                tile_bg_rgb[t][2] * 1000/255);
            init_extended_pair(pair_id, fg_id, bg_id);
        } else if (g_color_support == COLOR_SUPPORT_256) {
            init_pair(pair_id, tile_fg_256[t], tile_bg_256[t]);
        } else {
            // basic fallback — pull fg/bg from TILE_LIST
            #define X(id, name, fg, bg, solid, passable) \
                if (t == id) init_pair(pair_id, fg, bg);
            TILE_LIST
            #undef X
        }
        tile_defs[t].color_pair = pair_id;
    }
}

static void setup_entity_colors(void) {
    // Palette display pairs: entity fg on TILE_EMPTY background
    for (int e = 0; e < NUM_ENTITY_TYPES; e++) {
        int pair_id = NUM_TILE_TYPES + 1 + e;
        if (g_color_support == COLOR_SUPPORT_TRUE) {
            int fg_id = ENT_COLOR_BASE + e;
            int bg_id = TILE_COLOR_BASE + TILE_EMPTY * 2 + 1;  // EMPTY bg
            init_extended_color(fg_id,
                entity_fg_rgb[e][0] * 1000/255,
                entity_fg_rgb[e][1] * 1000/255,
                entity_fg_rgb[e][2] * 1000/255);
            init_extended_pair(pair_id, fg_id, bg_id);
        } else if (g_color_support == COLOR_SUPPORT_256) {
            init_pair(pair_id, entity_fg_256[e], tile_bg_256[TILE_EMPTY]);
        } else {
            init_pair(pair_id, COLOR_WHITE, COLOR_BLACK);
        }
        entity_defs[e].color_pair = pair_id;
    }

    // Blended pairs: entity fg × tile bg — one per (entity, tile) combo
    for (int e = 0; e < NUM_ENTITY_TYPES; e++) {
        for (int t = 0; t < NUM_TILE_TYPES; t++) {
            int pair_id = BLEND_BASE + e * NUM_TILE_TYPES + t;
            if (g_color_support == COLOR_SUPPORT_TRUE) {
                int fg_id = ENT_COLOR_BASE + e;
                int bg_id = TILE_COLOR_BASE + t * 2 + 1;
                init_extended_pair(pair_id, fg_id, bg_id);
            } else if (g_color_support == COLOR_SUPPORT_256) {
                init_pair(pair_id, entity_fg_256[e], tile_bg_256[t]);
            } else {
                #define X(id, name, fg, bg, solid, passable) \
                    if (t == id) init_pair(pair_id, COLOR_WHITE, bg);
                TILE_LIST
                #undef X
            }
            s_blend_pair[e][t] = pair_id;
        }
    }
}

void world_setup_colors(void) {
    g_color_support = detect_color_support();
    setup_tile_colors();
    setup_entity_colors();
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
    w->status_window = newwin(6, MAP_WIDTH * 2 + 2, MAP_HEIGHT + 2, 0);

    keypad(w->map_window, TRUE);
    keypad(w->status_window, TRUE);

    box(w->map_window, 0, 0);
    box(w->status_window, 0, 0);
}

//  Tile accessors
TileType world_get_tile(const World *w, int x, int y) {
    if (x < 0 || x >= w->width || y < 0 || y >= w->height)
        return TILE_WALL;
    return w->tiles[y][x];
}

void world_set_tile(World *w, int x, int y, TileType t) {
    if (x < 0 || x >= w->width || y < 0 || y >= w->height) return;
    w->tiles[y][x] = t;
}

//  Entity helpers
Entity *world_find_entity(World *w, int x, int y) {
    for (int i = 0; i < w->entity_count; i++) {
        Entity *e = &w->entities[i];
        if (e->active && e->x == x && e->y == y)
            return e;
    }
    return NULL;
}

static void entity_erase(World *w, int x, int y) {
    Entity *e = world_find_entity(w, x, y);
    if (e) e->active = 0;
}

static Entity *entity_write(World *w, int x, int y, EntityType type) {
    Entity *e = world_find_entity(w, x, y);
    if (!e) {
        for (int i = 0; i < w->entity_count; i++)
            if (!w->entities[i].active) { e = &w->entities[i]; break; }
        if (!e) {
            if (w->entity_count >= MAX_ENTITIES) return NULL;
            e = &w->entities[w->entity_count++];
        }
    }
    *e = (Entity){ x, y, type, 0, 1 };
    return e;
}

// Placement rules — returns NULL on success, error string on violation.
const char *world_place_entity(World *w, int x, int y, EntityType type) {
    if (type == ENTITY_NONE) {
        entity_erase(w, x, y);
        return NULL;
    }

    TileType tile = world_get_tile(w, x, y);

    if (tile == TILE_EMPTY)
        return "can't place entity on empty tile";

    if (type == ENTITY_BUTTON && tile != TILE_WALL)
        return "button must be placed on a wall tile";

    if ((type == ENTITY_SPAWN || type == ENTITY_GOAL) && tile != TILE_FLOOR)
        return "spawn and goal must be placed on a floor tile";

    if (type == ENTITY_TRIGGER && tile != TILE_FLOOR)
        return "trigger must be placed on a floor tile";

    if (!entity_write(w, x, y, type))
        return "entity limit reached";

    return NULL;
}

//  Save / load
int world_save(const World *w, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return 0;

    fprintf(f, "%d %d\n", w->width, w->height);
    for (int y = 0; y < w->height; y++) {
        for (int x = 0; x < w->width; x++)
            fprintf(f, "%d%c", (int)w->tiles[y][x],
                    x == w->width - 1 ? '\n' : ' ');
    }

    // Count active entities
    int active = 0;
    for (int i = 0; i < w->entity_count; i++)
        if (w->entities[i].active) active++;

    fprintf(f, "%d\n", active);
    for (int i = 0; i < w->entity_count; i++) {
        const Entity *e = &w->entities[i];
        if (!e->active) continue;
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
            int t; fscanf(f, "%d", &t);
            w->tiles[y][x] = (TileType)t;
        }

    w->entity_count = 0;
    memset(w->entities, 0, sizeof(w->entities));

    int count = 0;
    fscanf(f, "%d\n", &count);
    for (int i = 0; i < count && i < MAX_ENTITIES; i++) {
        Entity *e = &w->entities[w->entity_count++];
        int type;
        fscanf(f, "%d %d %d %d\n", &e->x, &e->y, &type, &e->param);
        e->type   = (EntityType)type;
        e->active = 1;
    }

    fclose(f);
    return 1;
}

//  Flood fill
void world_flood_fill(World *w, int x, int y, TileType replace, TileType target) {
    if (replace == target) return;
    if (world_get_tile(w, x, y) != target) return;

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

        if (cx > 0)             { stack[top][0]=cx-1; stack[top][1]=cy;   top++; }
        if (cx < w->width - 1)  { stack[top][0]=cx+1; stack[top][1]=cy;   top++; }
        if (cy > 0)             { stack[top][0]=cx;   stack[top][1]=cy-1; top++; }
        if (cy < w->height - 1) { stack[top][0]=cx;   stack[top][1]=cy+1; top++; }
    }
}
