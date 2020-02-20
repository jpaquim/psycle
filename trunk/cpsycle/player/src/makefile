CC = gcc
INC = -I../../container/src -I../../file/src -I../../diversalis/src -I../../src -I../../audio/src -I../../dsp/src
LIBDIRS = -L ../../container/src -L ../../dsp/src -L ../../audio/src -L ../../file/src -L ../../src
psyplayer: coniotermios.c player.c 
	$(CC) $(LIBDIRS) $(INC) -g coniotermios.c player.c -o psyplayer -lncurses -laudio -lcontainer -ldsp -lfile -lcontainer -lXm -lXt -lX11 -lXft -lm -llua5.3 -lncurses -lpthread -ldl -lstdc++
	

.PHONY: clean	
clean:
	rm -f psyplayer.o psyplayer 
