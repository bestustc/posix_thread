# Set CPPFLAGS environment variable for debugging:
#
# 	export CPPFLAGS=-gdwarf-2
#
# Created by Samvel Khalatyan, Sep 08, 2011
# Copyright 2011, All rights reserved

CXX ?= g++

# Get list of all heads, sources and objects. Each source (%.cc) whould have
# an object file except programs listed in PROGS
#
heads = $(wildcard ./interface/*.h)
srcs = $(wildcard ./src/*.cc)

objs = $(foreach obj,${srcs},$(addprefix ./obj/,$(patsubst %.cc,%.o,$(notdir ${obj}))))

# List of programs with main functions to be filtered out of objects
#
progs = $(foreach prog,$(wildcard ./src/*.cpp),$(addprefix ./bin/test_,$(patsubst ./src/%.cpp,%,${prog})))

CPPFLAGS += ${debug} -fPIC -pipe -Wall -I../
#LDFLAGS +=
ifeq ($(shell uname),Linux)
	LDFLAGS  += -L/usr/lib64 -lpthread
else
	CPPFLAGS += -I/opt/local/include
	LDFLAGS += -L/opt/local/lib
endif

# Rules to be always executed: empty ones
#
.PHONY: prog

all: prog

obj: ${objs}

prog: ${progs}



# Regular compilcation
#
${objs}: obj/%.o: src/%.cc interface/%.h
	@echo "[+] Compiling objects $@ ..."
	${CXX} ${CPPFLAGS} -c $(addprefix ./src/,$(patsubst %.o,%.cc,$(notdir $@))) -o $@
	@echo



# Executables
#
${progs}: bin/test_%: src/%.cpp
	@echo "[+] Compiling programs $@ ..."
	$(eval prog_name=$(patsubst bin/test_%,%,$@))
	${CXX} ${CPPFLAGS} -c src/${prog_name}.cpp -o ./obj/${prog_name}.o
	${CXX} ${LDFLAGS} ./obj/${prog_name}.o -o $@
	@echo



# Cleaning
#
clean:
	rm -f ./obj/*.o
	rm -f bin/test_*
