CXX = g++

CXXFLAGS = -g -O2 -Wall -I$(DAQINC)
LDFLAGS = -L$(DAQLIB) -Wl,-rpath=$(DAQLIB)
LDLIBS = -ldataformat -ldaqshm -ldaqio -lException

EXEC = faziaToRingBuffer
OBJECTS=faziatoringbufferargs.o faziaToRingBuffer.o

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

faziatoringbufferargs.o:
	gengetopt < faziatoringbufferargs.ggo --file-name=faziatoringbufferargs
	$(CC) -c faziatoringbufferargs.c

clean:
	rm -f $(EXEC) $(OBJECTS)
	rm -f faziatoringbufferargs.c faziatoringbufferargs.h
