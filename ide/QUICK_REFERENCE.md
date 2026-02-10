# GRS/KRL Quick Reference Guide

## Syntax Overview

### Comments
```grs
; This is a single-line comment
```

### Variable Declaration
```grs
DECL INT counter
DECL REAL speed = 0.5
DECL BOOL flag = TRUE
DECL CHAR status = 'A'
DECL POS position
```

### Function Definition
```grs
DEF FunctionName()
  ; Function body
END

DEF ReturnType FunctionName(Type param1, Type param2)
  ; Function body
  RETURN value
ENDDEF
```

### Control Structures

#### If-Then-Else
```grs
IF condition THEN
  ; statements
ELSE
  ; statements
ENDIF
```

#### For Loop
```grs
FOR counter = 1 TO 10
  ; statements
ENDFOR

FOR counter = 1 TO 10 STEP 2
  ; statements
ENDFOR
```

#### While Loop
```grs
WHILE condition
  ; statements
ENDWHILE
```

#### Switch-Case
```grs
SWITCH variable
  CASE value1
    ; statements
  CASE value2
    ; statements
  DEFAULT
    ; statements
ENDSWITCH
```

### Motion Commands

#### Point-to-Point
```grs
PTP HOME
PTP {X 500, Y 0, Z 600, A 0, B 90, C 0}
PTP_REL {X 100, Y 0, Z 0, A 0, B 0, C 0}
```

#### Linear Motion
```grs
LIN {X 500, Y 200, Z 600, A 0, B 90, C 0}
LIN_REL {Z -50}
```

#### Circular Motion
```grs
CIRC {X 400, Y 100, Z 600, A 0, B 90, C 0}, {X 500, Y 0, Z 600, A 0, B 90, C 0}
```

### Position Types

#### POS (Position)
```grs
DECL POS myPos
myPos = {X 500.0, Y 200.0, Z 600.0, A 0.0, B 90.0, C 0.0}
```

#### Accessing Position Components
```grs
DECL REAL xValue
xValue = myPos.X
myPos.Z = 700.0
```

### Operators

#### Arithmetic
```grs
+ - * / 
```

#### Comparison
```grs
== != < > <= >=
```

#### Logical
```grs
AND OR NOT XOR
```

### Built-in Functions

#### Mathematical
```grs
SQRT(x)      ; Square root
SIN(x)       ; Sine
COS(x)       ; Cosine
TAN(x)       ; Tangent
ABS(x)       ; Absolute value
POW(x, y)    ; Power (x^y)
EXP(x)       ; Exponential (e^x)
LOG(x)       ; Natural logarithm
MIN(x, y)    ; Minimum
MAX(x, y)    ; Maximum
```

#### String Functions
```grs
STRLEN(str)        ; Get string length
STRFIND(str, sub)  ; Find substring
STRCOMP(str1, str2); Compare strings
```

#### Robot Control
```grs
SET_PTP_VEL(vel)   ; Set PTP velocity (0.0-1.0)
SET_LIN_VEL(vel)   ; Set linear velocity
BRAKE_ON()         ; Enable brake
BRAKE_OFF()        ; Disable brake
STOP_ROBOT()       ; Stop robot motion
GET_POS()          ; Get current position
GET_AXIS()         ; Get current axis position
SET_TOOL(num)      ; Set active tool
SET_BASE(num)      ; Set active base frame
```

### Special Commands

#### Wait
```grs
WAIT SEC 1.5       ; Wait for 1.5 seconds
```

#### Flow Control
```grs
RETURN            ; Return from function
RETURN value      ; Return value from function
CONTINUE          ; Continue to next iteration
BREAK             ; Break from loop
```

## Common Patterns

### Simple Movement Sequence
```grs
DEF SimpleMovement()
  PTP HOME
  WAIT SEC 1.0
  LIN {X 500, Y 0, Z 600, A 0, B 90, C 0}
  WAIT SEC 0.5
  PTP HOME
END
```

### Loop with Position
```grs
DEF RepeatMotion()
  DECL INT i
  DECL POS target
  
  target = {X 500, Y 0, Z 600, A 0, B 90, C 0}
  
  FOR i = 1 TO 5
    LIN target
    WAIT SEC 1.0
    PTP HOME
    WAIT SEC 0.5
  ENDFOR
END
```

### Conditional Movement
```grs
DEF ConditionalMove(BOOL safe)
  IF safe == TRUE THEN
    LIN {X 500, Y 0, Z 600, A 0, B 90, C 0}
  ELSE
    STOP_ROBOT()
  ENDIF
END
```

### Function with Return Value
```grs
DEF REAL CalculateDistance(POS p1, POS p2)
  DECL REAL dx, dy, dz, result
  
  dx = p2.X - p1.X
  dy = p2.Y - p1.Y
  dz = p2.Z - p1.Z
  
  result = SQRT(dx*dx + dy*dy + dz*dz)
  RETURN result
ENDDEF
```

## Data Types

| Type | Description | Example |
|------|-------------|---------|
| INT | Integer | `42` |
| REAL | Floating point | `3.14` |
| BOOL | Boolean | `TRUE`, `FALSE` |
| CHAR | Character | `'A'` |
| POS | Position (X,Y,Z,A,B,C) | `{X 0, Y 0, Z 0, A 0, B 0, C 0}` |
| E6POS | Extended position | Complex position type |
| AXIS | Axis angles | Robot joint angles |
| E6AXIS | Extended axis | Extended joint angles |
| FRAME | Coordinate frame | Transformation frame |

## Keywords

### Control Flow
`DEF`, `ENDDEF`, `IF`, `THEN`, `ELSE`, `ENDIF`, `FOR`, `TO`, `STEP`, `ENDFOR`, `WHILE`, `ENDWHILE`, `SWITCH`, `CASE`, `DEFAULT`, `ENDSWITCH`, `RETURN`, `CONTINUE`, `BREAK`

### Data Types
`DECL`, `INT`, `REAL`, `BOOL`, `CHAR`, `POS`, `E6POS`, `AXIS`, `E6AXIS`, `FRAME`, `STRUC`, `ENUM`

### Motion
`PTP`, `LIN`, `CIRC`, `SPLINE`, `LIN_REL`, `PTP_REL`, `HOME`

### Special
`WAIT`, `SEC`, `TRUE`, `FALSE`, `NOT`, `AND`, `OR`, `XOR`

## Best Practices

1. **Always initialize variables**
   ```grs
   DECL INT counter = 0
   ```

2. **Use meaningful names**
   ```grs
   DECL POS pickup_position
   DECL REAL approach_speed
   ```

3. **Check safety before motion**
   ```grs
   IF is_position_safe(target) THEN
     LIN target
   ENDIF
   ```

4. **Use relative movements carefully**
   ```grs
   LIN_REL {Z 50}  ; Move up 50mm
   ```

5. **Add comments for clarity**
   ```grs
   ; Move to pickup position
   PTP pickup_position
   ```

## Debugging Tips

- Use WAIT commands to slow down execution
- Add position checks before movements
- Test with small movements first
- Use the IDE's syntax highlighting to catch errors
- Check variable types carefully

## IDE Shortcuts (ZeroBrane Studio)

- `Ctrl+Space` - Autocomplete
- `F5` - Run program
- `Ctrl+/` - Toggle comment
- `Ctrl+F` - Find
- `Ctrl+H` - Find and replace

---

For more information, see the full documentation in [ide/README.md](README.md)
