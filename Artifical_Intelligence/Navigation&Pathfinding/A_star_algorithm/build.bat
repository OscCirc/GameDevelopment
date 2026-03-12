@echo off
echo ============================================
echo   Building A* Pathfinding Visualizer
echo ============================================

:: Try g++ (MinGW) first
where g++ >nul 2>&1
if %errorlevel% == 0 (
    echo Compiler: g++ ^(MinGW^)
    g++ -std=c++17 -O2 -o astar.exe astar.cpp
    if %errorlevel% == 0 (
        echo Build successful! Run: astar.exe
    ) else (
        echo Build FAILED.
    )
    goto :end
)

:: Try cl (MSVC)
where cl >nul 2>&1
if %errorlevel% == 0 (
    echo Compiler: cl ^(MSVC^)
    cl /std:c++17 /O2 /EHsc astar.cpp /Fe:astar.exe
    if %errorlevel% == 0 (
        echo Build successful! Run: astar.exe
    ) else (
        echo Build FAILED.
    )
    goto :end
)

echo ERROR: No C++ compiler found.
echo Please install MinGW (g++) or MSVC (cl) and ensure it is in your PATH.

:end
pause
