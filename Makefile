CC=gcc
CFLAGS=-Wall -g

SRC=net.c parser.c response_builder.c serve.c path.c daemonize.c
OBJ=net.o parser.o response_builder.o serve.o path.o daemonize.o
EXE=sws

MAINSRC=${SRC} main.c
MAINOBJ=${OBJ} main.o

TESTSRC=parser.c response_builder.c test.c
TESTOBJ=parser.o response_builder.o test.o
TESTEXE=unittest

${EXE}: ${MAINOBJ}
	${CC} ${CFLAGS} ${MAINOBJ} -o ${EXE}

test: ${TESTOBJ}
	${CC} ${CFLAGS} ${TESTOBJ} -o ${TESTEXE}

clean:
	rm -f *.o ${EXE} ${TESTEXE} core.*
