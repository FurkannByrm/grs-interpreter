-- GRS/KRL Language Support Package for ZeroBrane Studio
-- Loads spec, interpreter, and API from ~/.zbstudio/

local home = os.getenv("HOME") or os.getenv("USERPROFILE")
local zbdir = home .. (ide.osname == "Windows" and "\\.zbstudio" or "/.zbstudio")

return {
  name = "GRS Language Support",
  description = "Adds GRS/KRL robot programming language support",
  author = "GRS Interpreter Project",

  onRegister = function(self)
    ide:Print("[GRS] Loading GRS language support package...")

    -- Custom GRS color scheme for keyword categories
    -- These override the default theme colors for keywords0-5
    local styles = ide.config.styles
    if styles then
      styles.keywords0 = {fg = {90, 50, 150},  b = false}  -- DEF/END: soft purple
      styles.keywords1 = {fg = {160, 32, 80},  b = true}   -- Control flow: bold magenta-red
      styles.keywords2 = {fg = {0, 128, 96},   b = false}  -- DECL & Data types: teal green
      styles.keywords3 = {fg = {180, 80, 0},   b = true}   -- Motion commands: bold orange
      styles.keywords4 = {fg = {70, 130, 180},  b = false}  -- System/IO: steel blue
      styles.keywords5 = {fg = {0, 100, 180},  b = true}   -- Constants/Logic: bold steel blue
    end

    -- Load language spec
    local specfile = zbdir .. "/spec/grs.lua"
    if wx.wxFileExists(specfile) then
      local ok, spec = pcall(dofile, specfile)
      if ok and spec then
        ide:AddSpec("grs", spec)
        ide:Print("[GRS] Language spec loaded.")
      else
        ide:Print("[GRS] ERROR loading spec: " .. tostring(spec))
      end
    else
      ide:Print("[GRS] Spec file not found: " .. specfile)
    end

    -- Load interpreter plugin
    local interpfile = zbdir .. "/interpreters/grs_interpreter.lua"
    if wx.wxFileExists(interpfile) then
      local ok, interp = pcall(dofile, interpfile)
      if ok and interp then
        ide:AddInterpreter("grs", interp)
        ide:Print("[GRS] Interpreter registered.")
      else
        ide:Print("[GRS] ERROR loading interpreter: " .. tostring(interp))
      end
    else
      ide:Print("[GRS] Interpreter file not found: " .. interpfile)
    end

    -- Load API (autocomplete)
    local apifile = zbdir .. "/api/grs/grs.lua"
    if wx.wxFileExists(apifile) then
      local ok, api = pcall(dofile, apifile)
      if ok and api then
        ide:AddAPI("grs", "grs", api)
        ide:Print("[GRS] API loaded.")
      else
        ide:Print("[GRS] ERROR loading API: " .. tostring(api))
      end
    else
      ide:Print("[GRS] API file not found: " .. apifile)
    end

    -- Set GRS as the default interpreter.
    -- NOTE: ide:SetInterpreter("grs") alone is NOT enough because
    -- ProjectUpdateInterpreters() runs AFTER onRegister and resets
    -- the selection based on ide.config.interpreter. So we must set
    -- the config value directly.
    ide.config.interpreter = "grs"
    ide:Print("[GRS] GRS set as default interpreter (ide.config.interpreter).")
  end,

  onEditorFocusSet = function(self, editor)
    -- Auto-switch to GRS interpreter when a .grs file is focused
    local doc = ide:GetDocument(editor)
    if doc then
      local path = doc:GetFilePath()
      if path and path:match("%.grs$") then
        local current = ide:GetInterpreter()
        if current and current:GetFileName() ~= "grs" then
          ide:SetInterpreter("grs")
        end
      end
    end
  end,

  onUnRegister = function(self)
    ide:RemoveInterpreter("grs")
    ide:RemoveSpec("grs")
    ide:RemoveAPI("grs", "grs")
  end,
}
