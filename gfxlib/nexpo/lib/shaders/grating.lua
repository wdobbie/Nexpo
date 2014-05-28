local shader = {}
shader.source = [[
  uniform vec4 color1, color2;
  uniform float phase;
  uniform float sf;

  vec4 getcolor() {
    return mix(color1, color2, 0.5+0.5*cos(sf*texcoord.x + radians(phase)));
  }
]]

shader.defaults = {
	color1 = {1, 1, 1, 1},
	color2 = {0, 0, 0, 1},
	phase = 0,
	sf = 4
}

return shader
