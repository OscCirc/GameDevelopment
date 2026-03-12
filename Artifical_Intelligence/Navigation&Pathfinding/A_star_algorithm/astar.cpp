/*
 * A* Pathfinding Algorithm with Console Visualization
 * =====================================================
 * Controls:
 *   - Arrow Keys / WASD : Move cursor
 *   - Space             : Toggle wall
 *   - S                 : Set start point
 *   - E                 : Set end point
 *   - Enter             : Run A* (animated)
 *   - C                 : Clear grid (keep walls)
 *   - R                 : Reset everything
 *   - 1                 : Load preset maze
 *   - Q / ESC           : Quit
 *
 * Legend:
 *   S  = Start    E  = End
 *   ## = Wall     .  = Empty
 *   o  = Open     x  = Closed
 *   *  = Path
 */

#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <windows.h>
#include <conio.h>

// ─── Constants ────────────────────────────────────────────────────────────────
const int ROWS = 22;
const int COLS = 50;

// Windows console colors (foreground | background)
enum Color : WORD {
    C_DEFAULT  = 0x07,  // White on Black
    C_WALL     = 0x80,  // Black on Dark Gray (wall block)
    C_START    = 0x2A,  // Bright Green on Green
    C_END      = 0x4C,  // Bright Red on Red
    C_OPEN     = 0x0B,  // Bright Cyan
    C_CLOSED   = 0x09,  // Bright Blue
    C_PATH     = 0x0E,  // Bright Yellow
    C_CURSOR   = 0x70,  // Black on White
    C_HEADER   = 0x0A,  // Bright Green
    C_INFO     = 0x0F,  // Bright White
    C_LEGEND   = 0x0D,  // Bright Magenta
    C_ERROR    = 0x0C,  // Bright Red
};

// ─── Cell State ───────────────────────────────────────────────────────────────
enum CellState { EMPTY, WALL, START, END, OPEN_SET, CLOSED_SET, PATH_CELL };

// ─── Node for A* ──────────────────────────────────────────────────────────────
struct Node {
    int row, col;
    double g, h, f;
    Node* parent;
    Node(int r, int c) : row(r), col(c), g(0), h(0), f(0), parent(nullptr) {}
};

// ─── Globals ──────────────────────────────────────────────────────────────────
CellState grid[ROWS][COLS];
Node*     nodeGrid[ROWS][COLS];
HANDLE    hConsole;
HANDLE    hStdIn;

int cursorRow = ROWS / 2;
int cursorCol = COLS / 2;
int startRow = -1, startCol = -1;
int endRow   = -1, endCol   = -1;

// Display offset (leave room for header & legend on right)
const int GRID_ORIGIN_X = 2;
const int GRID_ORIGIN_Y = 3;
const int LEGEND_X      = COLS * 2 + 6;

// ─── Console Helpers ──────────────────────────────────────────────────────────
void setCursorPos(int x, int y) {
    COORD c = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hConsole, c);
}

void setColor(WORD color) {
    SetConsoleTextAttribute(hConsole, color);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO ci;
    GetConsoleCursorInfo(hConsole, &ci);
    ci.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &ci);
}

void clearScreen() {
    COORD topLeft = { 0, 0 };
    DWORD written;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacter(hConsole, ' ', cells, topLeft, &written);
    FillConsoleOutputAttribute(hConsole, C_DEFAULT, cells, topLeft, &written);
    SetConsoleCursorPosition(hConsole, topLeft);
}

