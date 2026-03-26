-- GRS/KRL Language Support Package for ZeroBrane Studio
-- Loads spec, interpreter, API, and provides debug UI integration

local home = os.getenv("HOME")
local zbdir = home .. "/.zbstudio"

-- Menu item IDs for debug commands
local ID_GRS_STEP = ID("grs.debug.step")
local ID_GRS_CONTINUE = ID("grs.debug.continue")
local ID_GRS_STOP = ID("grs.debug.stop")
local ID_GRS_VARS = ID("grs.debug.variables")
local ID_GRS_IO = ID("grs.debug.io")

return {
  name = "GRS Language Support",
  description = "Adds GRS/KRL robot programming language support with debug UI",
  author = "GRS Interpreter Project",

  onRegister = function(self)
    ide:Print("[GRS] Loading GRS language support package...")

    -- Custom GRS color scheme for keyword categories
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

    -- ═══════════════════════════════════════════════════════════
    -- Debug Menu Items — added to Project menu
    -- ═══════════════════════════════════════════════════════════
    local projMenu = ide:FindTopMenu("&Project")
    if projMenu then
      projMenu:AppendSeparator()

      projMenu:Append(ID_GRS_STEP, "GRS Step\tF10",
        "Step one GRS statement")
      projMenu:Append(ID_GRS_CONTINUE, "GRS Continue\tF8",
        "Continue execution until breakpoint or end")
      projMenu:Append(ID_GRS_STOP, "GRS Stop Debug\tShift+F5",
        "Stop the GRS debug session")
      projMenu:Append(ID_GRS_VARS, "GRS Show Variables\tF7",
        "Display current variable values")
      projMenu:Append(ID_GRS_IO, "GRS Show I/O\tCtrl+F7",
        "Display current I/O state")

      -- Connect menu handlers
      local frame = ide:GetMainFrame()

      frame:Connect(ID_GRS_STEP, wx.wxEVT_COMMAND_MENU_SELECTED, function()
        if GRS_DEBUG and GRS_DEBUG.active then GRS_DEBUG:doStep() end
      end)

      frame:Connect(ID_GRS_CONTINUE, wx.wxEVT_COMMAND_MENU_SELECTED, function()
        if GRS_DEBUG and GRS_DEBUG.active then GRS_DEBUG:doContinue() end
      end)

      frame:Connect(ID_GRS_STOP, wx.wxEVT_COMMAND_MENU_SELECTED, function()
        if GRS_DEBUG and GRS_DEBUG.active then GRS_DEBUG:doStop() end
      end)

      frame:Connect(ID_GRS_VARS, wx.wxEVT_COMMAND_MENU_SELECTED, function()
        if GRS_DEBUG and GRS_DEBUG.active then GRS_DEBUG:requestVariables() end
      end)

      frame:Connect(ID_GRS_IO, wx.wxEVT_COMMAND_MENU_SELECTED, function()
        if GRS_DEBUG and GRS_DEBUG.active then GRS_DEBUG:requestIO() end
      end)

      -- Enable/disable menu items based on debug state
      frame:Connect(ID_GRS_STEP, wx.wxEVT_UPDATE_UI, function(event)
        event:Enable(GRS_DEBUG ~= nil and GRS_DEBUG.active)
      end)
      frame:Connect(ID_GRS_CONTINUE, wx.wxEVT_UPDATE_UI, function(event)
        event:Enable(GRS_DEBUG ~= nil and GRS_DEBUG.active)
      end)
      frame:Connect(ID_GRS_STOP, wx.wxEVT_UPDATE_UI, function(event)
        event:Enable(GRS_DEBUG ~= nil and GRS_DEBUG.active)
      end)
      frame:Connect(ID_GRS_VARS, wx.wxEVT_UPDATE_UI, function(event)
        event:Enable(GRS_DEBUG ~= nil and GRS_DEBUG.active)
      end)
      frame:Connect(ID_GRS_IO, wx.wxEVT_UPDATE_UI, function(event)
        event:Enable(GRS_DEBUG ~= nil and GRS_DEBUG.active)
      end)

      self.menuItemsAdded = true
      ide:Print("[GRS] Debug menu items added (F10=Step, F8=Continue, Shift+F5=Stop, F7=Vars, Ctrl+F7=I/O)")
    end

    -- ═══════════════════════════════════════════════════════════
    -- Override ZeroBrane's built-in toolbar/menu button handlers
    -- so Step Into/Over/Out buttons work during GRS debug sessions.
    -- When GRS debug is NOT active, event:Skip() lets ZeroBrane's
    -- original handlers run normally.
    -- ═══════════════════════════════════════════════════════════
    local frame = ide:GetMainFrame()

    -- Step Into (F7) → GRS Step
    frame:Connect(ID_STEP, wx.wxEVT_COMMAND_MENU_SELECTED, function(event)
      if GRS_DEBUG and GRS_DEBUG.active then GRS_DEBUG:doStep()
      else event:Skip() end
    end)
    frame:Connect(ID_STEP, wx.wxEVT_UPDATE_UI, function(event)
      if GRS_DEBUG and GRS_DEBUG.active then event:Enable(true)
      else event:Skip() end
    end)

    -- Step Over (F8) → GRS Step (single-statement stepping, no call depth)
    frame:Connect(ID_STEPOVER, wx.wxEVT_COMMAND_MENU_SELECTED, function(event)
      if GRS_DEBUG and GRS_DEBUG.active then GRS_DEBUG:doStep()
      else event:Skip() end
    end)
    frame:Connect(ID_STEPOVER, wx.wxEVT_UPDATE_UI, function(event)
      if GRS_DEBUG and GRS_DEBUG.active then event:Enable(true)
      else event:Skip() end
    end)

    -- Step Out (F10) → GRS Step
    frame:Connect(ID_STEPOUT, wx.wxEVT_COMMAND_MENU_SELECTED, function(event)
      if GRS_DEBUG and GRS_DEBUG.active then GRS_DEBUG:doStep()
      else event:Skip() end
    end)
    frame:Connect(ID_STEPOUT, wx.wxEVT_UPDATE_UI, function(event)
      if GRS_DEBUG and GRS_DEBUG.active then event:Enable(true)
      else event:Skip() end
    end)

    -- Start Debug / Continue (F5/F6) → when GRS active: Continue
    frame:Connect(ID_STARTDEBUG, wx.wxEVT_COMMAND_MENU_SELECTED, function(event)
      if GRS_DEBUG and GRS_DEBUG.active then GRS_DEBUG:doContinue()
      else event:Skip() end
    end)
    frame:Connect(ID_STARTDEBUG, wx.wxEVT_UPDATE_UI, function(event)
      if GRS_DEBUG and GRS_DEBUG.active then event:Enable(true)
      else event:Skip() end
    end)

    -- Stop Debug → when GRS active: Stop
    frame:Connect(ID_STOPDEBUG, wx.wxEVT_COMMAND_MENU_SELECTED, function(event)
      if GRS_DEBUG and GRS_DEBUG.active then GRS_DEBUG:doStop()
      else event:Skip() end
    end)
    frame:Connect(ID_STOPDEBUG, wx.wxEVT_UPDATE_UI, function(event)
      if GRS_DEBUG and GRS_DEBUG.active then event:Enable(true)
      else event:Skip() end
    end)

    -- Disable Run (F5) while GRS debug is active to prevent double execution
    frame:Connect(ID_RUN, wx.wxEVT_UPDATE_UI, function(event)
      if GRS_DEBUG and GRS_DEBUG.active then event:Enable(false)
      else event:Skip() end
    end)

    ide:Print("[GRS] Toolbar buttons hooked for GRS debug sessions")

    -- Set GRS as the default interpreter
    ide.config.interpreter = "grs"
    ide:Print("[GRS] GRS set as default interpreter.")

    -- Hook margin click on any already-open editors (startup scenario)
    for _, doc in pairs(ide:GetDocuments()) do
      local ed = doc:GetEditor()
      if ed then self:_setupMarginClick(ed) end
    end
  end,

  -- ═══════════════════════════════════════════════════════════
  -- Breakpoint handling via direct Scintilla margin click
  -- ZeroBrane's built-in handler only toggles markers when its
  -- own debugger (mobdebug) is active. Since we use a custom
  -- JSON debug protocol, we handle margin clicks ourselves.
  -- ═══════════════════════════════════════════════════════════

  -- Helper: connect Scintilla margin click handler to an editor
  _setupMarginClick = function(self, editor)
    if not editor then return end
    -- Guard: don't double-connect
    if editor._grsMarginConnected then return end
    editor._grsMarginConnected = true

    -- Ensure the symbol margin (margin 1) is sensitive to mouse clicks
    editor:SetMarginSensitive(1, true)

    editor:Connect(wxstc.wxEVT_STC_MARGINCLICK, function(event)
      -- Only handle symbol margin (margin 1)
      if event:GetMargin() ~= 1 then
        event:Skip()
        return
      end

      -- When GRS debug is NOT active, let ZeroBrane handle it normally
      if not (GRS_DEBUG and GRS_DEBUG.active) then
        event:Skip()
        return
      end

      -- GRS debug is active: toggle breakpoint marker ourselves
      local line = editor:LineFromPosition(event:GetPosition())
      local markers = editor:MarkerGet(line)
      local hasBreakpoint = math.floor(markers / 2) % 2 == 1

      if hasBreakpoint then
        -- Remove breakpoint
        editor:MarkerDelete(line, 1)
        GRS_DEBUG:removeBreakpoint(line + 1)  -- 0-based → 1-based
      else
        -- Add breakpoint (red circle)
        editor:MarkerAdd(line, 1)
        GRS_DEBUG:setBreakpoint(line + 1)  -- 0-based → 1-based
      end
      -- Do NOT Skip() — consume the event so ZeroBrane's handler doesn't interfere
    end)
  end,

  onEditorNew = function(self, editor)
    self:_setupMarginClick(editor)
  end,

  onEditorLoad = function(self, editor)
    self:_setupMarginClick(editor)
  end,

  -- Also forward breakpoints set via onEditorMarkerUpdate (fallback for
  -- cases where ZeroBrane's internal handler does toggle the marker)
  onEditorMarkerUpdate = function(self, editor, marker, line, value)
    -- marker 1 = BREAKPOINT_MARKER in ZeroBrane
    -- line = 0-based Scintilla line number
    if marker ~= 1 then return end
    if not (GRS_DEBUG and GRS_DEBUG.active) then return end

    -- Check current state of the breakpoint marker at this line
    local markers = editor:MarkerGet(line)
    local hasBreakpoint = math.floor(markers / 2) % 2 == 1

    if hasBreakpoint then
      GRS_DEBUG:setBreakpoint(line + 1)   -- 0-based → 1-based
    else
      GRS_DEBUG:removeBreakpoint(line + 1)
    end
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

  -- Clean up debug session if the debugged file is closed
  onEditorPreClose = function(self, editor)
    if not (GRS_DEBUG and GRS_DEBUG.active) then return end
    local doc = ide:GetDocument(editor)
    if doc then
      local path = doc:GetFilePath()
      if path and path == GRS_DEBUG.filepath then
        GRS_DEBUG:doStop()
      end
    end
  end,

  -- ═══════════════════════════════════════════════════════════
  -- Keyboard shortcut handler — intercepts F10/F8/F7/Ctrl+F7
  -- BEFORE ZeroBrane's built-in debug shortcuts
  -- ═══════════════════════════════════════════════════════════
  onEditorKeyDown = function(self, editor, event)
    if not (GRS_DEBUG and GRS_DEBUG.active) then return end

    local keycode = event:GetKeyCode()
    -- Use robust ControlDown/ShiftDown/AltDown methods instead of GetModifiers()
    -- (GetModifiers may include NumLock/CapsLock bits on some systems)
    local ctrl  = event:ControlDown()
    local shift = event:ShiftDown()
    local alt   = event:AltDown()

    -- Numeric fallback constants for wxWidgets key codes
    local WXK_F5  = wx.WXK_F5  or 344
    local WXK_F7  = wx.WXK_F7  or 346
    local WXK_F8  = wx.WXK_F8  or 347
    local WXK_F10 = wx.WXK_F10 or 349

    -- F10 = Step one statement (no modifiers)
    if keycode == WXK_F10 and not ctrl and not shift and not alt then
      GRS_DEBUG:doStep()
      return false  -- consume event, prevent ZeroBrane's Step Out
    end

    -- F8 = Continue execution (no modifiers)
    if keycode == WXK_F8 and not ctrl and not shift and not alt then
      GRS_DEBUG:doContinue()
      return false  -- consume event, prevent ZeroBrane's Step Over
    end

    -- F7 = Show variables (no modifiers)
    if keycode == WXK_F7 and not ctrl and not shift and not alt then
      GRS_DEBUG:requestVariables()
      return false  -- consume event, prevent ZeroBrane's Step Into
    end

    -- Ctrl+F7 = Show I/O state
    if keycode == WXK_F7 and ctrl and not shift and not alt then
      GRS_DEBUG:requestIO()
      return false
    end

    -- Shift+F5 = Stop debug
    if keycode == WXK_F5 and shift and not ctrl and not alt then
      GRS_DEBUG:doStop()
      return false
    end
  end,

  onUnRegister = function(self)
    -- Stop any active debug session
    if GRS_DEBUG and GRS_DEBUG.active then
      GRS_DEBUG:doStop()
    end

    -- Remove menu items
    if self.menuItemsAdded then
      local projMenu = ide:FindTopMenu("&Project")
      if projMenu then
        pcall(function() projMenu:Delete(ID_GRS_STEP) end)
        pcall(function() projMenu:Delete(ID_GRS_CONTINUE) end)
        pcall(function() projMenu:Delete(ID_GRS_STOP) end)
        pcall(function() projMenu:Delete(ID_GRS_VARS) end)
        pcall(function() projMenu:Delete(ID_GRS_IO) end)
      end
    end

    ide:RemoveInterpreter("grs")
    ide:RemoveSpec("grs")
    ide:RemoveAPI("grs", "grs")
  end,
}
