CXX = g++

CXXFLAGS = -g -O2 -Wall -I$(DAQINC)
LDFLAGS = -L$(DAQLIB) -Wl,-rpath=$(DAQLIB)
LDLIBS = -ldataformat -ldaqshm

OBJECTS=faziaToRingBuffer.o

all: faziaToRingBuffer

faziaToRingBuffer: $(OBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f faziaToRingBuffer $(OBJECTS)
