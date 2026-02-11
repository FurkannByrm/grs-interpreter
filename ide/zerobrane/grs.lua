-- GRS/KRL Language Specification for ZeroBrane Studio
-- ASM lexer is used because it natively supports ';' as line comment (like GRS)
-- Token categories derived from grs_interpreter/include/lexer/token.hpp

-- Scintilla ASM lexer style IDs (hardcoded for reliability)
local SCE_ASM_DEFAULT          = (wxstc or {}).wxSTC_ASM_DEFAULT          or 0
local SCE_ASM_COMMENT          = (wxstc or {}).wxSTC_ASM_COMMENT          or 1
local SCE_ASM_NUMBER           = (wxstc or {}).wxSTC_ASM_NUMBER           or 2
local SCE_ASM_STRING           = (wxstc or {}).wxSTC_ASM_STRING           or 3
local SCE_ASM_OPERATOR         = (wxstc or {}).wxSTC_ASM_OPERATOR         or 4
local SCE_ASM_IDENTIFIER       = (wxstc or {}).wxSTC_ASM_IDENTIFIER       or 5
local SCE_ASM_CPUINSTRUCTION   = (wxstc or {}).wxSTC_ASM_CPUINSTRUCTION   or 6
local SCE_ASM_MATHINSTRUCTION  = (wxstc or {}).wxSTC_ASM_MATHINSTRUCTION  or 7
local SCE_ASM_REGISTER         = (wxstc or {}).wxSTC_ASM_REGISTER         or 8
local SCE_ASM_DIRECTIVE        = (wxstc or {}).wxSTC_ASM_DIRECTIVE        or 9
local SCE_ASM_DIRECTIVEOPERAND = (wxstc or {}).wxSTC_ASM_DIRECTIVEOPERAND or 10
local SCE_ASM_COMMENTBLOCK     = (wxstc or {}).wxSTC_ASM_COMMENTBLOCK     or 11
local SCE_ASM_CHARACTER        = (wxstc or {}).wxSTC_ASM_CHARACTER        or 12
local SCE_ASM_STRINGEOL        = (wxstc or {}).wxSTC_ASM_STRINGEOL        or 13
local SCE_ASM_EXTINSTRUCTION   = (wxstc or {}).wxSTC_ASM_EXTINSTRUCTION   or 14
local SCE_ASM_COMMENTDIRECTIVE = (wxstc or {}).wxSTC_ASM_COMMENTDIRECTIVE or 15
local SCE_LEX_ASM              = (wxstc or {}).wxSTC_LEX_ASM              or 34

return {
  exts = {"grs", "krl", "src", "dat"},

  -- ASM lexer recognizes ';' as line comment natively
  lexer = SCE_LEX_ASM,

  apitype = "grs",
  linecomment = ";",

  -- 6 keyword groups → 6 different colors in the editor
  -- Each group maps to a different ASM lexer style
  -- IMPORTANT: Scintilla ASM lexer lowercases text before matching (GetCurrentLowered)
  -- so all keywords MUST be lowercase here, even though GRS code is written in UPPERCASE
  keywords = {
    -- [1] → SCE_ASM_CPUINSTRUCTION (style 6) — Program boundaries
    [[def end]],

    -- [2] → SCE_ASM_MATHINSTRUCTION (style 7) — Control flow
    [[if then else endif for to step endfor while endwhile repeat until switch case default endswitch goto halt return]],

    -- [3] → SCE_ASM_REGISTER (style 8) — Declarations & Data types
    [[decl int real bool char pos axis frame e6pos e6axis]],

    -- [4] → SCE_ASM_DIRECTIVE (style 9) — Motion commands
    [[ptp lin circ spline ptp_rel lin_rel circ_rel spline_rel]],

    -- [5] → SCE_ASM_DIRECTIVEOPERAND (style 10) — System commands & I/O
    [[wait delay in out]],

    -- [6] → SCE_ASM_EXTINSTRUCTION (style 14) — Constants & Logical operators
    [[true false pi and or not]],
  },

  -- Map ASM lexer styles to ZeroBrane's generic style categories
  lexerstyleconvert = {
    text       = {SCE_ASM_IDENTIFIER},
    lexerdef   = {SCE_ASM_DEFAULT},
    comment    = {SCE_ASM_COMMENT, SCE_ASM_COMMENTBLOCK, SCE_ASM_COMMENTDIRECTIVE},
    stringtxt  = {SCE_ASM_STRING, SCE_ASM_CHARACTER},
    stringeol  = {SCE_ASM_STRINGEOL},
    number     = {SCE_ASM_NUMBER},
    operator   = {SCE_ASM_OPERATOR},
    keywords0  = {SCE_ASM_CPUINSTRUCTION},     -- Program structure (DEF, END, DECL)
    keywords1  = {SCE_ASM_MATHINSTRUCTION},     -- Control flow (IF, FOR, WHILE...)
    keywords2  = {SCE_ASM_REGISTER},            -- Data types (INT, REAL, POS...)
    keywords3  = {SCE_ASM_DIRECTIVE},           -- Motion commands (PTP, LIN...)
    keywords4  = {SCE_ASM_DIRECTIVEOPERAND},    -- System (WAIT, DELAY, IN, OUT)
    keywords5  = {SCE_ASM_EXTINSTRUCTION},      -- Constants & Logic (TRUE, FALSE, AND...)
  },

  isfncall = function(str)
    return str:match("^[%w_]+%s*%(")
  end,

  ac = {
    sep = ".",
    trigger = ".",
  },

  settings = {
    tabwidth = 2,
    usetabs = false,
    smartindent = true,
  },
}
