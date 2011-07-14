/*
 * This example program prints a line every second while you're typing a line of input.
 * See the strcmp calls in line_handler for special things you can type to test features.
 */

#include "consoline.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static char printing = 1;

static void eof_handler()
{
    exit(0);
}
#define QUIT_COMMAND "quit"
#define PASSWORD_COMMAND "password"
#define STOP_COMMAND "stop"
#define START_COMMAND "start"
#define SHOW_COMMAND "show"
#define HIDE_COMMAND "hide"
static char * all_commands[] = {
    QUIT_COMMAND,
    PASSWORD_COMMAND,
    STOP_COMMAND,
    START_COMMAND,
    SHOW_COMMAND,
    HIDE_COMMAND,
    NULL,
};
static char * strip_spaces(char * line)
{
    int line_len = strlen(line);
    // skip leading spaces
    int i;
    for (i = 0; line[i] != '\0' && line[i] == ' '; i++)
        if (line[i] != ' ')
            break;
    int start = i;
    // skip trailing spaces
    for (i = line_len - 1; i >= 0 && line[i] == ' '; i--)
        if (line[i] != ' ')
            break;
    int end = i + 1;
    // copy middle
    int result_len = end - start;
    char * result = (char *)malloc((result_len + 1) * sizeof(char *));
    for (i = 0; i < result_len; i++)
        result[i] = line[i + start];
    result[i] = '\0';
    return result;
}
static void line_handler(char * line)
{
    char * stripped_line = strip_spaces(line);
    if (strcmp(stripped_line, QUIT_COMMAND) == 0) {
        exit(0);
    } else if (strcmp(stripped_line, PASSWORD_COMMAND) == 0) {
        char * password = consoline_getpass("password: ");
        consoline_printfln("you entered: %s", password);
        free(password);
    } else if (strcmp(stripped_line, STOP_COMMAND) == 0) {
        printing = 0;
    } else if (strcmp(stripped_line, START_COMMAND) == 0) {
        printing = 1;
    } else if (strcmp(stripped_line, SHOW_COMMAND) == 0) {
        consoline_set_leave_entered_lines_on_stdout(1);
    } else if (strcmp(stripped_line, HIDE_COMMAND) == 0) {
        consoline_set_leave_entered_lines_on_stdout(0);
    } else if (strcmp(stripped_line, ">>>") == 0) {
        consoline_set_prompt(">>> ");
    } else if (strcmp(stripped_line, "") == 0) {
        consoline_set_prompt("");
    }
    free(stripped_line);
}

static char ** completion_handler(char * line, int start, int end, const char * text)
{
    int text_len = strlen(text);
    char ** matches = (char **)malloc(sizeof(all_commands));
    int matches_index = 0;
    int i;
    int loop_terminator = sizeof(all_commands) / sizeof(char *) - 1;
    for (i = 0; i < loop_terminator; i++)
        if (strncmp(all_commands[i], text, text_len) == 0)
            matches[matches_index++] = strdup(all_commands[i]);
    matches[matches_index] = NULL;
    return matches;
}

int main()
{
    consoline_init("demo", "");
    atexit(consoline_deinit);
    consoline_set_eof_handler(eof_handler);
    consoline_set_line_handler(line_handler);
    consoline_set_completion_handler(completion_handler);
    //consoline_set_ctrl_c_handled();

    int i;
    for (i = 0;; i++) {
        if (printing && i % 60 == 0)
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

