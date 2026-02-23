#ifndef WORLD_H
#define WORLD_H

#include <ncurses.h>

//  TILE DEFINITIONS
//  X(id, name, glyph, fg, bg, solid, passable)
//  Colors are used as fallback for basic color support
#define TILE_LIST \
    X(TILE_EMPTY,  "Empty",  "  ", COLOR_WHITE,   COLOR_BLACK,   0, 1) \
    X(TILE_WALL,   "Wall",   "  ", COLOR_BLACK,   COLOR_MAGENTA,   1, 0) \
    X(TILE_FLOOR,  "Floor",  "  ", COLOR_BLACK,   COLOR_WHITE,   1, 1) \
    X(TILE_BUTTON, "Button", "()", COLOR_BLACK,   COLOR_YELLOW,  0, 1) \
    X(TILE_DOOR,   "Door",   "||", COLOR_BLACK,   COLOR_MAGENTA, 1, 0) \
    X(TILE_SPAWN,  "Spawn",  "@ ", COLOR_BLACK,   COLOR_GREEN,   0, 1) \
    X(TILE_GOAL,   "Goal",   "><", COLOR_BLACK,   COLOR_CYAN,    0, 1)

//  TileType enum — generated from TILE_LIST
typedef enum {
    #define X(id, name, glyph, fg, bg, solid, passable) id,
    TILE_LIST
    #undef X
    NUM_TILE_TYPES
} TileType;

//  TileDef — per-tile display and gameplay data
typedef struct {
    const char *name;
    const char *glyph;
    int         color_pair; // ncurses color pair index (assigned at setup)
    int         solid;
    int         passable;
} TileDef;

extern TileDef tile_defs[NUM_TILE_TYPES];

//  Entity — interactive objects with parameters
#define MAX_ENTITIES 256

typedef struct {
    int      x, y;
    TileType type;
    int      param;  // e.g. door ID that a button triggers
    int      active; // 0 = slot unused
} Entity;

//  World — the shared source of truth
#define MAP_WIDTH 40
#define MAP_HEIGHT 20

typedef struct {
    TileType tiles[MAP_HEIGHT][MAP_WIDTH];
    Entity   entities[MAX_ENTITIES];
    int      entity_count;
    int      width, height;
} World;

//  API
void      world_init(World *w);
void      world_setup_colors(void);   // call after start_color()
TileType  world_get_tile(const World *w, int x, int y);
void      world_set_tile(World *w, int x, int y, TileType t);
void      world_draw_tile(int y, int x, TileType t);
int       world_save(const World *w, const char *path);
int       world_load(World *w, const char *path);
void      world_flood_fill(World *w, int x, int y, TileType replace, TileType target);

#endif
