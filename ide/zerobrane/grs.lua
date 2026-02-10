-- GRS/KRL Language Specification for ZeroBrane Studio
-- Copyright (c) 2024 GRS Interpreter Project

return {
  -- File extension
  exts = {"grs", "krl", "src", "dat"},
  
  -- Lexer configuration
  lexer = wxstc.wxSTC_LEX_CPP, -- Using C++ lexer as base
  
  -- Keywords
  keywords = {
    -- Keywords (set 0)
    [[DEF ENDDEF DECL IF THEN ELSE ENDIF FOR TO STEP ENDFOR WHILE ENDWHILE 
      SWITCH CASE DEFAULT ENDSWITCH WAIT SEC RETURN CONTINUE BREAK
      PTP LIN CIRC SPLINE LIN_REL PTP_REL HOME
      INT REAL BOOL CHAR STRUC ENUM POS E6POS AXIS E6AXIS FRAME
      TRUE FALSE NOT AND OR XOR]],
    
    -- Types (set 1)
    [[INT REAL BOOL CHAR POS E6POS AXIS E6AXIS FRAME STRUC ENUM]],
    
    -- Built-in functions (set 2)
    [[SQRT SIN COS TAN ABS POW EXP LOG MIN MAX STRLEN STRFIND STRCOMP
      SET_PTP_VEL SET_LIN_VEL BRAKE_ON BRAKE_OFF STOP_ROBOT
      GET_POS GET_AXIS SET_TOOL SET_BASE]],
  },
  
  -- API file for autocomplete
  api = {"grs"},
  
  -- Syntax highlighting colors
  stylingbits = 7,
  
  -- Comment syntax
  linecomment = ";",
  
  -- Code folding
  isfncall = function(str)
    return str:match("^[%w_]+%s*%(")
  end,
  
  -- Auto-completion settings
  ac = {
    -- Separator for member access
    sep = ".",
    -- Characters that trigger autocomplete
    trigger = ".",
  },
  
  -- Interpreter settings
  interpreter = {
    name = "GRS Interpreter",
    description = "GRS/KRL Robot Programming Language Interpreter",
    api = {"baselib"},
    frun = function(self, wfilename, rundebug)
      local grs = self:fworkdir(wfilename)
      local interpreter_path = grs.."/../grs_interpreter/build/interpreter"
      
      -- Build command to run interpreter
      local cmd = '"'..interpreter_path..'" "'..wfilename..'"'
      
      -- Run the command
      CommandLineRun(cmd, self:fworkdir(wfilename), true, false)
    end,
    fprojdir = function(self, wfilename)
      return wfilename:GetPath(wx.wxPATH_GET_VOLUME)
    end,
    fworkdir = function(self, wfilename)
      return wfilename:GetPath(wx.wxPATH_GET_VOLUME)
    end,
  },
  
  -- Editor settings
  settings = {
    tabwidth = 2,
    usetabs = false,
    smartindent = true,
    }
}
