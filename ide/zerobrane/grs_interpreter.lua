-- GRS/KRL Interpreter Plugin for ZeroBrane Studio
-- Place this file in ~/.zbstudio/interpreters/

local cached_path = nil  -- cache after first successful find

local function findInterpreter(filepath)
  -- Relative candidates from any base directory
  local candidates = {
    "grs_interpreter/build/interpreter",
    "../grs_interpreter/build/interpreter",
    "../../grs_interpreter/build/interpreter",
    "build/interpreter",
    "../build/interpreter",
  }

  -- Windows binary name
  local wincandidates = {}
  if ide.osname == "Windows" then
    for _, c in ipairs(candidates) do
      table.insert(wincandidates, c .. ".exe")
    end
    for _, c in ipairs(wincandidates) do
      table.insert(candidates, c)
    end
  end

  -- 1. Check relative to the .grs file being run
  if filepath then
    local filedir = filepath:match("(.+)[/\\]") or "."
    for _, rel in ipairs(candidates) do
      local full = filedir .. "/" .. rel
      if wx.wxFileExists(full) then return full end
    end
  end

  -- 2. Check relative to ZeroBrane project directory
  local projdir = ide:GetProject()
  if projdir then
    for _, rel in ipairs(candidates) do
      local full = projdir .. rel
      if wx.wxFileExists(full) then return full end
    end
  end

  -- 3. Check user-configured path (set in user.lua: path.grs = "/path/to/interpreter")
  if ide.config.path and ide.config.path.grs then
    local p = ide.config.path.grs
    if wx.wxFileExists(p) then return p end
  end

  -- 4. Check system PATH
  local sep = ide.osname == "Windows" and ";" or ":"
  local pathenv = os.getenv("PATH") or ""
  for dir in pathenv:gmatch("[^" .. sep .. "]+") do
    local full = dir .. "/interpreter"
    if wx.wxFileExists(full) then return full end
    if ide.osname == "Windows" then
      full = dir .. "\\interpreter.exe"
      if wx.wxFileExists(full) then return full end
    end
  end

  return nil
end

return {
  name = "GRS",
  description = "GRS/KRL Robot Programming Language Interpreter",
  api = {"grs"},
  frun = function(self, wfilename, rundebug)
    local filepath = wfilename:GetFullPath()
    local exe = cached_path

    -- Verify cached path still exists
    if exe and not wx.wxFileExists(exe) then
      exe = nil
      cached_path = nil
    end

    -- Find if not cached
    if not exe then
      exe = findInterpreter(filepath)
      if exe then cached_path = exe end
    end

    if not exe then
      ide:Print("GRS Interpreter not found!")
      ide:Print("  File: " .. filepath)
      ide:Print("  Project dir: " .. (ide:GetProject() or "(none)"))
      ide:Print("")
      ide:Print("  Cozum:")
      ide:Print("    1. grs_interpreter'i derleyin: cd grs_interpreter/build && cmake .. && make")
      ide:Print("    2. Ya da user.lua'ya ekleyin: path.grs = '/full/path/to/interpreter'")
      return
    end

    local cmd = '"' .. exe .. '" "' .. filepath .. '"'
    local wdir = wfilename:GetPath(wx.wxPATH_GET_VOLUME)

    ide:Print("Running: " .. cmd)
    return CommandLineRun(cmd, wdir, true, false)
  end,
  fprojdir = function(self, wfilename)
    return wfilename:GetPath(wx.wxPATH_GET_VOLUME)
  end,
  fworkdir = function(self, wfilename)
    return wfilename:GetPath(wx.wxPATH_GET_VOLUME)
  end,
  hasdebugger = true,
  skipaliases = true,
}
