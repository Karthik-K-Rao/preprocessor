CC = gcc
CFLAGS = -Wall -Wextra

TARGET = mypreprocessor

SOURCES = main.c comment_removal.c header_inclusion.c macro_handler.c
OBJECTS = $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.c header.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
