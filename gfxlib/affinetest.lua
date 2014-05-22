

function identity()
  return {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
  }
end

function matMul(a, b)
  -- This replaces the contents of b with the result
  
  local m11 = a[1]*b[1] + a[2]*b[4] + a[3]*b[7]
  local m12 = a[1]*b[2] + a[2]*b[5] + a[3]*b[8]
  local m13 = a[1]*b[3] + a[2]*b[6] + a[3]*b[9]
  
  local m21 = a[4]*b[1] + a[5]*b[4] + a[6]*b[7]
  local m22 = a[4]*b[2] + a[5]*b[5] + a[6]*b[8]
  local m23 = a[4]*b[3] + a[5]*b[6] + a[6]*b[9]
  
  local m31 = a[7]*b[1] + a[8]*b[4] + a[9]*b[7]
  local m32 = a[7]*b[2] + a[8]*b[5] + a[9]*b[8]
  local m33 = a[7]*b[3] + a[8]*b[6] + a[9]*b[9]
  
  b[1] = m11
  b[2] = m12
  b[3] = m13
  b[4] = m21
  b[5] = m22
  b[6] = m23
  b[7] = m31
  b[8] = m32
  b[9] = m33
end

function transformPoint(v, m)
  -- This replaces v with the result of transforming it by the 3x3 matrix m
  -- v should be a 2-element vector. The homogenous coordinate is assumed to be 1.
  
  local u1 = m[1]*v[1] + m[2]*v[2] + m[3]
  local u2 = m[4]*v[1] + m[5]*v[2] + m[6]
  local u3 = m[7]*v[1] + m[8]*v[2] + m[9]
  
  v[1] = u1
  v[2] = u2
end

function scaleMat(xscale, yscale)
  return {
    xscale,      0, 0,
         0, yscale, 0,
         0,      0,  1,
   }
end

function shearMat(shearx, sheary)
  return {
         1, shearx, 0,
    sheary,      1, 0,
         0,      0, 1,
  }
end

function translationMat(x, y)
  return {
    1, 0, x,
    0, 1, y,
    0, 0, 1,
  }
end



 function matrixScaleRotTranslate(xscale, yscale, rotation, translatex, translatey)
   -- Returns a matrix which performs: scale then rotate then translate
   local rad = math.rad(rotation)
   local cos = math.cos(rad)
   local sin = math.sin(rad)
    
   return {
      xscale*cos, xscale*sin, translatex,
     -yscale*sin, yscale*cos, translatey,
               0,          0,          1,
   }
 end
 
function matrixScaleTranslate(xscale, yscale, translatex, translatey)
  -- Returns a matrix which performs: scale then then translate
  return {
    xscale,      0, translatex,
         0, yscale, translatey,
         0,      0,          1,
  }
end

 