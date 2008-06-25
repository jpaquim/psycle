#! /usr/bin/make -f

all: bin bin/strip-comments.exe

bin:
	mkdir $@

bin/strip-comments.exe: StripComments.cs
	mcs -t:exe -out:$@ $^

test: all
	rm -Rf test-files.out
	cp -R test-files.{in,out}
	./bin/strip-comments.exe test-files.out

clean:
	rm -Rf bin test-files.out
