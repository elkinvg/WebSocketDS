SUBDIRS = server_mode client_mode

all: $(SUBDIRS)

$(SUBDIRS):
		$(MAKE) -C $@

.PHONY: $(SUBDIRS)