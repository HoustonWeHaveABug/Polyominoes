POLYOMINOES_C_FLAGS=-O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

polyominoes: polyominoes.o
	gcc -o polyominoes polyominoes.o

polyominoes.o: polyominoes.c polyominoes.make
	gcc -c ${POLYOMINOES_C_FLAGS} -o polyominoes.o polyominoes.c

clean:
	rm -f polyominoes polyominoes.o
