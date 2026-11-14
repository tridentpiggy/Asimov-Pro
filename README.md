# Asimov Editor

Asimov Editor is a small, dependency-free C++17 terminal editor for source files in the new Asimov programming language. It includes basic line editing, file open/save, and lightweight syntax coloring for Asimov keywords, strings, numbers, and comments.

## Build

```sh
cmake -S . -B build
cmake --build build
```

The executable is `build/asimov-editor` (on Windows, `build/Debug/asimov-editor.exe` or the corresponding configuration directory).

## Run

```sh
./build/asimov-editor hello.asimov
```

The editor starts with an empty buffer if the file does not exist. Enter one of the following commands at the `asimov>` prompt:

- Type a line of source code to append it to the buffer.
- `:list` — display the buffer with syntax colors.
- `:insert N` — insert one line before line N, then enter the line at the next prompt.
- `:replace N` — replace line N with the next line you enter.
- `:delete N` — delete line N.
- `:open PATH` — load a file into the buffer.
- `:save [PATH]` — save to the current file or the specified path.
- `:help` — show command help.
- `:quit` — exit (asks before discarding unsaved changes).

Line numbers in commands start at 1. Comments begin with `//`. The initial keyword set is intentionally small and can evolve alongside the Asimov language design.
