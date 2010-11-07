
CFLAGS = -Wall -g
CC = gcc

all: ant.o draw.o
	$(CC) $(CFLAGS) -o antsim draw.o ant.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o antsim *~

