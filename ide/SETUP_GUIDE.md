# IDE Setup Screenshots and Visual Guide

## ZeroBrane Studio

### 1. Installation Directory Structure
After installation, your ZeroBrane Studio configuration should look like:

```
~/.zbstudio/                (Linux/macOS)
%USERPROFILE%\.zbstudio\    (Windows)
├── spec/
│   └── grs.lua             ← Language specification
└── api/
    └── grs.api             ← Autocomplete definitions
```

### 2. Expected Features

#### Syntax Highlighting
The following code elements should be highlighted:

- **Blue Bold**: Control structures (IF, FOR, WHILE, DEF, etc.)
- **Purple Bold**: Data types (INT, REAL, BOOL, POS, etc.)
- **Red Bold**: Motion commands (PTP, LIN, CIRC, etc.)
- **Orange**: Built-in functions (SQRT, SIN, COS, etc.)
- **Brown Bold**: Boolean operators (TRUE, FALSE, AND, OR, NOT)
- **Green Italic**: Comments (lines starting with ;)
- **Gray**: Strings
- **Orange**: Numbers

#### Autocomplete
When you type:
- `D` → Shows: DEF, DECL, DEFAULT
- `INT` → Shows: INT type with description
- `P` → Shows: PTP, POS, POW, etc.
- `.` after a POS variable → Shows: X, Y, Z, A, B, C

#### Code Folding
You can fold/unfold these blocks:
- `DEF...ENDDEF`
- `IF...ENDIF`
- `FOR...ENDFOR`
- `WHILE...ENDWHILE`
- `SWITCH...ENDSWITCH`

### 3. Running Programs
Press `F5` or select `Project > Run` to execute the current file.

Output will appear in the console at the bottom of the window.

## Notepad++

### 1. Importing Language Definition

Steps to import:
1. Open Notepad++
2. Menu: `Language` → `User Defined Language` → `Define your language...`
3. Click `Import...` button
4. Select `ide/scintilla/grs-language.xml`
5. Restart Notepad++

### 2. Expected Result

After importing, when you open a `.grs`, `.krl`, `.src`, or `.dat` file:
- The language should automatically be set to "GRS-KRL"
- Syntax highlighting should be applied
- You should see the language name in the status bar

### 3. Color Scheme

The default colors are:
- **Blue Bold**: Keywords (control structures)
- **Purple Bold**: Data types
- **Red Bold**: Motion commands
- **Orange**: Built-in functions
- **Brown Bold**: Boolean/operators
- **Green Italic**: Comments
- **Gray**: Strings
- **Orange**: Numbers

## SciTE

### 1. Properties File Location

Copy `grs.properties` to:
- **Linux**: `~/.scite/` or `/usr/share/scite/`
- **Windows**: `%USERPROFILE%\SciTEUser.properties`

### 2. Activation

Add to your `SciTEUser.properties` or `SciTEGlobal.properties`:
```
import grs
```

### 3. Expected Features

When editing `.grs`, `.krl`, `.src`, or `.dat` files:
- Syntax highlighting active
- Autocomplete triggered with Ctrl+Space
- Indentation: 2 spaces (tabs converted to spaces)
- Line comments with `;`
- Code folding for major blocks

## Geany

### 1. File Type Configuration

Files should be placed in:
```
~/.config/geany/filedefs/filetypes.GRS.conf
~/.config/geany/filetype_extensions.conf
```

### 2. Expected Features

- File extensions `.grs`, `.krl`, `.src`, `.dat` recognized
- Syntax highlighting active
- Symbol list showing functions
- Auto-indentation
- Comment toggling with Ctrl+E

## Testing the Installation

### Test 1: Basic Syntax Highlighting

Create a test file `test.grs`:
```grs
DEF Test()
  DECL INT x = 10
  IF x > 5 THEN
    PTP HOME
  ENDIF
END
```

Expected result:
- `DEF`, `IF`, `THEN`, `ENDIF`, `END` in blue bold
- `DECL`, `INT` in purple bold
- `PTP`, `HOME` in red bold
- `10`, `5` in orange
- Proper indentation

### Test 2: Autocomplete (ZeroBrane Studio only)

1. Type `DE` and press Ctrl+Space
   - Should show: DEF, DECL
2. Type `INT` and press Ctrl+Space
   - Should show: INT with description
3. Create a POS variable and type `.`
   - Should show: X, Y, Z, A, B, C components

### Test 3: Code Folding

1. Create a function with DEF...ENDDEF
2. Look for `-` or `▼` symbol in the margin
3. Click to fold/unfold the code block

