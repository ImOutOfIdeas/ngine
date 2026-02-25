#include <ncurses.h>
#include "world.h"
#include "editor.h"

int main(void) {
    initscr();
    start_color();
    use_default_colors();   // enables transparent background in color pairs
    noecho();
    cbreak();
    curs_set(0);

    world_setup_colors();

    World       world;
    EditorState editor;

    world_init(&world);
    editor_init(&editor);

    while (1) {
        editor_render(&editor, &world);
        doupdate();

        int ch = wgetch(world.map_window);
        if (!editor_handle_input(&editor, &world, ch))
            break;
    }

    endwin();
    return 0;
}
