CC=gcc
CXX=g++
CXXLD=g++
CCLD=gcc

COMMONFLAGS= -g -I. -I$(DAQINC) -I$(DAQINC)/sbsreadout -DHAVE_VME_MAPPING \
-DHAVE_SBSVME_INTERFACE -I/usr/include/tcl8.6 -I/usr/include/tcl8.6 -pthread

USERCCFLAGS=
USERCXXFLAGS=$(USERCCFLAGS)
USERLDFLAGS=

CXXFLAGS = $(COMMONFLAGS)  $(USERCXXFLAGS)
CCFLAGS= $(COMMONFLAGS) $(USERCCFLAGS)

##
# Note THREAD_LDFLAGS are left off because:
#  1. We need a threaded build of Tcl and the TCL_LDFLAGS get those.
#  2. Both include -lieee which has a symbol that's not as weak as it should
#     be and results in double definitions if it appears twice on the link line.

LDFLAGS=-L$(DAQLIB) -lSBSProductionReadout -lSBSDeviceSupport -lSBSVmeAPI \
-lException -ldaqthreads -ldaqshm -ltclPlus -L/usr/lib/x86_64-linux-gnu \
-ltcl8.6 -ldl -lz -lpthread -lm  -Wl,"-rpath=$(DAQLIB)"

OBJECTS=faziaReadout.o CFaziaEventSegment.o CUdpEventSegment.o CUdpTrigger.o

faziaReadout: $(OBJECTS)
	$(CXXLD) -o faziaReadout $(OBJECTS) $(USERLDFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJECTS) faziaReadout

help:
	echo make        - Build faziaReadout.
	echo make clean  - Remove products from previous builds.