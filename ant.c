
// Configuration
float DELAY = .0005;
float BORDER = .8;
int WIDTH = 80; //TODO: Get terminal size
int HEIGHT = 40; //NOTE: I never pay attention to TODO crap >_>
int NEST_SIZE = 2;
int SEED = 0;
int USE_TIME = 1;
int DISABLE_FALLEN = 1;
int LOOP = 0;


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <assert.h>
#include <time.h>

#define SECS_USEC 1000000

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
  return ret;
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
    printf("\x1b[7m"); //reverse
  }
  printf(" ");
  if (cell) {
    printf("\x1b[0m"); //normal
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
  //(Besides, it isn't used anymore)
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
    //White on right. Queen on color.
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
        fprintf(stderr, "Ant not pointing in a known direction.\n");
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
        fprintf(stderr, "Ant not pointing in a known direction.\n");
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
        fprintf(stderr, "Ant not pointing in a known direction.\n");
        exit(EXIT_FAILURE);
  }
}

void cursor_to_end() {
  cursor_set(1, HEIGHT);
}

void show_cursor() {
  printf("\x1b[?25h"); //cursor show
}

void clear_screen() {
  printf("\x1b[2J"); //clear screen
}

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
  printf("\x1b[?25l"); //cursor hide
  atexit(show_cursor);
  signal(SIGINT, handle_sig);
}

float frandom() {
  //returns a random float in [0.0, 1.0] probably
  return ((float)random()) / ((float)RAND_MAX);
}

void parse_args(int argc, char **argv) {
  char usage[] =
  "Usage: \n" \
  "  ant [-a ANTCOUNT] [-w WIDTH] [-h HEIGHT] [-l] [-e] [-d DELAY] [-s SEED] [-b BORDER]\n" \
  "-a   Sets how many ants to use\n" \
  "-w   Sets the map width\n" \
  "-h   Sets the map height\n" \
  "-l   Loop the simulation\n" \
  "-e   Ends the simulation if an ant touches the edge (default = no)\n" \
  "-d   Delay, in seconds (default = .0005)\n" \
  "-s   Sets the seed for ant placement (default = current time)\n" \
  "-b   Sets the border for random ant placement (default = 0.8)\n" \
  ;
#define USE_FAIL do { \
  fprintf(stderr, "%s\n", usage); \
  exit(EXIT_FAILURE); } while (0)
#define CNV_ARG(var, func) do { \
  if (optarg) { \
    var = func(optarg); \
  } \
  else { \
    fprintf(stderr, "-%c expected argument.\n", c); \
    USE_FAIL; \
  } \
} while (0)
  opterr = 1;
  char c;
  while ((c = getopt (argc, argv, "a:w:h:led:s:b:")) != -1) {
    switch (c) {
      case 'a':
        CNV_ARG(NEST_SIZE, atoi);
        break;
      case 'w':
        CNV_ARG(WIDTH, atoi);
        break;
      case 'h':
        CNV_ARG(HEIGHT, atoi);
        break;
      case 'l':
        LOOP = 1;
        break;
      case 'e':
        DISABLE_FALLEN = 0;
        break;
      case 'd':
        CNV_ARG(DELAY, atof);
        break;
      case 's':
        CNV_ARG(SEED, atoi);
        USE_TIME = 0;
        break;
      case 'b':
        CNV_ARG(BORDER, atof);
        break;
      case '?':
        USE_FAIL;
        abort();
        break;
      default:
        abort();
        break;
    }
  }
}

void run_simulation(s_plane *plane, s_ant *nest) {
  //simulate
  int i;
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
      //ACTIVE;
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
  cursor_to_end();

  if (1 == NEST_SIZE && dead_ants == 1) {
    if (frandom() > .95) {
      fprintf(stderr, "The soldier ant bites! You die...\n");
    }
    else {
      char *names[] = {"Alice", "Albert", "Alyssa", "Alex", "Al"};
      int names_len = 5;
      char *name = names[(int)(names_len*frandom())];
      char *title;
      if (steps_run > 9000) {
        title = "Amazing ";
      }
      else if (steps_run > 100000) {
        title = "Awesome ";
      }
      else if (steps_run == 69) {
        title = "Awkward ";
      }
      else {
        title = "";
      }
      char *actions[] = {"has moved on to greener pastures.",
      "has given up on you.",
      "has decided to take a vacation.",
      "dislikes your face.",
      "is sick and tired of your carp.",
      "has fallen off the terminal.",
      "has fallen off a cliff!",
      "is going back to Hex.",
      "is going on an antventure.",
      "has an interesting antecdotes to share with ver friends.",
      "has misplaced the antidote.",
      "fears the anteater."};
      int action_length = 11; //this really isn't sustainable
      char *action = actions[(int)(action_length*frandom())];
      fprintf(stderr, "%s the %sAnt %s\n", name, title, action);
    }
  }
  else {
    if (ant_fell) fprintf(stderr, "An ant has wandered off the map.\n");
    if (dead_ants == NEST_SIZE) fprintf(stderr, "All the ants have fallen off the map.\n");
  }
  fprintf(stderr, "Seed: %i\n", SEED);
  fprintf(stderr, "Size: %ix%i\n", WIDTH, HEIGHT);
  fprintf(stderr, "Nest size: %i\n", NEST_SIZE);
  fprintf(stderr, "Simulation time: %li steps\n", steps_run);
  fprintf(stderr, "\n");

}

void setup_simulation(s_plane *plane, s_ant *nest) {
  //initialize random
  if (USE_TIME) {
    SEED = time(NULL);
  }
  srandom(SEED);

  //place ants
  int i;
  for (i = 0; i != NEST_SIZE; i++) {
    #define NBORDER (1.0-BORDER)
    nest[i].x = (frandom()*WIDTH*NBORDER)+WIDTH*BORDER*.5;
    nest[i].y = (frandom()*HEIGHT*NBORDER)+HEIGHT*BORDER*.5;
    nest[i].angle = random() % 4;
    nest[i].active = 1;
  }

  //reset infos
  simulate = 1;
}

int main(int argc, char **argv) {
  parse_args(argc, argv);
  init_terminal();
  //init
  do {
    s_plane *plane = new_plane(WIDTH, HEIGHT);
    s_ant nest[NEST_SIZE];
    setup_simulation(plane, nest);
    run_simulation(plane, (s_ant*)&nest);
    free(plane);
    if (LOOP) {
      clear_screen();
    }
  } while (LOOP);
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
