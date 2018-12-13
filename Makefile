.PHONY: all nro clean

export GITREV  := $(shell git rev-parse HEAD 2>/dev/null | cut -c1-8)
export VERSION_MAJOR := 0
export VERSION_MINOR := 0
export VERSION_MICRO := 1
export VERSION := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MICRO)

ifneq ($(strip $(GITREV)),)
export VERSION := $(VERSION)-$(GITREV)
endif

info:
	@echo $(MAKE)
	@echo $(CC)
	@echo $(CXX)

all:
	@echo please choose 3dsx, cia, linux, or nro

nro:
	@$(MAKE) -f Makefile.switch all

clean:
	@$(MAKE) -f Makefile.switch   clean
