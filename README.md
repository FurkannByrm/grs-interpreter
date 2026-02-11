# GRS Interpreter

An interpreter for GRS/KRL industrial robot programming languages.

## Overview

GRS Interpreter can parse and execute programs written in GRS/KRL, a language used for programming industrial robots. It supports variable declarations, control structures, motion commands, and more.

## Features

- Lexical analysis and tokenization
- Abstract Syntax Tree (AST) generation
- Intermediate code generation
- Program execution engine
- IDE support for ZeroBrane Studio and Scintilla-based editors

## Quick Start

### Requirements

- CMake 3.10 or higher
- C++17 compatible compiler
- [frozen](https://github.com/serge-sans-paille/frozen) (constexpr map library)

### Building

#### Linux / macOS

```bash
git clone https://github.com/FurkannByrm/grs-interpreter.git
cd grs-interpreter/grs_interpreter

mkdir build && cd build
cmake ..
make

./interpreter
```

#### Windows

You can build with Visual Studio or MinGW.

**Visual Studio (Developer Command Prompt):**

```cmd
git clone https://github.com/FurkannByrm/grs-interpreter.git
cd grs-interpreter\grs_interpreter

mkdir build && cd build
cmake ..
cmake --build . --config Release

Release\interpreter.exe
```

**MinGW (MSYS2 / Git Bash):**

```bash
git clone https://github.com/FurkannByrm/grs-interpreter.git
cd grs-interpreter/grs_interpreter

mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make

./interpreter.exe
```

## IDE Setup

Full IDE support is available for the GRS language.

### Supported IDEs

- **ZeroBrane Studio** — Syntax highlighting, autocomplete, code folding, integrated interpreter
- **Scintilla-based Editors** — Notepad++ (Windows), SciTE (cross-platform), Geany (Linux)

### Installation

#### Linux / macOS

```bash
# 1. Build the interpreter
cd grs_interpreter
mkdir -p build && cd build
cmake .. && make
cd ../..

# 2. Install IDE support
cd ide
chmod +x install.sh
./install.sh
```

The install script copies ZeroBrane files, symlinks the interpreter binary to `~/bin/interpreter`, and adds it to your PATH. After installation you can run `.grs` files from any directory with F5.

#### Windows

```cmd
REM 1. Build the interpreter
cd grs_interpreter\build
cmake ..
cmake --build . --config Release
cd ../..

REM 2. Install IDE support
cd ide
install.bat
```

The install script copies ZeroBrane Studio files to `%USERPROFILE%\.zbstudio` and sets up syntax highlighting, autocomplete, and the interpreter plugin. Add the directory containing `interpreter.exe` to your system PATH so you can run `.grs` files from any location with F5.

For detailed instructions, see [ide/README.md](ide/README.md).

## Language Features

### Data Types
- `INT` — Integer
- `REAL` — Real number
- `BOOL` — Boolean
- `CHAR` — Character
- `POS` — Position
- `E6POS` — Extended position
- `AXIS` — Axis
- `FRAME` — Coordinate frame

### Control Structures
- `IF...THEN...ELSE...ENDIF`
- `FOR...TO...ENDFOR`
- `WHILE...ENDWHILE`
- `SWITCH...CASE...ENDSWITCH`

### Motion Commands
- `PTP` — Point-to-point motion
- `LIN` — Linear motion
- `CIRC` — Circular motion
- `SPLINE` — Spline motion

## Example Code

```grs
DEF ExampleProgram()
  DECL INT counter := 5
  DECL REAL speed := 0.75
  DECL BOOL is_safe := TRUE
  DECL POS target

  target := {X 500, Y 0, Z 600, A 0, B 90, C 0}

  PTP target
  LIN target

  IF counter > 3 THEN
    speed := 1.0
  ELSE
    speed := 0.5
  ENDIF

  WAIT(1000)
END
```

More examples: [ide/sample.grs](ide/sample.grs) | [grs_interpreter/tests/](grs_interpreter/tests/)

## Tests

Test files are located in `grs_interpreter/tests/`:

- `executor_test.txt` — Executor tests
- `example_krl_code.txt` — KRL code examples
- `assign_and_type_convertion_test.txt` — Type conversion tests
- `if_else_wait_test.grs` — Control structure tests
- `pos_type_convertion.txt` — Position type tests

## Contributing

Contributions are welcome!

1. Fork the repository
2. Create a feature branch
   ```bash
   git checkout -b feature/amazing-feature
   ```
3. Commit your changes
   ```bash
   git commit -m 'Add some amazing feature'
   ```
4. Push your branch
   ```bash
   git push origin feature/amazing-feature
   ```
5. Create a Pull Request

## License

This project is open source.

## Links

- **Repository**: [github.com/FurkannByrm/grs-interpreter](https://github.com/FurkannByrm/grs-interpreter)
- **IDE Setup**: [ide/README.md](ide/README.md)
- **Examples**: [examples/](examples/)

## Contact

For questions, please use GitHub Issues.

---

If you like this project, don't forget to give it a star!
