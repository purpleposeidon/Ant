
#include "draw.h"
#include "ant.h"

char *ant_symbol[] = {"↑", "↓", "→", "←"};



void show_cursor() {
  printf("\x1b[?25h"); //cursor show
}

void hide_cursor() {
  printf("\x1b[?25l"); //hide cursor
}

void clear_screen() {
  printf("\x1b[2J"); //clear screen
}

void cursor_set(int x, int y) {
  assert(x >= 0);
  assert(y >= 0);
  printf("\x1b[%i;%iH", y, x);
}

void cursor_home() {
  printf("\x1b[H"); //cursor home
}

//////////////////////////////////////


void handle_sig(int signum) {
  if (simulate) {
    simulate = 0;
    LOOP = 0; //otherwise it loops forever
  }
  else {
    //seriously!
    exit(EXIT_FAILURE);
  }
}

void init_terminal() {
  //crap for nice output
  setvbuf(stdin, (char *) NULL, _IONBF, 0); //printf flushes immediately
  //XXX It's supposed to, anyways. It doesn't actually.
  clear_screen();
  hide_cursor();
  atexit(show_cursor);
  signal(SIGINT, handle_sig);
}


void set_size(int *wid, int *hei) {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  *wid = w.ws_col - 2;
  *hei = w.ws_row - 2;
}


//////////////////////////////////////


void delay(float time) {
  useconds_t utime = time*SECS_USEC;
  usleep(utime);
}


//////////////////////////////////////

void print_cell(char cell) {
  if (cell) {
    printf("\x1b[7m"); //reverse
  }
  #if DRAW_DOTS
  printf(".");
  #else
  printf(" ");
  #endif
  if (cell) {
    printf("\x1b[0m"); //normal
  }
}

void draw_plane(s_plane *plane) {
  //this is pretty inefficient
  cursor_home();
  int x, y;
  for (y = 0; y != plane->height; y++){
    for (x = 0; x != plane->width; x++) {
      char cell = *get_cell(plane, x, y);
      print_cell(cell);
    }
    printf("\n"); //next line
  }
}

void draw_ant(s_ant *ant) {
  cursor_set(1+ant->x, 1+ant->y);
  printf("\x1b[31m"); //red
  if (ant->active) {
    printf("\x1b[1m"); //bold
    printf("%s", ant_symbol[ant->angle]);
  }
  else {
    printf("a");
  }
  printf("\x1b[0m"); //normal
  printf("\n");
}

void clear_ant(s_plane *plane, s_ant *ant) {
  //redraws the area around an ant (but not the ant itself)
  int dx, dy;
  int x = ant->x, y = ant->y;
  for (dy = -1; dy < 2; dy++) {
    for (dx = -1; dx < 2; dx++) {
      //XXX This is weird. Uhhhhh
      //if (x && y) continue; //don't need diagonals
      //if (!(!!x ^ !!y)) continue;
      //if (x == 1 && y == 0) continue;
      //if (!(x && y)) continue;
      if (x == y && x == 1) continue;
      if (!(
        (y + dy >= 0 && x + dx >= 0) && (y + dy < plane->height && x + dx < plane->width)
        )) continue;
      if (dx == -1) {
        cursor_set(1+x+dx, 1+y+dy);
      }
      char cell = *get_cell(plane, dx+x, dy+y);
      print_cell(cell);
    }
  }
  printf("\n");
}