// ─── Draw Functions ──────────────────────────────────────────────────────────
void drawCell(int row, int col, bool isCursor = false) {
    int x = GRID_ORIGIN_X + col * 2;
    int y = GRID_ORIGIN_Y + row;
    setCursorPos(x, y);

    if (isCursor) {
        setColor(C_CURSOR);
        std::cout << "  ";
        setColor(C_DEFAULT);
        return;
    }

    switch (grid[row][col]) {
    case EMPTY:
        setColor(C_DEFAULT);
        std::cout << ". ";
        break;
    case WALL:
        setColor(0x88); // Dark Gray on Dark Gray
        std::cout << "  ";
        break;
    case START:
        setColor(C_START);
        std::cout << " S";
        break;
    case END:
        setColor(C_END);
        std::cout << " E";
        break;
    case OPEN_SET:
        setColor(C_OPEN);
        std::cout << " o";
        break;
    case CLOSED_SET:
        setColor(C_CLOSED);
        std::cout << " x";
        break;
    case PATH_CELL:
        setColor(C_PATH);
        std::cout << " *";
        break;
    }
    setColor(C_DEFAULT);
}

void drawGrid(bool showCursor = true) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            bool isCur = showCursor && (r == cursorRow && c == cursorCol);
            drawCell(r, c, isCur);
        }
    }
}

void drawBorder() {
    // Top border
    setCursorPos(GRID_ORIGIN_X - 1, GRID_ORIGIN_Y - 1);
    setColor(C_HEADER);
    std::cout << "+";
    for (int c = 0; c < COLS; c++) std::cout << "--";
    std::cout << "+";

    // Side borders
    for (int r = 0; r < ROWS; r++) {
        setCursorPos(GRID_ORIGIN_X - 1, GRID_ORIGIN_Y + r);
        setColor(C_HEADER);
        std::cout << "|";
        setCursorPos(GRID_ORIGIN_X + COLS * 2, GRID_ORIGIN_Y + r);
        std::cout << "|";
    }

    // Bottom border
    setCursorPos(GRID_ORIGIN_X - 1, GRID_ORIGIN_Y + ROWS);
    setColor(C_HEADER);
    std::cout << "+";
    for (int c = 0; c < COLS; c++) std::cout << "--";
    std::cout << "+";

    setColor(C_DEFAULT);
}

void drawHeader() {
    setCursorPos(0, 0);
    setColor(C_HEADER);
    std::cout << "  *** A* Pathfinding Visualizer ***";
    setColor(C_INFO);
    setCursorPos(0, 1);
    std::cout << "  Grid: " << COLS << "x" << ROWS
              << "  |  Heuristic: Euclidean  |  Movement: 8-Dir";
    setColor(C_DEFAULT);
}

void drawLegend() {
    int x = LEGEND_X;
    int y = GRID_ORIGIN_Y;

    auto line = [&](const char* label, WORD color, const char* sym) {
        setCursorPos(x, y++);
        setColor(color);
        std::cout << sym;
        setColor(C_DEFAULT);
        std::cout << " " << label;
    };

    setCursorPos(x, y++);
    setColor(C_LEGEND);
    std::cout << "[ LEGEND ]";

    line("Empty",     C_DEFAULT,    ". ");
    line("Wall",      0x88,         "  ");
    line("Start",     C_START,      " S");
    line("End",       C_END,        " E");
    line("Open Set",  C_OPEN,       " o");
    line("Closed",    C_CLOSED,     " x");
    line("Path",      C_PATH,       " *");
    line("Cursor",    C_CURSOR,     "  ");

    y++;
    setCursorPos(x, y++);
    setColor(C_LEGEND);
    std::cout << "[ CONTROLS ]";

    setColor(C_DEFAULT);
    auto ctrl = [&](const char* key, const char* desc) {
        setCursorPos(x, y++);
        setColor(C_INFO);
        std::cout << key;
        setColor(C_DEFAULT);
        std::cout << " " << desc;
    };

    ctrl("Arrow/WASD", "Move cursor");
    ctrl("Space      ", "Toggle wall");
    ctrl("S          ", "Set start");
    ctrl("E          ", "Set end");
    ctrl("Enter      ", "Run A* (animated)");
    ctrl("C          ", "Clear path");
    ctrl("R          ", "Reset all");
    ctrl("1          ", "Load preset maze");
    ctrl("Q / ESC    ", "Quit");

    setColor(C_DEFAULT);
}

