# Kotte

Kotte is a small C++ game engine and teaching project built with [raylib](https://www.raylib.com/).

It is a work in progress currently being developed for the course [Scalable Game Worlds in C++](https://www.uu.se/en/study/syllabus?query=6751b031-a906-11f0-b7df-f9101f074b4e) at Uppsala University.

The project uses C++26, raylib 6.0, and the C++ standard library to explore scalable game architecture and implementation. Each course week evolves one cumulative tile world in response to an observable scaling problem.

## Requirements

* Visual Studio Community 2026
* raylib 6.0

## Current demo

Run the x64 Debug or Release configuration. Move the player with WASD or the arrow keys, and quit with Q or Escape.

The Week 4 demo contains a few thousand fixed-lifetime player, bomb, crate, and enemy entities in the 200 by 120 tile world. The player moves continuously in world pixels while the other entity prototypes remain inert. The raw Bombman sprites are not loaded yet; coloured primitives keep this stage focused on architecture.

The entity vector owns each world entity exactly once. A separate 25 by 15 uniform grid covers the 8,000 by 4,800 pixel world with 320-pixel cells. Each cell stores non-owning entity indices for every entity whose complete world bounds overlap that cell. These indices are safe under the current fixed-lifetime rule: the entity vector is not erased, reordered, or structurally changed during play.

After the player moves, Kotte updates its old and new grid membership before the camera or renderer queries the world. The camera view maps directly to a clamped, half-open spatial-cell range. Kotte gathers entity references from only those cells, sorts and removes duplicate indices, and exact-tests the remaining candidates against the camera rectangle.

World entities still do not draw themselves. Exact camera overlaps become owning `RenderCommand` values, the renderer sorts those commands by layer and ground-y depth, and raylib executes them. Bombs use the ground layer, while standing objects sort from top to bottom within the world layer. Week 4 changes the source of visibility candidates without changing the Week 3 extraction, sorting, or execution policy.

The tile map retains its more direct Week 2 selection. It converts the camera view into a clamped, half-open tile range and visits only that range. Tile terrain and dynamic entity membership remain separate structures because they have different ownership and update rules.

The on-screen diagnostics show:

* the complete map dimensions and tile count;
* the camera's world-space view rectangle;
* the selected column and row ranges;
* the number of rendered tiles as a percentage of the complete world;
* the spatial-grid dimensions and cell size;
* cells visited and raw candidate references gathered;
* total entities, exact visibility tests, visible entities, and render commands.

The original Week 1 baseline remains available from the `2026-week-01` tag. The Week 2 camera and tile-culling reference remains available from `2026-week-02`. The Week 3 entity-rendering reference remains available from `2026-week-03`.

The spatial grid covers the complete world and is not limited to the camera. A later collision system can issue its own world-space queries for a proposed movement rectangle or bomb blast. It reuses the grid and query operation, not the camera's possibly-visible candidate list; off-screen gameplay therefore remains independent of rendering.

## Random numbers

Kotte uses the open-source [`Random<RomuDuoJr>`](https://github.com/ulfben/cpp_prngs) random-number generator. It is fast, compact, easy to seed, deterministic across platforms, compatible with standard-library algorithms, and provides convenient game-oriented operations such as ranges, normalized values, coin flips, and selecting elements from collections.

## Status

Week 4 reference implementation: synchronized full-bounds entity membership, uniform-grid visibility candidates, exact culling, render extraction, and layer/depth sorting. Collision detection/response, fixed simulation timing, stable entity handles, gameplay lifetimes, and sprite/asset management are intentionally deferred to later course work.
