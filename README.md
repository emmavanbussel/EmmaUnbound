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
| Pick a level | 1, 2, 3 or 4 | |

Collect orbs, stomp or punch the enemies, and reach the golden star. Flags are checkpoints and refill your hearts.

**Level 1, Heidestein.** Heath and pine forest: purple heather, pines, sandy plains with fallen logs and pines on stilt roots, two misty lakes with reeds and birches (one crossed on a floating fallen pine), shy roe deer that bolt when you come near, wolves, a schaapskooi with sheep, the Theehuis by the big lake, and mushrooms to bounce on.

**Level 2, Viñales.** Dusk over red-brown earth, with mogotes behind. A village with cottages, power lines and the old yellow bus, a tobacco shed and field, then a broken plank bridge over the muddy river (Emma cannot swim). After that the level climbs up and to the right: lianas to climb, plank platforms on ropes to ride, a drum to bounce on, crabs and beetles.

**Level 3, Lofoten.** Icy islands under the northern lights. The ground is slippery and the slopes pull you downhill. Curved bridges and a fishing boat link the islands; falling in the sea sends you back to the checkpoint. Polar bears need two hits and get faster after the first.

**Level 4, Japan.** A journey from the ryokan in Izu Nagaoka to Tokyo, with the day turning to dusk and night on the way: Mount Fuji, red bridges, the tunnel of red gates, a temple hall, lanterns and a pagoda in Kyoto, the pagoda, cedars and rock garden of Koyasan, a boat past the great gate in the sea at Hiroshima, Himeji's white castle, then Tokyo's tower and rooftops and a room of hanging lights you climb. A shinkansen passes in the background. Wild boars and crows get in the way, and the gravel of Koyasan's rock garden must not be touched: a monk guards it, and you cross on the planks above.

Add `#level2`, `#level3` or `#level4` to the address (for example `.../index.html#level4`) to start in that level.

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
| `src/game.hpp`, `src/game.cpp` | Game rules: player physics, slopes, bridges, lianas, platforms, water, fist, enemies, pickups, camera. Tuning constants are at the top of `game.hpp` |
| `src/world.cpp` | Sky, background scenery, terrain and level scenery, with the colours for each level's theme |
| `src/japan.cpp` | Everything drawn only in level 4: background, scenery, the red bridge, the room of lights |
| `src/actors.cpp` | Emma, enemies, platforms (floating pine, rope platform, boat), lianas and pickups |
| `src/render.cpp`, `src/draw.hpp` | Frame assembly, HUD, and shared drawing declarations |
| `src/gfx.hpp`, `src/gfx.cpp` | Thin C++ wrapper around the browser's Canvas 2D API |
| `src/level.hpp` | The four levels as ASCII maps (legend at the top of the file) |
| `tools/make_level.py` | Script that generates `level.hpp` |
| `web/index.html` | Page template, copied into `docs/` by the build |
| `docs/` | Build output served by GitHub Pages |

The art is procedural vector shapes, so there are no image files.

## Edit the levels

Open `src/level.hpp` and change the characters; the legend is at the top of the file. Each character is a 48-pixel tile.
Keep every row of a level the same length, then run `./build.sh`. (Running `tools/make_level.py` overwrites hand edits.)
