CC = g++
INC = -I ../../ -I ../../../dsp/src
LIBDIRS =
LDFLAGS = -lstk -shared
OUTPUTFILE = stk-reverbs.so
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

