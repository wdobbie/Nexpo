

local p = newpath()
p.lineTo(1, 1)
p.quadraticBezierTo(-1, 3, 0.5, 0.5)

shape = fillpath(p)

outline = strokepath(p, 0.1)

-- c = circle(x, y, radius) => sets c.x, c.y, c.scale.. or maybe Mesh has its own hidden scale factor
c = circle()
-- there will now be c.geometry == opaque pointer
-- c is otherwise an empty vanilla table
c.shader = 'grating'
c.color1 = color 'white'
c.color2 = rgb(0, 0, 0)
c.sf = 10
c.rotation = 45

d = strokedcircle()

e = rect()
e.shader = 'image'
e.texture = loadtexture 'pelican.jpg'

e2 = image 'pelican.jpg'	-- creates rect geom with appropriate aspect ratio and shader == 'image'
f = strokedrect()

g = line()

h = line(x1, y1, x2, y2)

function drawframe()
	local x, y = mouse()
	c.phase = time()
	c.x = x
	c.y = y

	for i=1,360 do
		c.rotation = i
		draw(c)
	end

end

function mousemove(x, y)

end

function mousedown(btn, x, y)

end

function mouseup(btn, x, y)

end

function keydown(...)

end



