src = $(wildcard *.c)
obj = $(src:.c=.o)

libcontainer.a: $(obj)	
	ar rsv $@ $^

%.o : %.c
	$(CC) -c -O2 -g -fPIC $(src)

.PHONY: clean	
clean:
	rm -f $(obj) libcontainer.a
