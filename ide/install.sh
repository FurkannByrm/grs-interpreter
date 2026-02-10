#!/bin/bash

# GRS/KRL IDE Setup Installation Script
# This script installs GRS language support for ZeroBrane Studio and Scintilla-based editors

set -e

echo "=========================================="
echo "GRS/KRL IDE Setup Installation"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Function to print colored messages
print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${YELLOW}ℹ $1${NC}"
}

# Check if running on Linux or macOS
if [[ "$OSTYPE" == "linux-gnu"* ]] || [[ "$OSTYPE" == "darwin"* ]]; then
    print_success "Detected OS: $OSTYPE"
else
    print_error "This script is designed for Linux/macOS. For Windows, please follow manual installation in README.md"
    exit 1
fi

# Function to install ZeroBrane Studio support
install_zerobrane() {
    echo ""
    echo "Installing ZeroBrane Studio support..."
    
    # Determine ZeroBrane config directory
    if [[ "$OSTYPE" == "darwin"* ]]; then
        ZB_DIR="$HOME/Library/Application Support/ZeroBraneStudio"
    else
        ZB_DIR="$HOME/.zbstudio"
    fi
    
    # Create directories if they don't exist
    mkdir -p "$ZB_DIR/spec"
    mkdir -p "$ZB_DIR/api"
    
    # Copy files
    if [ -f "$SCRIPT_DIR/zerobrane/grs.lua" ]; then
        cp "$SCRIPT_DIR/zerobrane/grs.lua" "$ZB_DIR/spec/"
        print_success "Installed GRS language spec to $ZB_DIR/spec/"
    else
        print_error "Could not find grs.lua"
        return 1
    fi
    
    if [ -f "$SCRIPT_DIR/zerobrane/grs.api" ]; then
        cp "$SCRIPT_DIR/zerobrane/grs.api" "$ZB_DIR/api/"
        print_success "Installed GRS API file to $ZB_DIR/api/"
    else
        print_error "Could not find grs.api"
        return 1
    fi
    
    print_success "ZeroBrane Studio support installed successfully!"
    print_info "Please restart ZeroBrane Studio to use GRS language support"
}

# Function to install SciTE support
install_scite() {
    echo ""
    echo "Installing SciTE support..."
    
    # User-level SciTE config
    SCITE_USER="$HOME/.scite"
    mkdir -p "$SCITE_USER"
    
    if [ -f "$SCRIPT_DIR/scintilla/grs.properties" ]; then
        cp "$SCRIPT_DIR/scintilla/grs.properties" "$SCITE_USER/"
        print_success "Installed GRS properties to $SCITE_USER/"
        
        # Add import to SciTEUser.properties if not already there
        if [ -f "$HOME/.SciTEUser.properties" ]; then
            if ! grep -q "import grs" "$HOME/.SciTEUser.properties"; then
                echo "" >> "$HOME/.SciTEUser.properties"
                echo "import grs" >> "$HOME/.SciTEUser.properties"
                print_success "Added GRS import to SciTEUser.properties"
            fi
        else
            echo "import grs" > "$HOME/.SciTEUser.properties"
            print_success "Created SciTEUser.properties with GRS import"
        fi
        
        print_success "SciTE support installed successfully!"
        print_info "Please restart SciTE to use GRS language support"
    else
        print_error "Could not find grs.properties"
        return 1
    fi
}

# Function to install Geany support
install_geany() {
    echo ""
    echo "Installing Geany support..."
    
    GEANY_DIR="$HOME/.config/geany/filedefs"
    mkdir -p "$GEANY_DIR"
    
    if [ -f "$SCRIPT_DIR/scintilla/grs.properties" ]; then
        # Convert properties to Geany format
        cp "$SCRIPT_DIR/scintilla/grs.properties" "$GEANY_DIR/filetypes.GRS.conf"
        print_success "Installed GRS filetype to $GEANY_DIR/"
        
        # Update filetype_extensions.conf
        GEANY_EXT="$HOME/.config/geany/filetype_extensions.conf"
        if [ -f "$GEANY_EXT" ]; then
            if ! grep -q "GRS=" "$GEANY_EXT"; then
                echo "" >> "$GEANY_EXT"
                echo "GRS=*.grs;*.krl;*.src;*.dat;" >> "$GEANY_EXT"
                print_success "Added GRS extensions to filetype_extensions.conf"
            fi
        else
            echo "GRS=*.grs;*.krl;*.src;*.dat;" > "$GEANY_EXT"
            print_success "Created filetype_extensions.conf with GRS extensions"
        fi
        
        print_success "Geany support installed successfully!"
        print_info "Please restart Geany to use GRS language support"
    else
        print_error "Could not find grs.properties"
        return 1
    fi
}

# Main installation menu
echo "Select editor(s) to install GRS support for:"
echo "1) ZeroBrane Studio"
echo "2) SciTE"
echo "3) Geany"
echo "4) All of the above"
echo "5) Cancel"
echo ""
read -p "Enter your choice (1-5): " choice

case $choice in
    1)
        install_zerobrane
        ;;
    2)
        install_scite
        ;;
    3)
        install_geany
        ;;
    4)
        install_zerobrane
        install_scite
        install_geany
        ;;
    5)
        echo "Installation cancelled."
        exit 0
        ;;
    *)
        print_error "Invalid choice. Installation cancelled."
        exit 1
        ;;
esac

echo ""
echo "=========================================="
echo "Installation completed!"
echo "=========================================="
echo ""
print_info "For more information, see: $SCRIPT_DIR/README.md"
