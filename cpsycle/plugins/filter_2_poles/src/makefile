CC = g++
INC = -I ../../ -I ../../psycle -I ../../../dsp/src
LIBDIRS = -L ../../../dsp/src -l dsp
LDFLAGS = -shared
OUTPUTFILE = filter-2-poles.so
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
