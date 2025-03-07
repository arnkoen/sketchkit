#/bin/bash 2>nul || goto :windows

# -------------------
# LINUX / MACOS PART
# -------------------

#!/bin/bash
set -e

# Check if a main file is provided
MAIN_FILE=${1:-hello.c}

# Choose build type
if [ "$2" == "debug" ]; then
    CFLAGS="-g -O0 -DDEBUG"
    OUTFILE="${MAIN_FILE%.c}_debug"
else
    CFLAGS="-O2 -DNDEBUG"
    OUTFILE="${MAIN_FILE%.c}"
fi

# Set library search path (adjust as needed)
LIB_PATH="./libs"
LDFLAGS="-L$LIB_PATH -lbimg -lbx -lbgfx"

echo "Building $OUTFILE with main file $MAIN_FILE..."

gcc $CFLAGS $MAIN_FILE sk/sk.c sk/RGFW.c $LDFLAGS -o $OUTFILE

echo "Cleaning up unnecessary files..."
rm -f *.o

echo "Done."
exit 0

# -------------------
# WINDOWS PART
# -------------------
:windows
@echo off
setlocal enabledelayedexpansion

REM Check if a main file is provided
if "%1"=="" (
    echo "Please provide an input file!"
    exit
) else (
    set "MAIN_FILE=%1"
)

REM Choose build type
if "%2"=="debug" (
    set "CFLAGS=/Od /Zi /MDd /DDEBUG"
    set "OUTFILE=%~n1_debug.exe"
) else (
    set "CFLAGS=/O2 /MD /DNDEBUG"
    set "OUTFILE=%~n1.exe"
)

echo Building %OUTFILE% with main file %MAIN_FILE%...

cl %CFLAGS% %MAIN_FILE% sk/sk.c sk/RGFW.c /Zc:preprocessor /link /LIBPATH:libs bimg.lib bx.lib bgfx.lib /OUT:%OUTFILE%

if %errorlevel% neq 0 (
    echo Build failed!
    exit /b %errorlevel%
)

echo Cleaning up unnecessary files...
del /Q *.obj *.ilk *.pdb *.lib *.exp
echo Done.
