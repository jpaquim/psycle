
app=hologram
App=Hologram
dest=/tmp/++$(app)
bld=$(dest)/build
install=$(dest)/install
bundle=$(dest)/bundle/$(App).app

all: bundle

bundle:
	../../../../cross-compilation/iphone/env.sh ../wonderbuild_script.py \
		--bld-dir=$(bld) \
		--install-prefix-dir=/ \
		--install-dest-dir=$(install) \
		--ld-flags=-bind_at_load
	mkdir -p $(bundle)
	cp $(install)/bin/$(app) $(bundle)/$(app)_
	cp $(app).sh $(bundle)/$(app)
	cp icon.png Info.plist $(bundle)

deploy: clean-bundle bundle
	ssh root@$(IP) "cd /Applications/$(App).app && rm -R * || echo 'not found'"
	scp -rp $(bundle) root@$(IP):/Applications
	ssh root@$(IP) "cd /Applications/$(App).app && ldid -S $(app)_ && killall SpringBoard"

clean-bundle:
	rm -Rf $(bundle)

clean:
	rm -Rf $(dest)

