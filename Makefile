CC=gcc
CFLAGS=-Wall -g
LIBS=-lmagic

SRC=net.c parser.c response_builder.c serve.c daemonize.c
OBJ=net.o parser.o response_builder.o serve.o daemonize.o
EXE=sws

MAINSRC=${SRC} main.c
MAINOBJ=${OBJ} main.o

TESTSRC=parser.c response_builder.c test.c
TESTOBJ=parser.o response_builder.o test.o
TESTEXE=unittest

${EXE}: ${MAINOBJ}
	${CC} ${CFLAGS} ${MAINOBJ} -o ${EXE} ${LIBS}

test: ${TESTOBJ}
	${CC} ${CFLAGS} ${TESTOBJ} -o ${TESTEXE} ${LIBS}

clean:
	rm -f *.o ${EXE} ${TESTEXE} core.*
