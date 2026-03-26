# GRS Interpreter

An interpreter for GRS/KRL industrial robot programming languages with a full-featured ZeroBrane Studio IDE integration, TCP hardware bridge, and step debugger.

## Overview

GRS Interpreter parses and executes programs written in GRS/KRL — a language used for programming industrial robots (KUKA KRL subset). It supports variable declarations, control structures, motion commands (PTP, LIN, CIRC, SPLINE), digital I/O, and real-time hardware communication over TCP.

## Features

- **Lexer & Parser** — Tokenization, AST generation, and KRL syntax support
- **Step Executor** — Statement-by-statement execution with breakpoints
- **Three Execution Modes** — Run (batch), Step (interactive terminal), Debug (JSON protocol for IDE)
- **TCP Hardware Bridge** — Unified 128-byte protocol for sending motion commands and I/O to a real robot controller (Holy / EtherCAT)
- **ZeroBrane Studio IDE** — Syntax highlighting, autocomplete, F5 run, F6 debug with stepping, breakpoints, variable inspection, and I/O monitoring
- **Offline Mode** — Runs without hardware; automatically falls back when TCP connection is unavailable

## Requirements

- Linux (Ubuntu 20.04+ recommended)
- CMake 3.10+
- GCC 9+ or any C++17 compiler
- [FurkannByrm](https://github.com/FurkannByrm/constexpr_map) header-only library (constexpr map)

### Installing Dependencies

```bash
# Build tools
sudo apt update
sudo apt install build-essential cmake

# FurkannByrm library (header-only — install to home directory)
git clone https://github.com/FurkannByrm/constexpr_map.git ~/constexpr_map
```

> The CMakeLists.txt expects FurkannByrm headers at `$HOME/constexpr_map/include`. If you install it elsewhere, update the path in `grs_interpreter/CMakeLists.txt`.

## Building

```bash
git clone https://github.com/FurkannByrm/grs-interpreter.git
cd grs-interpreter/grs_interpreter

mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

This produces two binaries in the `build/` directory:

| Binary | Purpose |
|--------|---------|
| `interpreter` | Basic interpreter (legacy, runs programs without step/debug) |
| `grs_step` | Full-featured executor with Run, Step, and Debug modes + TCP support |

## Usage

### Run Mode (batch execution)

Executes the entire program, printing robot commands to stdout:

```bash
./grs_step /path/to/program.grs
```

### Run with TCP Hardware

Connects to a robot controller over TCP, sending motion and I/O commands:

```bash
./grs_step /path/to/program.grs --tcp 10.42.0.43:12345
```

If the TCP connection fails, it automatically falls back to offline mode.

### Step Mode (interactive terminal)

Step through the program one statement at a time:

```bash
./grs_step /path/to/program.grs --step
```

Commands in step mode:
- `Enter` — Step one statement
- `r` — Run until breakpoint or end
- `b <line>` — Set breakpoint at a line
- `v` — Show variables
- `io` — Show I/O state
- `q` — Quit

### Debug Mode (JSON protocol for IDE)

Used by ZeroBrane Studio. Communicates via JSON on stdin/stdout:

```bash
./grs_step /path/to/program.grs --debug
```

This mode is not intended for manual use — the IDE plugin handles it automatically.

## IDE Setup (ZeroBrane Studio)

Full IDE documentation: [ide/README.md](ide/README.md)

### Quick Setup

```bash
# 1. Install ZeroBrane Studio
chmod +x ZeroBraneStudioEduPack-2.01-linux.sh
./ZeroBraneStudioEduPack-2.01-linux.sh

# 2. Build the interpreter
cd grs_interpreter && mkdir -p build && cd build
cmake .. && make -j$(nproc)
cd ../..

# 3. Install IDE support
cd ide && chmod +x install.sh && ./install.sh
```

### IDE Keyboard Shortcuts

| Key | Action |
|-----|--------|
| **F5** | Run program (batch mode) |
| **F6** | Start debug session |
| **F10** | Step one statement |
| **F8** | Continue (run until breakpoint or end) |
| **Shift+F5** | Stop debug session |
| **F7** | Show variables |
| **Ctrl+F7** | Show I/O state |
| **Margin click** | Toggle breakpoint |

## Language Features

### Data Types

| Type | Description | Example |
|------|-------------|---------|
| `INT` | Integer | `DECL INT count := 5` |
| `REAL` | Floating point | `DECL REAL speed := 0.75` |
| `BOOL` | Boolean | `DECL BOOL flag := TRUE` |
| `CHAR` | Character | `DECL CHAR c := "A"` |
| `POS` | Cartesian position (x,y,z,a,b,c) | `DECL POS P1 := {x 100, y 0, z 500, a 0, b 90, c 0}` |
| `E6POS` | Extended position | Same as POS with additional axes |
| `AXIS` | Joint angles (a1–a6) | `DECL AXIS A1 := {a1 0, a2 0, a3 0, a4 0, a5 0, a6 0}` |
| `FRAME` | Coordinate frame | `DECL FRAME F1 := {x 0, y 0, z 0, a 0, b 0, c 0}` |


### Motion Commands

```grs
PTP P1           ; Point-to-point motion
LIN P1           ; Linear motion
CIRC P1, P2      ; Circular motion
PTP_REL P1       ; Relative PTP
LIN_REL P1       ; Relative LIN
```

### Digital I/O

```grs
$OUT[1] := TRUE      ; Set digital output 1
$OUT[3] := FALSE     ; Clear digital output 3

IF $IN[1] THEN       ; Read digital input 1
  ; input is high
ENDIF
```

### Timing

```grs
WAIT(1000)           ; Wait 1000 milliseconds
```

## Example Program

```grs
DEF ExampleProgram()
  DECL INT counter := 5
  DECL REAL speed := 0.75
  DECL BOOL is_safe := TRUE
  DECL POS target := {x 500, y 0, z 600, a 0, b 90, c 0}

  $OUT[1] := TRUE

  PTP target
  LIN target

  IF counter > 3 THEN
    speed := 1.0
  ELSE
    speed := 0.5
  ENDIF

  WAIT(1000)
  $OUT[1] := FALSE
END
```

More examples: [ide/sample.grs](ide/sample.grs), [grs_interpreter/tests/](grs_interpreter/tests/)

## Project Structure

```
grs-interpreter/
├── grs_interpreter/               # Core interpreter
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── ast/                   # Abstract Syntax Tree
│   │   ├── common/                # Utility functions
│   │   ├── executor/              # Step executor engine
│   │   ├── interpreter/           # Instruction generator
│   │   ├── io/                    # I/O providers (Local, TCP)
│   │   ├── lexer/                 # Tokenizer
│   │   └── parser/                # Parser
│   ├── src/
│   │   ├── main.cpp               # Legacy interpreter entry
│   │   ├── step_main.cpp          # Step executor entry (Run/Step/Debug)
│   │   ├── ast/
│   │   ├── executor/
│   │   ├── interpreter/
│   │   ├── io/                    # tcp_io_provider, io_provider
│   │   ├── lexer/
│   │   └── parser/
│   ├── tests/                     # Test GRS programs
│   └── build/                     # Build output
├── ide/                           # IDE support files
    ├── install.sh                 # Linux install script
    ├── sample.grs                 # Sample GRS program
    └── zerobrane/                 # ZeroBrane Studio plugin files
        ├── grs.lua                # Language spec
        ├── grs.api                # Autocomplete API
        ├── grs_interpreter.lua    # Run/Debug interpreter plugin
        └── grs-support.lua        # Master package loader

```

## Hardware Architecture (rt_interpreter)

For TCP-connected operation, the system communicates with **rt_interpreter** — a real-time EtherCAT robot controller running on an Realtime embedded PC:

```
┌─────────────────────┐       TCP (128-byte protocol)     ┌────────────────────────────────┐
│   Development PC    │ ──────────────────────────────────│   Realtime (rt_interpreter)    │
│                     │                                   │                                │
│  ZeroBrane Studio   │    GrsRobotCommand (128B) ──→     │    network_server              │
│  grs_step --debug   │    ←── GrsRobotState (128B)       │    rt_loop (1ms cycle)         │
│  --tcp host:port    │                                   │    EtherCAT master             │
└─────────────────────┘                                   └────────────────────────────────┘
                                                                  │
                                                           EtherCAT bus
                                                                  │
                                                           ┌──────┴──────┐
                                                           │  EL1008     │  8× Digital In
                                                           │  EL2008     │  8× Digital Out
                                                           └─────────────┘
```

The unified 128-byte protocol carries:
- **GrsRobotCommand** — Motion type, target position (x,y,z,a,b,c or a1–a6), I/O index/value
- **GrsRobotState** — Current joint positions, Cartesian position, I/O state, system status flags

## Tests

Test files are in `grs_interpreter/tests/`:

| File | Description |
|------|-------------|
| `executor_test.txt` | Step executor tests |
| `example_krl_code.txt` | KRL code examples |
| `assign_and_type_convertion_test.txt` | Type conversion tests |
| `if_else_wait_test.txt` | Control structure tests |
| `pos_type_convertion.txt` | Position type tests |
| `io_hardware_test.grs` | I/O and motion test (AXIS + $OUT) |
| `general_system_test.txt` | General system tests |

Run a test:
```bash
cd grs_interpreter/build
./grs_step ../tests/io_hardware_test.grs
```

## Contributing

Contributions are welcome!

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Commit changes: `git commit -m 'Add amazing feature'`
4. Push: `git push origin feature/amazing-feature`
5. Open a Pull Request

## License

This project is open source.

## Links

- **Repository**: [github.com/FurkannByrm/grs-interpreter](https://github.com/FurkannByrm/grs-interpreter)
- **IDE Setup**: [ide/README.md](ide/README.md)
- **Examples**: [examples/](examples/)

## Contact

For questions, please use GitHub Issues.
