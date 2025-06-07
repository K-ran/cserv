# application name
APP = cserv

# compiler
CC = gcc

DEBUG = 0

# include dir
INCDIR = include

# compiler flags
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99

# add -g flag if debugging
ifeq ($(DEBUG), 1)
	CFLAGS += -g
endif

# src dir
SRCDIR = src

# output dir
OUTDIR = bin

# Source files and object files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OUTDIR)/%.o)

# Create bin directory if it doesn't exist
$(shell mkdir -p $(OUTDIR))

# Main target
$(APP): $(OBJS)
	$(CC) $(CFLAGS) -o $(OUTDIR)/$(APP) $^

# Pattern rule for object files
$(OUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Clean target should remove object files too
clean:
	rm -rf $(OUTDIR)/*.o $(OUTDIR)/$(APP)

style-check:
	clang-format -style=file -n $(SRCDIR)/*.c $(INCDIR)/*.h

style-fix:
	clang-format -style=file -i $(SRCDIR)/*.c $(INCDIR)/*.h

docs:
	doxygen Doxyfile

doc-check:
	doxygen-check $(SRCDIR)/*.c $(INCDIR)/*.h

.PHONY: docs doc-check style-check style-fix
