#include <bits/stdc++.h>
#define fi first
#include <format>
#define se second
using namespace std;

// var
string path;
double sen = 2.2;
double m_yaw = 0.022;
double m_pitch = 0.022;
double CVARSLEEP = 0;
double tickrate = 64;
double ticktime = 1 / tickrate;
double yawspeed = 300;
double pitchspeed = 45;
double usedtime = 0;
int N = 0;
int lineid;

filesystem::path tempdir("Temp");

ofstream fout;
ifstream fin;

pair<double, double> lst(-1, -1);

double tottime = 0;

const int keywordmaxlen = 20;
unordered_set<string> keyword;
unordered_set<string> keywordbylen[keywordmaxlen + 1];
unordered_map<string, int> cmdstatus;
double yawstatus;

bool fpslocked = 0;

// Utilities
void deletefile()
{
    fin.close();
    fout.close();
    filesystem::remove_all(tempdir);
}
void genfailed()
{
    deletefile();
    exit(EXIT_FAILURE);
}
string UnionWithSpace(const vector<string> &ls)
{
    if (ls.size() == 0)
        return "";
    string ret = ls[0];
    for (int i = 1; i < ls.size(); i++)
    {
        ret += ' ';
        ret += ls[i];
    }
    return ret;
}
string toUpper(const string &s)
{
    auto ret = s;
    for (int i = 0; i < s.size(); i++)
    {
        if ('a' <= s[i] && s[i] <= 'z')
        {
            ret[i] += 'A' - 'a';
        }
    }
    return ret;
}
string toLower(const string &s)
{
    auto ret = s;
    for (int i = 0; i < s.size(); i++)
    {
        if ('A' <= s[i] && s[i] <= 'Z')
        {
            ret[i] += 'a' - 'A';
        }
    }
    return ret;
}
template <typename T>
vector<T> popfront(const vector<T> &ls, int N = 1)
{
    auto ret = ls;
    ret.erase(ret.begin(), ret.begin() + N);
    return ret;
}

void copyfile()
{
    filesystem::path program_dir = filesystem::current_path();
    filesystem::path temp_dir = program_dir / "Temp";

    if (!filesystem::exists(temp_dir) || !filesystem::is_directory(temp_dir))
    {
        cerr << "Temp directory does not exist or is not a directory." << endl;
        genfailed();
    }

    for (const auto &entry : filesystem::directory_iterator(temp_dir))
    {
        if (filesystem::is_regular_file(entry))
        {
            filesystem::path target_path = program_dir / entry.path().filename();
            if (filesystem::exists(target_path))
            {
                filesystem::remove(target_path);
            }
            filesystem::copy(entry, target_path, filesystem::copy_options::overwrite_existing);
        }
    }
}
void RequireArg(string opt, int num, int of, bool overwar = 0)
{
    if (of == num)
        return;
    if (of > num)
    {
        if (overwar)
        {
            cerr << format("At line {}:\n---- warring: too many arguments to function '{}'.\n", lineid, opt);
            cerr << format("---- '{}' requires {} argument{}, but there {} {}...... ignoring from {}!\n", opt, num, num <= 1 ? "" : "", of == 1 ? "is" : "are", of, num + 1);
        }
        return;
    }
    cerr << format("At line {}:\n---- error: too few arguments to function '{}'.\n", lineid, opt);
    cerr << format("---- '{}' requires {} argument{}, but there {} only {}.\n", opt, num, num <= 1 ? "" : "", of == 1 ? "is" : "are", of);
    genfailed();
}
void isValidNumber(const string &opt, const string &s)
{
    try
    {
        size_t pos;
        if (fabs(stod(s, &pos)) > 999999984306749440.000000)
            throw out_of_range("Value is out of range!");
        if (pos != s.size())
        {
            cerr << format("At line {}:\n---- error: function '{}' requires a number argument, but '{}' is provided.\n", lineid, opt, s);
            genfailed();
        }
        return;
    }
    catch (invalid_argument)
    {
        cerr << format("At line {}:\n---- error: function '{}' requires a number argument, but '{}' is provided.\n", lineid, opt, s);
        genfailed();
    }
    catch (out_of_range)
    {
        cerr << format("At line {}:\n---- error: argument {} overflowed. it shouldn't be bigger than 999999984306749440.000000\n", lineid, s);
        genfailed();
    }
}
string ensure_extension(const string &filename)
{
    filesystem::path file_path(filename);
    if (file_path.extension().empty())
    {
        return filename + ".cs2script";
    }
    return filename;
}
void genInit()
{
    fout = ofstream("Temp/run.cfg", ios::out);
    fout << "//[Gen]\n";
    fout << format("alias hzScheduler_launchpath exec {}/0\n", path);
    fout << format("hzScheduler_launch\n", path);
}
void tryReplaceTick(string &s)
{
    if (s.size() <= 6)
        return;
    if (s.substr(0, 5) != "TICK(")
        return;
    if (s[s.size() - 1] != ')')
        return;
    double ticknum = 0;
    try
    {
        size_t pos;
        ticknum = fabs(stod(s.substr(5, s.size() - 6), &pos));
        if (ticknum > 999999984306749440.000000)
            return;
        if (pos != s.size() - 6)
        {
            return;
        }
    }
    catch (invalid_argument)
    {
        return;
    }
    catch (out_of_range)
    {
        return;
    }
    s = to_string(ticknum / tickrate);
}

