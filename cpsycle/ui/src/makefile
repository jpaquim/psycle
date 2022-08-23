CC = gcc
inc = -I ../src -I../../thread/src -I ../src -I../../container/src -I../../file/src -I../../../diversalis/src -I/usr/include/freetype2
cflags = -Wfatal-errors -Wparentheses
src = $(wildcard *.c)
srcimp = $(wildcard imps/x11/*.c)
obj = $(src:.c=.o)
objimp = $(srcimp:.c=.o)
objall = $(obj) $(objimp)

libpsyui.a: $(obj)
	ar rsv $@ *.o
	
%.o: %.c
	$(CC) $(cflags) $(inc) -O3 -g3 -c $(src);
	$(CC) $(cflags) $(inc) -O3 -g3 -c $(srcimp);	

.PHONY: clean	
clean:
	rm -f $(obj) $(objimp) libpsyui.a	
