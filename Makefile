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

all: $(APP)

$(APP): $(SRCDIR)/main.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$(APP) -I$(INCDIR) $(SRCDIR)/main.c

clean:
	rm -rf $(OUTDIR)/$(APP)

style-check:
	clang-format -style=file -n $(SRCDIR)/*.c $(INCDIR)/*.h

style-fix:
	clang-format -style=file -i $(SRCDIR)/*.c $(INCDIR)/*.h

docs:
	doxygen Doxyfile

doc-check:
	doxygen-check $(SRCDIR)/*.c $(INCDIR)/*.h

.PHONY: docs doc-check style-check style-fix
