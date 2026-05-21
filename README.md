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

---

## Building on Windows

The interpreter builds and runs on Windows using **MSYS2** — a free Linux-like development environment. Follow these steps from a freshly installed Windows PC.

> **Note:** Only `grs_step.exe` runs on Windows. The EtherCAT hardware bridge (`ec_bridge_node`) runs only on the Linux controller PC.

---

### Step 1 — Install MSYS2

MSYS2 gives you `gcc`, `cmake`, `make`, and `git` in a Linux-style terminal.

1. Open a browser and go to **https://www.msys2.org**
2. Download `msys2-x86_64-YYYYMMDD.exe` (the big green button)
3. Run the installer — accept the default path `C:\msys64`
4. When the installer finishes, leave the checkbox **"Run MSYS2 now"** ticked and click Finish

A terminal window opens. Keep it open for the next step.

---

### Step 2 — Install Build Tools

In the MSYS2 UCRT64 terminal (purple title bar), run these commands **one by one**:

```bash
pacman -Syu
```
> MSYS2 may close itself to apply updates. If it does, open **MSYS2 UCRT64** from the Start menu and continue.

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-cmake
pacman -S mingw-w64-ucrt-x86_64-make
pacman -S git
```

Press `Y` (or `Enter`) when asked to confirm each installation.

**Verify everything works:**
```bash
gcc --version    # should print: gcc 13.x.x ...
cmake --version  # should print: cmake version 3.x.x
make --version   # should print: GNU Make 4.x
git --version    # should print: git version 2.x.x
```

---

### Step 3 — Get the Source Code

Still in the MSYS2 UCRT64 terminal:

```bash
cd ~
git clone https://github.com/FurkannByrm/grs-interpreter.git
```

This creates `~/grs-interpreter/` which maps to `C:\msys64\home\<YourUsername>\grs-interpreter\` in Windows Explorer.

---

### Step 4 — Install the constexpr_map Library

```bash
cd ~
git clone https://github.com/FurkannByrm/constexpr_map.git
```

This creates `~/constexpr_map/include/` which CMake looks for automatically. Keep it in the home directory.

---

### Step 5 — Build the Interpreter

```bash
cd ~/grs-interpreter/grs_interpreter
mkdir -p build && cd build
cmake .. -G "Unix Makefiles"
make -j$(nproc)
```

After a successful build you will have:

| File | Purpose |
|------|---------|
| `build/grs_step.exe` | Full executor — Run, Step, Debug + TCP |
| `build/interpreter.exe` | Basic interpreter (legacy) |

**Quick offline test:**
```bash
./grs_step.exe ../tests/io_hardware_test.grs
```
Expected output includes lines like `[ROBOT CMD] OUTPUT $OUT[1] = TRUE (line 6)`.

---

### Step 6 — Install ZeroBrane Studio

1. Go to **https://studio.zerobrane.com/download** and download the **Windows** installer
2. Run the installer — default location is fine
3. Launch ZeroBrane Studio once (so it creates its config folder), then close it

---

### Step 7 — Install GRS Language Support

**Option A — From Windows Explorer (easiest):**
1. Open `C:\msys64\home\<YourUsername>\grs-interpreter\ide\` in Explorer
2. Double-click `install.bat`
3. If Windows shows a security warning, click **More info → Run anyway**
4. The script installs the plugin files and adds the build directory to your PATH

**Option B — From the MSYS2 terminal:**
```bash
cd ~/grs-interpreter/ide
./install.sh
```

After either option, **restart ZeroBrane Studio**.

---

### Step 8 — Verify in ZeroBrane Studio

1. Open ZeroBrane Studio
2. Open `ide\sample.grs` (File → Open)
3. The status bar at the bottom should show **GRS** as the active interpreter
4. Press **F5** — the output panel should show robot commands

If ZeroBrane says "Interpreter not found", see the troubleshooting table below.

---

### TCP Hardware on Windows

Connecting to the robot controller works exactly as on Linux. Edit or create `%USERPROFILE%\.zbstudio\user.lua`:

```lua
-- %USERPROFILE%\.zbstudio\user.lua
grs = { tcp = "10.42.0.43:12345" }
```

Or pass `--tcp` directly from the MSYS2 terminal:

```bash
./grs_step.exe program.grs --tcp 10.42.0.43:12345
```

The interpreter connects to the controller and falls back to offline mode automatically if the connection fails — so you can develop and test programs without hardware connected.

---

### Windows Troubleshooting

| Symptom | Solution |
|---------|----------|
| `cmake` not found | Make sure you opened **MSYS2 UCRT64** (not MSYS2 MSYS or a regular Command Prompt) |
| `make` not found | Run `pacman -S mingw-w64-ucrt-x86_64-make` in MSYS2 |
| Build error: `constexpr_map not found` | Check that `~/constexpr_map/include/` exists (`ls ~/constexpr_map/include`) |
| ZeroBrane: "Interpreter not found" | Restart ZeroBrane after running `install.bat`; check PATH includes the build directory |
| ZeroBrane: black/wrong colors | Open the sample.grs file; the spec activates on the first `.grs` file |
| TCP connection refused | Verify the IP/port; check Windows Firewall isn't blocking outgoing TCP on the port |
| `install.bat` closes instantly | Right-click → Run as Administrator is **not** needed; try running from Command Prompt to see the error |

---

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
│   ├── install.sh                 # Linux install script
│   ├── sample.grs                 # Sample GRS program
│   └── zerobrane/                 # ZeroBrane Studio plugin files
│       ├── grs.lua                # Language spec
│       ├── grs.api                # Autocomplete API
│       ├── grs_interpreter.lua    # Run/Debug interpreter plugin
│       └── grs-support.lua        # Master package loader
└── examples/                      # Standalone example projects
    ├── rt_interpreter/            # Real-time EtherCAT bridge (Holly)
    │   ├── common/                # Shared: protocol.hpp, spsc_queue.hpp, bitset.hpp
    │   └── pc_ecrt/               # EtherCAT bridge node (ec_bridge_node)
    │       ├── src/               # main.cpp, rt_loop.cpp, network_server.cpp
    │       └── include/
    ├── ecrt_control/              # Minimal TCP test client (holly_client)
    ├── rt_examples/               # Bare-metal EtherCAT I/O and servo examples
    ├── multiprocess/              # Shared-memory multi-process example
    └── regex/                     # Regex utility examples

```

