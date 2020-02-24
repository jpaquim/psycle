CC = gcc -Wfatal-errors
inc = -I../../dsp/src -I../../container/src
src = $(wildcard *.c)
obj = $(src:.c=.o)
LDFLAGS = -lasound -lcontainer -ldsp
LIBDIRS = -L ../../container/src -L ../../dsp/src

libpsyalsa.so: $(obj)	
	$(CC) -shared -o $@ $^ $(LIBDIRS) $(LDFLAGS)	

%.o: %.c
	$(CC) $(inc) -c -fPIC $(src)

.PHONY: clean	
clean:
	rm -f $(obj) libpsyalsa.so
