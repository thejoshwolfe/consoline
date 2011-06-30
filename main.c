/*
 * This program invokes a subprocess and wraps its stderr/stdout and stdin asynchronously.
 * stdout lines from the subprocess are line-buffered.
 */

#include "consoline.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wait.h>

static int child_pid;
static int child_stdin_fd;
static int child_stdout_fd;
static fd_set child_stdout_fd_set;

static void eof_handler()
{
    close(child_stdin_fd);
}
static void line_handler(char * line)
{
    consoline_println(line);
    write(child_stdin_fd, line, strlen(line));
    static char newline_char = '\n';
    write(child_stdin_fd, &newline_char, 1);
}

static void poll_subprocess()
{
    static int line_buffer_capacity = 0x100;
    static char * line_buffer = NULL;
    static int line_buffer_cursor = 0;

    struct timeval no_time;
    memset(&no_time, 0, sizeof(no_time));

    for (;;) {
        FD_SET(child_stdout_fd, &child_stdout_fd_set);
        int ready_count = select(FD_SETSIZE, &child_stdout_fd_set, NULL, NULL, &no_time);
        if (ready_count < 0) {
            exit(1);
        } else if (ready_count == 0) {
            return;
        }
        // read a character
        char c;
        int read_count = read(child_stdout_fd, &c, 1);
        if (read_count == 0) {
            // stdout has been closed. wait and terminate with child's exit code.
            int status;
            waitpid(child_pid, &status, 0);
            exit(WEXITSTATUS(status));
        }
        // expand buffer if needed
        if (line_buffer == NULL || !(line_buffer_cursor + 1 < line_buffer_capacity)) {
            line_buffer_capacity *= 2;
            line_buffer = (char *)realloc(line_buffer, line_buffer_capacity * sizeof(char));
        }
        if (c != '\n') {
            // buffer the char
            line_buffer[line_buffer_cursor++] = c;
        } else {
            // flush line buffer. don't include newline.
            line_buffer[line_buffer_cursor] = '\0';
            consoline_println(line_buffer);
            line_buffer_cursor = 0;
        }
    }
}

static void launch_child_process(char ** child_argv)
{
    int child_stdin_pipe[2];
    if (pipe(child_stdin_pipe) == -1)
        exit(1);
    int child_stdout_pipe[2];
    if (pipe(child_stdout_pipe) == -1)
        exit(1);
    child_pid = fork();
    if (child_pid < 0) {
        exit(1);
    } else if (child_pid == 0) {
        // child
        dup2(child_stdin_pipe[0], STDIN_FILENO);
        dup2(child_stdout_pipe[1], STDOUT_FILENO);
        dup2(child_stdout_pipe[1], STDERR_FILENO);
        // child doesn't need any of these
        close(child_stdin_pipe[0]);
        close(child_stdin_pipe[1]);
        close(child_stdout_pipe[0]);
        close(child_stdout_pipe[1]);
        // exec
        execvp(child_argv[0], child_argv);
        // failure
        fprintf(stderr, "ERROR: Unable to start process: %s\n", child_argv[0]);
        exit(1);
    }
    // parent
    close(child_stdin_pipe[0]);
    child_stdin_fd = child_stdin_pipe[1];
    child_stdout_fd = child_stdout_pipe[0];
    close(child_stdout_pipe[1]);
    FD_ZERO(&child_stdout_fd_set);
}

int main(int argc, char ** argv)
{
    if (argc <= 1) {
        fprintf(stderr, "consoline version 0.0\n", argv[0]);
        fprintf(stderr, "Usage: %s command [args]\n", argv[0]);
        exit(1);
    }
    // prepare the child's command
    int child_argv_size = argc;
    int child_argv_offset = -1;
    char** child_argv = (char**)malloc(child_argv_size * sizeof(char *));
    int i;
    for (i = 0; i < child_argv_size - 1; i++) {
        child_argv[i] = argv[i - child_argv_offset];
    }
    child_argv[i] = NULL;

    consoline_init("prog_wrapper", "");
    atexit(consoline_deinit);
    consoline_set_eof_handler(eof_handler);
    consoline_set_line_handler(line_handler);

    launch_child_process(child_argv);

    for (;;) {
        consoline_poll();
        poll_subprocess();

        // poll input at 60 Hz or whatever
        struct timespec requested;
        memset(&requested, 0, sizeof(requested));
        requested.tv_nsec = 1000000000 / 60;
        nanosleep(&requested, NULL);
    }
}

