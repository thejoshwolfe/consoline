#ifndef _CONSOLINE_H_
#define _CONSOLINE_H_

// NOTE: if you're using any 'interruptible' system calls, like select(), ignore
// any EINTR errors you get and simply retry the system call. This is a side
// effect of handling ctrl+c.

// call this once before any other functions here. the prompt can be changed later.
void consoline_init(const char * profile_name, const char * prompt);
// typcially, provide this to atexit().
void consoline_deinit();
// you must call this frequently. 60Hz seems like a good speed.
void consoline_poll();

void consoline_set_eof_handler(void (*eof_handler)());
void consoline_set_line_handler(void (*line_handler)(char* line));
// return an expendable null-terminated array of expendable null-terminated strings.
// returning NULL or an empty array indicates no suggestions.
// results are not sorted.
void consoline_set_completion_handler(char** (*completion_handler)(char * line, int start, int end, const char * text));
// like all other string return values, delete it when you done with it.
char * consoline_get_completion_separators();

void consoline_set_prompt(const char * prompt);
void consoline_set_leave_entered_lines_on_stdout(char bool_value);

// enables bash-like ctrl+c, rather than killing the process.
// if you spawn child processes, you probably want them to ignore ctrl+c. see below.
void consoline_set_ctrl_c_handled(char bool_value);
// only call this as a child process after fork, before exec, if ctrl+c is handled in the parent.
void consoline_ignore_ctrl_c();

// need not include a newline
void consoline_println(char* line);
// need not include a newline. formatting like ptintf.
void consoline_printfln(const char* const fmt, ...);

// blocks. delete it when you're done with it
char * consoline_getpass(const char* prompt);

#endif
