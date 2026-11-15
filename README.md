# Emma Unbound

A small cartoon platformer written in C++ and compiled to WebAssembly with Emscripten.
Emma has no neck, arms or legs: her head, hands and feet float freely.

## Play

| Action | Keys | Touch |
| --- | --- | --- |
| Move | Arrow keys or A / D | ◀ ▶ |
| Jump (hold while falling to glide) | Space, Up, W or Z | ▲ |
| Punch (flying fist) | X, J or K | ✊ |

Collect orbs, bounce on mushrooms, stomp or punch the grumpy critters, and reach the golden star.
Flags are checkpoints and refill your hearts.

## Publish on GitHub Pages

The ready-to-serve build is committed in `docs/`.

1. Push this repository to GitHub.
2. In the repository go to **Settings → Pages**.
3. Under **Build and deployment**, choose **Deploy from a branch**, branch `main`, folder `/docs`, and save.

The game appears at `https://<your-user>.github.io/<repository>/` after a minute or so.

## Build

Install the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html), then:

```sh
./build.sh
python3 -m http.server -d docs 8000   # open http://localhost:8000
```

The page has to be served over HTTP; opening `docs/index.html` straight from disk will not load the `.wasm` file.

## Layout

| Path | What it is |
| --- | --- |
| `src/main.cpp` | Main loop (fixed 120 Hz physics step) and keyboard / touch input |
| `src/game.hpp`, `src/game.cpp` | Game rules: player physics, fist, enemies, pickups, camera. Tuning constants are at the top of `game.hpp` |
| `src/render.cpp` | All drawing. The art is procedural vector shapes, so there are no image files |
| `src/gfx.hpp`, `src/gfx.cpp` | Thin C++ wrapper around the browser's Canvas 2D API |
| `src/level.hpp` | The level as an ASCII map (legend at the top of the file) |
| `tools/make_level.py` | Optional script that generated `level.hpp` |
| `web/index.html` | Page template, copied into `docs/` by the build |
| `docs/` | Build output served by GitHub Pages |

## Edit the level

Open `src/level.hpp` and change the characters. Each one is a 48-pixel tile:
`#` ground, `=` plank, `^` thorns, `o` orb, `M` mushroom, `~` cloud platform,
`E` walking enemy, `F` flying enemy, `C` checkpoint, `P` player start, `G` goal.
Keep every row the same length, then run `./build.sh`.
