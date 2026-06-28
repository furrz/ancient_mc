#pragma once

enum BlockAttribs
{
    LIGHT_BLOCKER = 1,
    COLLIDABLE = 2,
    BUOYANT = 4,
    PICKABLE = 8,
    DRAW_TRANSPARENT = 16,
    DRAW_OPAQUE = 32,
    FLAMMABLE = 64,
    FALLING = 128,
    DRAW_CUTOUT = 256,
    FOLIAGE_COLORING = 512
};