### Test 4: Running Code (ZeroBrane Studio)

1. Open `ide/sample.grs`
2. Press F5
3. Console should show output (if interpreter is built)
   - If interpreter is not found, you'll see an error
   - Update the interpreter path in `grs.lua`

## Troubleshooting Checklist

### ✓ Files Installed Correctly
- [ ] Language spec file in correct location
- [ ] API file in correct location
- [ ] Properties file imported/configured

### ✓ IDE Restarted
- [ ] Close and reopen the IDE completely
- [ ] Check if new files are recognized

### ✓ File Extension Correct
- [ ] File ends with .grs, .krl, .src, or .dat
- [ ] No extra extensions (e.g., test.grs.txt is wrong)

### ✓ Language Selected (if manual selection needed)
- [ ] Check language menu or status bar
- [ ] Manually select GRS/KRL language if needed

## Configuration Tips

### Adjusting Colors

#### In Notepad++:
1. Go to `Settings` → `Style Configurator`
2. Select `GRS-KRL` language
3. Customize colors for each element

#### In SciTE:
Edit `grs.properties` and modify `style.grs.*` entries:
```properties
# Example: Change keyword color to red
style.grs.5=fore:#FF0000,bold
```

#### In Geany:
Edit `~/.config/geany/filedefs/filetypes.GRS.conf`

### Adjusting Indentation

In ZeroBrane `grs.lua`:
```lua
settings = {
    tabwidth = 4,      -- Change from 2 to 4
    usetabs = true,    -- Use tabs instead of spaces
}
```

In SciTE `grs.properties`:
```properties
indent.size.$(file.patterns.grs)=4
use.tabs.$(file.patterns.grs)=1
```

### Adding Custom Keywords

In ZeroBrane `grs.lua`, add to keywords section:
```lua
keywords = {
    [[EXISTING_KEYWORDS YOUR_CUSTOM_KEYWORD]],
}
```

In Scintilla XML, add to Keywords section:
```xml
<Keywords name="Keywords1">EXISTING YOUR_CUSTOM</Keywords>
```

## Common Issues and Solutions

### Issue: No Syntax Highlighting
**Solution**: 
1. Verify file extension is correct
2. Check if files are in the right directory
3. Restart the IDE
4. Manually select language from menu

### Issue: Autocomplete Not Working
**Solution**:
1. Check API file location
2. Press Ctrl+Space manually
3. Type a few characters first
4. Verify autocomplete is enabled in settings

### Issue: Interpreter Won't Run
**Solution**:
1. Build the interpreter first
2. Update path in `grs.lua`:
   ```lua
   local interpreter_path = "YOUR/PATH/TO/interpreter"
   ```
3. Make sure interpreter is executable

### Issue: Wrong Colors
**Solution**:
1. Re-import the language file
2. Check theme compatibility
3. Customize colors in style configurator

## Advanced Setup

### Setting Up Build System

#### ZeroBrane Studio
Already configured! Just press F5.

#### Notepad++ (Windows)
Add to Run menu:
```
Command: C:\path\to\interpreter.exe "$(FULL_CURRENT_PATH)"
```

#### SciTE
Add to `grs.properties`:
```properties
command.build.$(file.patterns.grs)=/path/to/interpreter "$(FilePath)"
command.go.$(file.patterns.grs)=/path/to/interpreter "$(FilePath)"
```

### Creating Code Snippets

#### ZeroBrane Studio
Create `~/.zbstudio/snippets.lua`:
```lua
snippets = {
  grs = {
    ["def"] = "DEF %1()\n  %2\nEND",
    ["for"] = "FOR %1 = %2 TO %3\n  %4\nENDFOR",
  }
}
```

## Performance Notes

- Syntax highlighting is fast even for large files (10,000+ lines)
- Autocomplete responds instantly
- Code folding handles deeply nested structures
- File loading is optimized for robot programs

## Support Matrix

| Feature | ZeroBrane | Notepad++ | SciTE | Geany |
|---------|-----------|-----------|-------|-------|
| Syntax Highlighting | ✅ | ✅ | ✅ | ✅ |
| Autocomplete | ✅ | ❌ | ✅ | ✅ |
| Code Folding | ✅ | ✅ | ✅ | ✅ |
| Integrated Run | ✅ | ⚠️ | ⚠️ | ⚠️ |
| Calltips | ✅ | ❌ | ✅ | ✅ |
| Symbol List | ❌ | ❌ | ❌ | ✅ |

✅ = Fully supported
⚠️ = Requires manual setup
❌ = Not supported

---

For more information, see [README.md](README.md)
