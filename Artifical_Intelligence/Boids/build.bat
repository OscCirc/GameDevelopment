@echo off
setlocal

:: -----------------------------------------------------------------------
:: 用法：
::   build.bat          — 编译控制台版 (main.cpp)     -> boids_console.exe
::   build.bat raylib   — 编译 Raylib 版 (main_raylib.cpp) -> boids.exe
::
:: Raylib 版需要先安装 Raylib：
::   方式1 (推荐) — winget: winget install raylib
::   方式2       — 下载预编译包: https://github.com/raysan5/raylib/releases
::   并将 include/lib 路径填入下方 RAYLIB_INC / RAYLIB_LIB
:: -----------------------------------------------------------------------

if "%1"=="raylib" goto RAYLIB

:: ---- 控制台版 ----
echo [Build] Console version...
g++ -std=c++17 -O2 -Wall -o boids_console.exe main.cpp
if %errorlevel%==0 (echo [OK] boids_console.exe) else (echo [FAIL])
goto END

:RAYLIB
:: ---- Raylib 版 ----
:: 修改这两个路径指向你的 Raylib 安装目录
set RAYLIB_INC=C:\msys64\ucrt64\include
set RAYLIB_LIB=C:\msys64\ucrt64\lib

echo [Build] Raylib version...
g++ -std=c++17 -O2 -Wall ^
    -I"%RAYLIB_INC%" ^
    -o boids.exe main_raylib.cpp ^
    -L"%RAYLIB_LIB%" -lraylib -lopengl32 -lgdi32 -lwinmm
if %errorlevel%==0 (echo [OK] boids.exe) else (echo [FAIL] Check RAYLIB_INC/RAYLIB_LIB paths in build.bat)

:END
endlocal
