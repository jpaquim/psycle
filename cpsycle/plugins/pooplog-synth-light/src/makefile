CC = g++
INC = -I ../../ -I ../../../dsp/src
LDFLAGS = -lcontainer -ldsp -shared
LIBDIRS = -L ../../../container/src -L ../../../dsp/src
OUTPUTFILE = pooplog-fm-light.so
OUTPUTDIR = ../../build/
src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

$(OUTPUTFILE): $(obj)
	$(CC) $(INC) $(LIBDIRS) -fPIC -o $(OUTPUTDIR)$@ $^ $(LDFLAGS)
	
%.o: %.cpp
	$(CC) $(INC) -g -c $(src) -fPIC
	
.PHONY: clean	
clean:
	rm -f $(obj) $(OUTPUTFILE)
	
