src = $(wildcard *.c)
obj = $(src:.c=.o)

container.a: $(obj)	
	ar rsv $@ $^

%.o : %.c
	$(CC) -c $(src)

.PHONY: clean	
clean:
	rm -f $(obj) container.a
