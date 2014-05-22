#include <iostream>
#include <math.h>
#include "path.h"

int main()
{
    Path path;
    path.lineTo(100, 10);
    path.lineTo(100, 300);
    path.lineTo(10, 50);

    path.moveTo(40, 40);
    path.lineTo(50, 40);
    path.lineTo(30, 50);

    Mesh mesh = meshFromPath(&path);

    std::cout << "vert count:  " << mesh.vertices.size() << std::endl;
    std::cout << "index count: " << mesh.indices.size() << std::endl;
    return 0;
}

