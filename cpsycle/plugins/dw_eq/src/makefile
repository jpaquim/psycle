CC = g++
INC = -I ../../ -I ../../psycle -I ../../../dsp/src
LDFLAGS = -lcontainer -ldsp -shared
LIBDIRS = -L ../../../container/src -L ../../../dsp/src

OUTPUTFILE = dw-eq.so
OUTPUTDIR = ../../build/
src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

$(OUTPUTFILE): $(obj)
	$(CC) $(LIBDIRS) $(INC) -g -o $(OUTPUTDIR)$@ $^ $(LDFLAGS)
	
%.o: %.cpp
	$(CC) -g $(INC) -c $(src) -fPIC
	
.PHONY: clean	
clean:
	rm -f $(obj) $(OUTPUTFILE)
