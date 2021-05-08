CC = g++
INC = -I ../../
LDFLAGS = -shared
src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

blitz12.so: $(obj)
	$(CC) $(LIBDIRS) $(INC) -g -o $@ $^ $(LDFLAGS)
	
%.o: %.cpp
	$(CC) -g $(INC) -c $(src) -fPIC
	
.PHONY: clean	
clean:
	rm -f $(obj) blitz12.so
