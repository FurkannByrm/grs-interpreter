#!/bin/bash
# GRS IDE Kurulum Scripti — Linux/macOS
# ZeroBrane Studio için GRS dil desteği kurar
# Interpreter binary'sini PATH'e ekler

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
ZB_DIR="$HOME/.zbstudio"
BIN_DIR="$HOME/bin"
INTERPRETER="$REPO_DIR/grs_interpreter/build/interpreter"

echo "=== GRS IDE Kurulumu ==="
echo ""

# ── 1. ZeroBrane Studio dizinlerini oluştur ──
mkdir -p "$ZB_DIR/spec"
mkdir -p "$ZB_DIR/api/grs"
mkdir -p "$ZB_DIR/interpreters"
mkdir -p "$ZB_DIR/packages"

# ── 2. ZeroBrane dosyalarını kopyala ──
cp "$SCRIPT_DIR/zerobrane/grs.lua"             "$ZB_DIR/spec/"
cp "$SCRIPT_DIR/zerobrane/grs.api"             "$ZB_DIR/api/grs/grs.lua"
cp "$SCRIPT_DIR/zerobrane/grs_interpreter.lua" "$ZB_DIR/interpreters/"
cp "$SCRIPT_DIR/zerobrane/grs-support.lua"     "$ZB_DIR/packages/"

echo "ZeroBrane dosyalari kuruldu:"
echo "  $ZB_DIR/spec/grs.lua              (dil tanimi)"
echo "  $ZB_DIR/api/grs/grs.lua           (autocomplete)"
echo "  $ZB_DIR/interpreters/grs_interpreter.lua (F5 ile calistirma)"
echo "  $ZB_DIR/packages/grs-support.lua  (ana yukleyici)"
echo ""

# ── 3. Interpreter symlink (herhangi bir dizinden çalıştırma) ──
if [ -f "$INTERPRETER" ]; then
    mkdir -p "$BIN_DIR"
    ln -sf "$INTERPRETER" "$BIN_DIR/interpreter"
    echo "Interpreter symlink olusturuldu: $BIN_DIR/interpreter -> $INTERPRETER"
else
    echo "UYARI: Interpreter binary bulunamadi: $INTERPRETER"
    echo "  Once derleyin: cd grs_interpreter/build && cmake .. && make"
fi

# ── 4. $HOME/bin'i PATH'e ekle (yoksa) ──
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
            echo "PATH guncellendi: $SHELL_RC"
            echo "  Degisikligin aktif olmasi icin: source $SHELL_RC"
        fi
    fi
fi

# ── 5. Eski user.lua yaklaşımını temizle (varsa) ──
if [ -f "$ZB_DIR/user.lua" ] && grep -q "ide:LoadSpec\|GRS" "$ZB_DIR/user.lua" 2>/dev/null; then
    rm -f "$ZB_DIR/user.lua"
    echo "Eski user.lua temizlendi."
fi

echo ""
echo "Kurulum tamamlandi. ZeroBrane Studio'yu yeniden baslatin."
