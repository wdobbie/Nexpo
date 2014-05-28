#include "common.h"
#include <GLFW/glfw3.h>
#include "canvas.h"
#include "path.h"
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <stdio.h>      // fileno()
#include <unistd.h>     // close()

// MSVC doesn't define M_PI unless you do this
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <assert.h>

#include <iostream>


static std::vector<const char*>* gExportSignatures;

void addExport(const char* signature) {
    if (gExportSignatures == 0) {
        gExportSignatures = new std::vector<const char*>;
    }
    gExportSignatures->push_back(signature);
}


/*
 * These next two functions are the only ones that need to have their signatures manually
 * added to lua via ffi.cdef().
 * They enable the rest of the API signatures to be retrieved as strings.
 *
 */
DLLEXPORT unsigned int getNumExports() {
    return (unsigned int)gExportSignatures->size();
}

DLLEXPORT const char* getExportSignature(unsigned int i) {
    if (i > gExportSignatures->size()) {
        std::cerr << "Error: requested export index exceeds maximum" << std::endl;
        return 0;
    }

    return gExportSignatures->at(i);
}



class Canvas
{
public:
    Canvas();
    ~Canvas();

    GLFWwindow* m_window;
    GLFWmonitor* m_monitor;

    vec4 m_clearColor;
    bool m_wireframe;

    std::queue<std::string> m_inputQueue;
    std::thread m_inputThread;
    std::mutex m_inputQueueMutex;

    void (*m_mouseButtonCallback)(int, int, int);
    void (*m_scrollCallback)(double, double);
    void (*m_charCallback)(unsigned int);
    //void (*m_cursorEnterCallback)(int);
    void (*m_cursorPosCallback)(double, double);
    void (*m_keyCallback)(int, int, int, int);

};

static Canvas g_canvas;

static void inputThreadRoutine()
{
    std::string line;
    while(std::getline(std::cin, line)) {
        std::lock_guard<std::mutex> lock(g_canvas.m_inputQueueMutex);
        g_canvas.m_inputQueue.push(std::move(line));
    }
}



Canvas::Canvas()
    : m_window(0)
    , m_monitor(0)
    , m_clearColor(0, 0, 0, 0)
    , m_wireframe(false)
{
    assert(glfwInit());
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 16);
    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwWindowHint(GLFW_REFRESH_RATE, 0);       // highest available, full screen only
}

Canvas::~Canvas()
{
    glfwTerminate();
}

static void drawMesh(const Mesh* mesh) {
    if (!mesh) return;

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, &mesh->vertices[0]);

    glDrawElements(g_canvas.m_wireframe ? GL_LINES : GL_TRIANGLES,
                   (GLsizei)mesh->indices.size(),
                   GL_UNSIGNED_INT,
                   &mesh->indices[0]);

}

LUAEXPORT(bool getNextInputLine(char* dest, int bufsize))
{
    std::lock_guard<std::mutex> lock(g_canvas.m_inputQueueMutex);
    if (g_canvas.m_inputQueue.empty()) return false;
    std::string& line = g_canvas.m_inputQueue.back();
    dest[line.copy(dest, bufsize-1)] = 0;
    g_canvas.m_inputQueue.pop();
    return true;
}

LUAEXPORT(void setWindowHint(int target, int hint))
{
    glfwWindowHint(target, hint);
}

