CC = gcc
inc = -I../../container/src -I../../../diversalis/src
src = $(wildcard *.c)
obj = $(src:.c=.o)

libthread.a: $(obj)
	ar rsv $@ $^

%.o: %.c
	$(CC) -O3 -g3 $(inc) -c $(src)

.PHONY: clean	
clean:
	rm -f $(obj) libthread.a
