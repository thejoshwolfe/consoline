#ifndef _ASYNC_READLINE_H_
#define _ASYNC_READLINE_H_


void async_readline_init(const char * profile_name, const char * prompt);
void async_readline_deinit();
void async_readline_poll();

void async_readline_set_eof_handler(void (*eof_handler)());
void async_readline_set_line_handler(void (*line_handler)(char* line));

void async_readline_set_prompt(const char * prompt);

void async_readline_println(char* line);
void async_readline_printfln(const char* const fmt, ...);

// delete it when you're done with it
char * async_readline_getpass(const char* prompt);

#endif
