local nexpo = {}
nexpo.graphics = {}
nexpo.mouse = {}
nexpo.key = {}
nexpo.window = {}
nexpo.controls = {}
nexpo.console = {}

local ffi = require 'ffi'

ffi.cdef [[
  typedef struct {} Path;
  typedef struct {} Font;

unsigned int getNumExports();
const char* getExportSignature(unsigned int i);

]]

local pathPtrType = ffi.typeof('Path*')
local fontPtrType = ffi.typeof('Font*')

local function stripPath(p)
  return string.match(p, "(.-)[^\\/]-$")
end

local function getNexpoPath()
  return stripPath(debug.getinfo(2).source:sub(2))
end

function warn(...)
  io.stderr:write(..., '\n')
end

local gfxlib
if jit.os == 'Windows' then
  gfxlib = ffi.load(getNexpoPath() .. 'gfxlib.dll')
elseif jit.os == 'OSX' then
  gfxlib = ffi.load(getNexpoPath() .. 'libgfxlib.dylib')
else
  error 'Unimplemented OS'
end

local lastMousePos = ffi.new 'double[2]'
local windowPixelsPerUnit = 10
local windowCenterX = 0
local windowCenterY = 0
local windowScaleX
local windowScaleY
local shaders = {}
local shaderForName = {}
local currentShader
local ffiMat3 = ffi.new 'float[9]'
local startTime
local kControlPrefix = '\x05'
local controlHandlers = {}
local controls = {}

local function initShader(shader)
  local nparam = gfxlib.getShaderParameterCount(shader.id)
  shader.varnames = {}
  shader.vartypes = {}
  local rows = ffi.new 'int[1]'
  local cols = ffi.new 'int[1]'
  local varname = ffi.new 'char[256]'
  local vartype = ffi.new 'char[256]'
  for i=0,nparam-1 do
    if not gfxlib.getShaderParameterNameAndType(shader.id, i, varname, vartype, ffi.sizeof(varname)) then
    end
    shader.varnames[i] = ffi.string(varname)
    shader.vartypes[i] = ffi.string(vartype)

    if shader.vartypes[i] == 'unknown' then
      error 'Unsupported variable type in shader'
    end

    if shader.varnames[i] == 'v_transform' then
      shader.transformLocation = i
    end
  end

  if not shader.transformLocation then
    error 'Shader is missing transform variable'
  end
  
end


function nexpo.graphics.addshader(name, src, defaults)
  assert(type(name) == 'string' and type(src) == 'string', 'Wrong argument type')
  if shaderForName[name] then
    error 'A shader with that name already exists'
  end

  local shaderId = gfxlib.addShader(src)
  if shaderId == 0 then
    error(string.format('Unable to add shader %q', name))
  end

  local shader = {
    id = shaderId,
    name = name,
    defaults = defaults or {}
  }
  initShader(shader)

  shaders[shaderId] = shader
  shaderForName[name] = shader

end


local function updateWindowTransform()
  local width, height = nexpo.window.size()
  -- Scale from axis coordinates to normalized device coordinates
  -- The * 2 is because the OpenGL NDC is from -1 to 1, hence has width 2
  windowScaleX = windowPixelsPerUnit * 2 / width
  windowScaleY = windowPixelsPerUnit * 2 / height
end

function nexpo.window.scale(v)
  if v then
    windowPixelsPerUnit = v
    updateWindowTransform()
  end
  return v
end

function nexpo.window.center(x, y)
  if y then
    windowCenterX = x
    windowCenterY = y
    updateWindowTransform()
  end
  return x, y
end


local function setTransform(xscale, yscale, rotation, translatex, translatey)
  -- Combines the requested object transform with the window transform (to produce NDC)
  local rad = math.rad(rotation)
  local cos = math.cos(rad)
  local sin = math.sin(rad)
    
  ffiMat3[0] = windowScaleX *  xscale * cos
  ffiMat3[1] = windowScaleX * xscale * sin
  ffiMat3[2] = windowScaleX * (translatex - windowCenterX)
  ffiMat3[3] = windowScaleY * -yscale * sin
  ffiMat3[4] = windowScaleY * yscale * cos
  ffiMat3[5] = windowScaleY * (translatey - windowCenterY)
  ffiMat3[6] = 0
  ffiMat3[7] = 0
  ffiMat3[8] = 1

  gfxlib.setShaderParameter3x3(currentShader.transformLocation, ffiMat3)
