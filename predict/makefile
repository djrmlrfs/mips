CC = g++
CFLAGS = -Wall -std=c++11 -O2

################!! MODIFY HERE !!####################
_OBJ = global.o predictor.o water.o mips.o
_DEPS = global.h predicoto.h water.h mips.h
#####################################################

ODIR = obj
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

IDIR = .
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

$(ODIR)/%.o: %.c* $(DEPS)
	$(shell mkdir -p bin)
	$(shell mkdir -p obj)
	$(CC) -c -o $@ $< $(CFLAGS)

mips: $(OBJ)
	$(CC) -o bin/mips $^ $(CFLAGS)

.PHONY: clean all rebuild

clean:
	rm -f $(ODIR)/*
	rm -f bin/*

all:
	mips

rebuild: clean all
