#include "ulti.hpp"
#include <bits/stdc++.h>
using namespace std;

class scfg_number{
    int number;
public:
    scfg_number(const string& s){
        if(s.size()==0) error("wrong time format with string '"+s+"'");
        number=0;
        for(size_t i=0;i<s.size();i++){
            if(!isdigit(s[i]))  error("wrong time format with string '"+s+"'");
            number=number*10+(s[i]^48);
        }
    }
};
class scfg_command{
    typedef variant<scfg_command,scfg_number,vector<scfg_command>> scfg_args;
    string key_prefix;
    vector<scfg_args> arg_list;
};



void load(fs::path file)
{
    log("Reading scfg files...");

    string prog = get_raw(file);

    log("Done. Formatted code:\n" + add_linenumber(prog));

    log("Process \\c -> id");

    sanitize_string(prog);

    log("Done.");

    //begin
    
}

int main(int argc, char *argv[])
{
    // check argc
    if (argc < 2)
    {
        cout << "Usage: scfg.exe <file> <args>" << endl;
        cout << "args:" << endl;
        cout << "--allerror    Treat all warnings as errors, causing the compilation to terminate on any warning.";
        quitf(1);
    }

    // process addtion args
    for (int i = 2; i < argc; i++)
    {
        scfg_args::check_arg(string(argv[i]));
    }

    // get file path
    try_to_access_file(fs::path(string(argv[1])), load, {fs::path("command"), 1});

    quitf(0);
}