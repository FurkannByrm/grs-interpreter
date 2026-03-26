#!/bin/bash
# GRS IDE Install Script — Linux
# Installs GRS language support for ZeroBrane Studio
# Creates symlink for the interpreter binary in PATH

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
ZB_DIR="$HOME/.zbstudio"
BIN_DIR="$HOME/bin"
INTERPRETER="$REPO_DIR/grs_interpreter/build/grs_step"
INTERPRETER_LEGACY="$REPO_DIR/grs_interpreter/build/interpreter"

echo "=== GRS IDE Installation ==="
echo ""

# ── 1. Create ZeroBrane Studio directories ──
mkdir -p "$ZB_DIR/spec"
mkdir -p "$ZB_DIR/api/grs"
mkdir -p "$ZB_DIR/interpreters"
mkdir -p "$ZB_DIR/packages"

# ── 2. Copy ZeroBrane plugin files ──
cp "$SCRIPT_DIR/zerobrane/grs.lua"             "$ZB_DIR/spec/"
cp "$SCRIPT_DIR/zerobrane/grs.api"             "$ZB_DIR/api/grs/grs.lua"
cp "$SCRIPT_DIR/zerobrane/grs_interpreter.lua" "$ZB_DIR/interpreters/"
cp "$SCRIPT_DIR/zerobrane/grs-support.lua"     "$ZB_DIR/packages/"

echo "ZeroBrane files installed:"
echo "  $ZB_DIR/spec/grs.lua                (language spec)"
echo "  $ZB_DIR/api/grs/grs.lua             (autocomplete)"
echo "  $ZB_DIR/interpreters/grs_interpreter.lua (run/debug plugin)"
echo "  $ZB_DIR/packages/grs-support.lua    (master loader)"
echo ""

# ── 3. Interpreter symlink (run from any directory) ──
if [ -f "$INTERPRETER" ]; then
    mkdir -p "$BIN_DIR"
    ln -sf "$INTERPRETER" "$BIN_DIR/grs_step"
    echo "Symlink created: $BIN_DIR/grs_step -> $INTERPRETER"
elif [ -f "$INTERPRETER_LEGACY" ]; then
    mkdir -p "$BIN_DIR"
    ln -sf "$INTERPRETER_LEGACY" "$BIN_DIR/interpreter"
    echo "Symlink created: $BIN_DIR/interpreter -> $INTERPRETER_LEGACY"
else
    echo "WARNING: Interpreter binary not found: $INTERPRETER"
    echo "  Build first: cd grs_interpreter/build && cmake .. && make"
fi

# ── 4. Add $HOME/bin to PATH (if not already) ──
if ! echo "$PATH" | grep -q "$BIN_DIR"; then
    SHELL_RC=""
    if [ -f "$HOME/.bashrc" ]; then
        SHELL_RC="$HOME/.bashrc"
    elif [ -f "$HOME/.zshrc" ]; then
        SHELL_RC="$HOME/.zshrc"
    elif [ -f "$HOME/.profile" ]; then
        SHELL_RC="$HOME/.profile"
    fi

    if [ -n "$SHELL_RC" ]; then
        if ! grep -q 'export PATH="\$HOME/bin:\$PATH"' "$SHELL_RC" 2>/dev/null; then
            echo '' >> "$SHELL_RC"
            echo '# GRS Interpreter' >> "$SHELL_RC"
            echo 'export PATH="$HOME/bin:$PATH"' >> "$SHELL_RC"
            echo "PATH updated in: $SHELL_RC"
            echo "  Run: source $SHELL_RC  to activate"
        fi
    fi
fi

echo ""
echo "Installation complete. Restart ZeroBrane Studio to apply changes."
