CC = g++
INC = -I ../../
LDFLAGS = -shared
src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

alk-muter.so: $(obj)
	$(CC) $(LIBDIRS) $(INC) -g -o $@ $^ $(LDFLAGS)
	
%.o: %.cpp
	$(CC) -g $(INC) -c $(src) -fPIC
	
.PHONY: clean	
clean:
	rm -f $(obj) alk-muter.so
