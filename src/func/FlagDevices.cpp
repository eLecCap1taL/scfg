#include <iostream>
#include <cstdlib>
#include <string>
using namespace std;

int FlagDevicesWithEnv() {
#ifdef _WIN32
    const string key = "ScfgExecuted";
    const string value = "1";
    const string env_str = key + "=" + value;

    if (_putenv(env_str.c_str()) != 0) {
       cerr << "Failed to set environment variable for session.\n";
        return 2;
    }

    string cmd = "setx " + key + " " + value + " > nul";
    if (system(cmd.c_str()) != 0) {
        cerr << "Failed to persist environment variable using setx.\n";
        return 1;
    }

    return 0;
#else
    cerr << "Non-Windows platform detected. Skipping environment variable setup.\n";
    return -1;  
#endif
}
