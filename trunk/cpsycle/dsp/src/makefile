CC = gcc
inc = -I../../container/src -I../../../diversalis/src
src = $(wildcard *.c)
obj = $(src:.c=.o)

libdsp.a: $(obj)
	ar rsv $@ $^

%.o: %.c
	$(CC) -O2 $(inc) -g -fPIC -fopenmp -c $(src) 

.PHONY: clean	
clean:
	rm -f $(obj) libdsp.a
