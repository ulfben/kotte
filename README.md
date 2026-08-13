# Kotte

Kotte is a small C++ game engine and teaching project built with [raylib](https://www.raylib.com/).

It is a work in progress currently being developed for the course [Scalable Game Worlds in C++](https://www.uu.se/en/study/syllabus?query=6751b031-a906-11f0-b7df-f9101f074b4e) at Uppsala University.

The project uses C++26, raylib 6.0, and the C++ standard library to explore scalable game architecture and implementation. Each course week evolves one cumulative tile world in response to an observable scaling problem.

## Requirements

* Visual Studio Community 2026
* raylib 6.0

## Current demo

Run the x64 Debug or Release configuration. Move the player with WASD or the arrow keys, and quit with Q or Escape.

The Week 3 demo adds a few thousand fixed-lifetime player, bomb, crate, and enemy entities to the 200 by 120 tile world. The player moves continuously in world pixels while the other entity prototypes remain inert. The raw Bombman sprites are not loaded yet; coloured primitives keep this week focused on rendering architecture.

World entities do not draw themselves. Kotte scans the entity vector, rejects bounds outside the camera's world-space view, extracts visible entities into owning `RenderCommand` values, sorts the commands by layer and ground-y depth, and then executes raylib drawing. Bombs use the ground layer, while standing objects sort from top to bottom within the world layer.

The tile map retains its more direct Week 2 selection. It converts the same camera view into a clamped, half-open tile range and visits only that range. The entity vector has no spatial lookup yet, so Kotte still tests every entity each frame. That deliberate O(total entities) limitation motivates the uniform spatial partition planned for Week 4.

The on-screen diagnostics show:

* the complete map dimensions and tile count;
* the camera's world-space view rectangle;
* the selected column and row ranges;
* the number of rendered tiles as a percentage of the complete world;
* total, tested, and visible entities plus the extracted command count.

The original Week 1 baseline remains available from the `2026-week-01` tag. The Week 2 camera and tile-culling reference remains available from `2026-week-02`.

## Random numbers

Kotte uses the open-source [`Random<RomuDuoJr>`](https://github.com/ulfben/cpp_prngs) random-number generator. It is fast, compact, easy to seed, deterministic across platforms, compatible with standard-library algorithms, and provides convenient game-oriented operations such as ranges, normalized values, coin flips, and selecting elements from collections.

## Status

Week 3 reference implementation: fixed-lifetime world entities, continuous player movement, brute-force entity visibility, render extraction, and layer/depth sorting. Spatial entity indexing, collision, gameplay lifetimes, and sprite/asset management are intentionally deferred to later course work.
