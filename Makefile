
.PHONEY: all
all: consoline

consoline libconsoline.so: main.c consoline.c consoline.h HistoryDatabase.c HistoryDatabase.h RbTree.c RbTree.h
	gcc -Wall -g main.c consoline.c HistoryDatabase.c RbTree.c -lreadline $(if $(subst consoline,,$@),-fPIC -shared,) -o $@

test: consoline.c consoline.h test.c
	gcc -Wall -g consoline.c test.c -lreadline -o $@

.PHONEY: clean
clean:
	rm -f consoline libconsoline.so test

