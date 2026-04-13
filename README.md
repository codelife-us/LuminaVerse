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
- Command-line configurable tract name and Bible version

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

- **KJV**: King James Version
- **BSB**: Berean Standard Bible

## Files

- `gospel.cpp` - Main program source code

## Requirements

- C++11 or later
- Standard C++ library
