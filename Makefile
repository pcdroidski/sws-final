CC=gcc
CFLAGS=-Wall -g

SRC=net.c parser.c response_builder.c
OBJ=net.o parser.o response_builder.o
EXE=sws

MAINSRC=${SRC} main.c
MAINOBJ=${OBJ} main.o

TESTSRC=${SRC} test.c
TESTOBJ=${OBJ} test.o
TESTEXE=unittest

${EXE}: ${MAINOBJ}
	${CC} ${CFLAGS} ${MAINOBJ} -o ${EXE}

test: ${TESTOBJ}
	${CC} ${CFLAGS} ${TESTOBJ} -o ${TESTEXE}

clean:
	rm -f *.o ${EXE} ${TESTEXE} core.*
