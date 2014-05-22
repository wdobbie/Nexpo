#include <iostream>
#include <string>
#include <lua.hpp>

// for chdir
#ifdef _MSC_VER_
#include <direct.h>
#else
#include <unistd.h>
#endif


static std::string dirFromPath(const std::string& path) {
    for (int i=path.size(); i>0; i--) {
        if (path[i] == '/' || path[i] == '\\') {
            return path.substr(0, i);   // don't include a trailing slash
        }
    }
    return "./";
}

static std::string fileFromPath(const std::string& path) {
    for (int i=path.size()-1; i>=0; i--) {
        if (path[i] == '/' || path[i] == '\\') {
            return path.substr(i+1, path.size()-i+1);
        }
    }
    return path;
}

#ifdef __APPLE__
#include <mach-o/dyld.h>
static std::string getBinaryDirectory() {
    char binaryPath[1024];
    uint32_t bufsize = sizeof(binaryPath);
    if (_NSGetExecutablePath(binaryPath, &bufsize) != 0) {
        std::cerr << "Failed to get executable path" << std::endl;
    }
    std::string path = dirFromPath(binaryPath);

    std::string chomp = "/NexpoPlayer.app/Contents/MacOS";

    if (path.size() > chomp.size() &&
        path.compare(path.size() - chomp.size(), chomp.size(), chomp) == 0)
    {
        path = path.substr(0, path.size() - chomp.size());
    }

    return path;
}
#endif

const char* cmd = "\"\n"
    "package.path = package.path .. ';' .. nexpoPath .. '?.lua'\n"
    "io.stdout:setvbuf('no')\n"
    "io.stderr:setvbuf('no')\n"
    "local libPath=nexpoPath\n"
    "if jit.os == 'Win' then\n"
    "  libPath = libPath .. 'gfxlib.dll'\n"
    "elseif jit.os == 'OSX' then\n"
    "  libPath = libPath .. 'libgfxlib.dylib'\n"
    "end\n"
    "if not pcall(require('ffi').load, libPath) then io.stderr:write('Unable to load ' .. libPath) end"
;


static lua_State* L;


bool init()
{
    std::string nexpoPath = getBinaryDirectory() + "/lib";

    L = luaL_newstate();
    if (L == 0) {
        std::cerr << "Error: failed to create lua state" << std::endl;
        return false;
    }

    luaL_openlibs(L);

    std::string runstring = "local nexpoPath=\""+(nexpoPath+"/"+cmd);

    if (luaL_dostring(L, runstring.c_str()) != 0) {
        std::cerr << "Error executing preloader command" << std::endl;
        lua_close(L);
        return false;
    }

    return true;
}

void run(const std::string& scriptPath)
{
    // Set working directory to script's directory
    chdir(dirFromPath(scriptPath).c_str());

    std::string runCmd = "xpcall(dofile, function(e) io.stderr:write(debug.traceback(e)) end, \""
            + fileFromPath(scriptPath)
            + "\")";

    if (luaL_dostring(L, runCmd.c_str()) != 0) {
        std::cerr << "Error executing script call" << std::endl;
    }

    lua_close(L);
}

int main(int argc, char** argv)
{
    if (!init()) return -1;

    std::string scriptPath;
    if (argc > 1) {
        scriptPath = argv[1];
    } else {
        std::getline(std::cin, scriptPath);
    }

    run(scriptPath);

    return 0;
}

