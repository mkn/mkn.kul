#!/usr/bin/env bash
set -ex
PY="${PY:-python3}"; PYGET="get_data.py"; PYSER="ser_data.py"
export PATH="$PWD:/c/Python36-x64:/c/Program Files/Git/usr/bin:$PATH"
export MKN_CL_PREFERRED=1
which bash
where bash
curl -L https://github.com/Dekken/maiken/raw/binaries/win10_x64/mkn.exe -o mkn.exe
./mkn clean build run -dtKOg -p test
/c/Python36-x64/Scripts/pip install flawfinder
/c/Python36-x64/python /c/Python36-x64/Scripts/flawfinder .
