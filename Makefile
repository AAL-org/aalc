OUTPUT=aalc
OBJ=main.o lex.o parse.o sema.o tree.o

CFLAGS=-Isrxk.h/
LDFLAGS=

.PHONY: all clean debug release install uninstall

# Default target is debug
all: debug

debug: CFLAGS += -g -O0 -Ddebug
debug: ${OUTPUT}

release: CFLAGS += -O2 -Drelease
release: ${OUTPUT}

${OUTPUT}: ${OBJ}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${OUTPUT} $^

# Install the built program
install: ${OUTPUT}
	cp -p ${OUTPUT} /usr/bin/aalc

# Uninstall the program
uninstall:
	rm -f /usr/bin/aalc

# Clean build files
clean:
	rm -f ${OBJ}
	rm -f ${OUTPUT}