end

local function setObjectTransform(obj)
  local rotation = obj.rotation or 0
  local xscale = obj.width or obj.size or 1
  local yscale = obj.height or obj.size or 1
  local translatex = obj.x or 0
  local translatey = obj.y or 0
  setTransform(xscale, yscale, rotation, translatex, translatey)
end

local function setShaderParameter(index, param)
  if type(param) == 'number' then
    gfxlib.setShaderParameter1(index, param)
  elseif type(param) == 'table' then
    if #param == 4 then
      gfxlib.setShaderParameter4(index, param[1], param[2], param[3], param[4])
    elseif #param == 3 then
      gfxlib.setShaderParameter3(index, param[1], param[2], param[3])
    elseif #param == 2 then
      gfxlib.setShaderParameter2(index, param[1], param[3])
    elseif #param == 1 then
      gfxlib.setShaderParameter1(index, param[1])
    elseif #param == 0 then
      error(string.format('Invalid value for parameter %s: table is not an array', currentShader.varnames[index]))
    else
      error(string.format('Invalid size (%d) for parameter %s', #table, currentShader.varnames[index]))
    end
  else
    error 'Invalid or missing parameter'
  end
end


local function loadShaderFromDisk(name)
  local shader = require('shaders.' .. name)
  if not shader.source then
    error("Error loading shader " .. name .. ": shader source missing")
  end
  if type(shader.source) ~= 'string' then
    error("Error loading shader " .. name .. ": source not returned from shader file")
  end
  nexpo.graphics.addshader(name, shader.source, shader.defaults)
  return shaderForName[name]
end

local function useShader(name)
  name = name or 'color'

  -- Return if shader is already active
  if currentShader and currentShader.name == name then return end

  currentShader = shaderForName[name]

  if not currentShader then
    currentShader = loadShaderFromDisk(name)
  end

  gfxlib.useShader(currentShader.id)
end 

local function setShaderParameters(obj)
  local style = obj.style
  if style then
    useShader(style.shader)

    for i=0,#currentShader.varnames do
      if i ~= currentShader.transformLocation then
        local varname = currentShader.varnames[i]
        if style[varname] then
          setShaderParameter(i, style[varname])
        elseif currentShader.defaults[varname] then
          setShaderParameter(i, currentShader.defaults[varname])
        end
      end
    end
  else
    useShader()
  end

  setObjectTransform(obj)
end

local function isPath(p)
  return type(p) == 'cdata' and ffi.typeof(p) == pathPtrType
end

local function isFont(f)
  return type(f) == 'cdata' and ffi.typeof(f) == fontPtrType
end

local function drawMany(obj)
  for i=1,#obj do
    draw(obj[i])
  end
end

function nexpo.graphics.draw(obj)
  if #obj > 0 then
    return drawMany(obj)
  end

  if obj.shape == nil then
    error 'Missing "shape" field in object'
  end

  setShaderParameters(obj)

  local fill = true
  local strokewidth
  if obj.drawtype == 'stroke' then
    fill = false
    strokewidth = obj.strokewidth or 1
  end

  if isPath(obj.shape) then
    if fill then
      gfxlib.drawFilledPath(obj.shape)
    else
      gfxlib.drawStrokedPath(obj.shape, strokewidth)
    end
  elseif obj.shape == 'rect' then
    if fill then
      gfxlib.drawFilledSquare()
    else
      gfxlib.drawStrokedSquare()
    end
  elseif obj.shape == 'circle' then
    if fill then
      gfxlib.drawFilledCircle()
    else
      gfxlib.drawStrokedCircle()
    end
  else
    error 'Unknown shape field in object'
  end
end

local function pollEvents()
    gfxlib.pollEvents()
end

-- gfxlib.pollEvents is a C function which calls lua callbacks.
-- It's necessary to disable JIT compilation for the function call,
-- as per http://luajit.org/ext_ffi_semantics.html#callback
jit.off(pollEvents)

local function showHelp(target)
  assert(type(target) == 'string', 'parameter must be a string')
  print(kControlPrefix..'help '..target)
end

local function commandForInput(input)
  if input:lower() == 'help' or input:sub(1,5):lower() == 'help ' then
    return function() showHelp(input:sub(6)) end
  end
  
    -- treat as expression by prepending 'return '
  local command, err = loadstring('return ' .. input)
  if command then return command end

  -- try input as-is
  command, err = loadstring(input)
  if command then return command end

  -- both failed, return error
  return nil, err
end



local function createNumberSetter(target)
  if type(target) == 'string' then
    local setter = loadstring('return function(v) ' .. target .. ' = tonumber(v) end')()
    assert(setter, 'failed to create setter')
    return setter
  end
end

local function createStringSetter(target)
  if type(target) == 'string' then
    local setter = loadstring('return function(v) ' .. target .. ' = tostring(v) end')()
    assert(setter, 'failed to create setter')
    return setter
  end
end

local function createGetter(target)
  if type(target) == 'string' then
    local getter = loadstring('return ' .. target )
    assert(getter, 'failed to create getter')
    return getter
  elseif type(target) == 'function' then
    return target
  else
    error 'invalid target type, must be a function or string'
  end
end

local function removeSpaces(str)
  return str:gsub(' ', '')
end

local function createControl(target)
  local name
  if type(target) == 'string' then
    name = target
  elseif type(target) == 'function' then
    name = removeSpaces(tostring(target))
  else
    error('invalid target type, must be a string or funciton')
  end
  local control = {}
  control.getter = createGetter(target)
  control.numberSetter = createNumberSetter(target)
  control.stringSetter = createStringSetter(target)
  control.name = name
  controls[name] = control
  return control  
end

--- Create a slider control. The slider can be used to modify a variable in a running script.
-- @param target A string containing the target value to modify, eg 'mycircle.diameter'
-- @param min The lower bound of the slider
-- @param max The upper bound of the slider
-- @param initial The initial value of the slider
-- @usage c = circle()
-- slider('c.diameter', 0, 10, 0.1)
-- @see plot
-- @see combobox
-- @see editbox
-- @see checkbox
function nexpo.controls.slider(target, min, max, initial)
  assert(type(target) == 'string', 'target parameter must be a string')
  local control = createControl(target)
  local value = control.getter()
  
  if max and min then
    value = value or (max-min)/2
  else
    if value then
      min = min or 0.2*value
      max = max or 5.0*value
      if min > max then
        -- swap when value is negative
        local tmp = min
        min = max
        max = min
      end
      
    else
      min = min or 0
      max = max or 1
      value = (max-min)/2
    end
  end
  
  assert(max>=min, 'Maximum value must be >= minimum value')
 
  if not initial then
    initial = value or (max-min)/2
  end
  
  print(kControlPrefix..'slider '..control.name..' '..min..' '..max..' '..initial)
end

--- Create a time series plot.
-- The value of a variable or function will be plotted over time.
-- @param target A string containing the target variable to plot, eg 'mycircle.diameter'
-- @usage c = circle()
-- plot 'c.diameter'
-- @see slider
function nexpo.controls.plot(target)
  assert(target, 'missing target argument')
  assert(type(target) == 'string' or type(target) == 'function', 'invalid target type')
  local control = createControl(target)
  print(kControlPrefix..'timeseriesplot '.. control.name)
end

local function sendControlValue(name, value)
  if type(value) == 'number' then
    print(kControlPrefix .. 'numbervalue ' .. name .. ' ' .. value)
  elseif type(value) == 'string' then
    print(kControlPrefix .. 'stringvalue ' .. name .. ' ' .. value)
  end
end

local function sendControlUpdates()
  for name, control in pairs(controls) do
    local success, value = pcall(control.getter)
    if not success then
      warn(string.format("Error while executing getter for %q: %s", tostring(name), value))
      -- TODO: send message that this control is being deleted
      controls[name] = nil
    else
      if control.lastSent ~= value then
        sendControlValue(name, value)
        control.lastSent = value
      end
    end
  end
end



local function sendControlData()
  -- frame info
  print(kControlPrefix..'frameinfo '.. nexpo.graphics.time())
  
  sendControlUpdates()
end

function controlHandlers.numbervalue(params)
  if #params ~= 3 then return end
  local name = params[2]
  local value = tonumber(params[3])
  local control = controls[name]
  if control then
    control.numberSetter(value)
    control.lastSent = value
  end
end

local function handleControlCommand(cmd)
  local parts = {}
  local index = 0
  while index do
    local space = cmd:find(' ', index)
    if space then
      table.insert(parts, cmd:sub(index, space-1))
      index = space + 1
    else
      table.insert(parts, cmd:sub(index))
      break
    end
  end

  local handler = controlHandlers[parts[1]]
  if handler then handler(parts) end
end

local function handleInputLine(line)
  if line:sub(1,#kControlPrefix) == kControlPrefix then
    return handleControlCommand(line:sub(2))
  end
  
  local command, err = commandForInput(line)
  if not command then
    error(err)
    return
  end
  
  nexpo.console.dumpshort(command())
end


local inputLineBuffer = ffi.new 'char[1024]'
local function getInputLine()
  if not gfxlib.getNextInputLine(inputLineBuffer, ffi.sizeof(inputLineBuffer)) then
    return
  end
  return ffi.string(inputLineBuffer)
end

local function checkInput()
  while true do
    local line = getInputLine()
    if line == nil then return end
    xpcall(handleInputLine, function(e) warn(tostring(e)) end, line)
  end
end

--- Callback function for when a new frame is ready to draw. Implement this 
-- function to do your drawing for each frame. It will be called by Nexpo every
-- screen refresh until the user closes the window or the script calls
-- <code>nexpo.stop()</code>.
-- @param elapsed The seconds elapsed since the script started
-- @see nexpo.start
-- @see nexpo.stop
function nexpo.graphics.onframe(elapsed) end

-- we want to force the user to implement this
nexpo.graphics.onframe = false

--- Start running a script. This should be the last line of every Nexpo script.
-- It passes control to Nexpo, which will run the render loop and process user input.
-- @see nexpo.stop
-- @see nexpo.graphics.onframe
function nexpo.start()
  assert(type(nexpo.graphics.onframe) == 'function', 'Missing "nexpo.graphics.onframe" function, nothing to do')
  startTime = gfxlib.canvasTime()
  
  while not gfxlib.shouldClose() and type(nexpo.graphics.onframe) == 'function' do
    updateWindowTransform()   -- TODO: only need to call this on window resize callback
    nexpo.graphics.onframe(nexpo.graphics.time())
    sendControlData()
    io.stdout:flush()
    io.stderr:flush()
    collectgarbage 'collect'
    gfxlib.swapBuffers()
    pollEvents()
    checkInput()
  end

  gfxlib.destroyCanvas()
end

--- Stop running a script. This function can be called at any time to stop rendering.
-- Nexpo will stop its render loop and control will return to where the start() function
-- was called (which should be the end of the script).
-- @see nexpo.start
-- @see nexpo.graphics.onframe
function nexpo.stop()
  gfxlib.setWindowShouldClose(true)
end

function nexpo.graphics.rgb(r, g, b)
  return {r or 0, g or 0, b or 0, 1}
end

function nexpo.graphics.rgba(r, g, b, a)
  return {r or 0, g or 0, b or 0, a or 1}
end

-------

function nexpo.mouse.pos()
  gfxlib.cursorPos(lastMousePos)
  return lastMousePos[0] / windowPixelsPerUnit - windowCenterX,
         lastMousePos[1] / windowPixelsPerUnit - windowCenterY 
end

function nexpo.mouse.isdown(btn)
  btn = btn or 1
  assert(type(btn) == 'number', 'expected number parameter')
  return gfxlib.mouseButton(btn-1)
end
  
function nexpo.graphics.time()
  if startTime then
    return gfxlib.canvasTime() - startTime
  else
  	return 0
  end
end

function nexpo.graphics.path(svg)
  local p = gfxlib.newPath()
  assert(p ~= nil, "Couldn't create path object")
  ffi.gc(p, gfxlib.freePath)

  if type(svg) == 'string' then
    loadsvg(p, svg)
  end
  return p
end

function nexpo.graphics.moveto(p, x, y)
  assert(p ~= nil, 'Missing path parameter')
  gfxlib.moveTo(p, x, y)
end

function nexpo.graphics.lineto(p, x, y)
  assert(p ~= nil, 'Missing path parameter')
  gfxlib.lineTo(p, x, y)
end

function nexpo.graphics.curveto(p, p1x, p1y, p2x, p2y, p3x, p3y)
  assert(p ~= nil, 'Missing path parameter')
  if p3y then
    gfxlib.cubicCurveTo(p, p1x, p1y, p2x, p2y, p3x, p3y)
  elseif p2y then
    gfxlib.quadraticCurveTo(p, p1x, p1y, p2x, p2y)
  else
    error('Missing parameters', 2)
  end
end


function nexpo.graphics.bgcolor(r, g, b, a)
  gfxlib.setClearColor(r, g, b, a or 0)
end

function nexpo.graphics.loadsvg(path, str)
  gfxlib.appendSvgPath(path, str)
end


function nexpo.graphics.loadfont(path)
  assert(path, 'Missing font path parameter')
  local font = gfxlib.loadFont(path)
  if font == nil then
    error('Error loading font ' .. path, 2)
  end
  ffi.gc(font, gfxlib.freeFont)
  return font
end

function nexpo.graphics.codepoint(font, codepoint)
  assert(isFont(font), 'Invalid font parameter')
  assert(type(codepoint) == 'number', 'Invalid codepoint parameter (should be  a number)')
  local path = gfxlib.pathForCodepoint(font, codepoint)
  if path == nil then
    error('Error loading codepoint', 2)
  end
  return path
end


-------

-- Convenience functions for particular shape/style combinations
function nexpo.graphics.circle(x, y, diameter)
  return {
    shape = 'circle',
    x = x,
    y = y,
    size = diameter,
    style = {},
  }
end

function nexpo.graphics.ellipse(x, y, width, height)
  return {
    shape = 'circle',
    x = x,
    y = y,
    width = width,
    height = height,
    style = {},
  }
end

function nexpo.graphics.grating(x, y, diameter, sf)
  local g = nexpo.graphics.circle(x, y, diameter)
  g.style.shader = 'grating'
  g.style.sf = sf
  return g
end

function nexpo.graphics.rect(x, y, width, height)
  local r = {
    shape = 'rect',
    x = x,
    y = y,
  }

  -- Treat as a square if height omitted (use size field)
  if height then
    r.width = width
    r.height = height
  else
    r.size = width
  end

  return r
end

-------

--- Mouse button down callback function. Implement this function to receive
-- a callback whenever a mouse button is pressed.
-- @param button An integer representing the button pressed (1 = left, 2 = right, 3 = middle)
-- @param modifiers Table with boolean flags of keyboard modifiers that were pressed at the time.
-- @see nexpo.mouse.onup
-- @see nexpo.mouse.onmove
-- @see nexpo.mouse.onscroll
function nexpo.mouse.ondown(button, modifiers) end

--- Mouse button up callback function. Implement this function to receive
-- a callback whenever a mouse button is released.
-- @param button An integer representing the button pressed (1 = left, 2 = right, 3 = middle)
-- @param modifiers Table with boolean flags of keyboard modifiers that were pressed at the time.
-- @see nexpo.mouse.ondown
-- @see nexpo.mouse.onmove
-- @see nexpo.mouse.onscroll
function nexpo.mouse.onup(button, modifiers) end

--- Mouse move callback function. Implement this function to receive
-- a callback whenever the mouse position changes.
-- @param x x coordinate of the mouse cursor
-- @param y y coordinate of the mouse cursor
-- @see nexpo.mouse.onup
-- @see nexpo.mouse.ondown
-- @see nexpo.mouse.onscroll
function nexpo.mouse.onmove(x, y) end

--- Mouse scroll wheel callback function. Implement this function to receive
-- a callback whenever the mousewheel is scrolled.
-- @param yoffset Scroll amount in the vertical direction
-- @param xoffset Scroll amount in the horizontal direction
-- @see nexpo.mouse.onup
-- @see nexpo.mouse.ondown
-- @see nexpo.mouse.onmove
function nexpo.mouse.onscroll(yoffset, xoffset) end

--- Keyboard button down callback function. Implement this function to receive
-- a callback whenever a key is pressed.
-- @param keyname A string representation of the key name, eg 'a' or 'left_shift'
-- @param scancode Scancode of the key
-- @param modifiers Table with boolean flags of keyboard modifiers that were pressed at the time.
-- @see nexpo.key.onup
-- @see nexpo.key.onrepeat
-- @see nexpo.key.onchar
function nexpo.key.ondown(keyname, scancode, modifiers) end

--- Keyboard button down callback function. Implement this function to receive
-- a callback whenever a key is pressed.
-- @param keyname A string representation of the key name, eg 'a' or 'left_shift'
-- @param scancode Scancode of the key
-- @param modifiers Table with boolean flags of keyboard modifiers that were pressed at the time.
-- @see nexpo.key.ondown
-- @see nexpo.key.onrepeat
-- @see nexpo.key.onchar
function nexpo.key.onup(keyname, scancode, modifiers) end

--- Keyboard button repeat callback function. Implement this function to receive
-- a callback whenever a key has been held down and is producing repeated characters.
-- @param keyname A string representation of the key name, eg 'a' or 'left_shift'
-- @param scancode Scancode of the key
-- @param modifiers Table with boolean flags of keyboard modifiers that were pressed at the time.
-- @see nexpo.key.onup
-- @see nexpo.key.ondown
-- @see nexpo.key.onchar
function nexpo.key.onrepeat(keyname, scancode, modifiers) end

--- Keyboard unicode character function. Implement this function to receive
-- a callback with the unicode codepoint value when a key is pressed.
-- @param codepoint The unicode codepoint of the key pressed
-- @see nexpo.key.ondown
-- @see nexpo.key.onup
-- @see nexpo.key.onrepeat
function nexpo.key.onchar(codepoint) end


local modifierTable = {
  shift = false,
  control = false,
  alt = false,
  super = false,
}

local function getModifiers(m)
    modifierTable.shift   = bit.band(m, 0x0001) ~= 0
    modifierTable.control = bit.band(m, 0x0002) ~= 0
    modifierTable.alt     = bit.band(m, 0x0004) ~= 0
    modifierTable.super   = bit.band(m, 0x0008) ~= 0
    return modifierTable
end

local namesForKeys
local function keyCallback(key, scancode, action, modifierMask)
  if namesForKeys == nil then
    namesForKeys = require 'namesForKeys'
  end

  local keyname = namesForKeys[key] or key
  local mods = getModifiers(modifierMask)
  if action == 0 then
    if nexpo.key.onup then nexpo.key.onup(keyname, scancode, mods) end
  elseif action == 1 then
    if nexpo.key.ondown then nexpo.key.ondown(keyname, scancode, mods) end
  elseif action == 2 then
    if nexpo.key.onrepeat then nexpo.key.onrepeat(keyname, scancode, mods) end
  end

end

local function cursorPosCallback(x, y)
  if nexpo.mouse.onmove then nexpo.mouse.onmove(x, y) end
end

local function scrollCallback(xoffset, yoffset)
  -- NOTE: parameter order reversed on purpose
  if nexpo.mouse.onscroll then nexpo.mouse.onscroll(yoffset, xoffset) end
end

local function charCallback(codepoint)
  if nexpo.key.onchar then nexpo.key.onchar(codepoint) end
end

local function mouseButtonCallback(button, action, modifierMask)
  local mods = getModifiers(modifierMask)
  local btn = button + 1    -- GLFW starts button numbers at 0
  if action == 1 then
    if nexpo.mouse.ondown then nexpo.mouse.ondown(btn, mods) end
  else
    if nexpo.mouse.onup then nexpo.mouse.onup(btn, mods) end
  end
end


local function loadsettings()
  local settings = {
    window_width = 640,
    window_height = 480,
    fullscreen_width = 0,
    fullscreen_height = 0,
    title = "Nexpo",
    fullscreen = false,
    vsync = true,
  }

  local settingsFile = getNexpoPath() .. '../settings.lua'
  local f = loadfile(settingsFile, 't', settings)

  if f then
    local success, err = pcall(f)
    if not success then
      warn("Error while loading settings:\n" .. err)
    end
  end

  return settings
end

local function setWindowHints(settings)
  local hints = {
    samples = 0x0002100D,
    refresh_rate = 0x0002100F,
    resizable = 0x00020003,
    decorated = 0x00020005,
    iconified = 0x00020002,
    visible = 0x00020004,
    focused = 0x00020001,
  }

  for key, value in pairs(settings) do
    if hints[key] then
      gfxlib.setWindowHint(hints[key], value)
    end
  end

end

function nexpo.window.size()
  local size = ffi.new 'int[2]'
  gfxlib.windowSize(size)
  return size[0], size[1]
end

function nexpo.window.pos(x, y)
  gfxlib.setWindowPos(x, y)
end

function nexpo.window.monitorname()
  local name = gfxlib.getMonitorName()
  if name == nil then
    return nil
  else
    return ffi.string(name)
  end
end

function nexpo.window.setgamma(g)
  assert(type(g) == 'number', 'Argument must be a single number')
  gfxlib.setGamma(g)
end

function nexpo.window.setgammaramp(r, g, b)
  assert(type(r) == 'table' and type(g) == 'table' and type(b) == 'table',
    'Invalid gamma ramp parameters: expected three arrays of the same size')

  local n = #r
  assert(n > 0, 'Gamma ramp parameters must be arrays')
  assert(#g == n and #b == n, 'Gamma ramp parameters must be arrays of the same size')

  local rgb = ffi.new('unsigned short[?]', n*3)

  for i=1,n do
    rgb[i-1] = r[i]
    rgb[i-1+n] = g[i]
    rgb[i-1+n+n] = b[i]
  end

  gfxlib.setGammaRamp(rgb, n)
end

local function lookupMonitor(mon)
  if type(mon) == 'number' then
    return mon
  elseif type(mon) == 'string' then
    local index = gfxlib.getMonitorByName(mon)
    if index == -1 then
      print(string.format("Warning: no monitor named %q found"))
    end
    return index
  elseif mon == nil then
    return -1
  else
    error('Requested monitor must be an number (monitor index) or a string (monitor name)', 2)
  end
end

function ansi(...)
  return '\x1b[' .. table.concat({...}, ';') .. 'm'
end



local numberstyle = ansi(31)
local stringstyle = ansi(32)
local functionstyle = ansi(35)
local threadstyle = ansi(36)
local booleanstyle = ansi(91)
local otherstyle = ansi(95)
local userdatastyle = ansi(37)
local tablestyle = ansi(33)
local cdatastyle = ansi(47)
local endstyle = ansi(0)

local singleIndent = '    '
local bullet = '• '
local serializeOne
local function serializeTable(o, seen, maxindent, indent)
    seen = seen or {}

    local style = type(o) == 'table' and tablestyle or cdatastyle
    local s = style .. tostring(o) .. endstyle

    if #indent <= maxindent*#singleIndent then
        if seen[o] then return s .. ', would recurse' end
        seen[o] = true

        -- call pairs, or if it's missing, ipairs (cdata may only define ipairs, or nothing)
        if pcall(pairs, o) then
            s = s .. '\n'
            for k,v in pairs(o) do
                s = s  .. indent .. bullet .. serializeOne(k, seen, maxindent, indent)
                s = s .. ": " .. serializeOne(v, seen, maxindent, indent .. singleIndent)
                s = s .. '\n'
            end
        elseif pcall(ipairs, o) then
            s = s .. '\n'
            local limit = math.min(#o, 20)
            if #o - limit == 1 then limit = #o end
            for i=1,limit do
                s = s  .. indent .. bullet .. serializeOne(i, seen, maxindent, indent)
                s = s .. ": " .. serializeOne(o[i], seen, maxindent, indent..singleIndent)
                s = s .. '\n'
            end
            if limit < #o then
                s = s .. indent '..and ' .. (#o - limit) .. ' other elements\n'
            end
        end
    end

    seen[o] = nil
    return s --string.sub(s,1,-2)
end

serializeOne = function (o, seen, maxindent, indent)
    indent = indent or singleIndent
    maxindent = maxindent or 10
    --if reclevel > recmax then return '*' end
    
    if type(o) == "number" then
        return numberstyle .. tostring(o) .. endstyle
    elseif type(o) == "string" then
        return stringstyle .. tostring(o) .. endstyle
    elseif type(o) == "function" then
        return functionstyle .. tostring(o) .. endstyle
    elseif type(o) == "thread" then
        return threadstyle .. tostring(o) .. endstyle
    elseif type(o) == "boolean" then
        return booleanstyle .. tostring(o) .. endstyle
    elseif type(o) == 'userdata' then
      return userdatastyle .. tostring(o) .. endstyle
    elseif type(o) == "table" then
        return serializeTable(o, seen, maxindent, indent)
    elseif type(o) == 'cdata' then
        return serializeTable(o, seen, maxindent, indent)
    else
        return otherstyle .. tostring(o) .. endstyle
    end
end

function serialize(t, reclevel)
  local s = ''
  for k,v in pairs(t) do
    if #s > 0 then s = s .. " " end
    s = s .. serializeOne(v, {}, reclevel)
  end
  return s
end

--- Recursively print the contents of a variable.
-- @param ... One or more variables to print
function nexpo.console.dump(...)
    local output = serialize({...}, math.huge)
    if output and #output > 0 then print(output) end
end

--- Print the contents of a variable, recursing a maximum of one level into tables.
-- @param ... One or more variables to print
function nexpo.console.dumpshort(...)
    local output = serialize({...}, 1)
    if output and #output > 0 then print(output) end
end




local function init()
  -- Add exported API declarations to FFI
  local nsig = gfxlib.getNumExports()
  for i=0,nsig-1 do
    local sig = gfxlib.getExportSignature(i)
    assert(sig, 'Error: got null API signature')
    ffi.cdef(ffi.string(sig))
  end

  local settings = loadsettings()
  setWindowHints(settings)

  local width, height
  if settings.fullscreen then
    width = settings.fullscreen_width
    height = settings.fullscreen_height
  else
    width = settings.window_width
    height = settings.window_height
  end

  gfxlib.createCanvas(
    width,
    height,
    settings.title,
    settings.fullscreen,
    settings.vsync,
    lookupMonitor(settings.monitor),
    0)

  if settings.gamma_ramp then
    assert(type(settings.gamma_ramp) == 'table', 'Error: gamma_ramp setting must be a table')
    nexpo.window.setgammaramp(settings.gamma_ramp.r, settings.gamma_ramp.g, settings.gamma_ramp.b)
  elseif settings.gamma then
  end

  if settings.window_x and settings.window_y then
    nexpo.window.pos(settings.window_x, settings.window_y)
  end

  gfxlib.setKeyCallback(keyCallback)
  gfxlib.setScrollCallback(scrollCallback)
  gfxlib.setCharCallback(charCallback)
  --gfxlib.setCursorPosCallback(cursorPosCallback)
  gfxlib.setMouseButtonCallback(mouseButtonCallback)

end

init()

local function makereadonly(t)
  assert(type(t) == 'table', 'argument must be a table')
  setmetatable(t, {
    __newindex = function(s, key, value)
      error('table is read only, cannot insert "' .. tostring(key) .. '"',2)
    end,
  })
end

for k,v in pairs(nexpo) do
  if type(v) == 'table' then makereadonly(v) end
end

makereadonly(nexpo)

return nexpo


