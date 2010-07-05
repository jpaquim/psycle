
app=hologram
App=Hologram
env=../../../../cross-compilation/iphone/env.sh
dest=/tmp/++$(app)
bld=$(dest)/build
install=$(dest)/install
bundle=$(dest)/bundle/$(App).app
iphone=iphone

all: bundle

build:
	$(env) ../wonderbuild_script.py \
		--bld-dir=$(bld) \
		--install-prefix-dir=/ \
		--install-dest-dir=$(install) \
		--cxx-flags='-g -O0' \
		--ld-flags=-bind_at_load \
		--tasks=$(app)

bundle: build
	rm -Rf $(bundle) && \
	mkdir -p $(bundle) && \
	cp $(install)/bin/$(app) Info.plist icon.png $(bundle)

deploy: bundle
	rsync --archive --delete $(bundle) root@$(iphone):/Applications && \
	ssh root@$(iphone) 'cd /Applications/$(App).app && ldid -S $(app)'

respring:
	ssh root@$(iphone) 'killall SpringBoard'

clean:
	rm -Rf $(dest)