vector<string> readargs()
{
    vector<string> ret(0);
    char c;
    string s = "";
    while (1)
    {
        c = fin.get();
        if (c == '\r')
            continue;
        if (c == EOF)
        {
            if (s != "")
                ret.push_back(s);
            return ret;
        }
        if (c == '\n')
        {
            if (s != "")
                ret.push_back(s);
            if (ret.size() == 0)
            {
                lineid++;
                continue;
            }
            return ret;
        }
        if (c == ' ')
        {
            if (s != "")
            {
                ret.push_back(s);
                s = "";
            }
            continue;
        }
        s += c;
    }
}

// basic commands
void slp(double x, bool strict = 0)
{
    double L = floor(x * tickrate) / tickrate;
    double R = ceil(x * tickrate) / tickrate;
    if (fabs(x - L) < fabs(x - R))
    {
        x = L;
    }
    else
    {
        x = R;
    }
    // x-=0.5/tickrate;
    // x=max(x,0.0);
    tottime += x;
    usedtime += x;
    fout << endl;
    if (strict)
    {
        fout << format("alias syncerReg_delay \"incrementvar tv_window_size 0 99999999999999 {:.6f}\"\n", x);
    }
    else
    {
        fout << format("alias syncerReg_delay \"move_begintime_to_yawsen;incrementvar tv_window_size 0 99999999999999 {:.6f}\"\n", tottime);
    }
    fout << format("alias syncer_callback \"exec {}/{}\"\n", path, ++N);
    fout << "syncer_schedule";
    fout = ofstream(format("Temp/{}.cfg", N), ios::out);
    fout << "//[generated with scfg]\n\n";
}

void ang(double x, double y, bool needstd = 1)
{
    fout << format("yaw {:.6f} 1 1\npitch {:.6f} 1 1\n",
                   (double)(lst.fi - x) / (sen * m_yaw),
                   (double)(y - lst.se) / (sen * m_pitch));
    if (needstd)
        fout << "sensitivity 2.2;m_yaw 0.022;m_pitch 0.022\n";

    lst = {x, y};

    slp(CVARSLEEP);
    fout << "hzCVAR_mouse\n";
}

void src(string s)
{
    fout << s << endl;
}

void setang(double x, double y)
{
    fout << "yaw 99999999999999999 1 1;\npitch 9999999999999999 1 1;\nsensitivity 2.2;m_yaw 0.022;m_pitch 0.022\n";
    lst = {0, 0};
    slp(CVARSLEEP);
    ang(x, y, 0);
}

