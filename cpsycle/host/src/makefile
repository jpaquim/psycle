CC = gcc

INC = -I../../script/src -I../../thread/src -I../../luaui/src -I../../ui/src -I../../container/src -I../../file/src -I../../diversalis/src -I../../src -I../../audio/src -I../../dsp/src -I/usr/include/lua5.4
LIBDIRS = -L ../../thread/src -L ../../script/src -L ../../container/src -L ../../dsp/src -L ../../audio/src -L ../../file/src -L ../../src -L ../../ui/src
src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS = -no-pie -Wparentheses -laudio -lthread -llilv-0 -ldsp -laudio -lscript -lfile -lpsyui -lfontconfig -lfreetype -lGL -lX11 -lXft -lXext -lXmu -lm -llua5.4 -lpthread -ldl -lstdc++ -lcontainer

all: CFLAGS = -g3 -O3
all: psycle

debug: CFLAGS = -DDEBUG -g3
debug: psycle

psycle: $(obj)	
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIRS) $(LDFLAGS)
		
%.o: %.c
	$(CC) $(INC) -c $(src)
	

.PHONY: clean	
clean:
	rm -f $(obj) psycle	
