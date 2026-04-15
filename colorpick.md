# colorpick

An interactive terminal color picker. Navigate a hue × saturation grid with arrow keys, adjust brightness with `+` and `-`, and press Enter to select. The chosen color is printed to stdout as a hex string (`#RRGGBB`).

## Features

- Full hue × saturation grid rendered with ANSI true-color (24-bit)
- Brightness slider with live gradient preview
- Live color swatch with hex and RGB values
- TUI draws to stderr; result goes to stdout — works cleanly with shell capture
- Cursor wraps horizontally across hues
- Quit without selecting returns exit code 1 with no output

## Building

```bash
g++ -std=c++11 -o colorpick colorpick.cpp
```

## Usage

Run interactively:
```bash
./colorpick
```

Capture the result into a shell variable:
```bash
COLOR=$(./colorpick)
echo $COLOR
# → #FF8040
```

Pipe directly into bvi:
```bash
bvi "John 3:16" --bg=$(./colorpick)
bvi "Romans 8:28" --textcolor=$(./colorpick)
```

Set all three colors interactively:
```bash
bvi "John 3:16" \
  --bg=$(./colorpick) \
  --textcolor=$(./colorpick) \
  --citecolor=$(./colorpick)
```

Pick colors and save as bvi defaults:
```bash
bvi \
  --bg=$(./colorpick) \
  --textcolor=$(./colorpick) \
  --citecolor=$(./colorpick) \
  --saveconfig
```

## Controls

| Key | Action |
|---|---|
| `←` `→` | Move through hues |
| `↑` `↓` | Adjust saturation |
| `+` or `=` | Increase brightness |
| `-` or `_` | Decrease brightness |
| `Enter` | Select color — prints `#RRGGBB` to stdout |
| `q` `Q` `Esc` | Quit without selecting (exit code 1, no output) |

## Layout

```
  colorpick  –  ←→ hue  ↑↓ saturation  +/- brightness  Enter select  q quit

  [full hue reference strip]

  [36 × 18 hue × saturation grid]
       ><  ← cursor

  Brightness  [gradient bar with position indicator]  75%  (+ / -)

  ██████████  #FF8040    R:255  G:128  B: 64
```

- **Row 1** — title and key reference
- **Row 2** — hue strip showing the full spectrum at maximum saturation and brightness
- **Rows 3–20** — main grid: hue across (0°–350° in 10° steps), saturation down (100% to ~5%)
- **Row 21** — brightness slider; left portion shows the color gradient, right portion is dimmed
- **Row 23** — live preview swatch, hex value, and RGB components

## Shell Integration

Because the TUI is drawn to stderr and the result is printed to stdout, the picker works naturally in command substitution without any extra flags:

```bash
# The TUI appears on your terminal; the hex value is captured silently
BG=$(./colorpick)
TEXT=$(./colorpick)
CITE=$(./colorpick)

bvi "Philippians 4:6-7" --bg=$BG --textcolor=$TEXT --citecolor=$CITE
```

Exit code is `0` on selection and `1` on quit — usable in scripts:
```bash
COLOR=$(./colorpick) && bvi "John 3:16" --bg=$COLOR || echo "cancelled"
```

## Requirements

- C++11 or later
- A terminal with ANSI true-color support (Terminal.app, iTerm2, Windows Terminal, most modern Linux terminals)
