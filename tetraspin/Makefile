CXXFLAGS=-Wall -g -std=c++11
.DEFAULT:= all
.PHONY: all

OBJECTS=main.o vec_ops.o render.o world_setup.o input.o

all: main

main: $(OBJECTS)
	c++ $(CXXFLAGS) $(OBJECTS) -o main 

clean:
	rm *.o main
