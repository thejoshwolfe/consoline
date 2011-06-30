
.PHONEY: all
all: consoline

consoline: consoline.c consoline.h main.c
	gcc consoline.c main.c -lreadline -o $@

.PHONEY: clean
clean:
	rm -f consoline

