SHELL := cmd
.ONESHELL:

CC = gcc
CFLAGS = -g -O0 -fno-omit-frame-pointer -Wall -Wextra -fmax-errors=1
CPPFLAGS = -Isrc
LDFLAGS = -lalleg -lz -Wl,--allow-multiple-definition

INTDIR = int
OUTDIR = build
TARGET = $(OUTDIR)\factgame.exe

SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c,$(INTDIR)/%.o,$(SRC))

all: $(TARGET)

$(TARGET): $(OBJ)
	if not exist "$(OUTDIR)" mkdir "$(OUTDIR)"
	@echo [LD] $(TARGET)
	$(CC) $(OBJ) -o "$(TARGET)" $(LDFLAGS)

	if exist "ASSETS" (
		xcopy /E /I /Y "ASSETS" "$(OUTDIR)\ASSETS" >nul
	)

$(INTDIR)/%.o: src/%.c
	if not exist "$(INTDIR)" mkdir "$(INTDIR)"
	@echo [CC] $<
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	-del /q "$(INTDIR)\*.o" 2>nul
	-del /q "$(TARGET)" 2>nul

distclean: clean
	-rmdir /s /q "$(INTDIR)" 2>nul
	-rmdir /s /q "$(OUTDIR)" 2>nul