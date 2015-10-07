#ifndef _CONSOLINE_H_
#define _CONSOLINE_H_

// NOTE: if you're using any 'interruptible' system calls, like select(), ignore
// any EINTR errors you get and simply retry the system call. This is a side
// effect of handling ctrl+c.

// call this once before any other functions here. the prompt can be changed later.
void consoline_init(const char * profile_name, const char * prompt);
// call this when you're done with these functions. typically, provide this to atexit().
// this makes sure your terminal is back to normal.
void consoline_deinit();
// you must call this frequently. 60Hz seems like a good speed.
void consoline_poll();

// use this instead of printf("%s\n", line).
void consoline_println(char* line);
// use this instead of printf(fmt, ...). need not include a newline.
void consoline_printfln(const char* const fmt, ...);

// this callback is called when a line of input has been entered.
// use this callback instead of any other way of reading from stdin.
void consoline_set_line_handler(void (*line_handler)(char* line));
// this callback is called when the end of input is encountered, (e.g. user types ctrl+d).
void consoline_set_eof_handler(void (*eof_handler)());

// provide an autocomplete handler function.
// the function should return a null-terminated array of null-terminated strings.
// each string and the array will be freed.
// returning NULL or an empty array indicates no suggestions.
// results are not sorted.
void consoline_set_completion_handler(char** (*completion_handler)(char * line, int start, int end, const char * text));
// such as " \t\n\"'`@$><=;|&{(".
// free the return value when you done with it.
char * consoline_get_completion_separators();

// such as ">>> "
void consoline_set_prompt(const char * prompt);

// defaults to 1, which is probably what people are used to
void consoline_set_leave_entered_lines_on_stdout(char bool_value);

// enables bash-like ctrl+c, rather than killing the process.
// if you spawn child processes, you probably want them to ignore ctrl+c. see below.
// defaults to 0.
void consoline_set_ctrl_c_handled(char bool_value);
// if you're handling ctrl+c, call this as a child process after fork before exec.
void consoline_ignore_ctrl_c();

// gets a "password" line, which means the input line is hidden as it's being typed.
// this function blocks.
// free the return value when you're done with it.
char * consoline_getpass(const char* prompt);

#endif
