#! /usr/bin/make -f

all: bin bin/WebApplication1.dll

clean:
	rm -Rf bin

bin:
	mkdir $@


bin/WebApplication1.dll: AssemblyInfo.cs WebForm1.aspx.cs Global.asax.cs
	mcs \
		-t:library -out:$@ \
		-r:System.Web -r:System.Data -r:System.Drawing  \
		$^

start: all
	xsp
