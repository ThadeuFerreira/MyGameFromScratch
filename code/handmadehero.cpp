
#include "handmadehero.h"
#include "handmade_intrinsics.h"


typedef unsigned long DWORD;

#define TILE_MAP_COUNT_X 33
#define TILE_MAP_COUNT_Y 18


inline uint32
GetTileValueUnchecked(world *World, tile_map *TileMap, int32 TileX, int32 TileY)
{
    Assert(TileMap);
    Assert((TileX >= 0) && (TileX < World->CountX) &&
           (TileY >= 0) && (TileY < World->CountY));
    
    uint32 TileMapValue = TileMap->Tiles[TileY*World->CountX + TileX];
    return(TileMapValue);
}


inline tile_map *
GetTileMap(world *World, int32 TileMapX, int32 TileMapY)
{
    tile_map *TileMap = 0;

    if((TileMapX >= 0) && (TileMapX < World->TileMapCountX) &&
       (TileMapY >= 0) && (TileMapY < World->TileMapCountY))
    {
        TileMap = &World->TileMaps[TileMapY*World->TileMapCountX + TileMapX];
    }

    return(TileMap);
}

inline void
RecanonicalizeCoord(world *World, int32 TileCount, int32 *TileMap, int32 *Tile, real32 *TileRel)
{
    // TODO(casey): Need to do something that doesn't use the divide/multiply method
    // for recanonicalizing because this can end up rounding back on to the tile
    // you just came from.

    // TODO(casey): Add bounds checking to prevent wrapping
    
    int32 Offset = FloorReal32ToInt32(*TileRel / World->TileSideInMeters);
    *Tile += Offset;
    *TileRel -= Offset*World->TileSideInMeters;

    Assert(*TileRel >= 0);
    // TODO(casey): Fix floating point math so this can be <
    Assert(*TileRel <= World->TileSideInMeters);

    if(*Tile < 0)
    {
        *Tile = TileCount + *Tile;
        --*TileMap;
    }

    if(*Tile >= TileCount)
    {
        *Tile = *Tile - TileCount;
        ++*TileMap;
    }
}

inline canonical_position
RecanonicalizePosition(world *World, canonical_position Pos)
{
    canonical_position Result = Pos;

    RecanonicalizeCoord(World, World->CountX, &Result.TileMapX, &Result.TileX, &Result.TileRelX);
    RecanonicalizeCoord(World, World->CountY, &Result.TileMapY, &Result.TileY, &Result.TileRelY);
    
    return(Result);
}

inline bool32
IsTileMapPointEmpty(world *World, tile_map *TileMap, int32 TestTileX, int32 TestTileY)
{
    bool32 Empty = false;

    if(TileMap)
    {
        if((TestTileX >= 0) && (TestTileX < World->CountX) &&
           (TestTileY >= 0) && (TestTileY < World->CountY))
        {
            uint32 TileMapValue = GetTileValueUnchecked(World, TileMap, TestTileX, TestTileY);
            Empty = (TileMapValue == 0);
        }
    }
    
    return(Empty);
}

internal void
DrawRectangle(game_Off_Screen_Buffer *Buffer,
              real32 RealMinX, real32 RealMinY, real32 RealMaxX, real32 RealMaxY,
              real32 R, real32 G, real32 B)
{    
    int32 MinX = RoundReal32ToInt32(RealMinX);
    int32 MinY = RoundReal32ToInt32(RealMinY);
    int32 MaxX = RoundReal32ToInt32(RealMaxX);
    int32 MaxY = RoundReal32ToInt32(RealMaxY);

    if(MinX < 0)
    {
        MinX = 0;
    }

    if(MinY < 0)
    {
        MinY = 0;
    }

    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }

    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }

    uint32 Color = ((RoundReal32ToUInt32(R * 255.0f) << 16) |
                    (RoundReal32ToUInt32(G * 255.0f) << 8) |
                    (RoundReal32ToUInt32(B * 255.0f) << 0));

    uint8 *Row = ((uint8 *)Buffer->Memory +
                  MinX*Buffer->BytesPerPixel +
                  MinY*Buffer->Pitch);
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {            
            *Pixel++ = Color;
        }
        
        Row += Buffer->Pitch;
    }
}

internal void 
GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int toneHz)
{
		int16 toneVolume = 3000;
		if (toneHz == 0)
		{
			toneHz = 440;
		}
		int16 wavePeriod = (int16)(SoundBuffer->samplesPerSecond/toneHz);
		int16 *sampleOut = (int16*)SoundBuffer->samples;
		for (DWORD sampleIndex = 0; sampleIndex < (DWORD)SoundBuffer->sampleCount; ++sampleIndex)
		{
#if HANDMADE_DEBUGSOUND
			real32 sineValue = sin(GameState->tSine);
			int16 sampleValue = (int16)(sineValue * toneVolume);
#else
			int16 sampleValue = 0;
#endif
			*sampleOut++ = sampleValue;
			*sampleOut++ = sampleValue;
#if HANDMADE_DEBUGSOUND
			GameState->tSine += 2.0f*Pi32*1.0f / (real32)wavePeriod;
			if(GameState->tSine > 2.0f*Pi32)
			{
				GameState->tSine -= 2.0f*Pi32;
			}
#endif
		}
}



