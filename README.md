# ancient-mc

MC-clone based loosely off of RD-132211 ported to C++.

Very heavily modified from that state - new save file format,
inventory / block selection, flying, water flowing, etc.

Most importantly, the game is **heavily** configurable from JSON files:
New blocks, rules for block interactions and updates, and even parameters like
player movement speed values are all defined in JSON files in the `res`
directory.

You may need vcpkg in manifest mode to build.

## Controls:

- WASD: Move
- Break Block: Left Click
- Place Block: Right Click
- Sprint: Left Shift
- Jump: Space
- Reset Position: R
- Toggle Flying: F
- Move Down (while flying): Left Control
