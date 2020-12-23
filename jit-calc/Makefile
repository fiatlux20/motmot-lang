IDIR := include
SDIR := src
ODIR := build
OUT_FILE := calc
CC := gcc
DEFINES := -DDEBUG
CWARNS := -Wall -Wshadow -Wpointer-arith -Wcast-align -Wstrict-aliasing=1 # -Waggregate-return
CFLAGS := -I./$(IDIR) $(CWARNS) $(DEFINES) -g -fsanitize=address -fsanitize=undefined

HEADERS := $(wildcard $(IDIR)/*.h)
SOURCES := $(wildcard $(SDIR)/*.c)
OBJ := $(patsubst $(SDIR)%.c, $(ODIR)%.o, $(SOURCES))

$(ODIR)/%.o : $(SDIR)/%.c | $(ODIR)
	$(CC) -c $^ -o $@ $(CFLAGS)

$(OUT_FILE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(ODIR):
	mkdir $(ODIR)

docs: $(SOURCES) $(HEADERS)
	doxygen Doxyfile

.PHONY: clean
clean:
	rm -f $(OUT_FILE)
	rm -rf $(ODIR)
