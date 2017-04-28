CC=gcc
CFLAGS=-Wall
LIBS=
DEPS=common
TARGETS=extract temp-candidates key-states crc32 k2-partial-candidates \
	k1-msbs k1-08088405-inv k1-candidate-lists k0-reconstruct-list \
	k0-recover-4 key-prev-state flow-gen-message k2-candidate-lists \
	k0-lsbs flow-crack

%.o: %.c $(patsubst %, %.h, $(DEPS))
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(TARGETS)

$(TARGETS): %: %.o $(patsubst %, %.o, $(DEPS))
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(TARGETS) *.o
