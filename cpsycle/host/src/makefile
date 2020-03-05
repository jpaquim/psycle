CC = gcc
INC = -I../../ui/src -I../../container/src -I../../file/src -I../../diversalis/src -I../../src -I../../audio/src -I../../dsp/src -I../../lua53/src
LIBDIRS = -L ../../container/src -L ../../dsp/src -L ../../audio/src -L ../../file/src -L ../../src -L ../../ui/src
src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS = -lncurses -laudio -lcontainer -ldsp -lfile -lXm -lXt -lX11 -lXft -lm -llua5.3 -lncurses -lpthread -ldl -lstdc++ -lpsyui

psycle: $(obj)
		$(CC) -g -o $@ $^ $(LIBDIRS) $(LDFLAGS)	
		
%.o: %.c
	$(CC) -g $(INC) -c $(src)

.PHONY: clean	
clean:
	rm -f $(obj) psycle
