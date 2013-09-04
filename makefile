CC := gcc
SUBDIRS := lib src obj
BIN := main
PWD := $(shell pwd)
OBJS_DIR := $(PWD)/obj
BIN_DIR := $(PWD)/bin

IF_GDB = 1
ifeq ($(IF_GDB),0)
	CFLAGS := -g
else
	CFLAGS :=
endif
export CC PWD BIN OBJS_DIR BIN_DIR CFLAGS

all : CHECK_DIR $(SUBDIRS)
CHECK_DIR :
	mkdir -p $(BIN_DIR)

$(SUBDIRS) : ECHO
	make -C $@

ECHO :
	@echo $(SUBDIRS)
	@echo begin complie

clean : 
	@rm -rf $(OBJS_DIR)/*.o
	@rm -rf $(BIN_DIR)
