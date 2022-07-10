SUBDIRS = server_mode client_mode

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: $(SUBDIRS)

.PHONY: clean
clean:
	@rm -rf obj bin