# This file is meant to by included by other Makefiles
# Make sure the `UNIT` variable is set.

UNIT_PATH=/etc/systemd/system
CTL=systemctl

install: $(UNIT)
	cp $(UNIT) "$(UNIT_PATH)"
	$(CTL) daemon-reload
	$(CTL) enable $(UNIT)
	$(CTL) start $(UNIT)
.PHONY: install

reinstall: $(UNIT)
	cp $(UNIT) "$(UNIT_PATH)"
	$(CTL) daemon-reload
	$(CTL) restart $(UNIT)
.PHONY: reinstall

uninstall:
	$(CTL) stop $(UNIT)
	$(CTL) disable $(UNIT)
	$(CTL) daemon-reload
	rm "$(UNIT_PATH)/$(UNIT)"
.PHONY: uninstall

