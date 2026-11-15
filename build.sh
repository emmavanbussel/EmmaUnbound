#!/usr/bin/env bash
# Builds the game into docs/ (the folder GitHub Pages serves).
# Needs the Emscripten SDK on your PATH: https://emscripten.org/docs/getting_started/downloads.html
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p docs
em++ -std=c++17 -O2 -Wall -Wextra \
  src/main.cpp src/game.cpp src/render.cpp src/gfx.cpp \
  -sENVIRONMENT=web -sEXPORTED_FUNCTIONS=_main,_emma_button,_emma_state \
  -o docs/emma.js
cp web/index.html docs/index.html
touch docs/.nojekyll
echo "Built docs/. Test locally with:  python3 -m http.server -d docs 8000"
