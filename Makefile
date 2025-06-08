# application name
APP = cserv

# compiler
CC = gcc

DEBUG ?= 0

# include dir
INCDIR = include


# compiler flags
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99


# Optimization flags
ifeq ($(DEBUG), 1)
    # Debug build: minimal optimization for better debugging
    CFLAGS += -O0
else
    # Release build: balanced optimization
    CFLAGS += -O2
endif


# add -g flag if debugging
ifeq ($(DEBUG), 1)
	CFLAGS += -g
endif

# macros
# if debugging, print debug statements
ifeq ($(DEBUG), 1)
	CFLAGS += -DCSERV_DEBUG
endif

# src dir
SRCDIR = src

# output dir
OUTDIR = bin

# Source files and object files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OUTDIR)/%.o)

# Include header dependencies
DEPS = $(wildcard $(INCDIR)/*.h)

# Create bin directory if it doesn't exist
$(shell mkdir -p $(OUTDIR))

# Main target
$(APP): $(OBJS)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $(OUTDIR)/$(APP) $^

# Pattern rule for object files
$(OUTDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

all: $(APP)

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

.PHONY: all clean docs doc-check style-check style-fix
