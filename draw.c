
#include "draw.h"
#include "ant.h"

char *ant_symbol[] = {"↑", "↓", "→", "←"};
#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)
#define ATTR(x) "\x1b[" QUOTE(x) "m"
#define RGB(x) "\x1b[38;5;" QUOTE(x) "m" "\x1b[48;5;" QUOTE(x) "m"
char *color2escape[MAX_INSTRUCTIONS] = {
  //strings to output for each color
  //basic colors
  ATTR(30) "#",
  "\x1b[38;5;255m" "\x1b[48;5;255m" ".", //bright white; uses gray colors
  ATTR(31) ATTR(41) "R",
  ATTR(32) ATTR(42) "G",
  ATTR(33) ATTR(43) "O",
  ATTR(34) ATTR(44) "b",
  ATTR(35) ATTR(45) "M",
  ATTR(36) ATTR(46) "t",
  //rgb colors
  RGB(75)  "1",
  RGB(215) "2",
  RGB(205) "3",
  RGB(135) "4",
  RGB(67)  "5",
  RGB(97)  "6",
  RGB(137) "7",
  RGB(72), "8",
  
};

/*
char color2escape[MAX_COLORS] = {37,    30,    32,    34,   31,  36,   35,     33};
char colors[] = {WHITE, BLACK, RED, GREEN, BROWN, BLUE, PURPLE, CYAN, '\0'}; //'WHITE' is actually gray.
*/


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

void normal() {
  printf("\x1b[0m"); //normal
}

void clear_line() {
  //this doesn't actually work? Huh?
  printf("\x1b[2K\r\n");
  printf("\x1b[1A"); //cursor up
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
  char *esc = color2escape[(int)cell];
  if (esc == NULL) {
    printf("%c", cell+32);
  }
  else {
    printf("%s", esc);
    normal();
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
  normal();
  printf("\x1b[31m"); //red
  if (ant->active) {
    printf("\x1b[1m"); //bold
    printf("%s", ant_symbol[ant->angle]);
  }
  else {
    printf("a");
  }
  normal();
  printf("\n");
}

void clear_ant(s_plane *plane, s_ant *ant) {
  //redraws the area around an ant (but not the ant itself)
  int dx, dy;
  int x = ant->x, y = ant->y;
  for (dy = -1; dy < 2; dy++) {
    for (dx = -1; dx < 2; dx++) {
      //XXX This is weird. Uhhhhh
      /*if (!(dx ^ dy )) {
        //don't need diagonals
        continue;
      }*/
      
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









