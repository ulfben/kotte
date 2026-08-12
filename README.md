# Kotte

Kotte is a small C++ game engine and teaching project built with [raylib](https://www.raylib.com/).

It is a work in progress currently being developed for the course [Scalable Game Worlds in C++](https://www.uu.se/en/study/syllabus?query=6751b031-a906-11f0-b7df-f9101f074b4e) at Uppsala University.

The project uses C++26, raylib 6.0, and the C++ standard library to explore scalable game architecture and implementation. Each course week evolves one cumulative tile world in response to an observable scaling problem.

## Requirements

* Visual Studio Community 2026
* raylib 6.0

## Current demo

Run the x64 Debug or Release configuration. Move the marker with WASD or the arrow keys, and quit with Q or Escape.

The Week 2 demo contains a 200 by 120 tile world viewed through a small, translation-only Kotte camera. World positions are explicitly converted to screen positions so the transformation remains visible in the course code. The tile map converts the camera's world-space view into a clamped, half-open tile range and visits only that range while rendering.

The on-screen diagnostics show:

* the complete map dimensions and tile count;
* the camera's world-space view rectangle;
* the selected column and row ranges;
* the number of rendered tiles as a percentage of the complete world.

The original Week 1 baseline remains available from the `2026-week-01` tag. It deliberately draws every tile and contains no camera or culling.

## Random numbers

Kotte uses the open-source [`Random<RomuDuoJr>`](https://github.com/ulfben/cpp_prngs) random-number generator. It is fast, compact, easy to seed, deterministic across platforms, compatible with standard-library algorithms, and provides convenient game-oriented operations such as ranges, normalized values, coin flips, and selecting elements from collections.

## Status

Week 2 reference implementation: explicit camera transformation and visible-tile culling. Chunks, dynamic entity management, collision infrastructure, and a render-command queue are intentionally deferred to later course work.
