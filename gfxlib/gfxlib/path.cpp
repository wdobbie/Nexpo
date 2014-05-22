#include "path.h"
#include <iostream>
#include <sstream>
#include "tesselator.h"

TESSalloc talloc;

static void appendQuadraticCurveTo(std::vector<vec2>& contour, const vec2& p0, const vec2& p1, const vec2& p2, int level);
static void appendCubicCurveTo(std::vector<vec2>& contour, const vec2& p1, const vec2& c1, const vec2& c2, const vec2& p2, int segments);


static void* stdAlloc(void*, unsigned int size)
{
    return malloc(size);
}

static void stdFree(void*, void* ptr)
{
    free(ptr);
}

static void* stdRealloc(void*, void* ptr, unsigned int size)
{
    return realloc(ptr, size);
}

enum PathCommand {
    MOVE_TO,
    LINE_TO,
    QUADRATIC_CURVE_TO,
    CUBIC_CURVE_TO,
    ARC_TO,
    CLOSE_PATH,
};

typedef std::vector<vec2> Contour;

class Path {
public:
    Path();
    ~Path();

    void invalidateMeshes();
    std::vector<Contour> contoursFromPath();

    Mesh* m_strokedMesh;
    Mesh* m_filledMesh;
    float m_cachedStrokeWidth;

    std::vector<vec2> m_points;
    std::vector<PathCommand> m_commands;
};




Path::Path()
    : m_strokedMesh(0)
    , m_filledMesh(0)
    , m_cachedStrokeWidth(0)
{

}

Path::~Path() {
    delete m_strokedMesh;
    delete m_filledMesh;
}

LUAEXPORT(Path* newPath())
{
    return new Path;
}

LUAEXPORT(void freePath(Path* path))
{
    delete path;
    std::cerr << "Deleted path" << std::endl;
}

LUAEXPORT(void moveTo(Path* path, float x, float y)) {
    path->invalidateMeshes();
    path->m_commands.push_back(MOVE_TO);
    path->m_points.push_back(vec2(x, y));
}

LUAEXPORT(void lineTo(Path* path, float x, float y)) {
    path->invalidateMeshes();
    path->m_commands.push_back(LINE_TO);
    path->m_points.push_back(vec2(x, y));
}

LUAEXPORT(void quadraticCurveTo(Path* path, float cx, float cy, float x, float y)) {
    path->invalidateMeshes();
    path->m_commands.push_back(QUADRATIC_CURVE_TO);
    path->m_points.push_back(vec2(cx, cy));
    path->m_points.push_back(vec2(x, y));
}

LUAEXPORT(void cubicCurveTo(Path* path, float cx1, float cy1, float cx2, float cy2, float x, float y)) {
    path->invalidateMeshes();
    path->m_commands.push_back(CUBIC_CURVE_TO);
    path->m_points.push_back(vec2(cx1, cy1));
    path->m_points.push_back(vec2(cx2, cy2));
    path->m_points.push_back(vec2(x, y));
}

LUAEXPORT(void arcTo(Path* path, float x1, float y1, float x2, float y2, float radius))
{
    path->invalidateMeshes();
    path->m_commands.push_back(ARC_TO);
    path->m_points.push_back(vec2(x1, y1));
    path->m_points.push_back(vec2(x2, y2));
}

void Path::invalidateMeshes() {
    delete m_strokedMesh;
    delete m_filledMesh;
    m_strokedMesh = m_filledMesh = 0;

}

static bool readFloatsFromStream(std::stringstream& ss, float* dst, int n)
{
    for (int i=0; i<n; i++)
    {
        char peek = ss.peek();
        while(peek == ',') {
            char c;
            ss >> c;
            peek = ss.peek();
        }

        ss >> dst[i];

        if (ss.fail()) {
            std::cerr << "failed to read number from path string" << std::endl;
            return false;
        }

    }
    return true;
}


LUAEXPORT(void appendSvgPath(Path* path, const char* pathString))
{
    std::stringstream ss;
    ss << pathString;

    char cmd = 0;
    while (!ss.eof())
    {
        ss >> cmd;
        if (ss.fail()) {
            std::cerr << "Expected path command character but didn't read one" << std::endl;
            return;
        }

        switch(cmd) {
        case 'M':
        {
            float p[2];
            //std::cerr << "Move to: " << std::endl;
            if (!readFloatsFromStream(ss, p, 2)) return;
            moveTo(path, p[0], p[1]);
            break;
        }
        case 'C':
        {
            float p[6];
            //std::cerr << "Cubic to: " << std::endl;
            readFloatsFromStream(ss, p, 6);
            cubicCurveTo(path, p[0], p[1], p[2], p[3], p[4], p[5]);
            break;
        }
        case 'Q':
        {
            float p[4];
            readFloatsFromStream(ss, p, 4);
            quadraticCurveTo(path, p[0], p[1], p[2], p[3]);
            break;
        }

        case 'L':
        {
            float p[2];
            //std::cerr << "Line to: " << std::endl;
            readFloatsFromStream(ss, p, 2);
            lineTo(path, p[0], p[1]);
            break;

        }
        case 'z':
            //std::cerr << "End path" << std::endl;
            return;
        case ' ':
            break;
        default:
            std::cerr << "Unknown path command: " << cmd << std::endl;
            return;
        }

    }
}

