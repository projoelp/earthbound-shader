CC = gcc
CFLAGS = -I./include -Wall -Wextra

LDFLAGS = -lSDL2 -lGL -ldl

SRCS = main.c glad.c

OBJS = $(SRCS:.c=.o)

TARGET = earthbound

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)