void drawStatus(const std::string& msg, WORD color = C_DEFAULT) {
    setCursorPos(0, GRID_ORIGIN_Y + ROWS + 2);
    setColor(color);
    // pad to clear previous message
    std::cout << msg;
    for (int i = 0; i < 80 - (int)msg.size(); i++) std::cout << ' ';
    setColor(C_DEFAULT);
}

// ─── Grid Utilities ──────────────────────────────────────────────────────────
void resetGrid() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            grid[r][c] = EMPTY;
    startRow = startCol = endRow = endCol = -1;
}

void clearPath() {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (grid[r][c] == OPEN_SET ||
                grid[r][c] == CLOSED_SET ||
                grid[r][c] == PATH_CELL) {
                grid[r][c] = EMPTY;
            }
        }
    }
    // Restore start/end markers
    if (startRow >= 0) grid[startRow][startCol] = START;
    if (endRow   >= 0) grid[endRow][endCol]      = END;
}

void freeNodeGrid() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            if (nodeGrid[r][c]) { delete nodeGrid[r][c]; nodeGrid[r][c] = nullptr; }
}

void loadPresetMaze() {
    resetGrid();
    // Outer walls
    for (int c = 0; c < COLS; c++) { grid[0][c] = WALL; grid[ROWS-1][c] = WALL; }
    for (int r = 0; r < ROWS; r++) { grid[r][0] = WALL; grid[r][COLS-1] = WALL; }

    // Interior barriers
    for (int r = 1; r < 16; r++) grid[r][10] = WALL;
    for (int c = 10; c < 40; c++) grid[16][c] = WALL;
    for (int r = 5; r < 22; r++) grid[r][40] = WALL;
    for (int c = 1; c < 30; c++) grid[8][c]  = WALL;
    for (int c = 20; c < COLS-1; c++) grid[13][c] = WALL;

    // Gaps (corridors)
    grid[8][15]  = EMPTY;
    grid[16][25] = EMPTY;
    grid[5][40]  = EMPTY;
    grid[12][40] = EMPTY;
    grid[13][22] = EMPTY;

    // Default start / end
    startRow = 1;  startCol = 1;
    endRow   = ROWS-2; endCol = COLS-2;
    grid[startRow][startCol] = START;
    grid[endRow][endCol]     = END;
}

// ─── A* Algorithm ────────────────────────────────────────────────────────────
double heuristic(int r1, int c1, int r2, int c2) {
    // Euclidean distance
    double dr = r1 - r2, dc = c1 - c2;
    return std::sqrt(dr*dr + dc*dc);
}

