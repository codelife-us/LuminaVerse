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
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <chrono>
using namespace std;

static const string SECTION = "tbr";

static string configPath() {
    const char* home = getenv("HOME");
    if (!home) home = getenv("USERPROFILE");
    return home ? string(home) + "/.verselumen" : ".verselumen";
}

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
    string paths[2] = {".verselumen", configPath()};

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

// Read a key from the global section (before any [section]) of the nearest .verselumen.
static string readGlobalConfig(const string& key) {
    string paths[2] = {".verselumen", configPath()};
    for (const string& path : paths) {
        if (path.empty()) continue;
        ifstream f(path);
        if (!f.good()) continue;
        string line;
        while (getline(f, line)) {
            size_t s = line.find_first_not_of(" \t\r\n");
            if (s == string::npos || line[s] == '#') continue;
            if (line[s] == '[') break;  // end of global section
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

// Add, update, or remove a key in the global section of ~/.verselumen.
// Passing an empty value removes the key.
static bool writeGlobalKey(const string& key, const string& value) {
    string path = configPath();
    vector<string> result;
    bool keyDone = false;
    bool inGlobal = true;

    ifstream f(path);
    if (f.good()) {
        string line;
        while (getline(f, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            size_t s = line.find_first_not_of(" \t");
            if (inGlobal && s != string::npos && line[s] == '[') {
                if (!keyDone && !value.empty()) {
                    result.push_back(key + " = " + value);
                    keyDone = true;
                }
                inGlobal = false;
            }
            if (inGlobal && !keyDone && s != string::npos && line[s] != '#') {
                size_t eq = line.find('=', s);
                if (eq != string::npos) {
                    string k = line.substr(s, eq - s);
                    size_t ke = k.find_last_not_of(" \t");
                    if (ke != string::npos) k = k.substr(0, ke + 1);
                    if (k == key) {
                        keyDone = true;
                        if (!value.empty())
                            result.push_back(key + " = " + value);
                        continue;  // skip old line; effectively deletes if value is empty
                    }
                }
            }
            result.push_back(line);
        }
    }
    if (!keyDone && !value.empty())
        result.insert(result.begin(), key + " = " + value);

    ofstream out(path);
    if (!out) return false;
    for (const string& l : result) out << l << "\n";
    return true;
}

// Write (or replace) the [tbr] section in ~/.verselumen.
static bool writeSection(const vector<string>& lines) {
    string path = configPath();
    vector<string> before, after;
    bool inTarget = false, found = false;
    ifstream f(path);
    if (f.good()) {
        string line;
        while (getline(f, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            size_t s = line.find_first_not_of(" \t");
            if (s != string::npos && line[s] == '[') {
                size_t e = line.find(']', s);
                string sec = (e != string::npos) ? line.substr(s + 1, e - s - 1) : "";
                if (sec == SECTION) { inTarget = true; found = true; continue; }
                else inTarget = false;
            }
            if (inTarget) continue;
            (found ? after : before).push_back(line);
        }
    }
    while (!before.empty() && before.back().empty()) before.pop_back();
    while (!after.empty() && after.front().empty()) after.erase(after.begin());
    ofstream out(path);
    if (!out) return false;
    for (const string& l : before) out << l << "\n";
    if (!before.empty()) out << "\n";
    out << "[" << SECTION << "]\n";
    for (const string& l : lines) out << l << "\n";
    if (!after.empty()) { out << "\n"; for (const string& l : after) out << l << "\n"; }
    return true;
}

static void runSetup(bool useGateway, const string& bverIn, bool openYoutube,
                     const string& currentBrowser) {
    string input, bver = bverIn;
    string browser = currentBrowser;
    bool useDefaultBrowser = browser.empty();

    cout << "\ntbr setup — configure your Bible reading preferences\n"
         << "-----------------------------------------------------\n\n";

    // Question 1: BibleGateway or ESV.org
    cout << "Open BibleGateway.com for your daily reading?\n"
         << "  (If no, ESV.org will open instead)\n"
         << "  [" << (useGateway ? "Y/n" : "y/N") << "]: " << flush;
    getline(cin, input);
    if (!input.empty()) {
        if (input[0] == 'y' || input[0] == 'Y') useGateway = true;
        if (input[0] == 'n' || input[0] == 'N') useGateway = false;
    }

    // Question 2: Bible version (only when BibleGateway selected)
    if (useGateway) {
        cout << "\nBible version for BibleGateway.com:\n"
             << "  Common codes: KJV, NKJV, NIV, ESV, NLT, CSB, NASB\n"
             << "  Tip: on BibleGateway.com open the version dropdown — the code\n"
             << "  to enter here is shown in parentheses next to the version name.\n"
             << "  [" << (bver.empty() ? "NKJV" : bver) << "]: " << flush;
        getline(cin, input);
        if (!input.empty())
            bver = input;
        else if (bver.empty())
            bver = "NKJV";
    } else {
        bver = "";
    }

    // Question 3: YouTube
    cout << "\nOpen YouTube for the daily Bible video search?\n"
         << "  [" << (openYoutube ? "Y/n" : "y/N") << "]: " << flush;
    getline(cin, input);
    if (!input.empty()) {
        if (input[0] == 'y' || input[0] == 'Y') openYoutube = true;
        if (input[0] == 'n' || input[0] == 'N') openYoutube = false;
    }

    // Question 4: Browser
    cout << "\nUse the system default browser?\n"
         << "  (If no, you can specify a browser like Microsoft Edge or Firefox)\n"
         << "  [" << (useDefaultBrowser ? "Y/n" : "y/N") << "]: " << flush;
    getline(cin, input);
    if (!input.empty()) {
        if (input[0] == 'y' || input[0] == 'Y') useDefaultBrowser = true;
        if (input[0] == 'n' || input[0] == 'N') useDefaultBrowser = false;
    }

    if (!useDefaultBrowser) {
        cout << "  Browser name (e.g. Microsoft Edge, Google Chrome, Firefox, Safari)\n"
             << "  [" << (browser.empty() ? "Microsoft Edge" : browser) << "]: " << flush;
        getline(cin, input);
        if (!input.empty())
            browser = input;
        else if (browser.empty())
            browser = "Microsoft Edge";
    } else {
        browser = "";
    }

    // Summary + confirm
    cout << "\nSettings to save:\n"
         << "  open         = " << (useGateway ? "g (BibleGateway.com)" : "e (ESV.org)") << "\n";
    if (useGateway)
        cout << "  bibleversion = " << bver << "\n";
    cout << "  youtube      = " << (openYoutube ? "yes" : "no") << "\n"
         << "  browser      = " << (browser.empty() ? "(system default)" : browser) << "\n"
         << "\nSave to " << configPath() << "? [Y/n]: " << flush;
    getline(cin, input);
    if (!input.empty() && (input[0] == 'n' || input[0] == 'N')) {
        cout << "Setup cancelled.\n";
        return;
    }

    vector<string> lines = {
        "open         = " + string(useGateway ? "g" : "e"),
        "bibleversion = " + (useGateway ? bver : ""),
        "youtube      = " + string(openYoutube ? "yes" : "no")
    };
    if (!writeSection(lines)) {
        cerr << "tbr: could not write '" << configPath() << "'\n";
        return;
    }
    if (!writeGlobalKey("browser", browser)) {
        cerr << "tbr: warning: could not save browser setting\n";
    }
    cout << "Saved [tbr] to " << configPath() << "\n";
}

int main(int argc, char* argv[]) {
    // Load config defaults
    string openPref    = readConfig("tbr", "open");
    string bver        = readConfig("tbr", "bibleversion");
    string youtubePref = readConfig("tbr", "youtube");
    string browser     = readGlobalConfig("browser");
    if (openPref.empty()) openPref = "e";

    bool useGateway  = (openPref == "g");
    bool openYoutube = (youtubePref != "no");
    bool saveConfig  = false;
    bool showConfig  = false;
    int  targetDay   = dayOfYearWithOffset(0);  // default: today

    if (argc > 1 && string(argv[1]) == "setup") {
        runSetup(useGateway, bver, openYoutube, browser);
        return 0;
    }

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            cout << "tbr — Today's Bible Reading\n\n"
                 << "Usage: tbr [DATE|OFFSET] [-e | -g | -g=VERSION] [-n] [-y]\n\n"
                 << "  (no args)          Open today's reading\n"
                 << "  N or +N            N days ahead of today\n"
                 << "  -N                 N days before today\n"
                 << "  mm/dd/yyyy         Specific date\n"
                 << "  -e                 Open on ESV.org\n"
                 << "  -g                 Open on BibleGateway.com\n"
                 << "  -g=VERSION         Open on BibleGateway with a specific version (e.g. -g=NKJV)\n"
                 << "  -n, --no-youtube   Skip opening YouTube\n"
                 << "  -y, --youtube      Open YouTube (overrides config youtube = no)\n"
                 << "  --showconfig       Print current effective settings and exit\n"
                 << "  --saveconfig       Save current settings to ~/.verselumen [tbr]\n"
                 << "  setup              Interactive setup wizard\n\n"
                 << "Config (~/.verselumen):\n"
                 << "  [tbr]\n"
                 << "  open = g            # e = ESV.org (default), g = BibleGateway\n"
                 << "  bibleversion = NKJV\n"
                 << "  youtube = no        # set to no to skip YouTube by default\n\n"
                 << "Examples:\n"
                 << "  tbr                        # today's reading\n"
                 << "  tbr -1                     # yesterday\n"
                 << "  tbr 3                      # 3 days from today\n"
                 << "  tbr 5/19/2026              # specific date\n"
                 << "  tbr -g=NKJV                # today on BibleGateway, NKJV\n"
                 << "  tbr -1 -g=NKJV             # yesterday on BibleGateway, NKJV\n"
                 << "  tbr -n                     # today's reading, no YouTube\n"
                 << "  tbr -g=NKJV --saveconfig   # save BibleGateway + NKJV as defaults\n"
                 << "  tbr setup                  # interactive setup wizard\n";
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            cout << "tbr v1.1\n";
            return 0;
        } else if (arg == "--saveconfig") {
            saveConfig = true;
        } else if (arg == "--showconfig") {
            showConfig = true;
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

    if (showConfig) {
        cout << "Effective settings:\n"
             << "  open         = " << (useGateway ? "g" : "e")
             << " (" << (useGateway ? "BibleGateway.com" : "ESV.org") << ")\n"
             << "  bibleversion = " << (bver.empty() ? "(not set)" : bver) << "\n"
             << "  youtube      = " << (openYoutube ? "yes" : "no") << "\n"
             << "  browser      = " << (browser.empty() ? "(system default)" : browser) << "\n";
        string path = configPath();
        ifstream check(path);
        cout << "\nConfig file: " << path
             << (check.good() ? " (loaded)" : " (not found — using defaults)") << "\n";
        return 0;
    }

    if (saveConfig) {
        vector<string> lines = {
            "open         = " + string(useGateway ? "g" : "e"),
            "bibleversion = " + (bver.empty() ? "" : bver),
            "youtube      = " + string(openYoutube ? "yes" : "no")
        };
        if (!writeSection(lines)) {
            cerr << "tbr: could not write '" << configPath() << "'\n";
            return 1;
        }
        cout << "Saved [" << SECTION << "] to " << configPath() << "\n";
        return 0;
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
