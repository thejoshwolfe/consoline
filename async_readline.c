/*
 * Adapted from code by Dawid Ciężarkiewicz.
 * https://github.com/dpc/xmppconsole/blob/master/src/io.c
 */

#include "async_readline.h"

#include <readline/readline.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

static const char* current_prompt = NULL;
static fd_set stdin_fd_set;
static void (*current_eof_handler)() = NULL;
static void (*current_line_handler)(const char * line) = NULL;

static void handle_line_fake(char* line)
{
    if (line != NULL) {
        rl_set_prompt(current_prompt);
        rl_already_prompted = 1;
    } else {
        if (current_eof_handler != NULL)
            current_eof_handler();
    }
}

static int handle_enter(int x, int y)
{
    char* line = NULL;

    line = rl_copy_text(0, rl_end);
    rl_set_prompt("");
    rl_replace_line("", 1);
    rl_redisplay();

    if (current_line_handler != NULL)
        current_line_handler(line);

    if (strcmp(line, "") != 0) {
        add_history(line);
    }
    free(line);

    rl_set_prompt(current_prompt);
    rl_redisplay();

    /* force readline to think that the current line was "eaten" and executed */
    rl_done = 1;
    return 0;
}

static void install_line_handler()
{
    if (rl_bind_key(RETURN, handle_enter)) {
        async_readline_printfln("failed to bind RETURN");
        abort();
    }
    rl_callback_handler_install(current_prompt, handle_line_fake);
}

static void remove_line_handler()
{
    rl_unbind_key(RETURN);
    rl_callback_handler_remove();
}

void async_readline_poll()
{
    struct timeval no_time;
    memset(&no_time, 0, sizeof(no_time));

    for (;;) {
        FD_SET(STDIN_FILENO, &stdin_fd_set);
        int count = select(FD_SETSIZE, &stdin_fd_set, NULL, NULL, &no_time);
        if (count < 0) {
            exit(1);
        } else if (count == 0) {
            return;
        }
        rl_callback_read_char();
    }
}

void async_readline_set_prompt(const char * prompt)
{
    current_prompt = prompt;
    rl_set_prompt(current_prompt);
    rl_redisplay();
}

static void async_print(void (*print_func)(void*), void* data)
{
    char* saved_line;
    int saved_point;

    saved_point = rl_point;
    saved_line = rl_copy_text(0, rl_end);

    rl_set_prompt("");
    rl_replace_line("", 0);
    rl_redisplay();
    (*print_func)(data);

    rl_set_prompt(current_prompt);
    rl_replace_line(saved_line, 0);
    rl_point = saved_point;
    rl_redisplay();

    free(saved_line);
}

struct printf_data {
    const char* fmt;
    va_list args;
};
static void printf_func(void* data)
{
    struct printf_data* d = (struct printf_data*)data;

    vprintf(d->fmt, d->args);
    printf("\n");
}
void async_readline_printfln(const char* const fmt, ...)
{
    struct printf_data d;
    d.fmt = fmt;

    va_start(d.args, fmt);
    async_print(printf_func, &d);
    va_end(d.args);
}

static void print_func(void* data)
{
    puts((char *)data);
}
void async_readline_println(char * line)
{
    async_print(print_func, line);
}

struct getpass_data {
    const char * prompt;
    char ** return_pointer;
};
void getpass_func(void* d)
{
    struct getpass_data* data = (struct getpass_data*)d;
    char* result;

    result = getpass(data->prompt);
    *(data->return_pointer) = strdup(result);
}
char* async_readline_getpass(const char * prompt)
{
    struct getpass_data data;
    data.prompt = prompt;
    char * result;
    data.return_pointer = &result;
    remove_line_handler();
    async_print(getpass_func, &data);
    install_line_handler();
    return result;
}

static char** attempt_completion(const char *text, int start, int end)
{
    rl_attempted_completion_over = 1;
    return NULL;
}

void async_readline_init(const char * profile_name, const char * prompt)
{
    FD_ZERO(&stdin_fd_set);
    rl_readline_name = profile_name;
    async_readline_set_prompt(prompt);
    install_line_handler();
    rl_attempted_completion_function = attempt_completion;
}

void async_readline_set_eof_handler(void (*eof_handler)())
{
    current_eof_handler = eof_handler;
}
void async_readline_set_line_handler(void (*line_handler)(const char* line))
{
    current_line_handler = line_handler;
}

void async_readline_deinit()
{
    rl_set_prompt("");
    rl_replace_line("", 0);
    rl_redisplay();
    remove_line_handler();
}

