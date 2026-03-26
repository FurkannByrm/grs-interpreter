-- GRS/KRL Interpreter Plugin for ZeroBrane Studio
-- Full debug support via JSON protocol with grs_step --debug
--
-- F5  (Run)   → runs grs_step in batch mode (output in panel)
-- F6  (Debug) → launches grs_step --debug, enables stepping/breakpoints
-- F10         → Step one statement
-- F8          → Continue execution (respects breakpoints)
-- Shift+F5   → Stop debugging
-- F7          → Show variables
-- Ctrl+F7    → Show I/O state
-- Margin clicks toggle breakpoints during debug sessions

-- ═══════════════════════════════════════════════════════════════
-- Minimal JSON encoder/decoder (no external dependency)
-- Only handles flat objects with string/number/boolean/null values
-- ═══════════════════════════════════════════════════════════════
local json = {}

function json.encode(tbl)
  if type(tbl) ~= "table" then return tostring(tbl) end
  local parts = {}
  for k, v in pairs(tbl) do
    local key = '"' .. tostring(k) .. '"'
    local val
    if type(v) == "string" then
      val = '"' .. v:gsub('\\', '\\\\'):gsub('"', '\\"') .. '"'
    elseif type(v) == "boolean" then
      val = v and "true" or "false"
    elseif type(v) == "number" then
      val = tostring(v)
    elseif v == nil then
      val = "null"
    else
      val = '"' .. tostring(v) .. '"'
    end
    table.insert(parts, key .. ":" .. val)
  end
  return "{" .. table.concat(parts, ",") .. "}"
end

function json.decode(str)
  if type(str) ~= "string" or #str == 0 then return nil end
  -- Remove leading/trailing whitespace
  str = str:match("^%s*(.-)%s*$")
  if str:sub(1,1) ~= "{" then return nil end

  local result = {}

  -- Pre-extract nested objects like "params":{...} before general parsing
  -- Replace them with a placeholder to avoid confusing the simple regex parser
  local nested = {}
  local cleaned = str:gsub('"([^"]-)":%s*({[^{}]*})', function(key, obj)
    -- Parse the nested flat object
    local inner = {}
    for k, v in obj:gmatch('"([^"]-)":%s*([^,}]+)') do
      v = v:match("^%s*(.-)%s*$")
      local num = tonumber(v)
      if num then inner[k] = num
      elseif v == "true" then inner[k] = true
      elseif v == "false" then inner[k] = false
      elseif v:sub(1,1) == '"' then inner[k] = v:sub(2,-2)
      else inner[k] = v end
    end
    nested[key] = inner
    return ""  -- remove from string for flat parsing
  end)

  -- Match key-value pairs: "key":value
  -- Values can be: "string", number, true, false, null, [array]
  for key, val in cleaned:gmatch('"([^"]-)":%s*([^,}]+)') do
    -- Trim whitespace from value
    val = val:match("^%s*(.-)%s*$")
    if val == "true" then
      result[key] = true
    elseif val == "false" then
      result[key] = false
    elseif val == "null" then
      result[key] = nil
    elseif val:sub(1,1) == '"' then
      -- String value: remove quotes and unescape
      result[key] = val:sub(2, -2):gsub('\\"', '"'):gsub('\\\\', '\\')
    elseif val:sub(1,1) == '[' then
      -- Array value: parse array of objects for variables/io
      local arr = {}
      for obj in val:gmatch('{(.-)}') do
        local item = {}
        for k2, v2 in obj:gmatch('"([^"]-)":%s*"([^"]-)"') do
          item[k2] = v2
        end
        table.insert(arr, item)
      end
      result[key] = arr
    else
      -- Try number
      local num = tonumber(val)
      if num then
        result[key] = num
      else
        result[key] = val
      end
    end
  end

  -- Merge nested objects back
  for k, v in pairs(nested) do
    result[k] = v
  end

  return result
end

local cached_path = nil

-- Forward declarations for local functions
local findInterpreter
local sendCommand, handleEvent, processJsonLine
local updateCurrentLine, syncBreakpoints
local cleanup, pollOutput

