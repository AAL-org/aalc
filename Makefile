OUTPUT=aalc
OBJ=main.o

CFLAGS=-g -O2 -Wall -Wextra
CPPFLAGS=-Ddebug
LDFLAGS=

.PHONY: all clean
all: ${OUTPUT}

%.o: %.cpp
	${CXX} ${CFLAGS} ${CPPFLAGS} -c -o $@ $<

${OUTPUT}: ${OBJ}
	${CXX} ${CFLAGS} ${LDFLAGS} -o $@ $<

clean:
	rm -f ${OBJ}
	rm -f ${OUTPUT}