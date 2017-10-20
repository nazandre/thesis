# Get current directory's name
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))
APPDIR = ../../applicationsBin/$(current_dir)

#####################################################################
#
# --- Sample User Makefile ---
#
# GLOBAL_LIBS, GLOBAL_INCLUDES and GLOBAL_CFLAGS are set by parent Makefile
# HOWEVER: If calling make from the codeBlock directory (for more convenience to the user),
#	these variables will be empty. Hence we test their value and if undefined,
#	set them to predefined values.
#
# You will find instructions below on how to edit the Makefile to fit your needs.
#
# SRCS contains all the sources of your codeBlocks
ABC_CENTER_SRCS =  $(wildcard abcCenterV1/*.cpp) $(wildcard abcCenterV2/*.cpp)
E2ACE_SRCS = $(wildcard e2ace/*.cpp)
SIMPLE_SRCS = $(wildcard simple/*.cpp)
TBCE_SRCS = $(wildcard tbce/*.cpp)
EXACT_SRCS = $(wildcard exact/*.cpp)
KORACH_SRCS = $(wildcard korach/*.cpp)
RAND_CENTER_SRCS = $(wildcard randCenter/*.cpp)
RAND_CENTER_2_SRCS = $(wildcard randCenter2/*.cpp)
PARALLEL_RAND_CENTER_SRCS = $(wildcard parallelRandCenter/*.cpp)
EXTREMUM_CENTER_SRCS = $(wildcard extremumCenter/*.cpp)
DEE_SRCS = $(wildcard dee/*.cpp)
SIMULATION_SRCS = simulation.cpp
UTILS_SRCS = $(wildcard utils/*.cpp)
BFS_SRCS = bfsTree.cpp bfsTreeMsg.cpp bfsData.cpp
EA_SRCS = ea.cpp eaMsg.cpp
IEA_SRCS = initiator.cpp initiatorMsg.cpp
LOGICAL_ID_SRCS = logicalId.cpp
ALPHA_SYNCHRONIZED_BFS_SRCS = alphaSynchronizedBFS.cpp
SRCS =  centrality.cpp  centralityBlockCode.cpp $(SIMULATION_SRCS) $(UTILS_SRCS) $(LOGICAL_ID_SRCS) $(EA_SRCS) $(IEA_SRCS) $(BFS_SRCS) $(ABC_CENTER_SRCS) $(E2ACE_SRCS) $(SIMPLE_SRCS) $(TBCE_SRCS) $(EXACT_SRCS) $(KORACH_SRCS) $(RAND_CENTER_SRCS) $(EXTREMUM_CENTER_SRCS) $(DEE_SRCS) $(RAND_CENTER_2_SRCS) $(PARALLEL_RAND_CENTER_SRCS) $(ALPHA_SYNCHRONIZED_BFS_SRCS)
#
# OUT is the output binary, where APPDIR is its enclosing directory
OUT = $(APPDIR)/centrality
#
# MODULELIB is the library for your target module type: -lsim<module_name>
MODULELIB = -lsimCatoms3D -lsimCatoms2D -lsimRobotBlocks -lsimBlinkyBlocks -lsimSmartBlocks -lsimMultiRobots

# TESTS contains the commands that will be executed when `make test` is called
#TESTS = ../../utilities/blockCodeTest.sh meldbb $(OUT) -c configs/configBB.xml -p outprogs/rainbow.bb -k BB ;\
	../../utilities/blockCodeTest.sh meldsb $(OUT) -c configs/configSB.xml -p outprogs/rainbowSB.bb -k SB ;\
	../../utilities/blockCodeTest.sh meldrb $(OUT) -c configs/configRB.xml -p outprogs/rainbow.bb -k RB ;\
	../../utilities/blockCodeTest.sh meldc2d $(OUT) -c configs/configC2D.xml -p outprogs/rainbowC2D.bb -k C2D ;\
	../../utilities/blockCodeTest.sh meldc3d $(OUT) -c configs/configC3D.xml -p outprogs/rainbowC3D.bb -k C3D

#
# End of Makefile section requiring input by user
#####################################################################

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.depends)

OS = $(shell uname -s)
SIMULATORLIB = $(MODULELIB:-l%=../../simulatorCore/lib/lib%.a)

ifeq ($(GLOBAL_INCLUDES), )
INCLUDES = -I. -I../../simulatorCore/src -I/usr/local/include -I/opt/local/include -I/usr/X11/include
else
INCLUDES = -I. -I../../simulatorCore/src $(GLOBAL_INCLUDES)
endif

ifeq ($(GLOBAL_LIBS), )
	ifeq ($(OS),Darwin)
LIBS = -L./ -L../../simulatorCore/lib -L/usr/local/lib -lGLEW -lglut -framework GLUT -framework OpenGL -L/usr/X11/lib /usr/local/lib/libglut.dylib $(MODULELIB)
	else
LIBS = -L./ -L../../simulatorCore/lib -L/usr/local/lib -L/opt/local/lib -lm -L/usr/X11/lib  -lglut -lGL -lGLU -lGLEW -lpthread $(MODULELIB)
	endif				#OS
else
LIBS = $(GLOBAL_LIBS) -L../../simulatorCore/lib
endif				#GLOBAL_LIBS

ifeq ($(GLOBAL_CCFLAGS),)
CCFLAGS = -g -Wall -std=c++11 -DTINYXML_USE_STL -DTIXML_USE_STL
	ifeq ($(OS), Darwin)
	CCFLAGS += -DGL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED -Wno-deprecated-declarations -Wno-overloaded-virtual
	endif 
else
CCFLAGS = $(GLOBAL_CCFLAGS)
endif

CC = g++

.PHONY: clean all test

.cpp.o:
	$(CC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

%.depends: %.cpp
	$(CC) -M $(CCFLAGS) $(INCLUDES) $< > $@

all: $(OUT)
	@:

test:
	@$(TESTS)

autoinstall: $(OUT)
	cp $(OUT)  $(APPDIR)

$(APPDIR)/$(OUT): $(OUT)

$(OUT): $(SIMULATORLIB) $(OBJS)
	$(CC) -o $(OUT) $(OBJS) $(LIBS)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

clean:
	rm -f *~ $(OBJS) $(OUT) $(DEPS)
