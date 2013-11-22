CC=gcc
CFLAGS=-Wall -g

SRC=main.c net.c parser.c
OBJ=main.o net.o parser.o
EXE=sws

TESTSRC=net.c parser.c test.c
TESTOBJ=net.o parser.o test.o
TESTEXE=unittest


${EXE}: ${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o ${EXE}

test: ${TESTOBJ}
	${CC} ${CFLAGS} ${TESTOBJ} -o ${TESTEXE}

clean:
	rm -f ${OBJ} ${EXE} ${TESTOBJ} ${TESTEXE} core.*
