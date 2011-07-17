/*
 * Adapted from code by Dawid Ciężarkiewicz.
 * https://github.com/dpc/xmppconsole/blob/master/src/io.c
 */

#include "consoline.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>

static const char* current_prompt = NULL;
static char current_leave_entered_lines_on_stdout = 1;
static void (*current_eof_handler)() = NULL;
static void (*current_line_handler)(char * line) = NULL;
static char** (*current_completion_handler)(char * line, int start, int end, const char * text) = NULL;

static fd_set stdin_fd_set;

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

static void done_with_input_line()
{
    if (current_leave_entered_lines_on_stdout) {
        // leave the input line
        // put cursor at the end of the input line
        rl_point = rl_end;
        rl_redisplay();
        // move cursor to new line after the input line
        rl_crlf();
        rl_on_new_line();
        // forget about the line that was being typed
        rl_replace_line("", 1);
    } else {
        // erase current line
        rl_set_prompt("");
        rl_replace_line("", 1);
        rl_redisplay();
        rl_set_prompt(current_prompt);
    }
    // don't redisplay the current prompt yet
}

static int handle_enter(int x, int y)
{
    char* line = NULL;

    line = rl_copy_text(0, rl_end);
    done_with_input_line();

    if (current_line_handler != NULL)
        current_line_handler(line);
    if (strcmp(line, "") != 0)
        add_history(line);

    free(line);

    rl_redisplay();

    // force readline to think that the current line was "eaten" and executed
    rl_done = 1;
    return 0;
}

static void install_line_handler()
{
    if (rl_bind_key(RETURN, handle_enter)) {
        consoline_printfln("failed to bind RETURN");
        abort();
    }
    rl_callback_handler_install(current_prompt, handle_line_fake);
}

static void remove_line_handler()
{
    rl_unbind_key(RETURN);
    rl_callback_handler_remove();
}

static char pending_ctrl_c = 0;
void consoline_poll()
{
    struct timeval no_time;
    memset(&no_time, 0, sizeof(no_time));

    for (;;) {
        if (pending_ctrl_c) {
            pending_ctrl_c = 0;
            done_with_input_line();
            // print the prompt
            rl_redisplay();
        }
        FD_SET(STDIN_FILENO, &stdin_fd_set);
        int count = select(FD_SETSIZE, &stdin_fd_set, NULL, NULL, &no_time);
        if (count < 0) {
            if (errno == EINTR)
                continue; // ignore and try again
            exit(1);
        } else if (count == 0) {
            return;
        }
        rl_callback_read_char();
    }
}

void consoline_set_prompt(const char * prompt)
{
    current_prompt = prompt;
    rl_set_prompt(current_prompt);
}

void consoline_set_leave_entered_lines_on_stdout(char bool_value)
{
    current_leave_entered_lines_on_stdout = bool_value;
}

static void set_signal_handlers(void (*handler)(int code))
{
    struct sigaction handler_info;
    handler_info.sa_handler = handler;
    sigemptyset(&handler_info.sa_mask);
    handler_info.sa_flags = 0;
    sigaction(SIGINT, &handler_info, NULL);
    sigaction(SIGQUIT, &handler_info, NULL);
    sigaction(SIGTERM, &handler_info, NULL);
}

static void signal_handler(int code)
{
    switch (code) {
        case SIGINT:
            // don't do anything non-trivial in a signal handler
            pending_ctrl_c = 1;
            break;
        case SIGQUIT:
        case SIGTERM:
            // cleanup readline
            rl_cleanup_after_signal();
            // detach this handler and resend to ourselves
            set_signal_handlers(SIG_DFL);
            kill(getpid(), code);
            break;
    }
}

static char ctrl_c_handled = 0;
void consoline_set_ctrl_c_handled(char bool_value)
{
    if (ctrl_c_handled == bool_value)
        return;
    ctrl_c_handled = bool_value;
    rl_catch_signals = !bool_value;
    // register the one that actually does handle it second.
    if (bool_value)
        rl_set_signals();
    set_signal_handlers(bool_value ? signal_handler : SIG_DFL);
    if (!bool_value)
        rl_set_signals();
}
void consoline_ignore_ctrl_c()
{
    set_signal_handlers(SIG_IGN);
}

static void async_print(void (*print_func)(void*), void* data)
{
    char* saved_line = rl_copy_text(0, rl_end);
    int saved_point = rl_point;

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
void consoline_printfln(const char* const fmt, ...)
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
void consoline_println(char* line)
{
    async_print(print_func, line);
}

struct getpass_data {
    const char * prompt;
    char ** return_pointer;
};
static void getpass_func(void* d)
{
    struct getpass_data* data = (struct getpass_data*)d;
    char* result;

    result = getpass(data->prompt);
    *(data->return_pointer) = strdup(result);
}
char* consoline_getpass(const char * prompt)
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

static char ** current_matches = NULL;
static char * match_generator(const char * text, int index)
{
    return current_matches[index];
}

static char** attempt_completion(const char *text, int start, int end)
{
    if (current_completion_handler != NULL) {
        // try completion
        char ** matches = current_completion_handler(rl_line_buffer, start, end, text);
        if (matches != NULL) {
            // array of some length given
            if (matches[0] != NULL) {
                // non-empty array given. we have suggestions
                if (current_matches != NULL)
                    free(current_matches);
                current_matches = matches;
                return rl_completion_matches(text, match_generator);
            }
            // delete the empty array
            free(matches);
        }
    }
    // no suggestions.
    // disable default completion behavior.
    rl_attempted_completion_over = 1;
    return NULL;
}

void consoline_init(const char * profile_name, const char * prompt)
{
    FD_ZERO(&stdin_fd_set);
    rl_readline_name = profile_name;
    rl_attempted_completion_function = attempt_completion;

    consoline_set_prompt(prompt);
    install_line_handler();

    // doing this here makes things work better for some reason
    consoline_set_ctrl_c_handled(1);
    consoline_set_ctrl_c_handled(0);
}

void consoline_set_eof_handler(void (*eof_handler)())
{
    current_eof_handler = eof_handler;
}
void consoline_set_line_handler(void (*line_handler)(char* line))
{
    current_line_handler = line_handler;
}
void consoline_set_completion_handler(char** (*completion_handler)(char * line, int start, int end, const char * text))
{
    current_completion_handler = completion_handler;
}
char * consoline_get_completion_separators()
{
    return strdup(rl_basic_word_break_characters);
}

void consoline_deinit()
{
    rl_set_prompt("");
    rl_replace_line("", 0);
    rl_redisplay();
    remove_line_handler();
}

