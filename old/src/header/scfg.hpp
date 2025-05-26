
#include <bits/stdc++.h>

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
