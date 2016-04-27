CC = gcc
CFLAGS = -Wall -c -o
LDFLAGS = -Wall -lcrypt -o
EXE = userv
RM = rm -f

OBJECTS = main.o data.o networking.o misc.o common.o logging.o signal.o context.o

all: $(OBJECTS)
	$(CC) $(LDFLAGS) $(EXE) $(OBJECTS)

clean:
	$(RM) $(OBJECTS) $(EXE)

main.o: main.c
	$(CC) $(CFLAGS) main.o main.c

data.o: data.c
	$(CC) $(CFLAGS) data.o data.c

networking.o: networking.c
	$(CC) $(CFLAGS) networking.o networking.c

misc.o: misc.c
	$(CC) $(CFLAGS) misc.o misc.c

common.o: common.c
	$(CC) $(CFLAGS) common.o common.c

logging.o: logging.c
	$(CC) $(CFLAGS) logging.o logging.c

signal.o: signal.c
	$(CC) $(CFLAGS) signal.o signal.c

context.o: context.c
	$(CC) $(CFLAGS) context.o context.c

