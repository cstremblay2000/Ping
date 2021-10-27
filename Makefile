CC=gcc
TARGET = ping
FILES = networking.c set_uid.c
CFLAGS = -g -Wall -Wextra -pedantic -lm

all: $(TARGET).c $(FILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(FILES)
	sudo chown root:root $(TARGET)
	sudo chmod 4755 $(TARGET)
clean: $(TARGET)
	rm $(TARGET)
