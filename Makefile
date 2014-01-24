SUBDIRS = monks_kmodule monks
SUBCLEAN = $(addsuffix .clean,$(SUBDIRS))

.PHONY: clean $(SUBDIRS)

all: $(SUBDIRS)

clean: $(SUBCLEAN)

$(SUBDIRS):
	$(MAKE) -C $@

$(SUBCLEAN): %.clean:
	$(MAKE) -C $* clean