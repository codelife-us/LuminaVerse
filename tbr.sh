#!/bin/sh
# tbr - Today's Bible Reading
# Usage: tbr [DATE|OFFSET] [-e | -g | -g=VERSION]
#   DATE    = mm/dd/yyyy
#   OFFSET  = integer days from today (e.g. -1 for yesterday, 3 for 3 days ahead)
#   -e      = open on ESV.org
#   -g      = open on BibleGateway.com
#   -g=VER  = open on BibleGateway with a specific version (e.g. -g=NKJV)
#   -n      = skip opening YouTube
#
# Defaults can be set in ~/.verselumen under [tbr]:
#   [tbr]
#   open = g
#   bibleversion = NKJV

# Read a key from [tbr] section of nearest .verselumen
read_tbr_config() {
    local key="$1" file
    for file in ".verselumen" "$HOME/.verselumen"; do
        [ -f "$file" ] || continue
        awk -v k="$key" '
            /^\[tbr\]/ { s=1; next } /^\[/ { s=0 }
            s && $0 ~ "^[[:space:]]*"k"[[:space:]]*=" {
                sub(/^[^=]*=[[:space:]]*/, ""); print; exit
            }
        ' "$file"
        return
    done
}

# Load config defaults
OPEN=$(read_tbr_config "open")
BVER=$(read_tbr_config "bibleversion")
YTPREF=$(read_tbr_config "youtube")
[ -z "$OPEN" ] && OPEN="e"
[ "$YTPREF" = "no" ] && YOUTUBE=0 || YOUTUBE=1

# Parse arguments (order doesn't matter)
DAY_ARG=""
for arg in "$@"; do
    case "$arg" in
        -e)                    OPEN="e"; BVER="" ;;
        -g)                    OPEN="g" ;;
        -g=*)                  OPEN="g"; BVER="${arg#-g=}" ;;
        -n|--no-youtube)       YOUTUBE=0 ;;
        -y|--youtube)          YOUTUBE=1 ;;
        *)                     DAY_ARG="$arg" ;;
    esac
done

# Build the bv browser flag
if [ "$OPEN" = "g" ]; then
    OPEN_FLAG="-g${BVER:+ -bv=$BVER}"
else
    OPEN_FLAG="-e"
fi

# Resolve day number (empty = today)
if [ -z "$DAY_ARG" ]; then
    bv -d $OPEN_FLAG
    [ "$YOUTUBE" -eq 1 ] && day -a
else
    case "$DAY_ARG" in
        */*)
            DAY=$(date -jf "%m/%d/%Y" "$DAY_ARG" "+%j" | sed 's/^0*//')
            ;;
        *)
            OFFSET=$(printf '%s' "$DAY_ARG" | sed 's/^+//')
            if ! printf '%s' "$OFFSET" | grep -qE '^-?[0-9]+$'; then
                printf 'tbr: invalid argument "%s"\n' "$DAY_ARG" >&2
                printf 'Usage: tbr [mm/dd/yyyy | offset] [-e | -g | -g=VERSION]\n' >&2
                exit 1
            fi
            case "$OFFSET" in
                -*) ADJ="${OFFSET}d" ;;
                 *) ADJ="+${OFFSET}d" ;;
            esac
            DAY=$(date -v${ADJ} "+%j" | sed 's/^0*//')
            ;;
    esac
    bv -d="$DAY" $OPEN_FLAG
    [ "$YOUTUBE" -eq 1 ] && day -d="$DAY" -a
fi
