# GRS IDE Setup

ZeroBrane Studio IDE integration for the GRS/KRL robot programming language.

Provides syntax highlighting, autocomplete, program execution (F5), and a full step debugger (F6) with breakpoints, variable inspection, and I/O monitoring.

---

## Prerequisites

### 1. Build the GRS Interpreter

```bash
cd grs_interpreter
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

This produces `grs_interpreter/build/grs_step` — the step executor binary used by the IDE.

### 2. Install ZeroBrane Studio

```bash
# Use the bundled installer (from the repo root)
chmod +x ZeroBraneStudioEduPack-2.01-linux.sh
./ZeroBraneStudioEduPack-2.01-linux.sh
```

Default installation directory: `/opt/zbstudio`

Add to PATH (optional):
```bash
echo 'export PATH="/opt/zbstudio:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

---

## Install GRS Language Support

```bash
cd ide
chmod +x install.sh
./install.sh
```

### What the install script does

1. Creates ZeroBrane config directories under `~/.zbstudio/`
2. Copies 4 plugin files (see table below)
3. Creates a symlink `~/bin/grs_step` → `grs_interpreter/build/grs_step`
4. Adds `$HOME/bin` to your PATH (via `.bashrc` or `.zshrc`)

| File | Installed To | Purpose |
|------|-------------|---------|
| `grs.lua` | `~/.zbstudio/spec/` | Language definition (lexer, keywords, colors) |
| `grs.api` | `~/.zbstudio/api/grs/grs.lua` | Autocomplete definitions |
| `grs_interpreter.lua` | `~/.zbstudio/interpreters/` | Run (F5) and Debug (F6) integration |
| `grs-support.lua` | `~/.zbstudio/packages/` | Master loader — connects everything |

---

## Usage

1. Open ZeroBrane Studio:
   ```bash
   zbstudio ide/sample.grs
   ```

2. When a `.grs` file is opened:
   - GRS syntax highlighting activates automatically
   - GRS interpreter is selected as the default
   - Autocomplete works (type and press `Ctrl+Space`)

3. Press **F5** to run the program — output appears in the Output panel

4. Press **F6** to start a debug session — step through code with breakpoints

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| **F5** | Run program (batch execution) |
| **F6** | Start debug session |
| **F10** | Step one statement |
| **F8** | Continue execution (respects breakpoints) |
| **Shift+F5** | Stop debug session |
| **F7** | Show current variables |
| **Ctrl+F7** | Show I/O state (inputs/outputs) |
| **Margin click** | Toggle breakpoint on a line |

### Running from Any Directory

The install script symlinks the `grs_step` binary to `~/bin/grs_step` and adds `$HOME/bin` to PATH. This lets you run `.grs` files from any directory via F5/F6.

The interpreter is searched in this order:
1. Relative to the `.grs` file (e.g., `grs_interpreter/build/grs_step`)
2. Relative to the ZeroBrane project directory
3. Custom path configured in `user.lua` (`path.grs = '/full/path/to/grs_step'`)
4. System PATH (`~/bin/grs_step` symlink is found here)

---

## TCP Configuration (Hardware)

To send motion commands and I/O to a robot controller over TCP, edit `~/.zbstudio/user.lua`:

```lua
grs = { tcp = "10.42.0.43:12345" }
```

When configured, both F5 (Run) and F6 (Debug) automatically pass `--tcp host:port` to `grs_step`. If the TCP connection fails, the interpreter falls back to offline mode gracefully.

---

## Color Scheme

The GRS language spec defines color-coded keyword categories:

| Color | Keyword Group | Examples |
|-------|---------------|----------|
| Purple | Program structure | `DEF` `END` |
| Magenta-red (bold) | Control flow | `IF` `THEN` `ELSE` `ENDIF` `FOR` `WHILE` |
| Teal green | Declarations & types | `DECL` `INT` `REAL` `BOOL` `POS` `AXIS` |
| Orange (bold) | Motion commands | `PTP` `LIN` `CIRC` `SPLINE` |
| Steel blue | System / I/O | `WAIT` `DELAY` `IN` `OUT` |
| Blue (bold) | Constants & logic | `TRUE` `FALSE` `AND` `OR` `NOT` |
| Green (italic) | Comments | `; this is a comment` |
| Orange | Numbers | `42` `3.14` |
| Grey | Strings | `"hello"` |