internal void
moveTileMapPosition(tile_map *TileMap, 	int32 TileMapX,	int32 TileMapY)
{
	static uint32 updateRate = 0;
	
	if(!((updateRate++)%20))
	{
	tile_map tempTileMap;
		for(int Row = 0; Row < TILE_MAP_COUNT_Y; Row++)
		{
			for(int Column = 0; Column < TILE_MAP_COUNT_X; Column++)
			{
			  tempTileMap.Tiles = TileMap->Tiles;
			}
		}
		
		/*for(int Row = 0; Row < TILE_MAP_COUNT_Y; Row++)
		{
			for(int Column = 0; Column < TILE_MAP_COUNT_X; Column++)
			{
			  int32 YOffset = (Row - TileMapY)%TILE_MAP_COUNT_Y;

			  int32 XOffset = (Column + TileMapX)%TILE_MAP_COUNT_X;
			  TileMap[Row][Column] = tempTileMap[YOffset][XOffset];
			}
		}*/
	}
} 


internal void
invertTitleMap(tile_map *TileMap)
{
	for(int Row = 0; Row < TILE_MAP_COUNT_Y; Row++)
	{
		for(int Column = 0; Column < TILE_MAP_COUNT_X; Column++)
		{
            if(TileMap->Tiles[Row*TileMap->CountX + Column] == 1)
            {
                TileMap->Tiles[Row*TileMap->CountX + Column] = 0; 
            }
			else
			{
				TileMap->Tiles[Row*TileMap->CountX + Column] = 1;
			}
		
		}
	}

}

