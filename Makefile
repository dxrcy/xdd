CC = gcc
CFLAGS = -Wall -Wextra -pedantic

TARGET = xdd

main:
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

