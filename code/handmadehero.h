#if !defined(HANDMADEHERO_H)

#include "handmade_platform.h"

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

/*
  NOTE(casey): Services that the game provides to the platform layer.
  (this may expand in the future - sound on separate thread, etc.)
*/

// FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

// TODO(casey): In the future, rendering _specifically_ will become a three-tiered abstraction!!!


struct memory_arena
{
    memory_index Size;
    uint8 *Base;
    memory_index Used;
};

internal void
InitializeArena(memory_arena *Arena, memory_index Size, uint8 *Base)
{
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    
    return(Result);
}

#include "handmade_math.h"
#include "handmade_intrinsics.h"
#include "handmade_world.h"

struct loaded_bitmap
{
    int32 Width;
    int32 Height;
    uint32 *Pixels;
};

struct hero_bitmaps
{
    int32 AlignX;
    int32 AlignY;
    loaded_bitmap Head;
    loaded_bitmap Cape;
    loaded_bitmap Torso;
};

struct high_entity
{
    v2 P; // NOTE(casey): Relative to the camera!
    v2 dP;
    uint32 ChunkZ;
    uint32 FacingDirection;

    real32 Z;
    real32 dZ;

    uint32 LowEntityIndex;
};
enum entity_type
{
    EntityType_Null,    
    EntityType_Hero,
    EntityType_Wall,
};

struct low_entity
{
    entity_type Type;
    
    world_position P;
    real32 Width, Height;

    // NOTE(casey): This is for "stairs"
    bool32 Collides;
    int32 dChunkZ;

    uint32 HighEntityIndex;
};



enum entity_residence
{
    EntityResidence_Nonexistent,
    EntityResidence_Low,
    EntityResidence_High,
};

struct entity
{
    uint32 LowIndex;
    low_entity *Low;
    high_entity *High;
};

struct game_state
{
    memory_arena WorldArena;
    world *World;

    // TODO(casey): Should we allow split-screen?
    uint32 CameraFollowingEntityIndex;
    world_position CameraP;

    uint32 PlayerIndexForController[ArrayCount(((game_input *)0)->Controllers)];

    uint32 LowEntityCount;
    low_entity LowEntities[100000];
    
    uint32 HighEntityCount;
    high_entity HighEntities_[256];

    loaded_bitmap Backdrop;
    loaded_bitmap Shadow;
    hero_bitmaps HeroBitmaps[4];

    loaded_bitmap Tree;
};


/*
  NOTE(casey): Services that the platform layer provides to the game
*/
#if 1
/* IMPORTANT(casey):

   These are NOT for doing anything in the shipping game - they are
   blocking and the write doesn't protect against lost data!
*/

#endif

#define HANDMADEHERO_H
#endif
