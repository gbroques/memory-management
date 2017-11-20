CC = gcc
CFLAGS = -g -Wall -I.
EXECS = oss user
DEPS = lib/shm.c lib/sem.c

all: $(EXECS)

oss: $(DEPS)

clean:
	rm -f *.o $(EXECS)
