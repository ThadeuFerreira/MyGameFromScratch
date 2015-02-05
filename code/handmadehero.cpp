
#include "handmadehero.h"


typedef unsigned long DWORD;

#define TILE_MAP_COUNT_X 33
#define TILE_MAP_COUNT_Y 18

inline int32
TruncateReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)Real32;
    return(Result);
}

inline int32
RoundReal32ToInt32(real32 Real32)
{
	int32 Result;
	if(Real32 >= 0 ){
		Result = (int32)(Real32 + 0.5f);
	}
	else
	{
		Result = (int32)(Real32 - 0.5f);
	}
    // TODO(casey): Intrinsic????
    return(Result);
}
inline uint32
RoundReal32ToUInt32(real32 Real32)
{
    uint32 Result = (uint32)(Real32 + 0.5f);
    // TODO(casey): Intrinsic????
    return(Result);
}

inline uint32
GetTileValueUnchecked(tile_map *TileMap, int32 TileX, int32 TileY)
{
    uint32 TileMapValue = TileMap->Tiles[TileY*TileMap->CountX + TileX];
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

internal bool32
IsTileMapPointEmpty(tile_map *TileMap, real32 TestX, real32 TestY)
{
    bool32 Empty = false;

    int32 PlayerTileX = TruncateReal32ToInt32((TestX - TileMap->UpperLeftX) / TileMap->TileWidth);
    int32 PlayerTileY = TruncateReal32ToInt32((TestY - TileMap->UpperLeftY) / TileMap->TileHeight);

    if((PlayerTileX >= 0) && (PlayerTileX < TileMap->CountX) &&
       (PlayerTileY >= 0) && (PlayerTileY < TileMap->CountY))
    {
        uint32 TileMapValue = GetTileValueUnchecked(TileMap, PlayerTileX, PlayerTileY);
        Empty = (TileMapValue == 0);
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
RenderPlayer(game_Off_Screen_Buffer *Buffer, int PlayerX, int PlayerY, bool32 changeColor)
{
    uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
	uint32 Color;

	if (!changeColor){
		Color = 0xFFFFFFFF;
	}
	else
	{
		Color = 0x0;
	}
    int Top = PlayerY;
    int Bottom = PlayerY+10;
    for(int X = PlayerX;
        X < PlayerX+11;
        ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory +
                        X*Buffer->BytesPerPixel +
                        Top*Buffer->Pitch);
        for(int Y = Top;
            Y < Bottom;
            ++Y)
        {
            if((Pixel >= Buffer->Memory) &&
               ((Pixel + 4) <= EndOfBuffer))
            {
                *(uint32 *)Pixel = Color;
            }

            Pixel += Buffer->Pitch;
        }
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
IsWorldPointEmpty(world *World, int32 TileMapX, int32 TileMapY, real32 TestX, real32 TestY)
{
    bool32 Empty = false;

    tile_map *TileMap = GetTileMap(World, TileMapX, TileMapY);
    if(TileMap)
    {
        int32 PlayerTileX = TruncateReal32ToInt32((TestX - TileMap->UpperLeftX) / TileMap->TileWidth);
        int32 PlayerTileY = TruncateReal32ToInt32((TestY - TileMap->UpperLeftY) / TileMap->TileHeight);

        if((PlayerTileX >= 0) && (PlayerTileX < TileMap->CountX) &&
           (PlayerTileY >= 0) && (PlayerTileY < TileMap->CountY))
        {
            uint32 TileMapValue = GetTileValueUnchecked(TileMap, PlayerTileX, PlayerTileY);
            Empty = (TileMapValue == 0);
        }
    }
    
    return(Empty);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{

    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
	
    real32 UpperLeftX = -15;
    real32 UpperLeftY = 0;
    real32 TileWidth = 15;
    real32 TileHeight = 15;
	
	uint32 Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
		{
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1},
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
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
    };
	
	    uint32 Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
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
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
    };
    
    uint32 Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
    };
    
    uint32 Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
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
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
    };
	
	
	real32 moveX = 0;
	
	real32 moveY = 0;
	
	tile_map TileMaps[2][2];
    TileMaps[0][0].CountX = TILE_MAP_COUNT_X;
    TileMaps[0][0].CountY = TILE_MAP_COUNT_Y;
    
    TileMaps[0][0].UpperLeftX = -15;
    TileMaps[0][0].UpperLeftY = 0;
    TileMaps[0][0].TileWidth = 30;
    TileMaps[0][0].TileHeight = 30;

    TileMaps[0][0].Tiles = (uint32 *)Tiles00;

    TileMaps[0][1] = TileMaps[0][0];
    TileMaps[0][1].Tiles = (uint32 *)Tiles01;

    TileMaps[1][0] = TileMaps[0][0];
    TileMaps[1][0].Tiles = (uint32 *)Tiles10;

    TileMaps[1][1] = TileMaps[0][0];
    TileMaps[1][1].Tiles = (uint32 *)Tiles11;
	
	static bool32 loadTileMap = true;
	tile_map *TileMap = &TileMaps[0][0];
	if(!loadTileMap)
	{
		invertTitleMap(TileMap);
	}


    world World;
    World.TileMapCountX = 2;
    World.TileMapCountY = 2;

    World.TileMaps = (tile_map *)TileMaps;
    
    real32 PlayerWidth = 0.75f*TileMap->TileWidth;
    real32 PlayerHeight = TileMap->TileHeight;
	
    if(!Memory->IsInitialized)
    {
		char *Filename = "test.in";
        
        debug_read_file_result File = Memory->DEBUGPlatformReadEntireFile(Thread, Filename);
        if(File.Contents)
        {
            Memory->DEBUGPlatformWriteEntireFile(Thread, "test_out.out", File.ContentsSize, File.Contents);
            Memory->DEBUGPlatformFreeFileMemory(Thread, File.Contents);
        }
        GameState->ToneHz = 256;
		GameState->tSine = 0.0f;
		
		GameState->PlayerX = 100;
        GameState->PlayerY = 100;

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
			GameState->BlueOffset -= (int)(4.0f*(Controller->RightStickAverageX));
			GameState->GreenOffset += (int)(4.0f*(Controller->RightStickAverageY));
			GameState->ToneHz = 256 + (int)(128.0f*(Controller->RightStickAverageY));
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
            dPlayerX *= 64.0f;
            dPlayerY *= 64.0f;

            // TODO(casey): Diagonal will be faster!  Fix once we have vectors :)
            moveX = GameState->PlayerX + Input->dtForFrame*dPlayerX;
            moveY = GameState->PlayerY +Input->dtForFrame*dPlayerY;
		}

		// Input.AButtonEndedDown;
		// Input.AButtonHalfTransitionCount;
		        // Input.AButtonEndedDown;
        // Input.AButtonHalfTransitionCount;

        GameState->PlayerX += (10.0f*Controller->StickAverageX);
        GameState->PlayerY -= (10.0f*Controller->StickAverageY);
		
		// moveX = GameState->PlayerX + (10.0f*Controller->StickAverageX);		
		//moveY = GameState->PlayerY + (10.0f*Controller->StickAverageY);
		
		uint32 tileX = (uint32)((moveX - UpperLeftX)/TileWidth);
		uint32 tileY = (uint32)((moveY - UpperLeftY)/TileWidth);
		
		if(IsTileMapPointEmpty(TileMap, moveX - 0.5f*PlayerWidth, moveY) &&
               IsTileMapPointEmpty(TileMap, moveX + 0.5f*PlayerWidth, moveY) &&
               IsTileMapPointEmpty(TileMap, moveX, moveY))
        {
			GameState->PlayerX = moveX;
			GameState->PlayerY = moveY;
		}
		else
		{
			int x = 10;
		}
		

	
		
		//setTileMapPosition(GameState->TileMap, GameState->TileMapX, GameState->TileMapY);
		moveTileMapPosition(TileMap, 0, 0);
		if( GameState->PlayerX > Buffer->Width)
		{
			GameState->PlayerX = 0;

		}
		if( GameState->PlayerY > Buffer->Height)
		{
			GameState->PlayerY = 0;

		}
		
		if( GameState->PlayerX < 0)
		{
			GameState->PlayerX = (real32)Buffer->Width;

		}
		if( GameState->PlayerY < 0)
		{
			GameState->PlayerY = (real32)Buffer->Height;

		}
        if(GameState->tJump > 0)
        {
            GameState->PlayerY += (int)(5.0f*sinf(0.5f*Pi32*GameState->tJump));
        }
        if(Controller->ActionDown.EndedDown)
        {
            GameState->tJump = 4.0;
        }
        GameState->tJump -= 0.033f;
		
		if(Controller->ActionDown.EndedDown)
		{
			GameState->GreenOffset =  GameState->BlueOffset;
		}
		
		if(Controller->ActionUp.EndedDown)
		{
			loadTileMap = !loadTileMap;
		}
		
		
		if(ControllerIndex == 1){
			if (Controller->ActionUp.EndedDown)
			{
				GameState->PlayerColor = true;
			}
			else
			{
				GameState->PlayerColor = false;
			}
		}
	}

    

    for(int Row = 0;
        Row < TILE_MAP_COUNT_Y;
        ++Row)
    {
        for(int Column = 0;
            Column < TILE_MAP_COUNT_X;
            ++Column)
        {
            uint32 TileID = GetTileValueUnchecked(TileMap, Column, Row);
            real32 Gray = 0.5f;
            if(TileID == 1)
            {
                Gray = 1.0f;
            }

            real32 MinX = TileMap->UpperLeftX + ((real32)Column)*TileMap->TileWidth;
            real32 MinY = TileMap->UpperLeftY + ((real32)Row)*TileMap->TileHeight;
            real32 MaxX = MinX + TileMap->TileWidth;
            real32 MaxY = MinY + TileMap->TileHeight;
            DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
        }
    }
    
    real32 PlayerR = 1.0f;
    real32 PlayerG = 1.0f;
    real32 PlayerB = 0.0f;


    real32 PlayerLeft = GameState->PlayerX - 0.5f*PlayerWidth;
    real32 PlayerTop = GameState->PlayerY - PlayerHeight;
    DrawRectangle(Buffer,
                  PlayerLeft, PlayerTop,
                  PlayerLeft + PlayerWidth,
                  PlayerTop + PlayerHeight,
                  PlayerR, PlayerG, PlayerB);

    for(int ButtonIndex = 0;
        ButtonIndex < ArrayCount(Input->MouseButtons);
        ++ButtonIndex)
    {
        if(Input->MouseButtons[ButtonIndex].EndedDown)
        {
            RenderPlayer(Buffer, 10 + 20*ButtonIndex, 10, !GameState->PlayerColor);
        }
    }
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    GameOutputSound(GameState, SoundBuffer, GameState->ToneHz);
}

