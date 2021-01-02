C = gcc
CFLAGS = -g -Wall
LDLIBS = -lpthread

OBJS = SimulationLibrary.o SharedMemory.o
TARGET = main.ex

all: $(TARGET)

%.ex: %.o $(OBJS)
	$(C) $(CFLAGS) $< $(OBJS) -o $@ $(LDLIBS)

HashTable: HashTable.c
	$(C) $(CFLAGS) -c HashTable.c

clean:
	rm -f *.o *.ex $(TARGET)