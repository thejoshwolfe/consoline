#ifndef _CONSOLINE_H_
#define _CONSOLINE_H_


void consoline_init(const char * profile_name, const char * prompt);
void consoline_deinit();
void consoline_poll();

void consoline_set_eof_handler(void (*eof_handler)());
void consoline_set_line_handler(void (*line_handler)(char* line));

void consoline_set_prompt(const char * prompt);

void consoline_println(char* line);
void consoline_printfln(const char* const fmt, ...);

// delete it when you're done with it
char * consoline_getpass(const char* prompt);

#endif
