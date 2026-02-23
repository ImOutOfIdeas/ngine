#ifndef MAP_RENDER_H
#define MAP_RENDER_H

#include "world.h"

void map_render_canvas (const World *w);
void map_render_cursor (int x, int y);
void map_render_palette(TileType cur_tile);
void map_render_help   (const char *status);

#endif
