// MIT License
// Copyright (c) 2026 Code Life
//
// tbr.cpp — Today's Bible Reading
// Build (macOS/Linux): g++ -std=c++11 -o tbr tbr.cpp
// Build (Windows MSVC): cl /EHsc /std:c++17 /utf-8 tbr.cpp /Fe:tbr.exe
// Calls bv and day with the resolved day number.

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <chrono>
using namespace std;

static int dayOfYearWithOffset(int offset) {
    time_t now = time(nullptr);
    struct tm t = *localtime(&now);
    t.tm_mday += offset;
    t.tm_hour = 12;
    mktime(&t);
    return t.tm_yday + 1;
}

// Parse mm/dd/yyyy or mm/dd/yy → day of year; returns -1 on failure.
static int parseDateArg(const string& s) {
    int mm = 0, dd = 0, yyyy = 0;
    if (sscanf(s.c_str(), "%d/%d/%d", &mm, &dd, &yyyy) != 3) return -1;
    if (yyyy < 100) yyyy += 2000;
    struct tm t = {};
    t.tm_year = yyyy - 1900;
    t.tm_mon  = mm - 1;
    t.tm_mday = dd;
    t.tm_hour = 12;
    if (mktime(&t) == (time_t)-1) return -1;
    return t.tm_yday + 1;
}

// Read a key from a named [section] of the nearest .verselumen file.
static string readConfig(const string& section, const string& key) {
    string paths[2] = {".verselumen", ""};
    const char* home = getenv("HOME");
    if (!home) home = getenv("USERPROFILE");
    if (home) paths[1] = string(home) + "/.verselumen";

    for (const string& path : paths) {
        if (path.empty()) continue;
        ifstream f(path);
        if (!f.good()) continue;
        bool inSection = false;
        string line;
        while (getline(f, line)) {
            size_t s = line.find_first_not_of(" \t\r\n");
            if (s == string::npos || line[s] == '#') continue;
            if (line[s] == '[') {
                size_t e = line.find(']', s + 1);
                string sec = (e != string::npos) ? line.substr(s + 1, e - s - 1) : "";
                inSection = (sec == section);
                continue;
            }
            if (!inSection) continue;
            size_t eq = line.find('=', s);
            if (eq == string::npos) continue;
            string k = line.substr(s, eq - s);
            size_t ke = k.find_last_not_of(" \t");
            if (ke != string::npos) k = k.substr(0, ke + 1);
            if (k != key) continue;
            size_t vs = line.find_first_not_of(" \t", eq + 1);
            if (vs == string::npos) return "";
            size_t ve = line.find_last_not_of(" \t\r\n");
            return line.substr(vs, ve - vs + 1);
        }
    }
    return "";
}

int main(int argc, char* argv[]) {
    // Load config defaults
    string openPref    = readConfig("tbr", "open");
    string bver        = readConfig("tbr", "bibleversion");
    string youtubePref = readConfig("tbr", "youtube");
    if (openPref.empty()) openPref = "e";

    bool useGateway  = (openPref == "g");
    bool openYoutube = (youtubePref != "no");
    int  targetDay   = dayOfYearWithOffset(0);  // default: today

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            cout << "tbr — Today's Bible Reading\n\n"
                 << "Usage: tbr [DATE|OFFSET] [-e | -g | -g=VERSION] [-n]\n\n"
                 << "  (no args)          Open today's reading\n"
                 << "  N or +N            N days ahead of today\n"
                 << "  -N                 N days before today\n"
                 << "  mm/dd/yyyy         Specific date\n"
                 << "  -e                 Open on ESV.org\n"
                 << "  -g                 Open on BibleGateway.com\n"
                 << "  -g=VERSION         Open on BibleGateway with a specific version (e.g. -g=NKJV)\n"
                 << "  -n, --no-youtube   Skip opening YouTube\n"
                 << "  -y, --youtube      Open YouTube (overrides config youtube = no)\n\n"
                 << "Config (~/.verselumen):\n"
                 << "  [tbr]\n"
                 << "  open = g            # e = ESV.org (default), g = BibleGateway\n"
                 << "  bibleversion = NKJV\n"
                 << "  youtube = no        # set to no to skip YouTube by default\n\n"
                 << "Examples:\n"
                 << "  tbr                 # today's reading\n"
                 << "  tbr -1              # yesterday\n"
                 << "  tbr 3               # 3 days from today\n"
                 << "  tbr 5/19/2026       # specific date\n"
                 << "  tbr -g=NKJV         # today on BibleGateway, NKJV\n"
                 << "  tbr -1 -g=NKJV      # yesterday on BibleGateway, NKJV\n"
                 << "  tbr -n              # today's reading, no YouTube\n";
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            cout << "tbr v1.0\n";
            return 0;
        } else if (arg == "-n" || arg == "--no-youtube") {
            openYoutube = false;
        } else if (arg == "-y" || arg == "--youtube") {
            openYoutube = true;
        } else if (arg == "-e") {
            useGateway = false;
            bver = "";
        } else if (arg == "-g") {
            useGateway = true;
        } else if (arg.find("-g=") == 0) {
            useGateway = true;
            bver = arg.substr(3);
        } else if (arg.find('/') != string::npos) {
            int d = parseDateArg(arg);
            if (d < 0) {
                cerr << "tbr: invalid date \"" << arg << "\"\n";
                return 1;
            }
            targetDay = d;
        } else {
            // Numeric offset (+3, -1, 0, 3, etc.)
            try {
                targetDay = dayOfYearWithOffset(stoi(arg));
            } catch (...) {
                cerr << "tbr: invalid argument \"" << arg << "\"\n"
                     << "Usage: tbr [mm/dd/yyyy | offset] [-e | -g | -g=VERSION]\n";
                return 1;
            }
        }
    }

    string dayArg  = "-d=" + to_string(targetDay);
    string openArg = useGateway ? ("-g" + (bver.empty() ? "" : " -bv=" + bver)) : "-e";

    system(("bv " + dayArg + " " + openArg).c_str());
    if (openYoutube) {
        this_thread::sleep_for(chrono::milliseconds(750));
        system(("day " + dayArg + " -a").c_str());
    }
    return 0;
}