bool runAStar(bool animate) {
    if (startRow < 0 || endRow < 0) {
        drawStatus("  [!] Please set both a Start (S) and End (E) point first.", C_ERROR);
        return false;
    }

    clearPath();
    freeNodeGrid();

    // Allocate node grid
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            nodeGrid[r][c] = new Node(r, c);

    auto cmp = [](Node* a, Node* b) { return a->f > b->f; };
    std::priority_queue<Node*, std::vector<Node*>, decltype(cmp)> openPQ(cmp);

    std::vector<std::vector<bool>> inOpen(ROWS, std::vector<bool>(COLS, false));
    std::vector<std::vector<bool>> inClosed(ROWS, std::vector<bool>(COLS, false));

    Node* sn = nodeGrid[startRow][startCol];
    sn->h = heuristic(startRow, startCol, endRow, endCol);
    sn->f = sn->h;
    openPQ.push(sn);
    inOpen[startRow][startCol] = true;

    // 8-directional movement with diagonal costs
    const int dr[] = {-1,-1,-1, 0, 0, 1, 1, 1};
    const int dc[] = {-1, 0, 1,-1, 1,-1, 0, 1};
    const double cost[] = {1.414,1,1.414,1,1,1.414,1,1.414};

    int steps = 0;

    while (!openPQ.empty()) {
        Node* cur = openPQ.top(); openPQ.pop();
        int r = cur->row, c = cur->col;

        if (inClosed[r][c]) continue;
        inClosed[r][c] = true;

        // reached goal
        if (r == endRow && c == endCol) {
            // Trace and draw path
            int pathLen = 0;
            Node* nd = cur;
            while (nd) {
                if (grid[nd->row][nd->col] != START && grid[nd->row][nd->col] != END) {
                    grid[nd->row][nd->col] = PATH_CELL;
                    if (animate) {
                        drawCell(nd->row, nd->col);
                        std::this_thread::sleep_for(std::chrono::milliseconds(25));
                    }
                }
                pathLen++;
                nd = nd->parent;
            }
            if (!animate) drawGrid(false);

            std::ostringstream oss;
            oss << "  [OK] Path found!  Length: " << pathLen-1
                << " nodes  |  Explored: " << steps << " nodes";
            drawStatus(oss.str(), C_PATH);
            freeNodeGrid();
            return true;
        }

        // Mark closed
        if (grid[r][c] != START && grid[r][c] != END) {
            grid[r][c] = CLOSED_SET;
            if (animate) {
                drawCell(r, c);
                std::this_thread::sleep_for(std::chrono::milliseconds(8));
            }
        }
        steps++;

        // Expand neighbours
        for (int i = 0; i < 8; i++) {
            int nr = r + dr[i], nc = c + dc[i];
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if (grid[nr][nc] == WALL) continue;
            if (inClosed[nr][nc]) continue;

            // Cut diagonal corners through walls
            if (dr[i] != 0 && dc[i] != 0) {
                if (grid[r + dr[i]][c] == WALL || grid[r][c + dc[i]] == WALL) continue;
            }

            double newG = cur->g + cost[i];
            if (!inOpen[nr][nc] || newG < nodeGrid[nr][nc]->g) {
                nodeGrid[nr][nc]->g = newG;
                nodeGrid[nr][nc]->h = heuristic(nr, nc, endRow, endCol);
                nodeGrid[nr][nc]->f = nodeGrid[nr][nc]->g + nodeGrid[nr][nc]->h;
                nodeGrid[nr][nc]->parent = cur;

                if (!inOpen[nr][nc]) {
                    openPQ.push(nodeGrid[nr][nc]);
                    inOpen[nr][nc] = true;
                    if (grid[nr][nc] != START && grid[nr][nc] != END) {
                        grid[nr][nc] = OPEN_SET;
                        if (animate) {
                            drawCell(nr, nc);
                        }
                    }
                }
            }
        }

        if (animate && steps % 5 == 0) {
            std::ostringstream oss;
            oss << "  [..] Running A*...  Open: " << (int)openPQ.size()
                << "  Explored: " << steps;
            drawStatus(oss.str(), C_OPEN);
        }
    }

    drawStatus("  [X] No path found! The destination is unreachable.", C_ERROR);
    if (!animate) drawGrid(false);
    freeNodeGrid();
    return false;
}

// ─── Input Handling ──────────────────────────────────────────────────────────
void moveCursor(int newRow, int newCol) {
    int prevR = cursorRow, prevC = cursorCol;
    cursorRow = std::max(0, std::min(ROWS-1, newRow));
    cursorCol = std::max(0, std::min(COLS-1, newCol));
    drawCell(prevR, prevC, false);
    drawCell(cursorRow, cursorCol, true);
}

