CC = gcc
CFLAGS = -g -Wall -I.
EXECS = oss user
DEPS = lib/myclock.c lib/pagetable.c lib/shm.c lib/sem.c

all: $(EXECS)

oss: $(DEPS)

user: $(DEPS)

clean:
	rm -f *.o $(EXECS) oss.out
