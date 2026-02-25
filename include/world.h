#ifndef WORLD_H
#define WORLD_H

#include <ncurses.h>

//  TILE DEFINITIONS
//  X(id, name, fg, bg, solid, passable)
#define TILE_LIST \
    X(TILE_EMPTY,  "Empty",  COLOR_WHITE,   COLOR_BLACK,     0, 1) \
    X(TILE_WALL,   "Wall",   COLOR_BLACK,   COLOR_MAGENTA,   1, 0) \
    X(TILE_FLOOR,  "Floor",  COLOR_BLACK,   COLOR_WHITE,     1, 1) \
    X(TILE_DOOR,   "Door",   COLOR_BLACK,   COLOR_MAGENTA,   1, 0) \

typedef enum {
    #define X(id, name, fg, bg, solid, passable) id,
    TILE_LIST
    #undef X
    NUM_TILE_TYPES
} TileType;

typedef struct {
    const char *name;
    int         color_pair;
    int         solid;
    int         passable;
} TileDef;

extern TileDef tile_defs[NUM_TILE_TYPES];

//  ENTITY DEFINITIONS
//  Entities render as single wide-char glyphs layered on top of tiles.
//
//  X(id, name, glyph, basic_fg, r, g, b)
#define ENTITY_LIST \
    X(ENTITY_NONE,    "None",    "  ", COLOR_WHITE,   100, 100, 100) \
    X(ENTITY_PLAYER,  "Player",  "@@", COLOR_GREEN,   106, 255, 140) \
    X(ENTITY_ENEMY,   "Enemy",   "!!", COLOR_RED,     255, 100,  90) \
    X(ENTITY_ITEM,    "Item",    "**", COLOR_YELLOW,  255, 215,   0) \
    X(ENTITY_TRIGGER, "Trigger", "^^", COLOR_CYAN,    100, 220, 255) \
    X(ENTITY_BUTTON,  "Button",  "()", COLOR_YELLOW,  229, 192, 123) \
    X(ENTITY_SPAWN,   "Spawn",   "<>", COLOR_GREEN,   152, 195, 121) \
    X(ENTITY_GOAL,    "Goal",    "><", COLOR_CYAN,     86, 182, 194) \

typedef enum {

    #define X(id, name, glyph, basic_fg, r, g, b) id,
    ENTITY_LIST
    #undef X
    NUM_ENTITY_TYPES
} EntityType;

typedef struct {
    const char *name;
    const char *glyph;
    int         color_pair; // assigned at runtime — transparent background
} EntityDef;

extern EntityDef entity_defs[NUM_ENTITY_TYPES];

//  Entity
#define MAX_ENTITIES 256

typedef struct {
    int        x, y;
    EntityType type;
    int        param;   // e.g. door ID a trigger activates
    int        active;  // 0 = slot unused
} Entity;

//  PaintMode  shared between editor and map_renderer
typedef enum {
    PAINT_TILE,
    PAINT_ENTITY,
} PaintMode;

//  World
#define MAP_WIDTH  50
#define MAP_HEIGHT 25

typedef struct {
    TileType tiles[MAP_HEIGHT][MAP_WIDTH];
    WINDOW  *map_window;
    WINDOW  *status_window;
    Entity   entities[MAX_ENTITIES];
    int      entity_count;
    int      width, height;
} World;

//  API
void      world_init        (World *w);
void      world_setup_colors(void);
TileType  world_get_tile    (const World *w, int x, int y);
void      world_set_tile    (World *w, int x, int y, TileType t);
int       world_save        (const World *w, const char *path);
int       world_load        (World *w, const char *path);
void      world_flood_fill  (World *w, int x, int y, TileType replace, TileType target);

// Entity helpers
Entity     *world_find_entity     (World *w, int x, int y);
const char *world_place_entity    (World *w, int x, int y, EntityType type);
int         world_entity_blend_pair(EntityType e, TileType t);
void      world_remove_entity(World *w, int x, int y);

#endif
