local shader = {}
shader.source = [[
  uniform vec4 color1, color2;
  uniform float phase;
  uniform float sf;

  vec4 getcolor() {
  	return mod(texcoord.x * sf + phase/360.0, 1.0) < 0.5 ? color1 : color2;
  }
]]

shader.defaults = {
	color1 = {1, 1, 1, 1},
	color2 = {0, 0, 0, 1},
	phase = 0,
	sf = 4,
}

return shader