---

## File Structure

```
ide/
├── install.sh                  # Linux install script
├── sample.grs                  # Sample GRS program
├── README.md                   # This file
└── zerobrane/                  # ZeroBrane Studio plugin files
    ├── grs.lua                 # Language spec (lexer, keywords)
    ├── grs.api                 # Autocomplete API definitions
    ├── grs_interpreter.lua     # Interpreter plugin (Run + Debug)
    └── grs-support.lua         # Package loader (master connector)
```

---

## How It Works (Technical Details)

### The Package System

ZeroBrane Studio doesn't automatically load files from `~/.zbstudio/spec/`. The `user.lua` file loads before all editor modules, so calling `ide:LoadSpec()` there causes errors. Instead, we use ZeroBrane's **package** system (`~/.zbstudio/packages/`), which loads after all editor modules are initialized.

`grs-support.lua` runs as a package and:
1. Calls `ide:AddSpec("grs", spec)` — registers the language definition
2. Calls `ide:AddInterpreter("grs", interp)` — registers the Run/Debug handler
3. Calls `ide:AddAPI("grs", "grs", api)` — enables autocomplete
4. Sets `ide.config.interpreter = "grs"` — makes GRS the default
5. Hooks toolbar buttons (Step Into/Over/Out, Continue, Stop) to GRS debug commands
6. Installs Scintilla margin click handler for breakpoint toggling

### ASM Lexer

GRS uses `;` for line comments. Scintilla's built-in **ASM lexer** (`wxSTC_LEX_ASM = 34`) natively supports `;` comments and provides 6 keyword groups — perfect for GRS's different keyword categories.

> **Note:** The ASM lexer lowercases text before keyword matching (`GetCurrentLowered`), so all keywords in the spec file are defined in lowercase, even though GRS code is written in UPPERCASE. The matching is case-insensitive.

### Debug Protocol

The debug mode (`grs_step --debug`) communicates via JSON lines on stdin/stdout:

**IDE → grs_step (stdin):**
```json
{"cmd":"step"}
{"cmd":"continue"}
{"cmd":"setBreakpoint","line":5}
{"cmd":"removeBreakpoint","line":5}
{"cmd":"getVariables"}
{"cmd":"getIO"}
{"cmd":"disconnect"}
```

**grs_step → IDE (stdout):**
```json
{"event":"initialized","line":4}
{"event":"stopped","line":6,"reason":"step"}
{"event":"stopped","line":18,"reason":"breakpoint"}
{"event":"output","type":"LIN","target":"P1","params":{"a1":30,"a2":0},"line":22}
{"event":"output","type":"OUTPUT","index":1,"value":true,"line":6}
{"event":"variables","data":[{"name":"counter","value":"5"}]}
{"event":"io","inputs":0,"outputs":1}
{"event":"terminated"}
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| F5 grayed out | Check interpreter is selected: Project → Interpreter → GRS |
| "GRS Interpreter not found!" | Build the interpreter: `cd grs_interpreter/build && cmake .. && make` |
| Colors not showing | Close ZeroBrane, run `./install.sh` again, reopen |
| Lua syntax errors | The file is being run with Lua interpreter — switch to GRS: Project → Interpreter → GRS |
| Autocomplete not working | Open a `.grs` file, try `Ctrl+Space` |
| Breakpoints not toggling | Restart ZeroBrane after installing — the margin click handler loads on startup |
| .grs files from other dirs fail | Check symlink: `ls -la ~/bin/grs_step` |
| TCP connection fails | Verify `user.lua` config: `grs = { tcp = "host:port" }` — interpreter will fall back to offline mode |
