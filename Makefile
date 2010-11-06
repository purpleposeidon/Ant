
CFLAGS = -Wall

all: ant.o draw.o
	cc $(CFLAGS) -o antsim draw.o ant.o

%.o: %.c
	cc $(CFLAGS) -c $<

clean:
	rm -f *.o antsim *~

