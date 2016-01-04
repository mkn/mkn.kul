@ECHO off

if "%1"=="" GOTO blank
if "%1"=="libs" GOTO libs
else GOTO eof

:libs
SET PWD=%CD%
CALL :get
GOTO eof

:get
SET KUL_VER=master
SET HASH_VER=2.0.3
IF NOT EXIST %PWD%\ext\sparsehash\%HASH_VER% (
	git clone http://github.com/mkn/google.sparsehash.git --branch %HASH_VER% %PWD%/ext/sparsehash/%HASH_VER%
)
GOTO eof

:blank
SET OWD=%PWD%
SET PWD=%CD%\ext\kul\%KUL_VER%

CALL :get
SET PWD=%OWD%
GOTO eof

:eof
