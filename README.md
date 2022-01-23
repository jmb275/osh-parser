# Parser for `osh` for CSCE 451/851

This is a fork of the parser from the former GTA Yutaka Tsutano. It has a few bug fixes and is more tailored for the current version of the programming assignment.

## 1 Usage

Read [main.cpp](main.cpp) for usage and [command.hpp](command.hpp) for understanding the data structure.

### 1.1 Compiling & Running on CSE

A Makefile is provided to make compilation easy. For your programming assignment, this is where you would add any new commands for testing, or copying over the CSE servers, etc.

### 1.2 Parsing

`parse_command_string()` returns a vector of parsed commands (defined in
[command.hpp](command.hpp)) from a line of text input from the terminal. As an example:

```cpp
// 1. Get a line from the terminal.
std::string input_line;
std::getline(std::cin, input_line);
// input into terminal "echo hello world | sort | uniq > test.txt";

// 2. Parse.
std::vector<shell_command> shell_commands = parse_command_string(input_line);
```

This is all you need to do to parse the input string. Now you can step through the commands in the `shell_commands` vector to implement the rest of the assignment.

### 1.3 Printing the Parsed Commands

It is helpful to print the parsed commands when implementing the execution part
of the shell. You can do so with the following code:

```cpp
std::cout << "-------------------------\n";
for (const auto& cmd : shell_commands) {
    std::cout << cmd;
    std::cout << "-------------------------\n";
}
```

For example, the result of parsing `echo hello world | sort | uniq > test.txt` above would be:

```text
-------------------------
cmd: echo
arg: hello
arg: world
cin_file:
cin_mode: term
cout_file:
cout_mode: pipe
next_mode: always
-------------------------
cmd: sort
cin_file:
cin_mode: pipe
cout_file:
cout_mode: pipe
next_mode: always
-------------------------
cmd: uniq
cin_file:
cin_mode: pipe
cout_file: test.txt
cout_mode: file
next_mode: always
-------------------------
```