internal bool32
IsWorldPointEmpty(world *World, canonical_position CanPos)
{
    bool32 Empty = false;

    tile_map *TileMap = GetTileMap(World, CanPos.TileMapX, CanPos.TileMapY);
    Empty = IsTileMapPointEmpty(World, TileMap, CanPos.TileX, CanPos.TileY);

    return(Empty);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{

    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
   

	
	uint32 Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
		{
        {1, 1, 1, 1,  0, 0, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 1, 0,  0, 1, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 1, 0,  0, 1, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 1, 0,  0, 1, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0,  0, 1, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 1, 1, 1,  0, 0, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
    };
	
	    uint32 Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {1, 1, 1, 1,  1, 0, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 1, 1, 1,  0, 0, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
    };
    
    uint32 Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 1, 1, 1,  0, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
    };
    
    uint32 Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {1, 1, 1, 1,  0, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 1, 1, 1,  0, 0, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
    };
	
	
	real32 moveX = 0;
	
	real32 moveY = 0;
	
	tile_map TileMaps[2][2];
    TileMaps[0][0].CountX = TILE_MAP_COUNT_X;
    TileMaps[0][0].CountY = TILE_MAP_COUNT_Y;
    
    TileMaps[0][0].UpperLeftX = -15;
    TileMaps[0][0].UpperLeftY = 0;
    TileMaps[0][0].TileWidth = 15;
    TileMaps[0][0].TileHeight = 15;

    TileMaps[0][0].Tiles = (uint32 *)Tiles00;
    TileMaps[0][1].Tiles = (uint32 *)Tiles01;
    TileMaps[1][0].Tiles = (uint32 *)Tiles10;
    TileMaps[1][1].Tiles = (uint32 *)Tiles11;
	
	static bool32 loadTileMap = true;

    world World;
    World.TileMapCountX = 2;
    World.TileMapCountY = 2;
    World.CountX = TILE_MAP_COUNT_X;
    World.CountY = TILE_MAP_COUNT_Y;
	
	World.TileSideInMeters = 1.4f;
    World.TileSideInPixels = 30;
    World.MetersToPixels = (real32)World.TileSideInPixels / (real32)World.TileSideInMeters;

    World.UpperLeftX = -(real32)World.TileSideInPixels/2;
    World.UpperLeftY = 0;
    
    real32 PlayerHeight = 1.4f;
    real32 PlayerWidth = 0.75f*PlayerHeight;

    World.TileMaps = (tile_map *)TileMaps;
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!Memory->IsInitialized)
    {
        GameState->PlayerP.TileMapX = 0;
        GameState->PlayerP.TileMapY = 0;
        GameState->PlayerP.TileX = 3;
        GameState->PlayerP.TileY = 3;
        GameState->PlayerP.TileRelX = 5.0f;
        GameState->PlayerP.TileRelY = 5.0f;

        Memory->IsInitialized = true;
    }

	
	tile_map *TileMap = GetTileMap(&World, GameState->PlayerP.TileMapX, GameState->PlayerP.TileMapY);
    Assert(TileMap);
	
    if(!Memory->IsInitialized)
    {
		char *Filename = "test.in";
        
        debug_read_file_result File = Memory->DEBUGPlatformReadEntireFile(Thread, Filename);
        if(File.Contents)
        {
            Memory->DEBUGPlatformWriteEntireFile(Thread, "test_out.out", File.ContentsSize, File.Contents);
            Memory->DEBUGPlatformFreeFileMemory(Thread, File.Contents);
        }

        // TODO(casey): This may be more appropriate to do in the platform layer
        Memory->IsInitialized = true;
		
		
    }
	

    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
		game_controller_input *Controller = GetController(Input, ControllerIndex);	

		if(Controller->IsAnalog)
		{
			// NOTE(casey): Use analog movement tuning

		}
		else
		{
            // NOTE(casey): Use digital movement tuning
            real32 dPlayerX = 0.0f; // pixels/second
            real32 dPlayerY = 0.0f; // pixels/second
            
            if(Controller->MoveUp.EndedDown)
            {
                dPlayerY = -1.0f;
            }
            if(Controller->MoveDown.EndedDown)
            {
                dPlayerY = 1.0f;
            }
            if(Controller->MoveLeft.EndedDown)
            {
                dPlayerX = -1.0f;
            }
            if(Controller->MoveRight.EndedDown)
            {
                dPlayerX = 1.0f;
            }

            dPlayerX *= 2.0f;
            dPlayerY *= 2.0f;
            
            // TODO(casey): Diagonal will be faster!  Fix once we have vectors :)
            canonical_position NewPlayerP = GameState->PlayerP;
            NewPlayerP.TileRelX += Input->dtForFrame*dPlayerX;
            NewPlayerP.TileRelY += Input->dtForFrame*dPlayerY;
            NewPlayerP = RecanonicalizePosition(&World, NewPlayerP);
            // TODO(casey): Delta function that auto-recanonicalizes

            canonical_position PlayerLeft = NewPlayerP;
            PlayerLeft.TileRelX -= 0.5f*PlayerWidth;
            PlayerLeft = RecanonicalizePosition(&World, PlayerLeft);
            
            canonical_position PlayerRight = NewPlayerP;
            PlayerRight.TileRelX += 0.5f*PlayerWidth;
            PlayerRight = RecanonicalizePosition(&World, PlayerRight);
            
            if(IsWorldPointEmpty(&World, NewPlayerP) &&
               IsWorldPointEmpty(&World, PlayerLeft) &&
               IsWorldPointEmpty(&World, PlayerRight))
            {
                GameState->PlayerP = NewPlayerP;
            }
		}		
	}

    DrawRectangle(Buffer, 0.0f, 0.0f, (real32)Buffer->Width, (real32)Buffer->Height,
                  1.0f, 0.0f, 0.1f);

    for(int Row = 0;
        Row < TILE_MAP_COUNT_Y;
        ++Row)
    {
        for(int Column = 0;
            Column < TILE_MAP_COUNT_X;
            ++Column)
        {
            uint32 TileID = GetTileValueUnchecked(&World, TileMap, Column, Row);
            real32 Gray = 0.5f;
            if(TileID == 1)
            {
                Gray = 1.0f;
            }

            real32 MinX = World.UpperLeftX + ((real32)Column)*World.TileSideInPixels;
            real32 MinY = World.UpperLeftY + ((real32)Row)*World.TileSideInPixels;
            real32 MaxX = MinX + World.TileSideInPixels;
            real32 MaxY = MinY + World.TileSideInPixels;
            DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
        }
    }
    
    real32 PlayerR = 1.0f;
    real32 PlayerG = 1.0f;
    real32 PlayerB = 0.0f;
    real32 PlayerLeft = World.UpperLeftX + World.TileSideInPixels*GameState->PlayerP.TileX +
        World.MetersToPixels*GameState->PlayerP.TileRelX - 0.5f*World.MetersToPixels*PlayerWidth;
    real32 PlayerTop = World.UpperLeftY + World.TileSideInPixels*GameState->PlayerP.TileY +
        World.MetersToPixels*GameState->PlayerP.TileRelY - World.MetersToPixels*PlayerHeight;
    DrawRectangle(Buffer,
                  PlayerLeft, PlayerTop,
                  PlayerLeft + World.MetersToPixels*PlayerWidth,
                  PlayerTop + World.MetersToPixels*PlayerHeight,
                  PlayerR, PlayerG, PlayerB);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    GameOutputSound(GameState, SoundBuffer, 400);
}