LUAEXPORT(bool createCanvas(int width, int height, const char* title, bool fullscreen, bool vsync, int monitor, int bitsPerChannel))
{
    // Convert monitor parameter to 0-based index
    monitor--;

    // Get requested monitor if user has specified full screen
    if (fullscreen)
    {
        if (monitor < 0) {
            g_canvas.m_monitor = glfwGetPrimaryMonitor();
        } else {
            int nmonitors;
            GLFWmonitor** monitors = glfwGetMonitors(&nmonitors);
            if (monitors == 0) {
                std::cerr << "Failed to get list of monitors" << std::endl;
                return false;
            }

            if (monitor >= nmonitors) {
                std::cerr << "Warning: requested monitor exceeds number of monitors. Using primary monitor." << std::endl;
                g_canvas.m_monitor = glfwGetPrimaryMonitor();
            } else {
                g_canvas.m_monitor = monitors[monitor];
            }
        }

        if (width < 1 || height < 1) {
            const GLFWvidmode* mode = glfwGetVideoMode(g_canvas.m_monitor);
            width = mode->width;
            height = mode->height;
        }

        if (bitsPerChannel == 0) {
            const GLFWvidmode* mode = glfwGetVideoMode(g_canvas.m_monitor);
            bitsPerChannel = mode->redBits;
        }
    }


    g_canvas.m_window = glfwCreateWindow(
                width,
                height,
                title,
                g_canvas.m_monitor,
                NULL);

    if (!g_canvas.m_window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(g_canvas.m_window);

    glfwSwapInterval(vsync ? 1 : 0);

    // Start input thread
    g_canvas.m_inputThread = std::thread(inputThreadRoutine);
    g_canvas.m_inputThread.detach();     // let the OS terminate it with the process

    return true;
}

LUAEXPORT(int getMonitorByName(const char* name))
{
    int n;
    GLFWmonitor** monitors = glfwGetMonitors(&n);
    if (!monitors) return -1;
    for (int i=0; i<n; i++) {
        if (strcmp(name, glfwGetMonitorName(monitors[i])) == 0) {
            return i+1;     // Start indices at 1
        }
    }
    return -1;
}

LUAEXPORT(void setWindowPos(int x, int y))
{
    glfwSetWindowPos(g_canvas.m_window, x, y);
}


LUAEXPORT(const char* getMonitorName())
{
    if (g_canvas.m_monitor) {
        return glfwGetMonitorName(g_canvas.m_monitor);
    } else {
        return 0;
    }
}

LUAEXPORT(void destroyCanvas())
{
    glfwDestroyWindow(g_canvas.m_window);
}

LUAEXPORT(void setGamma(float gamma))
{
    if (g_canvas.m_monitor) {
        glfwSetGamma(g_canvas.m_monitor, gamma);
    }
}

LUAEXPORT(void setGammaRamp(unsigned short* rgb, int n))
{
    if (g_canvas.m_monitor) {
        GLFWgammaramp ramp;
        ramp.red = &rgb[0];
        ramp.green = &rgb[n];
        ramp.blue = &rgb[n+n];
        ramp.size = n;
        glfwSetGammaRamp(g_canvas.m_monitor, &ramp);
    }
}

LUAEXPORT(void swapBuffers())
{
    glfwSwapBuffers(g_canvas.m_window);

    int width, height;
    glfwGetWindowSize(g_canvas.m_window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(g_canvas.m_clearColor[0],
            g_canvas.m_clearColor[1],
            g_canvas.m_clearColor[2],
            g_canvas.m_clearColor[3]);

    glClear(GL_COLOR_BUFFER_BIT);
}

LUAEXPORT(bool shouldClose())
{
    return glfwWindowShouldClose(g_canvas.m_window) == 1;
}

LUAEXPORT(void setWindowShouldClose(bool close))
{
    glfwSetWindowShouldClose(g_canvas.m_window, close ? 1 : 0);
}

LUAEXPORT(void setClearColor(float r, float g, float b, float a))
{
    g_canvas.m_clearColor[0] = r;
    g_canvas.m_clearColor[1] = g;
    g_canvas.m_clearColor[2] = b;
    g_canvas.m_clearColor[3] = a;

}


LUAEXPORT(void windowSize(int* dest))
{
    glfwGetWindowSize(g_canvas.m_window, &dest[0], &dest[1]);
}

LUAEXPORT(void cursorPos(double* dest))
{
    glfwGetCursorPos(g_canvas.m_window, &dest[0], &dest[1]);

    // Make center be (0, 0), increasing up and right
    int wsize[2];
    windowSize(wsize);
    dest[0] = dest[0] - wsize[0]/2;
    dest[1] = wsize[1]/2 - dest[1];
}

LUAEXPORT(bool mouseButton(int btn))
{
    return glfwGetMouseButton(g_canvas.m_window, btn) == GLFW_PRESS;
}

LUAEXPORT(double canvasTime())
{
    return glfwGetTime();
}

LUAEXPORT(void setVerticalSync(bool enabled)) {
    glfwSwapInterval(enabled ? 1 : 0);
}

LUAEXPORT(void drawFilledPath(Path* path))
{
    drawMesh(fillPath(path));
}

LUAEXPORT(void drawStrokedPath(Path* path, float strokeWidth))
{
    drawMesh(strokePath(path, strokeWidth));
}

LUAEXPORT(void drawFilledSquare())
{
    float x1 = -.5f;
    float y1 = -.5f;
    float x2 =  .5f;
    float y2 =  .5f;

    std::vector<vec2> vert;
    vert.resize(4);
    vert[0][0] = x1;
    vert[0][1] = y1;
    vert[1][0] = x1;
    vert[1][1] = y2;
    vert[2][0] = x2;
    vert[2][1] = y1;
    vert[3][0] = x2;
    vert[3][1] = y2;

    std::vector<unsigned int> ind;
    ind.resize(6);
    ind[0] = 0;
    ind[1] = 1;
    ind[2] = 2;
    ind[3] = 1;
    ind[4] = 3;
    ind[5] = 2;

    Mesh m(vert, ind);
    drawMesh(&m);
}


LUAEXPORT(void drawStrokedRect())
{

}

// cached mesh of circle with diameter 1 and centred at origin
static Mesh* gUnitCircleMesh;

static void generateCircleMesh()
{
    const int nsteps = 128;

    std::vector<vec2> v;
    std::vector<unsigned int> ind;
    v.resize(nsteps+1);
    ind.resize(nsteps*3);

    v[nsteps][0] = 0;
    v[nsteps][1] = 0;

    for (int i=0; i<nsteps; i++) {
        float a = (float)M_PI * 2.0f * i / (float) nsteps;
        v[i][0] = 0.5f * cosf(a);
        v[i][1] = 0.5f * sinf(a);

        int j = i*3;
        ind[j] = i;
        ind[j+1] = (i + 1) % nsteps;
        ind[j+2] = nsteps;
    }

    gUnitCircleMesh = new Mesh(v, ind);
}


LUAEXPORT(void drawFilledCircle())
{
    if (gUnitCircleMesh == 0) {
        generateCircleMesh();
    }

    drawMesh(gUnitCircleMesh);
}

LUAEXPORT(void drawStrokedCircle())
{

}

static void mouseButtonCallback(GLFWwindow*, int button, int action, int mods)
{
    if (g_canvas.m_mouseButtonCallback) {
        g_canvas.m_mouseButtonCallback(button, action, mods);
    }
}

LUAEXPORT(void setMouseButtonCallback(void(*cb)(int, int, int)))
{
    g_canvas.m_mouseButtonCallback = cb;
    glfwSetMouseButtonCallback(g_canvas.m_window, mouseButtonCallback);
}

static void scrollCallback(GLFWwindow*, double xoffset, double yoffset)
{
    if (g_canvas.m_scrollCallback) {
        g_canvas.m_scrollCallback(xoffset, yoffset);
    }
}

LUAEXPORT(void setScrollCallback(void (*cb)(double, double)))
{
    g_canvas.m_scrollCallback = cb;
    glfwSetScrollCallback(g_canvas.m_window, scrollCallback);
}

static void charCallback(GLFWwindow*, unsigned int codepoint)
{
    if (g_canvas.m_charCallback) {
        g_canvas.m_charCallback(codepoint);
    }
}

LUAEXPORT(void setCharCallback(void (*cb)(unsigned int)))
{
    g_canvas.m_charCallback = cb;
    glfwSetCharCallback(g_canvas.m_window, charCallback);
}

static void cursorPosCallback(GLFWwindow*, double x, double y)
{
    if (g_canvas.m_cursorPosCallback) {
        g_canvas.m_cursorPosCallback(x, y);
    }
}

LUAEXPORT(void setCursorPosCallback(void (*cb)(double, double)))
{
    g_canvas.m_cursorPosCallback = cb;
    glfwSetCursorPosCallback(g_canvas.m_window, cursorPosCallback);
}

static void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods)
{
    if (g_canvas.m_keyCallback) {
        g_canvas.m_keyCallback(key, scancode, action, mods);
    }
}

LUAEXPORT(void setKeyCallback(void (*cb)(int, int, int, int)))
{
    g_canvas.m_keyCallback = cb;
    glfwSetKeyCallback(g_canvas.m_window, keyCallback);
}

LUAEXPORT(void setWireFrame(bool e)) {
    g_canvas.m_wireframe = e;
}

LUAEXPORT(void pollEvents())
{
    glfwPollEvents();
}
