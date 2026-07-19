# Kotte

Kotte is a small C++ game engine and teaching project built with [raylib](https://www.raylib.com/).

It is a work in progress currently being developed for the course [Scalable Game Worlds in C++](https://www.uu.se/en/study/syllabus?query=6751b031-a906-11f0-b7df-f9101f074b4e) at Uppsala University.

The project uses C++26, raylib 6.0, and the C++ standard library to explore scalable game architecture and implementation. The current starting point renders a deterministic tile room and deliberately draws every tile, providing a measurable baseline for later camera culling and spatial partitioning.

## Requirements

* Visual Studio Community 2026
* raylib 6.0

## Starting point

Run the x64 Debug or Release configuration. Move the marker with WASD or the arrow keys, and quit with Q or Escape. The on-screen diagnostics report the fixed world seed and the number of tiles rendered.

The initial room uses a contiguous tile map and immediate Raylib drawing. It intentionally does not include a camera, chunks, culling, entity management, collision infrastructure, or a render-command queue; those systems are introduced in response to scaling problems during the course.

## Random numbers

Kotte uses the open-source [`Random<RomuDuoJr>`](https://github.com/ulfben/cpp_prngs) random-number generator. It is fast, compact, easy to seed, deterministic across platforms, compatible with standard-library algorithms, and provides convenient game-oriented operations such as ranges, normalized values, coin flips, and selecting elements from collections.

## Status

Early development. The Week 1 tile-world baseline is in progress.
