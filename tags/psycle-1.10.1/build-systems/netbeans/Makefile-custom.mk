dir = ../wonderbuild

all:
	$(dir)/wonderbuild_script.py \
		--jobs=1 --sync-log --input-abs-paths=yes \
		--cxx-flags='-g3 -gdwarf-2'

clean:
	rm -Rfv $(dir)/++wonderbuild