local shader = {}
shader.source = [[
  uniform vec4 color;

  vec4 getcolor() {
    return color;
  }
]]

shader.defaults = {
	color = rgb(1, 1, 1),
}

return shader