CC = gcc
INC = -I../../container/src -I../../file/src -I../../diversalis/src -I../../src -I../../audio/src -I../../dsp/src
LIBDIRS = -L ../../container/src -L ../../dsp/src -L ../../audio/src -L ../../file/src -L ../../src
psyplayer: coniotermios.c player.c 
	$(CC) $(LIBDIRS) $(INC) -g coniotermios.c player.c -o psyplayer -laudio -lcontainer -ldsp -lfile -lcontainer -llua5.3 -lm -lpthread -ldl -lstdc++
	
.PHONY: clean	
clean:
	rm -f psyplayer.o psyplayer 
