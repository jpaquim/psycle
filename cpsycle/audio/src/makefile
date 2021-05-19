CC = gcc -Wfatal-errors
inc = -I../../dsp/src -I../../file/src -I../../container/src -I../../../diversalis/src -I/usr/include/lua5.3
src = $(wildcard *.c)
obj = $(src:.c=.o)

libaudio.a: $(obj)
	g++ -g3 -c $(inc) plugin_interface.cpp
	ar rsv $@ $^ plugin_interface.o

%.o: %.c
	$(CC) -g3 $(inc) -c $(src)

.PHONY: clean	
clean:
	rm -f $(obj) libaudio.a
