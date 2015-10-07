# consoline

consoline is GNU Readline with better support for asynchronous output.

This is recommended for the console interface for server programs that print
messages to stdout while you're trying to type something on stdin.
consoline ensures that your input line is uninterrupted by printing stdout
above where you're typing the line of input.

## Usage

```
consoline [cmd...]
```

NOTE: if you're running python with this program, be sure to use `python -u`.

## Bonus Features

* **Autocomplete** by pressing Tab.
  The suggested words are all the words that have shown up in the input or the output.
  Disable with `--no-completion`.
* **Ctrl+C** kills the input line, not the program;
  Ctrl+C twice on a blank line kills the program.
  Disable with `-c`.
* Configurable **prompt**.
  Example: `--prompt='>>> '`

NOTE: consoline options must precede the command to run.

## Build (Ubuntu)

```
sudo apt-get install libreadline-dev
make
```

## Using consoline as a Library

You can use some of this functionality as a library.
See consoline.h for the API and `make libconsoline.so`.
