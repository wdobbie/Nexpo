-------------------------------------------------------
--
-- This is the settings file for Nexpo scripts
--
-------------------------------------------------------


-- Window size in pixels
window_width = 600
window_height = 600

-- Window position
 --window_x = 1024
 --window_y = 300

-- Full screen resolution in pixels. Set to 0 for native resolution.
fullscreen_width = 0
fullscreen_height = 0

-- Set true for fullscreen, false for windowed
-- fullscreen = true

-- Samples per pixel for multisample antialiasing
samples = 16

-- Override default monitor gamma value, only applies when fullscreen
gamma = 2.2

-- Override monitor gamma with an explicit ramp for red, green blue
-- This variable should be a table with 3 fields, 'r', 'g', 'b', each of which
-- should be an equal sized array of numbers.
-- gamma_ramp = {
--	r = {...},
--	g = {...},
--	b = {...}
-- }

-- Target monitor, only applies when fullscreen is true
-- Can either be a number from 1 to n, where n is the number of attached monitors,
-- or it can be the name of a monitor, eg 'DELL U2713H'
-- The current monitor name can be retrieved from lua using print(monitorname()),
-- when fullscreen is active.
monitor = 1
