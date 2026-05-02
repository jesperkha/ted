TARGET := app
CC := $(shell command -v clang >/dev/null 2>&1 && echo clang || echo gcc)

ifeq ($(OS), Windows_NT)
	TARGET := $(TARGET).exe
endif

FLAGS = -Wall -Wpedantic -std=c99 -Iinclude
RELEASE_FLAGS = -O3
BINDIR = bin
SRC = src/*.c

all: $(TARGET)

run: $(TARGET)
	./$(BINDIR)/$(TARGET)

release: $(BINDIR)
	$(CC) $(FLAGS) $(RELEASE_FLAGS) -o $(BINDIR)/$(TARGET) $(SRC)

$(TARGET): $(BINDIR)
	$(CC) $(FLAGS) -o $(BINDIR)/$(TARGET) $(SRC)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -r $(BINDIR)
