#if !defined(HANDMADEHERO_H)

#include "handmade_platform.h"

#define internal static 
#define local_persist static 
#define global_variable static

#define PIXEL_BIT_COUNT 32
#define BYTES_PER_PIXEL 4
#define Pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#if HANDMADE_SLOW
// TODO(casey): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

inline uint32
SafeTruncateUInt64(uint64 Value)
{
    // TODO(casey): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return(Result);
}


/*
  NOTE(casey): Services that the game provides to the platform layer.
  (this may expand in the future - sound on separate thread, etc.)
*/

// FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

// TODO(casey): In the future, rendering _specifically_ will become a three-tiered abstraction!!!



struct canonical_position
{
    int32 TileMapX;
    int32 TileMapY;

    int32 TileX;
    int32 TileY;

    // NOTE(casey): This is tile-relative X and Y
    // TODO(casey): These are still in pixels... :/
    real32 TileRelX;
    real32 TileRelY;
};

struct tile_map
{
    int32 CountX;
    int32 CountY;
    
    real32 UpperLeftX;
    real32 UpperLeftY;
    real32 TileWidth;
    real32 TileHeight;

    uint32 *Tiles;
};
struct tile_chunk
{
    uint32 *Tiles;
};

struct world
{
    uint32 ChunkShift;
    uint32 ChunkMask;
    uint32 ChunkDim;
    
    real32 TileSideInMeters;
    int32 TileSideInPixels;
    real32 MetersToPixels;

    // TODO(casey): Beginner's sparseness
    int32 TileChunkCountX;
    int32 TileChunkCountY;
    
    tile_chunk *TileChunks;
};

struct tile_chunk_position
{
    uint32 TileChunkX;
    uint32 TileChunkY;

    uint32 RelTileX;
    uint32 RelTileY;
};


struct world_position
{
    /* TODO(casey):

       Take the tile map x and y
       and the tile x and y

       and pack them into single 32-bit values for x and y
       where there is some low bits for the tile index
       and the high bits are the tile "page"

       (NOTE we can eliminate the need for floor!)
    */
    uint32 AbsTileX;
    uint32 AbsTileY;

    // TODO(casey): Should these be from the center of a tile?
    // TODO(casey): Rename to offset X and Y
    real32 TileRelX;
    real32 TileRelY;
};

struct game_state
{
	world_position PlayerP;
};


/*
  NOTE(casey): Services that the platform layer provides to the game
*/
#if 1
/* IMPORTANT(casey):

   These are NOT for doing anything in the shipping game - they are
   blocking and the write doesn't protect against lost data!
*/


inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers));
    
    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return(Result);
}

#endif

#define HANDMADEHERO_H
#endif
