MAJOR_VERS := 0
MINOR_VERS := 1

IDIR := include
SDIR := src
ODIR := build
OUT_FILE := interp
CC := gcc
CWARNS := -Wall -Wshadow -Wpointer-arith -Wcast-align -Wstrict-aliasing=1 # -Waggregate-return
DEFINES := -DMAJOR_VERS=$(MAJOR_VERS) -DMINOR_VERS=$(MINOR_VERS)
CFLAGS := -I./$(IDIR) $(CWARNS) $(DEFINES) -Og -g -fsanitize=address

HEADERS := $(wildcard $(IDIR)/*.h)
SOURCES := $(wildcard $(SDIR)/*.c)
OBJ := $(patsubst $(SDIR)%.c, $(ODIR)%.o, $(SOURCES))

$(ODIR)/%.o : $(SDIR)/%.c | $(ODIR)
	$(CC) -c $^ -o $@ $(CFLAGS)

$(OUT_FILE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(ODIR):
	mkdir $(ODIR)

.PHONY: clean
clean:
	rm $(OUT_FILE)
	rm -r $(ODIR)
