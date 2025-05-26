#include <bits/stdc++.h>
using namespace std;
namespace fs = filesystem;

#define PAUSE system("pause")
#define IMPORT_MAX_LEVEL 16

// file pos
typedef pair<fs::path, int> PosInfo;

// functions
void quitf(int sta);

bool posValid(const PosInfo &pos);

namespace scfg_args
{
    bool allerror;
    void check_arg(const string &arg);
}

void error(const string &text, const PosInfo &pos = PosInfo());

void waring(const string &text, const PosInfo &pos = PosInfo());

template <typename T>
void try_to_access_file(const fs::path &file, T callback, const PosInfo &pos = PosInfo());

template <typename T>
void log(T msg);

string get_raw(fs::path file);

string add_linenumber(const string &s);

void sanitize_string(string &s);

string read_line(const string &s, int idx = 0);

string read_word(const string &s, int idx = 0);

pair<string,bool> read_arg_string(const string &s, int idx=0);

pair<string,bool> read_codeblocks(const string &s, int idx=0);

// -------------------------------------------------------------------------------------------

// quit with a status
void quitf(int sta)
{
    exit(sta);
}

// file pos check
bool posValid(const PosInfo &pos)
{
    return !pos.first.empty() && pos.second != 0;
}

// check_arg
void scfg_args::check_arg(const string &arg)
{
    if (arg == "--allerror")
        allerror = 1;
    else
    {
        error(format("unknown argument '{}'", arg));
    }
}

// report an error
void error(const string &text, const PosInfo &pos)
{
    cout << "error:" << endl;
    if (posValid(pos))
        cout << format("in file '{}', at line {}: ", pos.first.string(), pos.second) << endl;
    cout << text << endl;
    quitf(1);
}

// report an waring
void waring(const string &text, const PosInfo &pos)
{
    cout << "error:" << endl;
    if (posValid(pos))
        cout << format("in file '{}', at line {}: ", pos.first.string(), pos.second) << endl;
    cout << text << endl;
    if (scfg_args::allerror)
        quitf(1);
}

// try access files. if success it will call "callback()", or quit with an error
template <typename T>
void try_to_access_file(const fs::path &file, T callback, const PosInfo &pos)
{
    if (fs::exists(file))
    {
        callback(file);
    }
    else
    {
        error(format("file '{}' does not exist!", file.string()), pos);
    }
}

// add line number before \n for a string
string add_linenumber(const string &s)
{
    string ret = "";
    int n = 0;
    ret += format("{} | ", ++n);
    for (auto c : s)
    {
        ret += c;
        if (c == '\n')
        {
            ret += format("{} | ", ++n);
        }
    }
    return ret;
}

// log
template <typename T>
void log(T msg)
{
    time_t now_time_t = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm now_tm = *localtime(&now_time_t);
    ostringstream oss;
    oss << "["
        << setw(2) << setfill('0') << now_tm.tm_hour << ":"
        << setw(2) << setfill('0') << now_tm.tm_min << ":"
        << setw(2) << setfill('0') << now_tm.tm_sec
        << "]";
    cout << oss.str() << " " << msg << endl;
}

