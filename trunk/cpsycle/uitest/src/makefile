CC = gcc
INC = -I../../ui/src -I../../container/src -I../../file/src -I../../diversalis/src -I../../src -I../../audio/src -I../../dsp/src -I../../lua53/src
LIBDIRS = -L ../../container/src -L ../../dsp/src -L ../../audio/src -L ../../file/src -L ../../src -L ../../ui/src
src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS = -no-pie -lX11 -lXft -lXext -lXmu -lm -llua5.3 -lpthread -ldl -lstdc++ -lpsyui -lcontainer -lfile

uitest: $(obj)
		$(CC) -g -o $@ $^ $(LIBDIRS) $(LDFLAGS)	
		
%.o: %.c
	$(CC) -g $(INC) -c $(src)

.PHONY: clean	
clean:
	rm -f $(obj) uitest
