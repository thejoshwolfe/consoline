
.PHONEY: all
all: consoline

consoline: consoline.c consoline.h main.c
	gcc consoline.c main.c -lreadline -o $@

test: consoline.c consoline.h test.c
	gcc -g consoline.c test.c -lreadline -o $@

.PHONEY: clean
clean:
	rm -f consoline test