void handleInput() {
    while (true) {
        int ch = _getch();

        // Arrow keys return 224 as first byte on Windows
        if (ch == 0 || ch == 224) {
            int arrow = _getch();
            switch (arrow) {
            case 72: moveCursor(cursorRow - 1, cursorCol); break; // Up
            case 80: moveCursor(cursorRow + 1, cursorCol); break; // Down
            case 75: moveCursor(cursorRow, cursorCol - 1); break; // Left
            case 77: moveCursor(cursorRow, cursorCol + 1); break; // Right
            }
            continue;
        }

        switch (ch) {
        // ── WASD movement ──
        case 'W': case 'w':
            moveCursor(cursorRow - 1, cursorCol); break;
        case 'A': case 'a':
            moveCursor(cursorRow, cursorCol - 1); break;
        case 'D': case 'd':
            moveCursor(cursorRow, cursorCol + 1); break;

        // lowercase 's' → move down; uppercase 'S' → set start
        case 's':
            moveCursor(cursorRow + 1, cursorCol); break;
        case 'S': {
            if (startRow >= 0 && grid[startRow][startCol] == START) {
                grid[startRow][startCol] = EMPTY;
                drawCell(startRow, startCol, false);
            }
            startRow = cursorRow; startCol = cursorCol;
            grid[startRow][startCol] = START;
            drawCell(startRow, startCol, false);
            drawCell(cursorRow, cursorCol, true);
            drawStatus("  Start point set.", C_START);
            break;
        }

        case ' ': {
            // Toggle wall (cannot place on start/end)
            if (grid[cursorRow][cursorCol] == WALL) {
                grid[cursorRow][cursorCol] = EMPTY;
            } else if (grid[cursorRow][cursorCol] == EMPTY) {
                grid[cursorRow][cursorCol] = WALL;
            }
            drawCell(cursorRow, cursorCol, true);
            break;
        }

        case 'E': case 'e': {
            // Set end
            if (endRow >= 0 && grid[endRow][endCol] == END) {
                grid[endRow][endCol] = EMPTY;
                drawCell(endRow, endCol, false);
            }
            endRow = cursorRow; endCol = cursorCol;
            grid[endRow][endCol] = END;
            drawCell(endRow, endCol, false);
            drawCell(cursorRow, cursorCol, true);
            drawStatus("  End point set.", C_END);
            break;
        }

        case '\r': case '\n': // Enter — run A*
            drawStatus("  Running A* algorithm...", C_INFO);
            runAStar(true);
            drawCell(cursorRow, cursorCol, true);
            break;

        case 'C': case 'c':
            clearPath();
            drawGrid(true);
            drawStatus("  Path cleared.", C_INFO);
            break;

        case 'R': case 'r':
            resetGrid();
            drawGrid(true);
            drawStatus("  Grid reset.", C_INFO);
            break;

        case '1':
            loadPresetMaze();
            drawGrid(true);
            drawStatus("  Preset maze loaded! Press Enter to run A*.", C_INFO);
            break;

        case 'Q': case 'q': case 27: // ESC
            freeNodeGrid();
            setCursorPos(0, GRID_ORIGIN_Y + ROWS + 5);
            setColor(C_DEFAULT);
            return;
        }
    }
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    hStdIn   = GetStdHandle(STD_INPUT_HANDLE);

    // Set console size & font
    SMALL_RECT windowSize = { 0, 0, 119, 35 };
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
    COORD bufferSize = { 120, 36 };
    SetConsoleScreenBufferSize(hConsole, bufferSize);

    // Try to make console font smaller for bigger grid
    CONSOLE_FONT_INFOEX cfi = {};
    cfi.cbSize = sizeof(cfi);
    GetCurrentConsoleFontEx(hConsole, FALSE, &cfi);
    cfi.dwFontSize.X = 10;
    cfi.dwFontSize.Y = 18;
    wcscpy_s(cfi.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(hConsole, FALSE, &cfi);

    hideCursor();
    clearScreen();

    // Initialize node grid pointers
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            nodeGrid[r][c] = nullptr;

    // Init grid
    resetGrid();

    // Draw UI
    drawHeader();
    drawBorder();
    drawGrid(true);
    drawLegend();
    drawStatus("  Welcome! Use WASD/Arrows to move, S=Start, E=End, Space=Wall, Enter=Run, 1=Preset Maze.", C_INFO);

    // Main input loop
    handleInput();

    // Cleanup
    clearScreen();
    setColor(C_DEFAULT);
    std::cout << "Thanks for using A* Visualizer!\n";
    return 0;
}
