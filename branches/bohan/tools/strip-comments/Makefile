#! /usr/bin/make -f

all: bin bin/strip-comments.exe

clean:
	rm -Rf bin test-files.*

bin:
	mkdir $@


bin/strip-comments.exe: StripComments.cs
	mcs -t:exe -out:$@ $^

test: all test-files.out

test-files.out:
	rm -Rf test-files.tmp $@
	cp -R test-files test-files.tmp
	./bin/strip-comments.exe test-files.tmp && mv test-files.tmp $@
