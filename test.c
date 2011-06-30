/*
 * This example program prints a line every second while you're typing a line of input.
 * See the strcmp calls in line_handler for special things you can type to test features.
 */

#include "consoline.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static void eof_handler()
{
    exit(0);
}
static void line_handler(char * line)
{
    consoline_println(line);
    if (strcmp(line, "quit") == 0) {
        exit(0);
    } else if (strcmp(line, "password") == 0) {
        char * password = consoline_getpass("password: ");
        consoline_printfln("you entered: %s", password);
        free(password);
    } else if (strcmp(line, ">>>") == 0) {
        consoline_set_prompt(">>> ");
    } else if (strcmp(line, "") == 0) {
        consoline_set_prompt("");
    }
}

int main()
{
    consoline_init("demo", "");
    atexit(consoline_deinit);
    consoline_set_eof_handler(eof_handler);
    consoline_set_line_handler(line_handler);

    int i;
    for (i = 0;;i++) {
        if (i % 60 == 0)
            consoline_printfln("async %d", i / 60);
        consoline_poll();

        // poll input at 60 Hz or whatever
        struct timespec requested;
        memset(&requested, 0, sizeof(requested));
        requested.tv_nsec = 1000000000 / 60;
        nanosleep(&requested, NULL);
    }
    return 0;
}

