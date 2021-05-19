CC = gcc
inc = -I ../src -I../../container/src -I../../file/src -I../../../diversalis/src -I/usr/include/freetype2
cflags = -Wfatal-errors
src = $(wildcard *.c)
srcimp = $(wildcard imps/x11/*.c)
srcnativeimp = $(wildcard imps/natives/*.c)
obj = $(src:.c=.o)
objimp = $(srcimp:.c=.o)
objnativeimp = $(srcnativeimp:.c=.o)
objall = $(obj) $(objimp) $(objnativeimp)

libpsyui.a: $(obj)
	ar rsv $@ *.o
	
%.o: %.c
	$(CC) $(cflags) $(inc) -g3 -c $(src);
	$(CC) $(cflags) $(inc) -g3 -c $(srcimp);
	$(CC) $(cflags) $(inc) -g3 -c $(srcnativeimp);

.PHONY: clean	
clean:
	rm -f $(obj) $(objimp) $(objnativeimp) libpsyui.a	