std::vector<Contour> Path::contoursFromPath()
{
    std::vector<Contour> contours;

    contours.push_back(Contour());

    vec2 currentPos;
    currentPos.x = 0;
    currentPos.y = 0;

    int pointIndex = 0;

    for (size_t pathIndex = 0; pathIndex < m_commands.size(); pathIndex++) {
        switch(m_commands[pathIndex])
        {
        case MOVE_TO:
        {
            currentPos = m_points[pointIndex++];

            if (contours.back().size() == 0) {
                // Empty contour, append point
                contours.back().push_back(currentPos);
            } else if (contours.back().size() == 1) {
                // Contour only contains a moveTo, replace existing point
                std::cerr << "Replacing single moveTo" << std::endl;
                contours.back()[0] = currentPos;
            } else {
                // Contour contains line segments already, start a new contour
                contours.push_back(Contour());
                contours.back().push_back(currentPos);
            }
            break;
        }
        case LINE_TO:
        {
            currentPos = m_points[pointIndex++];
            contours.back().push_back(currentPos);
            break;
        }
        case QUADRATIC_CURVE_TO:
        {
            const vec2& c1 = m_points[pointIndex++];
            const vec2& p2 = m_points[pointIndex++];
            appendQuadraticCurveTo(contours.back(), currentPos, c1, p2, 0);
            currentPos = p2;
            break;
        }
        case CUBIC_CURVE_TO:
        {
            const vec2& c1 = m_points[pointIndex++];
            const vec2& c2 = m_points[pointIndex++];
            const vec2& p2 = m_points[pointIndex++];
            appendCubicCurveTo(contours.back(), currentPos, c1, c2, p2, 64);
            currentPos = p2;
            break;
        }
        default:
            std::cerr << "Unknown path command" << std::endl;
            return contours;
        }
    }

    return contours;
}



#define PATH_RECURSION_LIMIT 5
#define PATH_COLLINEARITY_EPSILON FLT_EPSILON

// cos(theta) where theta is maximum angle between line segments
const float BEZIER_FLATNESS = 0.998f;

void appendQuadraticCurveTo(std::vector<vec2>& contour, const vec2& p1, const vec2& p2, const vec2& p3, int level)
{
    vec2 m12 = 0.5f * (p1 + p2);
    vec2 m23 = 0.5f * (p2 + p3);
    vec2 m123 = 0.5f * (m12 + m23);

    //vec2 d31 = p3 - p1;
    //float d = fabsf(m23[0] * d31[1] - m23[1] * d31[0]);

    if (level > PATH_RECURSION_LIMIT) {
        contour.push_back(m123);
        contour.push_back(p3);
        return;
    }

    float dotp = glm::dot(glm::normalize(p1 - p2), glm::normalize(p3 - p2));
    if (fabs(dotp) > BEZIER_FLATNESS) {
        contour.push_back(m123);
        contour.push_back(p3);
        return;
    }


    appendQuadraticCurveTo(contour, p1, m12, m123, level+1);
    appendQuadraticCurveTo(contour, m123, m23, p3, level+1);
}

void appendCubicCurveTo(std::vector<vec2>& contour, const vec2& p1, const vec2& c1, const vec2& c2, const vec2& p2, int segments)
{
    // Algorithm from: http://www.antigrain.com/research/bezier_interpolation/index.html#PAGE_BEZIER_INTERPOLATION
    float subdiv_step = 1.0f / segments;
    float subdiv_step2 = subdiv_step * subdiv_step;
    float subdiv_step3 = subdiv_step * subdiv_step * subdiv_step;

    float pre1 = 3 * subdiv_step;
    float pre2 = 3 * subdiv_step2;
    float pre4 = 6 * subdiv_step2;
    float pre5 = 6 * subdiv_step3;

    vec2 tmp1 = p1 - c1 * 2.0f + c2;
    vec2 tmp2 = (c1 - c2) * 3.0f - p1 + p2;

    vec2 f = p1;
    vec2 df = (c1 - p1) * pre1 + tmp1 * pre2 + tmp2 * subdiv_step3;
    vec2 ddf = tmp1 * pre4 + tmp2 * pre5;
    vec2 dddf = tmp2 * pre5;

    while (segments-- > 0) {
        f += df;
        df += ddf;
        ddf += dddf;
        contour.push_back(f);
    }
}


