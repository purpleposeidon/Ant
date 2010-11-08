#ifndef ANT_H
#define ANT_H

#define DRAW_DOTS 0



#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <assert.h>
#include <time.h>
#include <sys/ioctl.h>

#define MAX_INSTRUCTIONS 50
#define SECS_USEC 1000000
#define DEFAULT_COLOR 0

int simulate;
int LOOP;

typedef struct {
  unsigned int x, y;
  enum {NORTH = 0, SOUTH, EAST, WEST} angle;
  char active;
} s_ant;

typedef struct {
  int width, height;
  char *data;
} s_plane;

char *get_cell(s_plane *plane, int x, int y);


#endif /* ANT_H */