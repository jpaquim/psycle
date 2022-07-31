CC = g++
INC = -I ../../ -I ../../psycle -I ../../../dsp/src
LDFLAGS = -ldsp -shared
LIBDIRS = -L ../../../dsp/src
OUTPUTFILE = legasynth-303.so
OUTPUTDIR = ../../build/
src = $(wildcard *.cpp) \
	  $(wildcard lib/*.cpp) \
	  $(wildcard 303/*.cpp)
obj = $(src:.cpp=.o)

$(OUTPUTFILE): $(obj)
	$(CC) $(LIBDIRS) $(INC) -g -fPIC -o $(OUTPUTDIR)$@ $^ $(LDFLAGS)
	
%.o: %.cpp
	$(CC) -g $(INC) -c $^ -fPIC -o $@
	
.PHONY: clean
clean:
	rm -f *.o
