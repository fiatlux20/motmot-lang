IDIR := ./include
SDIR := ./src
ODIR := ./build
CC := gcc
CFLAGS := -I$(IDIR) -Wall -Og -fsanitize=address

HEADERS := tokenize.h match.h grammar.h
DEPS := $(patsubst %,$(IDIR)/%,$(HEADERS))

_OBJ := main.o tokenize.o match.o grammar.o
OBJ  := $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

interp: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
