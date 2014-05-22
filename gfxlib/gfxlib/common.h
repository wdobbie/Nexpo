#ifndef COMMON_H
#define COMMON_H

#include "glm.hpp"
using glm::vec2;
using glm::vec3;
using glm::vec4;

#ifdef __APPLE__
#include <OpenGL/gl.h>
//#include <OpenGL/gl3.h>
//#include <OpenGL/gl3ext.h>
#else
#include "gl_2_1.h"
#endif




// Shenanigans for easily exporting function declarations to lua:
// The LUAEXPORT macro will stringify the signature and add it to a global list.
// Lua can then query the list of declarations. This saves manually synchronising
// the API declarations between the C source code and the lua ffi import code.
//
// The macro should only be used in definitions. The functions can also be declared
// in a header file, but in the usual fashion without the macro.
//
// Example:
// LUAEXPORT(int myExportedFunc(int p))
// {
//    return p*p;
// }

void addExport(const char* signature);

class ExportSignature {
public:
    ExportSignature(const char* sig) {
        addExport(sig);
    }
};

#ifdef _MSC_VER
#define DLLEXPORT extern "C" __declspec(dllexport)
#else
#define DLLEXPORT extern "C" __attribute__((visibility ("default")))
#endif

#define TOKENCONCAT(a, b) a ## b
#define TOKENCONCAT2(a, b) TOKENCONCAT(a, b)
#define LUAEXPORT(signature) \
    static ExportSignature TOKENCONCAT2(LuaSignature_, __LINE__)(#signature ";"); \
    DLLEXPORT signature

#endif // COMMON_H
