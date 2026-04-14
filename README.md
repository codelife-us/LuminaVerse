# gospel

A C++ program that displays various gospel presentation tracts, biblical frameworks for sharing the Gospel of Jesus Christ. The program presents different gospel presentations with biblical explanations and references.

## Features

- Support for multiple gospel presentation tracts/styles
- Currently includes "The Romans Road" (default)
- Easy to add new tracts by extending the code
- Display tract content with biblical explanations
- Support for multiple Bible translations:
  - **KJV** (King James Version) - default
  - **BSB** (Berean Standard Bible)
- Look up individual Bible verses, verse ranges, or entire chapters directly with `--ref`
- Multiple references supported using comma separation
- Optional verse numbers prefixed to each verse with `--versenumbers` / `-vn`
- Configurable citation style (attribution on new line or inline)
- Command-line configurable tract name and Bible version
- Auto-prompts to download the Bible translation file if not found

## Building

```bash
g++ -std=c++11 -o gospel gospel.cpp
```

## Usage

Show help information:
```bash
./gospel -h
./gospel --help
```

Show version information:
```bash
./gospel -v
./gospel --version
```

Run with default KJV translation and default tract:
```bash
./gospel
```

Specify a different Bible version:
```bash
./gospel -bv=BSB
./gospel --bibleversion=BSB
```

Specify a different tract presentation:
```bash
./gospel -tn="The Romans Road"
./gospel --tractname="The Romans Road"
```

Combine tract and Bible version options:
```bash
./gospel -tn="The Romans Road" -bv=BSB
./gospel --tractname="The Romans Road" --bibleversion=BSB
```

Output as Markdown:
```bash
./gospel --outputtype=md > romansroad.md
```

Look up a single Bible reference directly (bypasses tract output):
```bash
./gospel --ref="John 3:16"
./gospel --ref="Romans 10:9-10" -bv=BSB
```

Look up multiple references using a comma to separate each:
```bash
./gospel --ref="John 3:16,Romans 5:8"
./gospel --ref="Romans 3:23,Romans 6:23,Romans 10:9-10"
```

Look up an entire chapter:
```bash
./gospel --ref="Romans 8"
./gospel --ref="John 3" -bv=BSB
```

Show verse numbers prefixed to each verse:
```bash
./gospel --ref="Romans 8" --versenumbers
./gospel --ref="John 3:16,Romans 5:8" -vn
```

Change the citation style (default is style 1):
```bash
# Style 1 (default): citation on its own line below the verse
./gospel --ref="John 3:16" --refstyle=1

# Style 2: citation inline after the verse text
./gospel --ref="John 3:16" --refstyle=2
```

Style 1 output example:
```
For God so loved the world...
— John 3:16 (KJV)
```

Style 2 output example:
```
For God so loved the world... - John 3:16 (KJV)
```

## Copying Output

The program output can be easily copied to the clipboard using piping:

**macOS:**
```bash
./gospel | pbcopy
```

**Linux:**
```bash
./gospel | xclip -selection clipboard
# or
./gospel | xsel -b
```

This is useful for sharing gospel presentations or copying the content for use in documents, emails, or other applications.

## Bible Translations

- **KJV**: King James Version — `BibleKJV.txt` (downloaded from https://openbible.com/textfiles/kjv.txt)
- **BSB**: Berean Standard Bible — `BibleBSB.txt` (downloaded from https://bereanbible.com/bsb.txt)

If a Bible translation file is not found, the program will prompt you to download it automatically using `curl`.

## Files

- `gospel.cpp` - Main program source code
- `BibleKJV.txt` - KJV Bible text (downloaded on first use)
- `BibleBSB.txt` - BSB Bible text (downloaded on first use)

## Requirements

- C++11 or later
- Standard C++ library
- `curl` (for automatic Bible file download)
