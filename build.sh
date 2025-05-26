if g++ ./src/scfg.cpp ./src/lcfg_extra/LCFGFlagDevices.cpp -I./src/include -L./src/ -llua54 -o scfg.exe -std=c++20 -static;then
    echo build successfully
    ./scfg test.scfg
else
    echo build failed
fi