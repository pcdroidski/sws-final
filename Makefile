CC=gcc
CFLAGS=-Wall -g
LIBS=-lmagic

<<<<<<< HEAD
SRC=net.c parser.c response_builder.c serve.c daemonize.c
OBJ=net.o parser.o response_builder.o serve.o daemonize.o
=======
SRC=net.c parser.c response_builder.c serve.c path.c daemonize.c
OBJ=net.o parser.o response_builder.o serve.o path.o daemonize.o
>>>>>>> e607ec27052a07b4c9338367db5731630443c6a7
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
