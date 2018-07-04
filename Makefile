IDIR =.
CC=g++
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =.

LIBS=-lpthread

DEPS = raft.h

OBJ = test_sample.o raft.o 

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test_sample: $(OBJ) 
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o test_sample 
