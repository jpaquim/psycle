CC = gcc
inc = -I../../container/src -I../../../diversalis/src -I../../thread/src -I../../file/src -I/usr/include/lua5.4
src = $(wildcard *.c)
obj = $(src:.c=.o)

libscript.a: $(obj)
	ar rsv $@ $^

%.o: %.c
	$(CC) -O2 -g3 $(inc) -c $(src)

.PHONY: clean	
clean:
	rm -f $(obj) libscript.a
