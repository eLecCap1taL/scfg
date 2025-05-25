#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <regex>
#include <cmath>
#include <format>
#include "scfg.hpp"
using namespace std;
int main(){
    SCFG::SCFG scfg("res.txt");
    SCFG::command forward_and_jump({

    });
    scfg.gen();
    return 0;
}