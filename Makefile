# Pedro Amaral Chapelin
# GRR20206145

CC = cc
CFLAGS = -Wall -g -I.
DFLAGS = -DDEBUG
TARGET = ppos
SOURCES = ppos_core.c main.c queue/queue.c
HEADERS = ppos.h ppos_data.h queue/queue.h
OBJECTS = $(SOURCES:.c=.o)
PROJECT_DIR = p5
TGZ_FILE = p5.tgz

.PHONY: all debug clean dist
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

debug:
	$(MAKE) CFLAGS+="$(DFLAGS)" all

clean:
	rm -f $(TARGET) $(OBJECTS) $(TGZ_FILE)

dist: clean
	mkdir -p $(PROJECT_DIR)
	mkdir -p $(PROJECT_DIR)/queue
	cp ppos_core.c main.c ppos.h ppos_data.h Makefile $(PROJECT_DIR)/
	cp queue/queue.c queue/queue.h $(PROJECT_DIR)/queue/
	tar -czvf $(TGZ_FILE) $(PROJECT_DIR)
	rm -rf $(PROJECT_DIR)