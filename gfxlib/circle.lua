require 'Nexpo'

useshader 'grating'

function draw()
	local x, y = mouse()
	shaderparam('phase', time())
	circle(x, y, 1)
end

start()

