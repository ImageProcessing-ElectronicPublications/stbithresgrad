PNAME = stbithresgrad
CFLAGS = -std=c99 -O2 -Wall -Wextra -Wno-unused-but-set-variable -Wno-unused-parameter -Werror
LDLIBS = -lm -s
SRCS = src/gradsnip.c

all: $(PNAME)

$(PNAME): $(SRCS)
	$(CC) $(CFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -f $(PNAME)
