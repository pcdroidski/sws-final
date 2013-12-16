CC=gcc
CFLAGS=-Wall -g
LIBS=-lmagic

SRC=net.c parser.c response_builder.c serve.c path.c daemonize.c dir_index.c loggig.c
OBJ=net.o parser.o response_builder.o serve.o path.o daemonize.o dir_index.o logging.o
EXE=sws

MAINSRC=${SRC} main.c
MAINOBJ=${OBJ} main.o

TESTSRC=parser.c response_builder.c test.c path.c dir_index.c logging.c
TESTOBJ=parser.o response_builder.o test.o path.o dir_index.o logging.o
TESTEXE=unittest

${EXE}: ${MAINOBJ}
	${CC} ${CFLAGS} ${MAINOBJ} -o ${EXE} ${LIBS}

test: ${TESTOBJ}
	${CC} ${CFLAGS} ${TESTOBJ} -o ${TESTEXE} ${LIBS}

clean:
	rm -f *.o ${EXE} ${TESTEXE} core.*
