CC = gcc
inc = -I ../src -I../../container/src -I../../file/src -I../../../diversalis/src -I/usr/include/freetype2
cflags = -Wfatal-errors
src = $(wildcard *.c)
srcimp = $(wildcard imps/x11/*.c)
obj = $(src:.c=.o)
objimp = $(srcimp:.c=.o)
objall = $(obj) $(objimp)

libpsyui.a: $(obj)
	ar rsv $@ *.o
	
%.o: %.c
	$(CC) $(cflags) $(inc) -g -c $(src);
	$(CC) $(cflags) $(inc) -g -c $(srcimp);

.PHONY: clean	
clean:
	rm -f $(obj) $(objimp) libpsyui.a	
