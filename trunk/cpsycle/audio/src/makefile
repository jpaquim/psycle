CC = gcc -Wfatal-errors
inc = -I../../dsp/src -I../../file/src -I../../container/src -I../../../diversalis/src
src = $(wildcard *.c)
obj = $(src:.c=.o)

audio.a: $(obj)
	ar rsv $@ $^

%.o: %.c
	$(CC) $(inc) -c $(src)

.PHONY: clean	
clean:
	rm -f $(obj) audio.a
