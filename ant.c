
// Configuration
#define DELAY .0005
#define SECS_USEC 1000000
#define BORDER .8
int WIDTH = 80; //TODO: Get terminal size
int HEIGHT = 40; //NOTE: I never pay attention to TODO crap >_>
int NEST_SIZE = 2;
int SEED = 0;
int DISABLE_FALLEN = 1;



#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <assert.h>


int simulate = 1;
char *ant_symbol[] = {"↑", "↓", "→", "←"};

typedef struct {
  unsigned int x, y;
  enum {NORTH = 0, SOUTH, EAST, WEST} angle;
  char active;
} s_ant;

typedef struct {
  int width, height;
  char *data;
} s_plane;

void delay(float time) {
  useconds_t utime = time*SECS_USEC;
  usleep(utime);
}

s_plane *new_plane(int width, int height) {
  s_plane *ret = malloc(sizeof(s_plane));
  ret->width = width;
  ret->height = height;
  ret->data = calloc(sizeof(char), width*height);
}

inline char *get_cell(s_plane *plane, int x, int y) {
  assert(x >= 0);
  assert(y >= 0);
  assert(x < plane->width);
  assert(y < plane->height);
  return &plane->data[x+y*plane->width];
}

void cursor_set(int x, int y) {
  assert(x >= 0);
  assert(y >= 0);
  printf("\x1b[%i;%iH", y, x);
}

void print_cell(char cell) {
  if (cell) {
    printf("\x1b[7m");
  }
  printf(" ");
  if (cell) {
    printf("\x1b[0m");
  }
}

void draw_ant(s_ant *ant) {
  cursor_set(1+ant->x, 1+ant->y);
  printf("\x1b[31m"); //red
  printf("%s", ant_symbol[ant->angle]);
  printf("\x1b[0m"); //normal
  printf("\n");
}

void clear_ant(s_plane *plane, s_ant *ant) {
  int dx, dy;
  int x = ant->x, y = ant->y;
  for (dy = -1; dy < 2; dy++) {
    for (dx = -1; dx < 2; dx++) {
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


void draw_plane(s_plane *plane) {
  //this is horribly inefficient
  //I don't care
  printf("\x1b[H"); //cursor home
  int x, y;
  for (y = 0; y != plane->height; y++){
    for (x = 0; x != plane->width; x++) {
      print_cell(*get_cell(plane, x, y));
    }
    printf("\n"); //next line
  }
}


void step(s_plane *plane, s_ant *ant) {
  /*
  If white: Turn right
  Else: Turn left
  Flip color
  Move forward
  */
  char *here = get_cell(plane, ant->x, ant->y);
  //turn
  if (*here) {
    //white; right
    switch (ant->angle) {
      case NORTH:
        ant->angle = EAST;
        break;
      case EAST:
        ant->angle = SOUTH;
        break;
      case SOUTH:
        ant->angle = WEST;
        break;
      case WEST:
        ant->angle = NORTH;
        break;
      default:
        printf("Ant not pointing in a known direction.\n");
        exit(EXIT_FAILURE);
    }
  }
  else {
    //black; left
    switch (ant->angle) {
      case NORTH:
        ant->angle = WEST;
        break;
      case EAST:
        ant->angle = NORTH;
        break;
      case SOUTH:
        ant->angle = EAST;
        break;
      case WEST:
        ant->angle = SOUTH;
        break;
      default:
        printf("Ant not pointing in a known direction.\n");
        exit(EXIT_FAILURE);
    }
  }
  //flip
  *here = !(*here);
  //move
  switch (ant->angle) {
      case NORTH:
        ant->y--;
        break;
      case EAST:
        ant->x++;
        break;
      case SOUTH:
        ant->y++;
        break;
      case WEST:
        ant->x--;
        break;
      default:
        printf("Ant not pointing in a known direction.\n");
        exit(EXIT_FAILURE);
  }
}

void cursor_to_end() {
  cursor_set(0, HEIGHT);
}

void show_cursor() {
  printf("\x1b[?25h"); //cursor show
}


void handle_sig(int signum) {
  simulate = 0;
}

void init_terminal() {
  //crap for nice output
  setvbuf(stdin, (char *) NULL, _IONBF, 0); //printf flushes immediately
  printf("\x1b[2J"); //clear screen
  printf("\x1b[?25l"); //cursor hide
  atexit(show_cursor);
  signal(SIGINT, handle_sig);
}

float frandom() {
  return ((float)random()) / ((float)RAND_MAX);
}

int main() {
  init_terminal();
  //init
  s_plane *plane = new_plane(WIDTH, HEIGHT);

  if (SEED == 0) {
    SEED = time();
  }
  srandom(SEED);
  s_ant nest[NEST_SIZE];
  int i;
  for (i = 0; i != NEST_SIZE; i++) {
    #define NBORDER (1.0-BORDER)
    nest[i].x = (frandom()*WIDTH*NBORDER)+WIDTH*BORDER*.5;
    nest[i].y = (frandom()*HEIGHT*NBORDER)+HEIGHT*BORDER*.5;
    nest[i].angle = random() % 4;
    nest[i].active = 1;
  }

  //simulate
  int ant_fell = 0;
  int dead_ants = 0;
  unsigned long int steps_run = 0;
  while (simulate && dead_ants != NEST_SIZE) {
    #define ACTIVE if (!nest[i].active) continue
    //check ant bounding
    for (i = 0; i != NEST_SIZE; i++) {
      ACTIVE;
      s_ant ant = nest[i];
      if (!((ant.x && ant.y) && ((ant.x < plane->width) && (ant.y < plane->height)))) {
        if (DISABLE_FALLEN) {
          nest[i].active = 0;
          dead_ants++;
        }
        else {
          simulate = 0;
          ant_fell = 1;
        }
      }
    }
    if (!simulate) break;

    for (i = 0; i != NEST_SIZE; i++) {
      ACTIVE;
      draw_ant(&nest[i]);
    }
    delay(DELAY);

    for (i = 0; i != NEST_SIZE; i++) {
      ACTIVE;
      step(plane, &nest[i]);
    }
    steps_run++;

    for (i = 0; i != NEST_SIZE; i++) {
      ACTIVE;
      clear_ant(plane, &nest[i]);
    }
    for (i = 0; i != NEST_SIZE; i++) {
      ACTIVE;
      draw_ant(&nest[i]);
    }
    delay(DELAY);
  }
  free(plane);
  cursor_to_end();

  if (ant_fell) printf("\r\nAn ant has wandered off the map.\n");
  if (dead_ants == NEST_SIZE) printf("\r\nAll the ants have fallen off the map.\n");
  printf("Seed: %i\n", SEED);
  printf("Size: %ix%i\n", WIDTH, HEIGHT);
  printf("Nest size: %i\n", NEST_SIZE);
  printf("Simulation time: %li steps\n", steps_run);
  return 0;
}

/*
**THESE SEEDS ARE FROM THE FIRST COMMIT**
(no way to input seeds ATM)
Possibly stable pulsar:
Seed: 1289021114
Size: 280x90
Nest size: 2

Hella cool:
Seed: 1289021504
Size: 280x90
Nest size: 2

Incredible behavior:
Seed: 1289021560
Size: 280x90
Nest size: 2

*/
