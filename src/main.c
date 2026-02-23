#include <ncurses.h>
#include "world.h"
#include "editor.h"

int main(void) {
    initscr();
    start_color();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);

    world_setup_colors();

    World       world;
    EditorState editor;

    world_init(&world);
    editor_init(&editor);

    while (1) {
        editor_render(&editor, &world);
        refresh();

        int ch = getch();
        if (!editor_handle_input(&editor, &world, ch))
            break;
    }

    endwin();
    return 0;
}
