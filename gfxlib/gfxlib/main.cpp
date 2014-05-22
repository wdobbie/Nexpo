#include <iostream>
#include <lua.hpp>

extern "C" bool createCanvas(int width, int height, const char* title, bool fullscreen, bool vsync, int monitor, int bitsPerChannel);


int main(int argc, char** argv)
{
    std::cerr << "Starting Nexpo player!" << std::endl;

    if (argc > 10) {
        createCanvas(640, 480, "HI", false, true, -1, 8);
    }

    if (argc < 2) {
        std::cerr << "Usage: NexpoPlayer /path/to/file.lua" << std::endl;
        return -1;
    }

    const char* file = argv[1];

    lua_State* L = luaL_newstate();
    if (L == 0) {
        std::cerr << "Error: failed to create lua state" << std::endl;
    }

    luaL_openlibs(L);

    std::string cmd = "xpcall(dofile, function(e) print('Error!\\n', debug.traceback(e)) end, \"";
    cmd += file;
    cmd += "\")";

    std::cerr << cmd << std::endl;

    if (luaL_dostring(L, cmd.data()) != 0) {
        std::cerr << "Error executing " << file << std::endl;
    }

    lua_close(L);

    return 0;
}

