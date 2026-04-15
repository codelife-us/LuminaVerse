// MIT License
//
// Copyright (c) 2026 Code Life
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

////////////////////////////////////////////////////////////////////////////////
// colorpick.cpp — Interactive terminal color picker
//
// Navigate a hue × saturation grid with arrow keys.
// Adjust brightness with + and -.
// Press Enter to select; the hex color (#RRGGBB) is printed to stdout.
// TUI is drawn to stderr so the result can be captured:
//   COLOR=$(./colorpick)
//   bvi "John 3:16" --bg=$COLOR

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <string>
#ifdef _WIN32
#  include <conio.h>
#  include <windows.h>
#else
#  include <termios.h>
#  include <unistd.h>
#endif

using namespace std;

// ── HSV → RGB ─────────────────────────────────────────────────────────────────

struct RGB { int r, g, b; };

RGB hsvToRgb(double h, double s, double v) {
    // h: 0–360  s,v: 0–1
    double c  = v * s;
    double x  = c * (1.0 - fabs(fmod(h / 60.0, 2.0) - 1.0));
    double m  = v - c;
    double r1, g1, b1;
    if      (h <  60) { r1=c; g1=x; b1=0; }
    else if (h < 120) { r1=x; g1=c; b1=0; }
    else if (h < 180) { r1=0; g1=c; b1=x; }
    else if (h < 240) { r1=0; g1=x; b1=c; }
    else if (h < 300) { r1=x; g1=0; b1=c; }
    else              { r1=c; g1=0; b1=x; }
    return { (int)round((r1+m)*255), (int)round((g1+m)*255), (int)round((b1+m)*255) };
}

// Perceived luminance (0–255)
int luma(RGB c) { return (c.r * 299 + c.g * 587 + c.b * 114) / 1000; }

// ── Terminal I/O ───────────────────────────────────────────────────────────────

// All TUI output goes to stderr; only the final hex result goes to stdout.
#define T(...) fprintf(stderr, __VA_ARGS__)

#ifndef _WIN32
static struct termios gSavedTermios;
static bool gRawMode = false;

void enableRaw() {
    tcgetattr(STDIN_FILENO, &gSavedTermios);
    struct termios raw = gSavedTermios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    gRawMode = true;
}
void disableRaw() {
    if (gRawMode) { tcsetattr(STDIN_FILENO, TCSAFLUSH, &gSavedTermios); gRawMode = false; }
}
#endif

void termCleanup() {
#ifndef _WIN32
    disableRaw();
#endif
    T("\033[?25h");  // show cursor
    T("\033[0m");    // reset color
    fflush(stderr);
}

void termInit() {
    atexit(termCleanup);
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
    DWORD mode = 0; GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
    enableRaw();
#endif
    T("\033[?25l");  // hide cursor
    T("\033[2J\033[H"); // clear screen
}

void moveTo(int row, int col) { T("\033[%d;%dH", row, col); }
void reset()                  { T("\033[0m"); }
void setBg(RGB c)             { T("\033[48;2;%d;%d;%dm", c.r, c.g, c.b); }
void setFg(RGB c)             { T("\033[38;2;%d;%d;%dm", c.r, c.g, c.b); }

// Read one keypress; arrow keys → 'A' 'B' 'C' 'D'
int readKey() {
#ifdef _WIN32
    int c = _getch();
    if (c == 0 || c == 0xE0) {
        int c2 = _getch();
        if (c2 == 72) return 'A'; if (c2 == 80) return 'B';
        if (c2 == 77) return 'C'; if (c2 == 75) return 'D';
    }
    return c;
#else
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;
    if (c == '\033') {
        unsigned char s[2];
        if (read(STDIN_FILENO, &s[0], 1) != 1) return '\033';
        if (read(STDIN_FILENO, &s[1], 1) != 1) return '\033';
        if (s[0] == '[') return s[1];  // A/B/C/D
    }
    return c;
#endif
}

// ── Grid layout ────────────────────────────────────────────────────────────────
//
//  Row 1   : title + key hints
//  Row 2   : hue reference strip (full rainbow at max sat/val)
//  Row 3–20: hue × saturation grid (36 cols × 18 rows, 2 chars per cell)
//  Row 21  : brightness slider
//  Row 22  : (spacer)
//  Row 23  : color preview + hex + RGB
//

static const int COLS      = 36;   // hue steps (0°..350°, every 10°)
static const int ROWS      = 18;   // saturation steps
static const int MARGIN    = 3;    // left margin in chars
static const int GRID_ROW  = 3;    // row where the grid starts

