#include "world.h"

#include <stdio.h>
#include <string.h>

//  tile_defs[] — built from TILE_LIST at compile time
//  color_pair is filled in at runtime by world_setup_colors()
TileDef tile_defs[NUM_TILE_TYPES] = {
    #define X(id, name, glyph, fg, bg, solid, passable) \
    [id] = { name, glyph, 0, solid, passable },
    TILE_LIST
    #undef X
};

//  Color pair IDs — also generated from TILE_LIST
//  We use (enum value + 1) as the pair ID so pair 0 is never used
void world_setup_colors(void) {
    #define X(id, name, glyph, fg, bg, solid, passable) \
    init_pair(id + 1, fg, bg);                       \
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
}

//  Tile accessors
TileType world_get_tile(const World *w, int x, int y) {
    if (x < 0 || x >= w->width || y < 0 || y >= w->height)
        return TILE_WALL; // out of bounds treated as solid
    return w->tiles[y][x];
}

void world_set_tile(World *w, int x, int y, TileType t) {
    if (x < 0 || x >= w->width || y < 0 || y >= w->height) return;
    w->tiles[y][x] = t;
}

void world_draw_tile(int y, int x, TileType t) {
    TileDef *def = &tile_defs[t];
    attron(COLOR_PAIR(def->color_pair));
    mvprintw(y, x * 2, "%s", def->glyph);
    attroff(COLOR_PAIR(def->color_pair));
}

//  Save / load  (plain text format)
//
//  Line 1:  width height
//  Lines 2..height+1: space-separated tile IDs
//  Then:    entity_count
//  Then:    one "x y type param" per entity
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
    if (!f) {
        return 0;
    }

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
