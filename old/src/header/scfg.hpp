
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

namespace SCFG
{

    namespace fs = std::filesystem;

    class command{
        public:
        std::vector<command> cmdls;
    };

    class SCFG
    {
        fs::path output_file;

    public:
        SCFG(fs::path output_file) : output_file(output_file)
        {
        }

        void gen()
        {
        }
    };
}
