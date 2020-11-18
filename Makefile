IDIR := ./include
SDIR := ./src
ODIR := ./build
CC := gcc
CFLAGS := -I$(IDIR) -Wall -Og -g -fsanitize=address

HEADERS := common.h tokenize.h tokens.h grammar.h bytecode.h vm.h
DEPS := $(patsubst %,$(IDIR)/%,$(HEADERS))

_OBJ := main.o tokenize.o tokens.o grammar.o bytecode.o vm.o
OBJ  := $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

interp: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
