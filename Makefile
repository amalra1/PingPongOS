# Pedro Amaral Chapelin
# GRR20206145

# --- Variáveis de Compilação ---

CC = cc
CFLAGS = -Wall -I -g.
DFLAGS = -DDEBUG
TARGET = ppos

# --- Arquivos ---

SOURCES = ppos_core.c main.c queue/queue.c
HEADERS = ppos.h ppos_data.h queue/queue.h
OBJECTS = $(SOURCES:.c=.o)

# --- Variáveis para compactar ---

PROJECT_DIR = p3
TGZ_FILE = p3.tgz

# --- Regras ---

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: debug
debug:
	$(MAKE) CFLAGS+="$(DFLAGS)" all

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS) $(TGZ_FILE)

.PHONY: dist
dist: clean
	mkdir -p $(PROJECT_DIR)
	cp $(SOURCES) $(HEADERS) Makefile $(PROJECT_DIR)/
	tar -czvf $(TGZ_FILE) $(PROJECT_DIR)
	rm -rf $(PROJECT_DIR)

