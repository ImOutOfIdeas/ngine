#ifndef MAP_RENDER_H
#define MAP_RENDER_H

#include "world.h"

void map_render_tile   (WINDOW *win, int y, int x, TileType t);
void map_render_canvas (const World *w);
void map_render_cursor (WINDOW *win, int x, int y);
void map_render_palette(WINDOW *win, TileType cur_tile);
void map_render_status (WINDOW *win, const char *status);
void map_render_info   (int x, int y);

#endif