void turntoang(double usetime, double x, double y)
{
    double dx = x - lst.fi;
    double dy = y - lst.se;

    if (x > lst.fi)
    {
        double res = x - 180 + (-180 - lst.fi);
        if (fabs(res) < fabs(dx))
        {
            dx = res;
        }
    }
    else
    {
        double res = x - (-180) + (180 - lst.fi);
        if (fabs(res) < fabs(dx))
        {
            dx = res;
        }
    }

    // cerr<<dx<<' '<<dy<<endl;

    lst = {x, y};

    string sx = "";
    string sy = "";

    if (dx > 0)
    {
        sx = "turnleft";
    }
    else
    {
        sx = "turnright";
    }

    if (dy > 0)
    {
        sy = "turndown";
    }
    else
    {
        sy = "turnup";
    }

    double yawspd_need = fabs(dx) / usetime;
    double pitchspd_need = fabs(dy) / usetime;
    if (yawspd_need > yawspeed || pitchspd_need > pitchspeed)
    {
        cerr << format("At line {}:\n---- error: too fast angle move.\n", lineid);
        genfailed();
    }

    src(format("{} {:6f} 0 0", sx, yawspd_need / yawspeed));
    src(format("{} {:6f} 0 0", sy, pitchspd_need / pitchspeed));

    slp(usetime);

    src("-" + sx);
    src("-" + sy);
}
void checktime()
{
    src("move_begintime_to_yawsen");
    src(format("incrementvar tv_window_size 0 99999999999 {:.6f}", tottime));
    src("multvar tv_window_size -999999999 0 -1");
    src("incrementvar tv_window_size -99999999 0 -100000.0");

    src("syncer_fetch");
    src("syncer_curtime | toggle \"hzScheduler_cmd;tv_playcast_retry_timeout\"");
    src("incrementvar tv_playcast_retry_timeout -99999999 0 -100000.0");

    src("setinfo -99999999999 =");
    src("tv_window_size | toggle -99999999999");

    src("-99999999999 | incrementvar tv_playcast_retry_timeout");
    src("incrementvar tv_playcast_retry_timeout -99999999 99999999 200000.0");

    src("setinfo timingError =");
    src("tv_playcast_retry_timeout | toggle timingError");
    src("timingError | say");
}

void cvar(string cvarname, string value)
{
    src(cvarname + " " + value);
}

void play(string path)
{
    src("play " + path);
}

void setbegin()
{
    src("syncer_fetch;hzScheduler_begintime =;syncer_curtime | toggle hzScheduler_begintime");
}

