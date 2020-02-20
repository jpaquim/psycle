CC = gcc -Wfatal-errors
inc = -I../../dsp/src -I../../file/src -I../../container/src -I../../../diversalis/src -I../../lua53/src
src = $(wildcard *.c)
obj = $(src:.c=.o)

libaudio.a: $(obj)
	g++ -c $(inc) plugin_interface.cpp
	ar rsv $@ $^ plugin_interface.o

%.o: %.c
	$(CC) $(inc) -c $(src)

.PHONY: clean	
clean:
	rm -f $(obj) libaudio.a
