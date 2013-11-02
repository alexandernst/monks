SUBDIRS = procmon_kmodule procmon
SUBCLEAN = $(addsuffix .clean,$(SUBDIRS))

.PHONY: clean $(SUBDIRS)

all: $(SUBDIRS)

clean: $(SUBCLEAN)

$(SUBDIRS):
	$(MAKE) -C $@

$(SUBCLEAN): %.clean:
	$(MAKE) -C $* clean