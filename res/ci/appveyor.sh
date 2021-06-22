#!/usr/bin/env bash
set -ex
PY="${PY:-python3}"; PYGET="get_data.py"; PYSER="ser_data.py"
export PATH="$PWD:/c/Python36-x64:/c/Program Files/Git/usr/bin:$PATH"
export MKN_CL_PREFERRED=1
./mkn clean build run -dtKOg -p test
/c/Python36-x64/Scripts/pip install flawfinder
/c/Python36-x64/python -m flawfinder .