-- ═══════════════════════════════════════════════════════════════
-- Debug session state — shared with grs-support.lua via _G.GRS_DEBUG
-- ═══════════════════════════════════════════════════════════════
local D = {
  proc = nil,           -- wx.wxProcess
  pid = 0,              -- process PID
  timer = nil,          -- wx.wxTimer
  timerId = nil,        -- timer event ID
  buffer = "",          -- partial JSON line buffer
  active = false,       -- debug session running?
  initialized = false,  -- received "initialized" event?
  filepath = nil,       -- file being debugged
  currentLine = nil,    -- current execution line (1-based)
  runPid = 0,           -- PID of the last RUN mode process (F5)
}
rawset(_G, "GRS_DEBUG", D)

-- ═══════════════════════════════════════════════════════════════
-- Find interpreter binary
-- ═══════════════════════════════════════════════════════════════
findInterpreter = function(filepath)
  local candidates = {
    "grs_interpreter/build/grs_step",
    "../grs_interpreter/build/grs_step",
    "../../grs_interpreter/build/grs_step",
    "build/grs_step",
    "../build/grs_step",
    "grs_interpreter/build/interpreter",
    "../grs_interpreter/build/interpreter",
    "build/interpreter",
  }

  -- 1. Relative to .grs file
  if filepath then
    local filedir = filepath:match("(.+)/") or "."
    for _, rel in ipairs(candidates) do
      local full = filedir .. "/" .. rel
      if wx.wxFileExists(full) then return full end
    end
  end

  -- 2. Relative to project directory
  local projdir = ide:GetProject()
  if projdir then
    for _, rel in ipairs(candidates) do
      local full = projdir .. rel
      if wx.wxFileExists(full) then return full end
    end
  end

  -- 3. User-configured path
  if ide.config.path and ide.config.path.grs then
    local p = ide.config.path.grs
    if wx.wxFileExists(p) then return p end
  end

  -- 4. System PATH
  local pathenv = os.getenv("PATH") or ""
  for dir in pathenv:gmatch("[^:]+") do
    for _, name in ipairs({"grs_step", "interpreter"}) do
      local full = dir .. "/" .. name
      if wx.wxFileExists(full) then return full end
    end
  end

  return nil
end

