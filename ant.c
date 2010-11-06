
// Configuration
float DELAY = .0005;
float BORDER = .8;
int WIDTH = 80;
int HEIGHT = 40;
int NEST_SIZE = 2;
int SEED = 0;
int USE_TIME = 1;
int DISABLE_FALLEN = 1;
int LOOP = 0;
int JUMP = 0;
int DRAW_MOD = 1;



#include "ant.h"
#include "draw.h"


int simulate = 1;
char *arg0 = "antsim";


s_plane *new_plane(int width, int height) {
  s_plane *ret = malloc(sizeof(s_plane));
  ret->width = width;
  ret->height = height;
  ret->data = calloc(sizeof(char), width*height);
  return ret;
}

char *get_cell(s_plane *plane, int x, int y) {
  assert(x >= 0);
  assert(y >= 0);
  assert(x < plane->width);
  assert(y < plane->height);
  return &plane->data[x+y*plane->width];
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



float frandom() {
  //returns a random float in [0.0, 1.0] probably
  return ((float)random()) / ((float)RAND_MAX);
}

void parse_args(int argc, char **argv) {
  char usage[] =
  "Usage: \n" \
  "  ant [-a ANTCOUNT] [-w WIDTH] [-h HEIGHT] [-l] [-e] [-d DELAY] [-j JUMP] [-m SKIP] [-s SEED] [-b BORDER]\n" \
  "\n" \
  "-a   Sets how many ants to use.\n" \
  "-w   Sets the map width.\n" \
  "-h   Sets the map height.\n" \
  "-l   Loop the simulation. If -s is not given, a different seed will be used each time.\n" \
  "-e   Ends the simulation if an ant touches the edge. (default = ends if all ants have left)\n" \
  "-d   Delay, in seconds. (default = .0005)\n" \
  "-j   Don't draw the first JUMP frames.\n" \
  "-m   Sets how many frames are drawn. Setting to low values will not speed up the simulation.\n"
  "     If 0, no frames are drawn. (Except for the last one.) If 1, all frames are drawn, and they\n"
  "     are drawn efficiently. Otherwise, the entire grid is redrawn every SKIP frames.\n" \
  "-s   Sets the seed for ant placement. (default = current time)\n" \
  "-b   Sets the border for random ant placement. (default = 0.8)\n" \
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

  opterr = 0;
  char c;
  while ((c = getopt (argc, argv, "a:w:h:led:j:m:s:b:")) != -1) {
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
      case 'j':
        CNV_ARG(JUMP, atoi);
        break;
      case 'm':
        CNV_ARG(DRAW_MOD, atoi);
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
        exit(EXIT_FAILURE);
        break;
      default:
        exit(EXIT_FAILURE);
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
  int need_full_redraw = 0;
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
    int do_draw;
    if (DRAW_MOD) {
      do_draw = steps_run >= JUMP && !(steps_run % DRAW_MOD);
    }
    else {
      do_draw = 0;
    }
    if (!do_draw) need_full_redraw = 1;

    if (do_draw) {
      if (need_full_redraw) {
        //who knows what they've been up to...
        draw_plane(plane);
        need_full_redraw = 0;
      }
      for (i = 0; i != NEST_SIZE; i++) {
        //ACTIVE;
        draw_ant(&nest[i]);
      }
      delay(DELAY);
    }

    for (i = 0; i != NEST_SIZE; i++) {
      ACTIVE;
      step(plane, &nest[i]);
    }
    steps_run++;

    if (do_draw) {
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
  }
  if (need_full_redraw) {
    //draw the last frame
    draw_plane(plane);
    need_full_redraw = 0;
    for (i = 0; i != NEST_SIZE; i++) {
      //ACTIVE;
      draw_ant(&nest[i]);
    }
  }
  //printf("calling cursor_to_end();xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
  cursor_home();
  if (1 == NEST_SIZE && dead_ants == 1) {
    //only one ant, and it abandoned you
    if (frandom() > .95) {
      fprintf(stderr, "The soldier ant bites! You die...\n");
    }
    else {
      char *names[] = {"Alice", "Albert", "Alyssa", "Alex", "Al", "Lantgon"};
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
      "is leaving to share some fascinating antecdotes.",
      "has lost the antidote!",
      "fears the anteater.",
      "thinks you smell funny.",
      "is going to sit down and think up more ant puns.",
      "has wandered off the map."};
      int action_length = 14; //this really isn't sustainable
      char *action = actions[(int)(action_length*frandom())];
      fprintf(stderr, "%s the %sAnt %s\n", name, title, action);
    }
  }
  else {
    if (ant_fell) fprintf(stderr, "An ant has wandered off the map.\n");
    if (dead_ants == NEST_SIZE) fprintf(stderr, "All the ants have fallen off the map.\n");
  }
  cursor_to_end();
  fprintf(stderr, "Seed: %i\n", SEED);
  fprintf(stderr, "Size: %ix%i\n", WIDTH, HEIGHT);
  fprintf(stderr, "Nest size: %i\n", NEST_SIZE);
  fprintf(stderr, "Simulation time: %li steps\n", steps_run);
  fprintf(stderr, "Re-play:\n%s -s %i -a %i -w %i -h %i\n", arg0, SEED, NEST_SIZE, WIDTH, HEIGHT);
  if (!simulate) {
    fprintf(stderr, "Continue:\n%s -s %i -a %i -w %i -h %i -j %li\n", arg0, SEED, NEST_SIZE, WIDTH, HEIGHT, steps_run);
  }
  fprintf(stderr, "\n\n");
  
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
  arg0 = argv[0];
  set_size(&WIDTH, &HEIGHT);
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

//Some amusing seeds
/*
Makes a baseball diamond
$ ./run -s 1289035251 -w 127 -h 41

Seed: 1289030354                                       
Size: 80x40                                              
Nest size: 2                                             
Simulation time: 1381 steps 

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
