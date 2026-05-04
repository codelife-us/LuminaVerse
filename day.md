# day

A small C++ utility that prints the current day of the year (Jan 1 = 1).

## Building

```bash
g++ -std=c++11 -o day day.cpp
```

## Usage

```bash
./day                  # print day number of today
./day -r               # print Bible reference only
./day -p               # print day number of today, date, and Bible reference
./day -y               # print day number and open YouTube search
./day -d=4/30/2026 -r  # print Bible reference only for a specific date
```

## Options

| Option | Description |
|--------|-------------|
| `-d=N`, `--day=N` | Use day N instead of today |
| `-d=mm/dd/yyyy` | Use a date instead of a day number (4-digit or 2-digit year) |
| `-y`, `--youtube` | Open YouTube Bible Recap search |
| `-q=TEXT`, `--query=TEXT` | Override the YouTube search query (`{day}` = day number); implies `-y` |
| `-r`, `--refonly` | Print Bible reference only |
| `-p`, `--plan` | Print day number, date, and Bible reference |
| `-c`, `--csv` | Output as CSV: `day,date,"reference"` |
| `-t`, `--tab` | Output as TSV: `day<TAB>date<TAB>reference` |
| `-v`, `--version` | Print version |
| `-h`, `--help` | Show help |

## Config file (.day)

Create a `.day` file in the current directory or `$HOME` to set a default YouTube search query. The first non-blank, non-comment line is used. Lines starting with `#` are ignored.

```
# .day — default YouTube search query for day
Day {day} The Bible Recap
```

Query priority: `-q=` flag → `.day` in current dir → `.day` in `$HOME` → built-in default (`Day {day} The Bible Recap`).

## Examples

Print today's day number:
```bash
./day
```

Open YouTube for today's Bible Recap:
```bash
./day -y
```

Open YouTube for a specific day or date:
```bash
./day -d=203 -y
./day -d=4/30/2026 -y
./day -d=12/25/25 -y
```

Custom search query (opens YouTube automatically):
```bash
./day -q="Day {day} The Bible Recap"
./day --query="Day {day} The Bible Recap"
```

Print today's Bible reference only:
```bash
./day -r
./day -d=3/21/2026 -r
```

Print today's reading plan (day number, date, reference):
```bash
./day -p
```

Output today as a CSV row:
```bash
./day --csv
./day -d=203 --csv
```

Output today as a tab-delimited row:
```bash
./day --tab
./day -d=203 --tab
```

## Composing with other tools

```bash
./bv -d=$(./day)          # pipe day number into bv
./day -r && ./day -y      # print reference then open YouTube
```
