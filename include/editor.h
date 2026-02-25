#ifndef EDITOR_H
#define EDITOR_H

#include "world.h"

typedef struct {
    int      cur_x, cur_y;
    TileType cur_tile;
    char     status[256];
} EditorState;

void editor_init        (EditorState *e);
void editor_render      (const EditorState *e, const World *w);

// Returns 0 if the editor requests quit, 1 otherwise
int  editor_handle_input(EditorState *e, World *w, int ch);

void prompt_filename(WINDOW *win, const char *prompt, char *out, int maxlen);

#endif
