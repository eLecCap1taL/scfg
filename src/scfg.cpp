#include "cmd/baseCommand.hpp"
#include "cmd/sleep/sleepCommand.hpp"
#include "cmd/src/srcCommand.hpp"
#include "cmd/setExecPath/setExecPathCommand.hpp"
#include "cmd/jump/jumpCommand.hpp"
#include "cmd/jumpbug/jumpbugCommand.hpp"
#include "cmd/angle/angle.hpp"
#include "cmd/baseInstantCommand.hpp"
#include "cmd/boost/boostCommand.hpp"
#include "cmd/misc/miscCommand.hpp"
#include "utils.hpp"
#include "include/Global.h"

int lcfg_mode = 0;
bool smart_convert=true

void registerLuaFunctions(lua_State *L)
{
    registerLuaBaseFunctions(L);
    registerLuaSleepFunction(L);
    registerLuaSrcFunction(L);
    registerLuaSetExecPathFunction(L);
    registerLuaJumpFunction(L);
    registerLuaJumpbugFunction(L);
    registerLuaAngleFunction(L);
    registerLuaBaseInstantFunctions(L);
    registerLuaBoostFunction(L);
    registerLuaMiscFunction(L);
}

void warning_func(void *ud, const char *msg, int tocont)
{
    if (!tocont)
        std::cerr << "[Lua warning] ";
    std::cerr << msg;
    if (!tocont)
        std::cerr << std::endl;
}

int main(int argc, char *argv[])
{


bool smart_convert = true;
const char *scriptPath = nullptr;

for (int i = 1; i < argc; ++i)
{
    string arg = argv[i];

    if (arg == "-unsmart")
    {
        smart_convert = false;
    }
    else if (arg == "-i")
    {
        if (i + 1 >= argc)
        {
            cerr << "Error: -i requires a <script> argument." << endl;
            return 1;
        }
        scriptPath = argv[++i];
    }
    else if (arg == "-luotiany1" || arg == "-lty")
    {
        lcfg_mode = 1;
    }
    else if (arg == "-cap1tal" || arg == "-cap")
    {
        lcfg_mode = 0;
    }
    else
    {
        cerr << "Unknown argument: " << arg << endl;
        return 1;
    }
}

if (!scriptPath)
{
    cerr << "Usage: " << argv[0] << " -i <script> [-lty|-luotiany1|-cap|-cap1tal] [-unsmart]" << endl;
    return 1;
}

log("scfg is running. mode = " + std::to_string(lcfg_mode));


    if (lcfg_mode == 1) {
        ticker = "sq_sf";
    } else {
        ticker = "hzSche_t";
    }

    // 创建新的 Lua 状态
    lua_State *L = luaL_newstate();
    luaL = L;
    lua_setwarnf(L, warning_func, nullptr);

    if (L == nullptr)
    {
        cerr << "Failed to create Lua state." << endl;
        return 1;
    }

    luaL_openlibs(L); // 打开标准库
    registerLuaFunctions(L); // 注册 C++ 到 Lua 的函数

    if (luaL_dofile(L, scriptPath) != LUA_OK)
    {
        cerr << "Error executing script: " << lua_tostring(L, -1) << endl;
        lua_pop(L, 1);
        lua_close(L);
        return 1;
    }

    log(format("ok process scfg file. end with current tick={}.", event.time()));
    event.generate(fs::current_path() / fs::path(scriptPath).stem(), L);
    lua_close(L);
    return 0;
}
