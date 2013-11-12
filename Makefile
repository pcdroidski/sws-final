CC=gcc
CFLAGS=-Wall

SRC=main.c net.c
OBJ=main.o net.o
EXE=sws

all: clean ${EXE}

${EXE}: ${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o ${EXE}

clean:
	rm -f ${OBJ} ${EXE}
