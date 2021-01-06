C = gcc
CFLAGS = -g -Wall
LDLIBS = -lpthread -lrt

OBJS = SimulationLibrary.o ThreadRoutines.o
TARGET = main.ex

all: $(TARGET)

%.ex: %.o $(OBJS)
	$(C) $(CFLAGS) $< $(OBJS) -o $@ $(LDLIBS)

main: main.c
	$(C) $(CFLAGS) -c main.c
	
SimulationLibrary: SimulationLibrary.c
	$(C) $(CFLAGS) -c SimulationLibrary.c

ThreadRoutines: ThreadRoutines.c
	$(C) $(CFLAGS) -c ThreadRoutines.c

clean:
	rm -f *.o $(TARGET)