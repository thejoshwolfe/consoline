/*
 * This program invokes a subprocess and wraps its stderr/stdout and stdin asynchronously.
 */

#include "async_readline.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int child_pid;
static int child_stdin_fd;
static int child_stdout_fd;
static fd_set child_stdout_fd_set;

static void eof_handler()
{
    // TODO: push eof down
}
static void line_handler(const char * line)
{
    async_readline_println(line);
}

static void poll_subprocess()
{
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
            exit(0);
            int status;
            waitpid(child_pid, &status, 0);
            exit(WEXITSTATUS(status));
        }
        // TODO: print single character
        async_readline_printfln("%c", c);
    }
}

static void launch_child_process(int argc, char ** argv)
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
        char** child_argv = (char **)malloc(argc * sizeof(char *));
        int i;
        for (i = 1; i < argc; i++)
            child_argv[i - 1] = argv[i];
        child_argv[argc - 1] = NULL;
        execvp(argv[0], child_argv);
        // failure
        fprintf(stderr, "Failed to start process %s\n", argv[0]);
        exit(1);
    }
    // parent
    close(child_stdin_pipe[0]);
    child_stdin_fd = child_stdin_pipe[1];
    child_stdout_fd = child_stdout_pipe[0];
    close(child_stdout_pipe[0]);
    FD_ZERO(&child_stdout_fd_set);
}

int main(int argc, char ** argv)
{
    async_readline_init("prog_wrapper", "");
    atexit(async_readline_deinit);
    async_readline_set_eof_handler(eof_handler);
    async_readline_set_line_handler(line_handler);

    launch_child_process(argc - 1, argv + 1);

    for (;;) {
        async_readline_poll();
        poll_subprocess();

        // poll input at 60 Hz or whatever
        struct timespec requested;
        memset(&requested, 0, sizeof(requested));
        requested.tv_nsec = 1000000000 / 60;
        nanosleep(&requested, NULL);
    }
    return 0;
}

