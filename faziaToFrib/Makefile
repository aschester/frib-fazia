CXX = g++
CC = gcc

CXXFLAGS = -g -O2 -Wall -I$(DAQINC)
LDFLAGS = -L$(DAQLIB) -Wl,-rpath=$(DAQLIB)
LDLIBS = -ldataformat -ldaqshm -ldaqio -lException

EXEC = faziaToFrib
OBJECTS = faziatofribargs.o faziaToFrib.o

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LDLIBS)

faziaToFrib.o: faziaToFrib.cpp faziatofribargs.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

faziatofribargs.c faziatofribargs.h: faziatofribargs.ggo
	gengetopt < faziatofribargs.ggo --file-name=faziatofribargs

faziatofribargs.o: faziatofribargs.c
	$(CC) -c faziatofribargs.c -o faziatofribargs.o

clean:
	rm -f $(EXEC) $(OBJECTS)
	rm -f faziatofribargs.c faziatofribargs.h
