#ifndef SCFG_UTILS
#define SCFG_UTILS

#include <bits/stdc++.h>
#include <format>
#include <lua.hpp>
using namespace std;
namespace fs = filesystem;

string getCurrentTimeString()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm *localTime = std::localtime(&in_time_t);
    std::ostringstream oss;
    oss << '['
        << std::setw(2) << std::setfill('0') << localTime->tm_hour << ':'
        << std::setw(2) << std::setfill('0') << localTime->tm_min << ':'
        << std::setw(2) << std::setfill('0') << localTime->tm_sec << ']';
    return oss.str();
}
void log(const string &s)
{
    cerr << getCurrentTimeString() << " " << s << endl;
}

// 定义
const double sensitivity = 2.2;
const double m_yaw = 0.022;
const double m_pitch = 0.022;
const int TICKRATE = 64;
const size_t LENLIMIT = 128;
const size_t LINELIMIT = 300;
const double max_yaw_speed = 300;
const double max_pitch_speed = 45;
const string ticker = "hzSche_t";

lua_State* luaL;

string genList(const string &name, const vector<string> &ls)
{
    if (ls.empty())
        return format("alias {} \"\"", name);
    if (ls.size() == 1u)
        return format("alias {} {}", name, ls[0]);
    string res = "";
    for (const auto &s : ls)
    {
        if (!res.empty())
            res += ";";
        res += s;
    }
    return format("alias {} \"{}\"", name, res);
}
size_t getListLen(const vector<string> &ls)
{
    size_t ret = 0;
    for (const auto &s : ls)
    {
        ret += s.size();
    }
    return ret;
}

// 事件列表
class eventList
{
    map<int, vector<string>> ls;
    int curT;
    fs::path execpath;
    bool execpath_set;

    class Gen
    {
        int N;
        int linecnt = 0;
        ofstream fout;
        fs::path root, exec;
        void newpage()
        {
            N++;
            linecnt = 0;
            string nxtpage = format("cmd_{}.cfg", N);
            if (N > 1)
                fout << format("exec {}", (exec / nxtpage).string());
            fout = ofstream((root / nxtpage).string(), ios::out);
        }
        void remove_cmd_files(const fs::path &root)
        {
            if (!fs::exists(root) || !fs::is_directory(root))
                return;

            std::regex pattern(R"(cmd_\d+\.cfg)");

            for (const auto &entry : fs::directory_iterator(root))
            {
                if (fs::is_regular_file(entry.status()))
                {
                    const std::string filename = entry.path().filename().string();
                    if (std::regex_match(filename, pattern))
                    {
                        fs::remove(entry.path());
                    }
                }
            }
        }

    public:
        void init(fs::path workspace, fs::path execpath)
        {
            root = workspace;
            exec = execpath;
            remove_cmd_files(root);
            fs::create_directories(root);
            log("workspace: " + root.string());
            N = 0;
            newpage();
        }
        void append(const string &s)
        {
            linecnt++;
            fout << s << endl;
            if (linecnt >= LINELIMIT)
                newpage();
        }
    } gen;
    class AliasChain
    {
        const string seq_pre = "hzSche_seq_";
        string cur;
        int idN;
        Gen *gen;

    public:
        void newalias(bool secon = true)
        {
            idN++;
            if (idN > 1)
            {
                if (secon)
                    cur += ";";
                cur += format("{}{}\"", seq_pre, idN);
                gen->append(cur);
            }
            cur = format("alias {}{} \"", seq_pre, idN);
        }
        void init(Gen *ptr)
        {
            gen = ptr;
            idN = 0;
            cur = "";
            newalias();
        }
        void append(const string &s)
        {
            string apd = "";
            if (cur.back() != '"')
                apd += ";";
            apd += s;
            if (cur.size() + apd.size() + 1 > LENLIMIT)
            {
                newalias();
                cur += s;
            }
            else
            {
                cur += apd;
            }
        }
        void sleep(int tick)
        {
            if (cur.back() != '"')
                cur += ";";
            for (int i = 1; i <= tick; i++)
            {
                cur += format("alias {} ", ticker);
                if (cur.size() >= LENLIMIT)
                    newalias(false);
            }
            if (cur.back() != '"')
                newalias(false);
        }
        void end()
        {
            cur += '"';
            gen->append(cur);
        }
    } aliaschain;

public:
    eventList() : execpath("tmpdir"), execpath_set(false) {}
    void setExecPath(const string &s)
    {
        execpath_set = 1;
        execpath = fs::path(s);
    }
    void sleep(int tick)
    {
        curT += tick;
    }
    void pushEvent(const string &s)
    {
        ls[curT].push_back(s);
    }
    int time()
    {
        return curT;
    }
    bool getExecPath_set() const
    {
        return execpath_set;
    }
    void generate(fs::path workspace, lua_State *L)
    {
        if (!execpath_set)
            lua_warning(L, "exec path has not been set! Are you sure?", 0);
        gen.init(workspace, execpath);
        const string pkg_pre = "hzSche_pkg_";
        int idN = 0;

        //add hz_stop
        if(ls.empty()) ls[0].push_back("hzSche_stop_t");
        else    ls[ls.rbegin()->first + 1].push_back("hzSche_stop_t");
        // split command blocks into ids
        for (auto &[t, cmdls] : ls)
        {
            vector<string> idls;
            vector<string> cache;
            for (auto &s : cmdls)
            {
                if (getListLen(cache) + s.size() > LENLIMIT - pkg_pre.size() - 9 && getListLen(cache) > 0)
                {
                    idN++;
                    const auto &cmd = genList(pkg_pre + to_string(idN), cache);
                    cache.clear();
                    gen.append(cmd);
                    idls.push_back(format("{}{}", pkg_pre, idN));
                }
                cache.push_back(s);
            }
            if (!cache.empty())
            {
                idN++;
                const auto &cmd = genList(pkg_pre + to_string(idN), cache);
                gen.append(cmd);
                idls.push_back(format("{}{}", pkg_pre, idN));
            }
            cmdls = idls;
        }

        // gen real sequence
        aliaschain.init(&gen);
        for (auto it = ls.begin(); it != ls.end(); it++)
        {
            for (auto &s : it->second)
            {
                aliaschain.append(s);
            }
            if (next(it) != ls.end())
            {
                aliaschain.sleep(next(it)->first - it->first);
            }
        }
        aliaschain.end();
    }
} event;

#endif