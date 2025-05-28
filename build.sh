if g++ ./src/func/FlagDevices.cpp  ./src/scfg.cpp -I./src/include -L./src/ -llua54 -o scfg.exe -std=c++23 -static;then
    echo build successfully
    ./scfg test.scfg
else
    echo build failed
fi