-- ═══════════════════════════════════════════════════════════════
-- JSON Communication with grs_step --debug process
-- ═══════════════════════════════════════════════════════════════
sendCommand = function(tbl)
  if not D.proc then return false end
  local ostream = D.proc:GetOutputStream()
  if not ostream then return false end
  local str = json.encode(tbl) .. "\n"
  ostream:Write(str, #str)
  return true
end

-- Handle a parsed JSON event from the debug process
handleEvent = function(data)
  if not data or not data.event then return end
  local ev = data.event

  if ev == "initialized" then
    D.initialized = true
    local startLine = data.line or 1
    updateCurrentLine(startLine)
    ide:Print("[GRS DEBUG] Ready — execution starts at line " .. startLine)
    ide:Print("[GRS DEBUG] F10=Step  F8=Continue  Shift+F5=Stop  F7=Vars  Ctrl+F7=I/O")
    -- Sync any breakpoints already set in the editor
    syncBreakpoints()

  elseif ev == "stopped" then
    local line = data.line or 0
    local reason = data.reason or ""
    updateCurrentLine(line)
    if reason == "breakpoint" then
      ide:Print("[GRS] ● Breakpoint at line " .. line)
    else
      ide:Print("[GRS] Stopped at line " .. line)
    end

  elseif ev == "output" then
    local msg = "[GRS CMD] " .. (data.type or "?")
    if data.type == "OUTPUT" then
      msg = msg .. " $OUT[" .. (data.index or 0) .. "] = " .. tostring(data.value)
    elseif data.type == "WAIT" then
      msg = msg .. " time=" .. (data.time or 0)
    else
      msg = msg .. " → " .. (data.target or "?")
      -- Show position coordinates in proper order
      if data.params then
        local ordered_keys = {"x","y","z","a","b","c","a1","a2","a3","a4","a5","a6","A1","A2","A3","A4","A5","A6"}
        local parts = {}
        -- First add known keys in order
        for _, k in ipairs(ordered_keys) do
          if data.params[k] ~= nil then
            table.insert(parts, k .. "=" .. tostring(data.params[k]))
          end
        end
        -- Then add any remaining keys not in the ordered list
        for k, v in pairs(data.params) do
          local found = false
          for _, ok in ipairs(ordered_keys) do
            if k == ok then found = true; break end
          end
          if not found then
            table.insert(parts, tostring(k) .. "=" .. tostring(v))
          end
        end
        if #parts > 0 then
          msg = msg .. " {" .. table.concat(parts, ", ") .. "}"
        end
      end
    end
    msg = msg .. " (line " .. (data.line or 0) .. ")"
    ide:Print(msg)

  elseif ev == "variables" then
    if data.data and #data.data > 0 then
      ide:Print("[GRS Variables]")
      for _, v in ipairs(data.data) do
        ide:Print("  " .. (v.name or "?") .. " = " .. (v.value or "?"))
      end
    else
      ide:Print("[GRS] No variables")
    end

  elseif ev == "io" then
    ide:Print(string.format("[GRS I/O] Inputs=0x%X  Outputs=0x%X",
      data.inputs or 0, data.outputs or 0))
    if data.ready ~= nil then
      ide:Print("[GRS I/O] System Ready: " .. (data.ready and "YES" or "NO"))
    end

  elseif ev == "error" then
    ide:Print("[GRS ERROR] " .. (data.message or "unknown error"))
    cleanup()

  elseif ev == "terminated" then
    ide:Print("[GRS] Execution completed")
    cleanup()

  -- Silently acknowledge breakpoint confirmations
  elseif ev == "breakpointSet" then
  elseif ev == "breakpointRemoved" then
  elseif ev == "breakpointsCleared" then
  end
end

-- Process a single JSON line from process output
processJsonLine = function(line)
  if #line == 0 then return end
  local ok, data = pcall(json.decode, line)
  if ok and data then
    handleEvent(data)
  else
    -- Not JSON — raw output, print as-is
    ide:Print("[GRS] " .. line)
  end
end

-- ═══════════════════════════════════════════════════════════════
-- Editor UI: current line marker, breakpoint sync
-- ═══════════════════════════════════════════════════════════════
local CURRENT_LINE_MARKER = 3
local CURRENT_BG_MARKER = 4

local function setupMarkers(editor)
  if not editor then return end
  -- Marker 3: green arrow in margin
  editor:MarkerDefine(CURRENT_LINE_MARKER, wxstc.wxSTC_MARK_SHORTARROW)
  editor:MarkerSetBackground(CURRENT_LINE_MARKER, wx.wxColour(80, 200, 80))
  editor:MarkerSetForeground(CURRENT_LINE_MARKER, wx.wxColour(0, 80, 0))
  -- Marker 4: line background highlight (light yellow)
  editor:MarkerDefine(CURRENT_BG_MARKER, wxstc.wxSTC_MARK_BACKGROUND)
  editor:MarkerSetBackground(CURRENT_BG_MARKER, wx.wxColour(255, 255, 210))
end

updateCurrentLine = function(line)
  local editor = ide:GetEditor()
  if not editor then return end

  -- Remove previous markers
  editor:MarkerDeleteAll(CURRENT_LINE_MARKER)
  editor:MarkerDeleteAll(CURRENT_BG_MARKER)

  if line and line > 0 then
    setupMarkers(editor)
    -- Scintilla uses 0-based lines
    editor:MarkerAdd(line - 1, CURRENT_LINE_MARKER)
    editor:MarkerAdd(line - 1, CURRENT_BG_MARKER)
    editor:GotoLine(line - 1)
    editor:EnsureVisibleEnforcePolicy(line - 1)
  end

  D.currentLine = line
end

syncBreakpoints = function()
  local editor = ide:GetEditor()
  if not editor then return end

  -- Clear all breakpoints in the engine first
  sendCommand({cmd = "clearBreakpoints"})

  -- Scan editor for breakpoint markers (marker 1, bit value = 2)
  local count = 0
  for i = 0, editor:GetLineCount() - 1 do
    local markers = editor:MarkerGet(i)
    -- Check if bit 1 is set (breakpoint marker = 2^1 = 2)
    if math.floor(markers / 2) % 2 == 1 then
      sendCommand({cmd = "setBreakpoint", line = i + 1})
      count = count + 1
    end
  end

  if count > 0 then
    ide:Print("[GRS DEBUG] " .. count .. " breakpoint(s) synced")
  end
end

-- ═══════════════════════════════════════════════════════════════
-- Process lifecycle management
-- ═══════════════════════════════════════════════════════════════
cleanup = function()
  -- Stop timer
  if D.timer then
    D.timer:Stop()
    if D.timerId then
      pcall(function()
        ide:GetMainFrame():Disconnect(D.timerId, wx.wxEVT_TIMER)
      end)
    end
    pcall(function() D.timer:delete() end)
    D.timer = nil
    D.timerId = nil
  end

  -- Clear current line markers
  local editor = ide:GetEditor()
  if editor then
    editor:MarkerDeleteAll(CURRENT_LINE_MARKER)
    editor:MarkerDeleteAll(CURRENT_BG_MARKER)
  end

  -- Reset state
  D.proc = nil
  D.pid = 0
  D.active = false
  D.initialized = false
  D.buffer = ""

  ide:Print("[GRS DEBUG] Session ended")
end

pollOutput = function()
  if not D.proc or not D.active then return end

  -- Read stdout (JSON events)
  local istream = D.proc:GetInputStream()
  if istream then
    while istream:CanRead() do
      local chunk = istream:Read(4096)
      if not chunk or #chunk == 0 then break end
      -- wxLua Read returns null-padded buffer; trim trailing null bytes
      chunk = chunk:gsub("%z+$", "")
      if #chunk == 0 then break end
      D.buffer = D.buffer .. chunk
    end
  end

  -- Process complete JSON lines
  while true do
    local nl = D.buffer:find("\n")
    if not nl then break end
    local line = D.buffer:sub(1, nl - 1)
    D.buffer = D.buffer:sub(nl + 1)
    -- Also trim any embedded nulls within the line
    line = line:gsub("%z", "")
    if #line > 0 then
      processJsonLine(line)
    end
  end

  -- Guard: cleanup may have been called during processJsonLine (e.g., terminated event)
  if not D.proc or not D.active then return end

  -- Read stderr (show only actual errors, suppress parser debug noise)
  local estream = D.proc:GetErrorStream()
  if estream then
    while estream:CanRead() do
      local chunk = estream:Read(4096)
      if chunk and #chunk > 0 then
        chunk = chunk:gsub("%z+$", "")
        for errline in chunk:gmatch("[^\n]+") do
          if errline:find("[Ee]rror") or errline:find("[Ff]ailed") then
            ide:Print("[GRS] " .. errline)
          end
        end
      end
    end
  end

  -- Check if process is still alive
  if D.proc and D.pid > 0 then
    local ok, alive = pcall(function() return wx.wxProcess.Exists(D.pid) end)
    if ok and not alive then
      -- Drain any remaining output before cleanup
      if istream then
        while istream:CanRead() do
          local chunk = istream:Read(4096)
          if chunk and #chunk > 0 then
            chunk = chunk:gsub("%z+$", "")
            if #chunk > 0 then
              D.buffer = D.buffer .. chunk
            end
          end
        end
        while true do
          local nl = D.buffer:find("\n")
          if not nl then break end
          local line = D.buffer:sub(1, nl - 1)
          D.buffer = D.buffer:sub(nl + 1)
          line = line:gsub("%z", "")
          if #line > 0 then processJsonLine(line) end
          -- Guard: processJsonLine may have called cleanup
          if not D.proc then break end
        end
      end
      -- Only call cleanup if not already cleaned up
      if D.proc then cleanup() end
    end
  end
end

-- ═══════════════════════════════════════════════════════════════
-- Public Debug API — called from grs-support.lua via GRS_DEBUG
-- ═══════════════════════════════════════════════════════════════
function D:doStep()
  if not self.active then return end
  sendCommand({cmd = "step"})
end

function D:doContinue()
  if not self.active then return end
  syncBreakpoints()
  sendCommand({cmd = "continue"})
end

function D:doStop()
  if not self.active then return end
  pcall(sendCommand, {cmd = "disconnect"})
  -- Kill process
  if self.pid > 0 then
    pcall(function() wx.wxProcess.Kill(self.pid, wx.wxSIGTERM) end)
  end
  cleanup()
end

function D:setBreakpoint(line)
  if not self.active then return end
  sendCommand({cmd = "setBreakpoint", line = line})
end

function D:removeBreakpoint(line)
  if not self.active then return end
  sendCommand({cmd = "removeBreakpoint", line = line})
end

function D:requestVariables()
  if not self.active then return end
  sendCommand({cmd = "getVariables"})
end

function D:requestIO()
  if not self.active then return end
  sendCommand({cmd = "getIO"})
end

-- ═══════════════════════════════════════════════════════════════
-- Interpreter Plugin Registration
-- ═══════════════════════════════════════════════════════════════
return {
  name = "GRS",
  description = "GRS/KRL Robot Programming Language Interpreter (Debug Enabled)",
  api = {"grs"},
  hasdebugger = true,
  skipaliases = true,

  frun = function(self, wfilename, rundebug)
    local filepath = wfilename:GetFullPath()
    local exe = cached_path

    if exe and not wx.wxFileExists(exe) then
      exe = nil
      cached_path = nil
    end

    if not exe then
      exe = findInterpreter(filepath)
      if exe then cached_path = exe end
    end

    if not exe then
      ide:Print("GRS Interpreter not found!")
      ide:Print("  Compile: cd grs_interpreter/build && cmake .. && make")
      ide:Print("  Or set in user.lua: path.grs = '/full/path/to/grs_step'")
      return
    end

    local wdir = wfilename:GetPath(wx.wxPATH_GET_VOLUME)

    -- ═══════════════════════════════════════════════════
    -- TCP configuration from user.lua:
    --   grs = { tcp = "10.42.0.43:12345" }
    -- Always added to command line; grs_step will gracefully
    -- fall back to offline mode if connection fails.
    -- ═══════════════════════════════════════════════════
    local tcp_flag = ""
    local grs_cfg = ide.config.grs
    if grs_cfg and grs_cfg.tcp and grs_cfg.tcp ~= "" then
      tcp_flag = ' --tcp ' .. grs_cfg.tcp
    end

    if rundebug then
      -- ═══════════════════════════════════════════════════
      -- DEBUG MODE — F6
      -- Launch grs_step --debug with JSON stdin/stdout
      -- ═══════════════════════════════════════════════════

      -- Stop any previous session
      if D.active then D:doStop() end

      local cmd = '"' .. exe .. '" "' .. filepath .. '" --debug' .. tcp_flag
      ide:Print("[GRS DEBUG] Starting: " .. cmd)

      -- Launch with redirected stdin/stdout
      local proc = wx.wxProcess(ide:GetMainFrame())
      proc:Redirect()
      local pid = wx.wxExecute(cmd, wx.wxEXEC_ASYNC, proc)

      if not pid or pid <= 0 then
        ide:Print("[GRS DEBUG] Failed to start process!")
        return
      end

      -- Store session state
      D.proc = proc
      D.pid = pid
      D.active = true
      D.initialized = false
      D.buffer = ""
      D.filepath = filepath

      -- Setup current line markers in the editor
      setupMarkers(ide:GetEditor())

      -- Setup polling timer (100ms interval)
      local timer = wx.wxTimer(ide:GetMainFrame())
      D.timer = timer
      D.timerId = timer:GetId()
      ide:GetMainFrame():Connect(D.timerId, wx.wxEVT_TIMER, function()
        pollOutput()
      end)
      timer:Start(100)

      ide:Print("[GRS DEBUG] Process started (PID: " .. pid .. ")")
      return pid
    end

    -- ═══════════════════════════════════════════════════
    -- RUN MODE — F5
    -- Batch execution with output in panel
    -- grs_step runs the program straight through (auto-ACK)
    -- ═══════════════════════════════════════════════════

    -- Kill previous run process if still alive (prevents port conflict)
    if D.runPid and D.runPid > 0 then
      local ok, alive = pcall(function() return wx.wxProcess.Exists(D.runPid) end)
      if ok and alive then
        ide:Print("[GRS RUN] Stopping previous process (PID: " .. D.runPid .. ")")
        pcall(function() wx.wxProcess.Kill(D.runPid, wx.wxSIGTERM) end)
        -- Brief delay for TCP cleanup
        wx.wxMilliSleep(200)
      end
      D.runPid = 0
    end

    -- Also stop any active debug session
    if D.active then D:doStop() end

    local cmd = '"' .. exe .. '" "' .. filepath .. '"' .. tcp_flag
    ide:Print("[GRS RUN] " .. cmd)
    local pid = CommandLineRun(cmd, wdir, true, false)
    D.runPid = pid or 0
    return pid
  end,

  fprojdir = function(self, wfilename)
    return wfilename:GetPath(wx.wxPATH_GET_VOLUME)
  end,

  fworkdir = function(self, wfilename)
    return wfilename:GetPath(wx.wxPATH_GET_VOLUME)
  end,
}
