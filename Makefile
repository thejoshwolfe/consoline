
.PHONEY: all
all: consoline

consoline: main.c consoline.c consoline.h HistoryDatabase.c HistoryDatabase.h RbTree.c RbTree.h
	gcc -Wall -g main.c consoline.c HistoryDatabase.c RbTree.c -lreadline -o $@

libconsoline.so: consoline.c consoline.h
	gcc -Wall -g consoline.c -lreadline -fPIC -shared -o $@

test: consoline.c consoline.h test.c
	gcc -Wall -g consoline.c test.c -lreadline -o $@

libtest: consoline.h test.c libconsoline.so
	gcc -Wall -g test.c -lreadline -L. -lconsoline -o $@

run-libtest: libtest
	@LD_LIBRARY_PATH=. ./libtest

.PHONEY: clean
clean:
	rm -f consoline libconsoline.so test libtest

