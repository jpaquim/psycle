HOSTDIR := host
PLUGDIR := plugins

all: host plugins

host:
	$(MAKE) -C $(HOSTDIR)
	cp ./$(HOSTDIR)/src/psycle ./psycle

plugins:	
	$(MAKE) -C $(PLUGDIR)	

.PHONY: all host plugins clean clean-host clean-plugins

clean:
	$(MAKE) -C $(HOSTDIR) clean
	$(MAKE) -C $(PLUGDIR) clean
	rm -f psycle

clean-host:
	$(MAKE) -C $(HOSTDIR) clean
	rm -f psycle

clean-plugins:
	$(MAKE) -C $(PLUGDIR) clean
