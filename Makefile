CC=gcc
CFLAGS=-Wall
LIBS=
DEPS=common
TARGETS=extract temp-candidates key-states crc32 k2-initial-candidates k2-prev-candidates

%.o: %.c $(patsubst %, %.h, $(DEPS))
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(TARGETS)

$(TARGETS): %: %.o $(patsubst %, %.o, $(DEPS))
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(TARGETS) *.o
