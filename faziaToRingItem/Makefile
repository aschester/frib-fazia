CXX = g++

CXXFLAGS = -g -O2 -Wall -I$(DAQINC)
LDFLAGS = -L$(DAQLIB) -Wl,-rpath=$(DAQLIB)
LDLIBS = -ldataformat -ldaqshm

all: faziaToRingItem

faziaToRingItem: faziaToRingItem.o
	$(CXX) $^ -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f faziaToRingItem *.o
