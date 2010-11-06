#ifndef DRAW_H
#define DRAW_H

#include "ant.h"

void init_terminal();
void set_size(int *w, int *h);


void clear_screen();
void cursor_home();
void cursor_to_end();
void cursor_set(int x, int y);
#define cursor_to_end() cursor_set(1, HEIGHT)

void delay(float time);



void draw_plane(s_plane *plane);
void draw_ant(s_ant *ant);
void clear_ant(s_plane *plane, s_ant *ant);




#endif /* DRAW_H */