double hueOf(int hi) { return hi * (360.0 / COLS); }
double satOf(int si) { return 1.0 - si * (0.95 / (ROWS - 1)); }

// ── Drawing ────────────────────────────────────────────────────────────────────

void drawHueStrip() {
    moveTo(2, MARGIN);
    for (int hi = 0; hi < COLS; ++hi) {
        RGB c = hsvToRgb(hueOf(hi), 1.0, 1.0);
        setBg(c); T("  ");
    }
    reset();
}

void drawGrid(int hueIdx, int satIdx, double val) {
    for (int si = 0; si < ROWS; ++si) {
        moveTo(GRID_ROW + si, MARGIN);
        for (int hi = 0; hi < COLS; ++hi) {
            RGB c = hsvToRgb(hueOf(hi), satOf(si), val);
            setBg(c);
            if (hi == hueIdx && si == satIdx) {
                // Cursor: contrasting "><" marker
                if (luma(c) > 128) T("\033[30m"); else T("\033[97m");
                T("><");
            } else {
                T("  ");
            }
        }
        reset();
    }
}

void drawBrightnessBar(int hueIdx, int satIdx, double val) {
    moveTo(GRID_ROW + ROWS + 1, MARGIN);
    reset();
    T("Brightness  ");

    static const int BAR = 54;  // bar width in chars
    int mark = max(0, min(BAR - 1, (int)round(val * (BAR - 1))));

    for (int i = 0; i < BAR; ++i) {
        double v2 = (double)i / (BAR - 1);
        RGB c = hsvToRgb(hueOf(hueIdx), satOf(satIdx), v2);
        if (i < mark) {
            // Filled: show the color
            setBg(c); T(" ");
        } else if (i == mark) {
            // Current position: contrasting tick
            setBg(c);
            if (luma(c) > 128) T("\033[30m"); else T("\033[97m");
            T("|");
        } else {
            // Unfilled: dark version of the color
            RGB dim = { c.r / 5, c.g / 5, c.b / 5 };
            setBg(dim); T(" ");
        }
        reset();
    }
    T("  %3d%%  \033[2m(+ / -)\033[0m    ", (int)round(val * 100));
}

void drawPreview(int hueIdx, int satIdx, double val) {
    RGB c = hsvToRgb(hueOf(hueIdx), satOf(satIdx), val);
    char hex[8];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X", c.r, c.g, c.b);

    moveTo(GRID_ROW + ROWS + 3, MARGIN);
    reset();
    // 10-char wide color swatch
    for (int i = 0; i < 5; ++i) { setBg(c); T("  "); }
    reset();
    T("  \033[1m%s\033[0m    R:%3d  G:%3d  B:%3d    ", hex, c.r, c.g, c.b);
}

// ── Main ───────────────────────────────────────────────────────────────────────

int main() {
    termInit();

    // Title row
    moveTo(1, MARGIN);
    T("colorpick  \033[2m\xe2\x80\x93  "
      "\xe2\x86\x90\xe2\x86\x92 hue  "
      "\xe2\x86\x91\xe2\x86\x93 saturation  "
      "+ \xe2\x80\x93 brightness  "
      "Enter select  q quit"
      "\033[0m");

    int  hueIdx  = 0;
    int  satIdx  = 0;
    double value = 1.0;
    bool selected = false;
    bool running  = true;

    while (running) {
        drawHueStrip();
        drawGrid(hueIdx, satIdx, value);
        drawBrightnessBar(hueIdx, satIdx, value);
        drawPreview(hueIdx, satIdx, value);
        fflush(stderr);

        switch (readKey()) {
            case 'A': satIdx = max(0,        satIdx - 1); break;  // up   → more sat
            case 'B': satIdx = min(ROWS - 1, satIdx + 1); break;  // down → less sat
            case 'C': hueIdx = (hueIdx + 1)       % COLS; break;  // right
            case 'D': hueIdx = (hueIdx + COLS - 1) % COLS; break; // left
            case '+': case '=': value = min(1.0,  value + 0.05); break;
            case '-': case '_': value = max(0.02, value - 0.05); break;
            case '\r': case '\n': selected = true;  running = false; break;
            case 'q': case 'Q': case 27:             running = false; break;
        }
    }

    // Clear TUI, restore terminal
    T("\033[2J\033[H");
    fflush(stderr);

    if (selected) {
        RGB c = hsvToRgb(hueOf(hueIdx), satOf(satIdx), value);
        printf("#%02X%02X%02X\n", c.r, c.g, c.b);
    }

    return selected ? 0 : 1;
}
