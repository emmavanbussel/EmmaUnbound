# Emma Unbound

A small cartoon platformer written in C++ and compiled to WebAssembly with Emscripten.
Emma has no neck, arms or legs: her head, hands and feet float freely.

## Play

| Action | Keys | Touch |
| --- | --- | --- |
| Move | Arrow keys or A / D | ◀ ▶ |
| Jump (hold while falling to glide) | Space, Z, Up or W | JUMP |
| Climb a liana | Up / Down (or W / S) | ▲ ▼ |
| Leap off a liana | Space or Z, with a direction | JUMP |
| Punch (flying fist) | X, J or K | ✊ |
| Pick a level | 1 or 2 | |

Collect orbs, stomp or punch the enemies, and reach the golden star. Flags are checkpoints and refill your hearts.

**Level 1, Heidestein.** Heath and pine forest: purple heather, pines, wolves, a schaapskooi with sheep, mushrooms to bounce on and a drifting cloud.

**Level 2, Viñales.** A climb up and to the right over red earth and mogote hills: lianas to climb, floating fruit to ride (lifts and a ferry), a conga drum to bounce on, crabs and beetles. Jump at a liana to grab it.

Add `#level2` to the address (for example `.../index.html#level2`) to start in level 2.

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
| `src/game.hpp`, `src/game.cpp` | Game rules: player physics, lianas, platforms, fist, enemies, pickups, camera. Tuning constants are at the top of `game.hpp` |
| `src/world.cpp` | Sky, background scenery, terrain and level scenery, with the colours for each level's theme |
| `src/actors.cpp` | Emma, enemies, platforms, lianas and pickups |
| `src/render.cpp`, `src/draw.hpp` | Frame assembly, HUD, and shared drawing declarations |
| `src/gfx.hpp`, `src/gfx.cpp` | Thin C++ wrapper around the browser's Canvas 2D API |
| `src/level.hpp` | Both levels as ASCII maps (legend at the top of the file) |
| `tools/make_level.py` | Script that generates `level.hpp` |
| `web/index.html` | Page template, copied into `docs/` by the build |
| `docs/` | Build output served by GitHub Pages |

The art is procedural vector shapes, so there are no image files.

## Edit the levels

Open `src/level.hpp` and change the characters; the legend is at the top of the file. Each character is a 48-pixel tile.
Keep every row of a level the same length, then run `./build.sh`. (Running `tools/make_level.py` overwrites hand edits.)