const Mesh* fillPath(Path* path)
{
    if (path->m_filledMesh) {
        return path->m_filledMesh;
    }

    TESSalloc talloc;

    memset(&talloc, 0, sizeof(talloc));
    talloc.memalloc = stdAlloc;
    talloc.memfree = stdFree;
    talloc.memfree = stdFree;

    TESStesselator* tess = tessNewTess(&talloc);
    if (!tess) {
        std::cerr << "Couldn't allocate tessellator" << std::endl;
        return 0;
    }

    std::vector<Contour> contours = path->contoursFromPath();

    for (size_t i=0; i<contours.size(); i++) {
        tessAddContour(tess, 2, contours[i].data(), (int)sizeof(vec2), (int)contours[i].size());
    }

    // Triangulate
    int success = tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, 3, 2, 0);
    if (success == 0) {
        std::cerr << "Failed to tessellate path" << std::endl;
        return 0;
    }

    const unsigned int* pind = tessGetElements(tess);
    size_t nind = tessGetElementCount(tess) * 3;
    std::vector<unsigned int> indices(pind, pind + nind);

    const vec2* pvert = (const vec2*) tessGetVertices(tess);
    size_t nvert = tessGetVertexCount(tess);
    std::vector<vec2> vertices(pvert, pvert + nvert);

    tessDeleteTess(tess);


    path->m_filledMesh = new Mesh(vertices, indices);
    return path->m_filledMesh;

}

const Mesh* strokePath(Path* path, float strokeWidth)
{
    if (path->m_strokedMesh && path->m_cachedStrokeWidth == strokeWidth) {
        return path->m_strokedMesh;
    }

    delete path->m_strokedMesh;
    path->m_strokedMesh = new Mesh();
    path->m_cachedStrokeWidth = strokeWidth;

    std::vector<Contour> contours = path->contoursFromPath();
    size_t numverts = 0;
    for (size_t i=0; i<contours.size(); i++) {
        numverts += contours[i].size();
    }


    path->m_strokedMesh->vertices.reserve(numverts * 2);
    path->m_strokedMesh->indices.reserve((numverts-1) * 6);

    for (size_t c=0; c<contours.size(); c++) {
        const Contour& contour = contours[c];
        if (contour.size() < 2) {
            std::cerr << "Path contour has less than two points, not stroking it" << std::endl;
            continue;
        }

        path->m_strokedMesh->vertices.push_back(contour[0]);
        vec2& contourStart1 = path->m_strokedMesh->vertices.back();
        path->m_strokedMesh->vertices.push_back(contour[0]);
        vec2& contourStart2 = path->m_strokedMesh->vertices.back();

        for (size_t i=1; i<contour.size()-1; i++) {
            const vec2& p0 = contour[i-1];
            const vec2& p1 = contour[i];
            const vec2& p2 = contour[i+1];

            vec2 tangent1 = glm::normalize(p1 - p0);
            vec2 tangent2 = glm::normalize(p2 - p1);
            vec2 normal1(-tangent1[1], tangent2[0]);
            vec2 bisector = glm::normalize(0.5f * (tangent1 - tangent2));
            bisector *= strokeWidth / glm::dot(normal1, bisector);
            path->m_strokedMesh->vertices.push_back(p1 + bisector);
            path->m_strokedMesh->vertices.push_back(p1 - bisector);
        }

        // Fix up start of first segment (no bisector)
        vec2 tangent = glm::normalize(contour[1] - contour[0]) * strokeWidth;
        vec2 normal(-tangent[1], tangent[0]);
        contourStart1 += normal;
        contourStart2 -= normal;

        // Add end of last segment (no bisector)
        tangent = glm::normalize(contour.back() - contour[contour.size()-2]) * strokeWidth;
        normal[0] = -tangent[1];
        normal[1] = tangent[0];
        path->m_strokedMesh->vertices.push_back(contour.back() + normal);
        path->m_strokedMesh->vertices.push_back(contour.back() - normal);

        // Add indices
        for (size_t i=0; i<contour.size()-1; i++) {
            unsigned int j=(unsigned int)i*2;
            path->m_strokedMesh->indices.push_back(j);
            path->m_strokedMesh->indices.push_back(j+1);
            path->m_strokedMesh->indices.push_back(j+2);
            path->m_strokedMesh->indices.push_back(j+2);
            path->m_strokedMesh->indices.push_back(j+1);
            path->m_strokedMesh->indices.push_back(j+3);
        }
    }


    return path->m_strokedMesh;
}