## Hardware Architecture (rt_interpreter)

The `examples/rt_interpreter/` project is the real-time hardware bridge that runs on the robot controller PC (upxtreme with a Beckhoff EtherCAT master). `grs_step` connects to it over TCP to send motion and I/O commands during program execution.

```
┌─────────────────────┐       TCP (128-byte protocol)     ┌───────────────────────────────┐
│   Development PC    │ ────────────────────────────────── │   Controller PC (upxtreme)    │
│                     │                                    │                               │
│  ZeroBrane / CLI    │    GrsRobotCommand (128B) ──→      │  network_server               │
│  grs_step --debug   │    ←── GrsRobotState (128B)        │  rt_loop (1ms, SCHED_FIFO 95) │
│  --tcp host:port    │                                    │  IgH EtherCAT master          │
└─────────────────────┘                                    └───────────────────────────────┘
                                                                    │
                                                             EtherCAT bus
                                                                    │
                                                            ┌───────┴───────┐
                                                            │  EL1008       │  8× Digital In
                                                            │  EL2008       │  8× Digital Out
                                                            └───────────────┘
```

### rt_interpreter components

| File | Role |
|------|------|
| `common/protocol.hpp` | Shared struct definitions — `GrsRobotCommand` and `GrsRobotState` (128 bytes each) |
| `common/spsc_queue.hpp` | Lock-free single-producer single-consumer queue between RT and network threads |
| `common/bitset.hpp` | Beckhoff EtherCAT I/O bitfield helpers |
| `pc_ecrt/src/main.cpp` | Entry point — launches RT thread (priority 95) and network thread |
| `pc_ecrt/src/rt_loop.cpp` | 1ms EtherCAT control loop — reads EL1008 inputs, writes EL2008 outputs, updates position state |
| `pc_ecrt/src/network_server.cpp` | TCP server — receives `GrsRobotCommand`, pushes to RT queue; sends `GrsRobotState` to client |

### Building rt_interpreter (on the controller PC)

Requires [IgH EtherCAT Master](https://etherlab.org/en/ethercat/) (built from source):

```bash
# Set ETHERCAT_ROOT to your IgH EtherCAT build directory
export ETHERCAT_ROOT=/home/upxtreme/ethercat

cd examples/rt_interpreter/pc_ecrt
mkdir -p build && cd build
cmake ..
make

# Run (requires root for RT scheduling and EtherCAT)
sudo ./ec_bridge_node
```

The bridge node listens on port **12345** (TCP). On the development PC, configure `grs_step` to connect:

```bash
./grs_step program.grs --tcp <controller-ip>:12345
```

### Protocol

Both structs are exactly **128 bytes** (enforced by `#pragma pack(push, 1)` + padding fields):

- **`GrsRobotCommand`** (client → server) — `cmd_type` (`GrsCommandType` enum), target `coords[6]` (x,y,z,a,b,c), `axes[6]` (a1–a6), `io_index`/`io_value`, `wait_time`, `cmd_id`
- **`GrsRobotState`** (server → client) — `current_pos[6]`, `current_axes[6]`, `inputs`/`outputs` bytes, `system_ready`, `cmd_ack`, `seq_id`

### Test Client

`examples/ecrt_control/` contains a minimal standalone TCP client (`holly_client`) for testing the bridge without `grs_step` — useful for verifying the EtherCAT hardware independently:

```bash
cd examples/ecrt_control && mkdir -p build && cd build
cmake .. && make
./holly_client <controller-ip>   # '1'=LED ON, '0'=LED OFF, 'q'=quit
```

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
