#ifndef PATH_H
#define PATH_H
#include <vector>
#include "glm.hpp"
#include "common.h"

using glm::vec2;

struct Mesh
{
    Mesh(std::vector<vec2>& v, std::vector<unsigned int> i)
        : vertices(v)
        , indices(i)
    {
    }

    Mesh()
    {
    }

    std::vector<vec2> vertices;
    std::vector<unsigned int> indices;
};

class Path;

const Mesh* fillPath(Path* path);
const Mesh* strokePath(Path* path, float strokeWidth);


DLLEXPORT Path* newPath();
DLLEXPORT void moveTo(Path* path, float x, float y);
DLLEXPORT void lineTo(Path* path, float x, float y);
DLLEXPORT void quadraticCurveTo(Path* path, float cx, float cy, float x, float y);
DLLEXPORT void cubicCurveTo(Path* path, float cx1, float cy1, float cx2, float cy2, float x, float y);
DLLEXPORT void arcTo(Path* path, float x1, float y1, float x2, float y2, float radius);
DLLEXPORT void appendSvgPath(Path* path, const char* pathString);


#endif // PATH_H
