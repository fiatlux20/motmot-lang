IDIR := include
SDIR := src
ODIR := build
CC := gcc
CWARNS := -Wall -Wshadow -pedantic -Wpointer-arith -Wcast-align # -Waggregate-return
CFLAGS := -I./$(IDIR) $(CWARNS) -Og -g -fsanitize=address

HEADERS := $(wildcard $(IDIR)/*.h)
SOURCES := $(wildcard $(SDIR)/*.c)
OBJ := $(patsubst $(SDIR)%.c, $(ODIR)%.o, $(SOURCES))

$(ODIR)/%.o : $(SDIR)/%.c | $(ODIR)
	$(CC) -c $^ -o $@ $(CFLAGS)

interp: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(ODIR):
	mkdir $(ODIR)
