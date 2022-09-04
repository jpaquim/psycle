CC = gcc
INC = -I../../script/src -I../../thread/src -I../../container/src -I../../file/src -I../../diversalis/src -I../../src -I../../audio/src -I../../dsp/src -I/usr/include/lua5.4
LIBDIRS = -L ../../thread/src -L ../../script/src -L ../../container/src -L ../../dsp/src -L ../../audio/src -L ../../file/src -L ../../src
LDFLAGS = -no-pie -Wparentheses -laudio -lthread  -llilv-0 -ldsp -laudio -lscript -lfile -lm -llua5.4 -lpthread -ldl -lstdc++ -lcontainer
psyplayer: coniotermios.c player.c 
	$(CC) $(INC) -g coniotermios.c player.c -o psyplayer $(LIBDIRS) $(LDFLAGS)
	
.PHONY: clean	
clean:
	rm -f psyplayer.o psyplayer 