int main(int argc, char *argv[])
{
    deletefile();
    if (!tempdir.empty() && !filesystem::exists(tempdir))
    {
        filesystem::create_directories(tempdir);
    }

    vector<string> argls(argv, argv + argc);

    if (argls.size() != 2)
    {
        cerr << "Error: Invalid number of arguments.\n";
        cerr << "Usage: scfg <input file>\n";
        cerr << "Example: scfg input.cs2script\n";
        genfailed();
    }

    argls[1] = ensure_extension(argls[1]);

    fin = ifstream(argls[1], ios::in);
    if (!fin)
    {
        cerr << format("file {} does not exists.\n", argls[1]);
        genfailed();
    }

    keyword.insert("SRC");
    keyword.insert("SLEEP");
    keyword.insert("SLEEPTICK");
    keyword.insert("SETANG");
    keyword.insert("SETFAKEANG");
    keyword.insert("ANG");
    keyword.insert("MOVEANG");
    keyword.insert("PLAY");
    keyword.insert("BEEP");
    keyword.insert("CVAR");
    keyword.insert("LOCKMOUSE");
    keyword.insert("UNLOCKMOUSE");
    keyword.insert("END");
    keyword.insert("SETBEGIN");
    keyword.insert("DEFINEPATH");
    keyword.insert("CHECKTIME");

    keyword.insert("FPS_LOW");
    keyword.insert("FPS_RESET");

    keyword.insert("+FORWARD");
    keyword.insert("+RIGHT");
    keyword.insert("+BACK");
    keyword.insert("+LEFT");
    keyword.insert("-FORWARD");
    keyword.insert("-RIGHT");
    keyword.insert("-BACK");
    keyword.insert("-LEFT");
    keyword.insert("+DUCK");
    keyword.insert("-DUCK");
    keyword.insert("+SPRINT");
    keyword.insert("-SPRINT");
    keyword.insert("+ATTACK");
    keyword.insert("-ATTACK");
    keyword.insert("+ATTACK2");
    keyword.insert("-ATTACK2");
    keyword.insert("+USE");
    keyword.insert("-USE");

    keyword.insert("DROP");
    keyword.insert("SLOT1");
    keyword.insert("SLOT2");
    keyword.insert("SLOT3");
    keyword.insert("SLOT4");
    keyword.insert("SLOT5");

    keyword.insert("NADE_HE");
    keyword.insert("NADE_SMOKE");
    keyword.insert("NADE_MOLO");
    keyword.insert("NADE_FLASH");
    keyword.insert("NADE_DECOY");

    keyword.insert("SAY_ALL");
    keyword.insert("SAY_TEAM");
    keyword.insert("SAY_CONSOLE");

    keyword.insert("JUMP");
    keyword.insert("JUMPBUG");
    keyword.insert("SPEED_LEFT");
    keyword.insert("SPEED_RIGHT");
    keyword.insert("SPEED_STOP");

    keyword.insert("WASD_CAN_CANCEL");

    for (auto &s : keyword)
    {
        keywordbylen[s.size()].insert(s);
    }

    bool use_ANG_after_MOVEANG = 0;
    int pathinit = 0;

    for (lineid = 1;; lineid++)
    {
        auto lineargs = readargs();
        if (lineargs.size() == 0)
        {
            cerr << format("At line {}:\n---- error: Missing 'END' flag in the end of file.\n", lineid);
            genfailed();
        }
        auto opt = lineargs[0];
        lineargs = popfront(lineargs);
        for (auto &s : lineargs)
            tryReplaceTick(s);

        if (keyword.find(opt) == keyword.end())
        {
            if (opt.length() >= 2 && opt.substr(0, 2) == "//")
                continue;
            cerr << format("At line {}:\n---- error: Command {} Not Found.\n", lineid, opt);
            for (int i = 1; i <= opt.size() && i <= keywordmaxlen; i++)
            {
                string trys = toUpper(opt.substr(0, i));
                auto it = keywordbylen[i].find(trys);
                if (it == keywordbylen[i].end())
                    continue;
                cerr << format("---- did you mean '{}'?", trys);
            }
            genfailed();
        }

        if (opt == "DEFINEPATH")
        {
            if (pathinit)
            {
                cerr << format("At line {}:\n---- error: Path has already been defined at line {}.\n", lineid, pathinit);
                genfailed();
            }
            pathinit = lineid;
            RequireArg(opt, 1, lineargs.size(), 1);
            path = lineargs[0];

            fout = ofstream("Temp/0.cfg", ios::out);
            fout << "//[Gen]\n\n";
            fout << "+syncer_nomsg\n";
            fout << "hzSchduler_taskbegin\n";
            fout << "syncer_fetch;hzScheduler_begintime =;syncer_curtime | toggle hzScheduler_begintime\n";
            fout << "alias hzScheduler_launch\n";
            slp(CVARSLEEP);
            continue;
        }
        if (!pathinit)
        {
            cerr << format("At line {}:\n---- error: Path hasn't been defined in script.\n", lineid);
            genfailed();
        }
        if (opt == "SRC")
        {
            RequireArg(opt, 1, lineargs.size(), 0);
            src(UnionWithSpace(lineargs));
        }
        else if (opt == "WASD_CAN_CANCEL")
        {
            RequireArg(opt, 0, lineargs.size(), 0);
            src("hzSchduler_canceltask_enable");
        }
        else if (opt == "SLEEP")
        {
            RequireArg(opt, 1, lineargs.size(), 1);
            isValidNumber(opt, lineargs[0]);
            double slptime = stod(lineargs[0]);
            if (slptime > 3600)
            {
                cerr << format("At line {}:\n---- warring: 'SLEEP {}' will pause over 1 hour. Are you sure?\n", lineid, slptime);
            }
            slp(slptime, 0);
        }
        else if (opt == "SLEEPTICK")
        {
            RequireArg(opt, 1, lineargs.size(), 1);
            isValidNumber(opt, lineargs[0]);
            double slptime = stod(lineargs[0]) / tickrate;
            // cerr << format("At line {}:\n---- warring: 'SLEEPTICK' will soon been no longer supported, use 'SLEEP TICK(ticknum) instead!'\n", lineid, slptime);
            if (slptime > 3600)
            {
                cerr << format("At line {}:\n---- warring: 'SLEEPTICK {}' will pause over 1 hour. Are you sure?\n", lineid, slptime);
            }
            slp(slptime, 0);
        }
        else if (opt == "SETANG")
        {
            use_ANG_after_MOVEANG = 0;
            RequireArg(opt, 7, lineargs.size(), 1);
            setang(stod(lineargs[5]), stod(lineargs[4]));
        }
        else if (opt == "SETFAKEANG")
        {
            RequireArg(opt, 7, lineargs.size(), 1);
            lst = {stod(lineargs[5]), stod(lineargs[4])};
        }
        else if (opt == "ANG")
        {
            if (use_ANG_after_MOVEANG)
            {
                cerr << format("At line {}:\n---- warring: You are using 'ANG' after 'MOVEANG', which is inaccurate. Are you sure?\n", lineid);
                cerr << format("---- Suggest using 'SETANG' at least once between 'MOVEANG' and 'ANG'\n", lineid);
                use_ANG_after_MOVEANG = 0;
            }
            RequireArg(opt, 7, lineargs.size(), 1);
            ang(stod(lineargs[5]), stod(lineargs[4]));
        }
        else if (opt == "MOVEANG")
        {
            use_ANG_after_MOVEANG = 1;
            RequireArg(opt, 8, lineargs.size(), 1);
            isValidNumber(opt, lineargs[0]);
            turntoang(stod(lineargs[0]), stod(lineargs[6]), stod(lineargs[5]));
        }
        else if (opt == "PLAY")
        {
            RequireArg(opt, 1, lineargs.size(), 0);
            play(UnionWithSpace(lineargs));
        }
        else if (opt == "BEEP")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            play("sounds/tools/sfm/beep.vsnd_c");
        }
        else if (opt == "CVAR")
        {
            RequireArg(opt, 2, lineargs.size(), 0);
            cvar(lineargs[0], UnionWithSpace(popfront(lineargs)));
        }
        else if (opt == "LOCKMOUSE")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            src("hzScheduler_mouse_lock");
        }
        else if (opt == "UNLOCKMOUSE")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            src("hzScheduler_mouse_pass");
        }
        else if (opt == "SETBEGIN")
        {
            tottime = 0;
            RequireArg(opt, 0, lineargs.size(), 1);
            setbegin();
        }
        else if (opt == "CHECKTIME")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            checktime();
        }
        else if (opt[0] == '+')
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            string cmd = opt.substr(1);
            if (cmdstatus[cmd] != 0)
            {
                cerr << format("At line {}:\n---- warring: You are using '+{}' without '-{}'. Are you sure?\n", lineid, cmd, cmd);
            }
            cmdstatus[cmd]++;
            src("+" + toLower(cmd));
        }
        else if (opt[0] == '-')
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            string cmd = opt.substr(1);
            if (cmdstatus[cmd] == 0)
            {
                cerr << format("At line {}:\n---- warring: You are using '-{}' without '+{}'. Are you sure?\n", lineid, cmd, cmd);
            }
            cmdstatus[cmd]--;
            cmdstatus[cmd] = max(cmdstatus[cmd], 0);
            src("-" + toLower(cmd));
        }
        else if (opt == "JUMP")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            src("+jump;-jump");
        }
        else if (opt == "JUMPBUG")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            if (cmdstatus["DUCK"] != 0)
            {
                cerr << format("At line {}:\n---- error: You are using 'JUMPBUG' under '+DUCK'.\n", lineid);
                genfailed();
            }
            src("+duck");
            slp(ticktime);
            src("-duck;+jump;-jump");
        }
        else if (opt.substr(0, 4) == "FPS_")
        {
            if (opt == "FPS_LOW")
            {
                fpslocked = 1;
                RequireArg(opt, 0, lineargs.size(), 1);
                src("syncerUpd_forceLow");
            }
            else if (opt == "FPS_RESET")
            {
                fpslocked = 0;
                RequireArg(opt, 0, lineargs.size(), 1);
                src("hzCVAR_set_fps_max;syncer_updPolicy");
            }
        }
        else if (opt.substr(0, 6) == "SPEED_")
        {
            if (yawstatus != 0)
            {
                if (yawstatus < 0)
                {
                    src(format("turnleft -{:6f} 0 0;-left", fabs(yawstatus)));
                }
                else
                {
                    src(format("turnright -{:6f} 0 0;-right", fabs(yawstatus)));
                }
                yawstatus = 0;
            }
            if (cmdstatus["LEFT"] != 0 || cmdstatus["RIGHT"] != 0)
            {
                cerr << format("At line {}:\n---- error: You are using 'SPEED_CMD' under '+LEFT' or '+RIGHT'.\n", lineid);
                genfailed();
            }
            if (opt == "SPEED_LEFT")
            {
                RequireArg(opt, 1, lineargs.size(), 1);
                isValidNumber(opt, lineargs[0]);
                double res = stod(lineargs[0]) / 300;
                if (res <= 0 || res > 1)
                {
                    cerr << format("At line {}:\n---- error: angle changing speed must be in (0,300].\n", lineid);
                    genfailed();
                }
                src(format("turnleft {:6f} 0 0;+left", res));
                yawstatus = -stod(lineargs[0]);
            }
            else if (opt == "SPEED_RIGHT")
            {
                RequireArg(opt, 1, lineargs.size(), 1);
                isValidNumber(opt, lineargs[0]);
                double res = stod(lineargs[0]) / 300;
                if (res <= 0 || res > 1)
                {
                    cerr << format("At line {}:\n---- error: angle changing speed must be in (0,300].\n", lineid);
                    genfailed();
                }
                src(format("turnright {:6f} 0 0;+right", res));
                yawstatus = stod(lineargs[0]);
            }
            else if (opt == "SPEED_STOP")
            {
                RequireArg(opt, 0, lineargs.size(), 1);
            }
        }
        else if (opt == "DROP")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            src("+hzBind_drop;-hzBind_drop");
        }
        else if (opt == "SLOT1")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            src("+hzBind_wp1;-hzBind_wp1");
        }
        else if (opt == "SLOT2")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            src("+hzBind_wp2;-hzBind_wp2");
        }
        else if (opt == "SLOT3")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            src("+hzBind_wp3;-hzBind_wp3");
        }
        else if (opt == "SLOT4")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            src("+hzBind_wp4;-hzBind_wp4");
        }
        else if (opt == "SLOT5")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            src("+hzBind_wp5;-hzBind_wp5");
        }
        else if (opt.substr(0, 5) == "NADE_")
        {
            RequireArg(opt, 0, lineargs.size(), 1);
            if (opt == "NADE_HE")
            {
                src("+hzBind_HE;+hzBind_HE");
            }
            else if (opt == "NADE_SMOKE")
            {
                src("+hzBind_smoke;+hzBind_smoke");
            }
            else if (opt == "NADE_MOLO")
            {
                src("+hzBind_molo;+hzBind_molo");
            }
            else if (opt == "NADE_FLASH")
            {
                src("+hzBind_flash;+hzBind_flash");
            }
            else if (opt == "NADE_DECOY")
            {
                src("+hzBind_decoy;+hzBind_decoy");
            }
        }
        else if (opt.substr(0, 4) == "SAY_")
        {
            RequireArg(opt, 1, lineargs.size(), 0);
            string cmd;
            if (opt == "SAY_ALL")
            {
                cmd = "say";
            }
            else if (opt == "SAY_TEAM")
            {
                cmd = "say_team";
            }
            else if (opt == "SAY_CONSOLE")
            {
                cmd = "echoln";
            }
            src(format("{} {}", cmd, UnionWithSpace(lineargs)));
        }
        else if (opt == "END")
        {
            RequireArg(opt, 0, lineargs.size(), 1);

            for (auto &[cmd, cmdnum] : cmdstatus)
            {
                if (cmdnum != 0)
                {
                    cerr << format("At line {}:\n---- warring: Script endes without '-{}'. Are you sure?\n", lineid, cmd);
                }
            }
            if (yawstatus != 0)
            {
                cerr << format("At line {}:\n---- warring: Script endes without 'SPEED_STOP'. Are you sure?\n", lineid);
            }
            if (fpslocked != 0)
            {
                cerr << format("At line {}:\n---- warring: Script endes without 'FPS_RESET'. Are you sure?\n", lineid);
            }

            fout << "-syncer_nomsg_later" << endl;
            fout << "hzSchduler_taskend" << endl;
            fout << "alias hzScheduler_launch hzScheduler_launchpath" << endl;
            cerr << format("OK generate {} file{} successfully ({} seconds) ({} ticks)", N, N == 1 ? "" : "s", usedtime, usedtime / ticktime) << endl;
            genInit();
            fout.close();
            copyfile();
            deletefile();
            exit(0);
        }
    }
    exit(0);
}