CC = gcc -Wfatal-errors
inc = -I../../luaui/src -I../../ui/src -I../../thread/src -I../../script/src -I../../dsp/src -I../../file/src -I../../container/src -I../../../diversalis/src -I/usr/include/lua5.4 -I/usr/include -I/usr/include/lilv-0/
src = $(wildcard *.c)
obj = $(src:.c=.o)

libaudio.a: $(obj)
	g++ -O3 -g3 -c $(inc) plugin_interface.cpp
	ar rsv $@ $^ plugin_interface.o

%.o: %.c
	$(CC) -O2 -g3 $(inc) -c $(src)

.PHONY: clean	
clean:
	rm -f $(obj) libaudio.a