// get a raw content of a file.
string get_raw(fs::path file)
{
    ifstream fin(file, ios::in);

    auto chkflash = [](size_t &i, const string &s, string &tmp) -> bool
    {
        if (s[i] != '\\')
            return false;
        if (i == s.size())
            error("a single '\\' is found");
        if (s[i + 1] != '(' && s[i + 1] != ')' && s[i + 1] != '{' && s[i + 1] != '}' && s[i + 1] != ' ')
            error("a single '\\' is found");
        tmp += '\\';
        tmp += s[++i];
        return true;
    };

    string ret = "\n";
    string tmp = "";

    // input
    char c;
    while ((c = fin.get()) != EOF)
    {
        if (c == '\r')
            continue;
        if (c == '\t')
            c = ' ';
        ret += c;
    }

    // process /**/
    for (size_t i = 0, in = 0; i < ret.size(); i++)
    {
        if (i + 1 < ret.size())
        {
            if (in)
            {
                if (ret[i] == '*' && ret[i + 1] == '/')
                {
                    in = 0;
                    i = i + 1;
                    continue;
                }
            }
            else
            {
                if (ret[i] == '/' && ret[i + 1] == '*')
                {
                    in = 1;
                    i = i + 1;
                    continue;
                }
            }
        }
        if (!in)
            tmp += ret[i];
    }
    swap(ret, tmp), tmp.clear();

    // process //
    for (size_t i = 0; i < ret.size(); i++)
    {
        if (i + 1 < ret.size() && ret[i] == '/' && ret[i + 1] == '/')
        {
            do
            {
                i++;
            } while (i < ret.size() && ret[i] != '\n');
        }
        else
        {
            tmp += ret[i];
        }
    }
    swap(ret, tmp), tmp.clear();

    // process ;
    // for (size_t i = 0, in = 0; i < ret.size(); i++)
    // {
    //     if (in)
    //     {
    //         if (i + 1 < ret.size() && ret[i] == '\\')
    //         {
    //             i++;
    //             continue;
    //         }
    //         if (ret[i] == '"')
    //             in = 0;
    //     }
    //     else
    //     {
    //         if (ret[i] == '"')
    //             in = 1;
    //         if (!in && ret[i] == ';')
    //             ret[i] = '\n';
    //         tmp += ret[i];
    //     }
    // }
    for (size_t i = 0; i < ret.size(); i++)
    {
        if (ret[i] == ';')
            ret[i] = '\n';
        tmp += ret[i];
    }
    swap(ret, tmp), tmp.clear();

    // process {} ()
    for (size_t i = 0; i < ret.size(); i++)
    {
        if (chkflash(i, ret, tmp))
            continue;
        if (ret[i] == '\\')
        {
            tmp += '\\';
            if (i + 1 == ret.size())
                error("a single '\\' is found at the end of the file.");
            tmp += ret[++i];
            continue;
        }
        if (ret[i] == '{' || ret[i] == '}')
            tmp += '\n';
        if (ret[i] == '(')
            tmp += ' ';
        tmp += ret[i];
        if (ret[i] == '{' || ret[i] == '}')
            tmp += '\n';
        if (ret[i] == ')')
            tmp += ' ';
    }
    swap(ret, tmp), tmp.clear();

    // process tab
    for (size_t i = 0, newline = 1; i < ret.size(); i++)
    {
        if (newline)
        {
            while (i < ret.size() && ret[i] == ' ')
                i++;
            if (i == ret.size())
                break;
            newline = 0;
        }
        tmp += ret[i];
        if (ret[i] == '\n')
            newline = 1;
    }
    swap(ret, tmp), tmp.clear();

    // process \n\n \space\space (\space)
    for (size_t i = 0; i < ret.size(); i++)
    {
        if (chkflash(i, ret, tmp))
            continue;
        if (i + 1 == ret.size() && ret[i] == '\n')
            continue;
        if (i + 1 < ret.size() && ret[i] == '\n' && ret[i + 1] == '\n')
            continue;
        if (i + 1 < ret.size() && ret[i] == ' ' && ret[i + 1] == ' ')
            continue;
        if (i + 1 < ret.size() && ret[i] == ' ' && ret[i + 1] == '\n')
            continue;
        if (i + 1 < ret.size() && ret[i] == ' ' && ret[i + 1] == ')')
            continue;
        tmp += ret[i];
    }
    swap(ret, tmp), tmp.clear();
    for (int i = (int)ret.size() - 1; i >= 0; i--)
    {
        if (i - 1 >= 0 && ret[i] == ' ' && ret[i - 1] == '(')
            continue;
        tmp += ret[i];
    }
    reverse(tmp.begin(), tmp.end());
    swap(ret, tmp), tmp.clear();

    return ret.substr(1);
}

// replace \c
void sanitize_string(string &s)
{
    string tmp = "";
    for (size_t i = 0; i < s.size(); i++)
    {
        if (i + 1 < s.size() && s[i] == '\\')
        {
            if (s[i + 1] == '(')
            {
                tmp += char(-1);
            }
            else if (s[i + 1] == ')')
            {
                tmp += char(-2);
            }
            else if (s[i + 1] == '{')
            {
                tmp += char(-3);
            }
            else if (s[i + 1] == '}')
            {
                tmp += char(-4);
            }
            else if (s[i + 1] == ' ')
            {
                tmp += char(-5);
            }
            else
            {
                error("Unexpected error in sanitize_string");
            }
            i++;
            continue;
        }
        tmp += s[i];
    }
    swap(tmp, s);
}

// read until \n
string read_line(const string &s, int idx)
{
    string ret = "";
    for (size_t i = idx; i < s.size(); i++)
    {
        if (s[i] == '\n')
            break;
        ret += s[i];
    }
    return ret;
}

// read word
string read_word(const string &s, int idx)
{
    string ret = "";
    for (size_t i = idx; i < s.size(); i++)
    {
        if (s[i] == ' ' || s[i] == '\n')
            break;
        ret += s[i];
    }
    return ret;
}

// read ()
pair<string,bool> read_arg_string(const string &s, int idx)
{
    string ret = "";
    int in=0;
    for (size_t i = idx; i < s.size(); i++)
    {
        if(in==1){
            if(s[i]==')'){
                in=2;
                break;
            }
            ret+=s[i];
        }else{
            if(s[i]=='(')   in=1;
        }
    }
    return make_pair(ret,in==2);
}
// read {}
pair<string,bool> read_codeblocks(const string &s, int idx)
{
    string ret = "";
    int in=0,hv=0;
    for (size_t i = idx; i < s.size(); i++)
    {
        if(s[i]=='{'){
            hv=1;
            in++;
        }
        if(s[i]=='}'){
            in--;
            if(in==0)   break;
        }
        if(in>0 && !(in==1 && s[i]=='{')){
            ret+=s[i];
        }
    }
    return make_pair(ret,in==0 && hv==1);
}