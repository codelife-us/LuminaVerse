# tbr

A C++ utility that opens today's Bible reading plan in the browser (via `bv`) and launches the daily YouTube video search (via `day -a`).

## Building

**macOS / Linux:**
```bash
g++ -std=c++11 -o tbr tbr.cpp
```

**Windows (MSVC):**
```bat
cl /EHsc /std:c++17 /utf-8 tbr.cpp /Fe:tbr.exe
```

## Usage

```bash
tbr                  # today's reading
tbr -1               # yesterday
tbr 3                # 3 days from today
tbr +3               # same as above
tbr 5/19/2026        # specific date (mm/dd/yyyy)

tbr -e               # open on ESV.org (default)
tbr -g               # open on BibleGateway.com
tbr -g=NKJV          # open on BibleGateway with a specific version
tbr -1 -g=NKJV       # combine date offset and version
tbr -n               # today's reading, no YouTube
tbr -1 -n            # yesterday, no YouTube
```

## Options

| Argument | Description |
|----------|-------------|
| *(none)* | Use today's date |
| `N` or `+N` | N days ahead of today |
| `-N` | N days before today |
| `mm/dd/yyyy` | Specific date |
| `-e` | Open on ESV.org |
| `-g` | Open on BibleGateway.com |
| `-g=VERSION` | Open on BibleGateway with a specific version (e.g. `NKJV`, `NIV`, `ESV`) |
| `-n`, `--no-youtube` | Skip opening YouTube |
| `-y`, `--youtube` | Open YouTube (overrides `youtube = no` in config) |

All flags and the date/offset can appear in any order.

## Config file (.verselumen)

Set your preferred defaults in `~/.verselumen` under a `[tbr]` section so you don't have to pass flags every time. Command-line flags override the config for that run.

```ini
[tbr]
open = g
bibleversion = NKJV
youtube = no
```

| Key | Values | Default |
|-----|--------|---------|
| `open` | `e` (ESV.org) or `g` (BibleGateway) | `e` |
| `bibleversion` | Any version code BibleGateway supports (e.g. `NKJV`, `NIV`, `ESV`) | *(none)* |
| `youtube` | `yes` (default) or `no` to skip YouTube by default | `yes` |

## How it works

`tbr` resolves the argument to a day number (1–365) and passes it to both `bv -d=N` (opens the reading in a browser) and `day -d=N -a` (opens YouTube in the browser configured by `browser=` in `.verselumen`). With no argument it uses today's day number.

## Dependencies

- [`bv`](bv.md) — Bible verse lookup
- [`day`](day.md) — day-of-year utility with YouTube integration
