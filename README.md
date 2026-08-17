# Kotte

Kotte is a small C++ game engine and teaching project built with [raylib](https://www.raylib.com/).

It is a work in progress currently being developed for the course [Scalable Game Worlds in C++](https://www.uu.se/en/study/syllabus?query=6751b031-a906-11f0-b7df-f9101f074b4e) at Uppsala University.

The project uses C++26, raylib 6.0, and the C++ standard library to explore scalable game architecture and implementation. Each course week evolves one cumulative tile world in response to an observable scaling problem.

## Requirements

* Visual Studio Community 2026
* raylib 6.0

## Current demo

Run the x64 Debug or Release configuration. Move the player with WASD or the arrow keys, and quit with Q or Escape.

The Week 5 demo contains a few thousand fixed-lifetime player, bomb, crate, and enemy entities in the 200 by 120 tile world. The player moves continuously in world pixels, and every enemy wanders in one of the four cardinal directions. The raw Bombman sprites are not loaded yet; coloured primitives keep this stage focused on architecture.

The entity vector owns each world entity exactly once. A separate 25 by 15 uniform grid covers the 8,000 by 4,800 pixel world with 320-pixel cells. Each cell stores non-owning entity indices for every entity whose complete world bounds overlap that cell. These indices are safe under the current fixed-lifetime rule: the entity vector is not erased, reordered, or structurally changed during play.

Collision reuses the same grid independently of the camera. For each proposed player axis or enemy movement, Kotte first checks the room boundary and then queries only the cells touched by the proposed gameplay collision rectangle. The spatial result is a broad-phase candidate list, not a collision answer: Kotte removes the moving entity and non-solid kinds before exact-testing the remaining candidates' collision rectangles.

Gameplay collision footprints are smaller lower rectangles around each entity's contact with the floor. The complete visual rectangles remain in the grid for conservative candidate selection and are still used for visibility and rendering. Keeping these representations separate prevents a crate's tall presentation shape from becoming an oversized collision obstacle.

Players, crates, and enemies are solid. The inert bomb prototypes are non-solid so the filtering stage remains observable; active bomb placement and its more specific blocking rules are deferred. A player proposal into a crate or enemy rejects only that movement axis, which allows sliding along a clear axis. An enemy proposal into a crate, the player, another enemy, or the room boundary leaves the enemy in place and selects a different cardinal heading for the next update. Contact does not push, damage, destroy, or remove either entity.

The player updates first and synchronizes its old and final visual bounds once after both axes. Kotte then visits a fixed `enemy_indices_` list in entity-vector order, regardless of camera visibility. Each enemy makes one movement attempt: an accepted move is synchronized in the spatial grid immediately so the next enemy sees its current location, while a blocked enemy turns without changing position or membership. This simple sequential policy is deterministic for the same seed, input, and frame-time sequence, although it intentionally gives earlier and later enemies slightly different views of the current update.

Enemy headings are initialized only after the complete entity-population loop. The additional random draws therefore do not change the Week 4 seeded world layout or entity-kind sequence. Headings are small authoritative entity state; the spatial grid and enemy-index list remain derived, non-owning indexes.

After all movement is resolved and synchronized, the camera view maps to its own clamped, half-open spatial-cell range. Kotte gathers entity references from only those cells, sorts and removes duplicate indices, and exact-tests the remaining candidates against the camera rectangle. Gameplay never consumes this visibility result, so enemies continue moving, colliding, turning, and changing spatial cells while off-screen.

World entities still do not draw themselves. Exact camera overlaps become owning `RenderCommand` values, the renderer sorts those commands by layer and ground-y depth, and raylib executes them. Bombs use the ground layer, while standing objects sort from top to bottom within the world layer. Week 4 changes the source of visibility candidates without changing the Week 3 extraction, sorting, or execution policy.

The tile map retains its more direct Week 2 selection. It converts the camera view into a clamped, half-open tile range and visits only that range. Tile terrain and dynamic entity membership remain separate structures because they have different ownership and update rules.

The on-screen diagnostics show:

* the complete map dimensions and tile count;
* the camera's world-space view rectangle;
* the selected column and row ranges;
* the number of rendered tiles as a percentage of the complete world;
* the spatial-grid dimensions and cell size;
* camera-query cells visited and raw candidate references gathered;
* total entities, exact visibility tests, visible entities, and render commands;
* enemy updates, movement attempts, blocked moves, and enemy turns;
* collision queries, cells visited, raw references, and unique candidates;
* exact collision tests, contacts, and room-boundary blocks.

The original Week 1 baseline remains available from the `2026-week-01` tag. The Week 2 camera and tile-culling reference remains available from `2026-week-02`. The Week 3 entity-rendering reference remains available from `2026-week-03`, and the Week 4 spatial-query reference remains available from `2026-week-04`.

Movement still uses the current variable-time update: normalized direction multiplied by speed and frame delta time. Ordinary player and enemy steps are small compared with the tile-sized obstacles, so Week 5 does not introduce a fixed-timestep accumulator or movement substeps without a demonstrated need. A sufficiently large frame delta can theoretically move an entity completely through a blocker; this discrete-collision limitation is accepted and documented rather than hidden.

Bomb placement, fuse timers, four-direction tile propagation, crate destruction, stable identity, events, and deferred structural mutation remain later problems. Those mechanics will extend the same off-screen simulation rule beyond persistent enemy movement and create a concrete reason to revisit lifetime and event architecture.

## Random numbers

Kotte uses the open-source [`Random<RomuDuoJr>`](https://github.com/ulfben/cpp_prngs) random-number generator. It is fast, compact, easy to seed, deterministic across platforms, compatible with standard-library algorithms, and provides convenient game-oriented operations such as ranges, normalized values, coin flips, and selecting elements from collections.

## Status

Week 5 reference implementation: camera-independent collision broad phase, exact gameplay footprints, solid-category filtering, axis-rejecting player response, stop-and-turn autonomous enemies, immediate moving-entity grid synchronization, and separate collision diagnostics. Fixed simulation timing, stable entity handles, gameplay lifetimes, damage, bomb mechanics, and sprite/asset management are intentionally deferred to later course work.
