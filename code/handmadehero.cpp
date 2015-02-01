
#include "handmadehero.h"


typedef unsigned long DWORD;

internal int32
RoundReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)(Real32 + 0.5f);
    // TODO(casey): Intrinsic????
    return(Result);
}
internal uint32
RoundReal32ToUInt32(real32 Real32)
{
    uint32 Result = (uint32)(Real32 + 0.5f);
    // TODO(casey): Intrinsic????
    return(Result);
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

#if HANDMADE_DEBUG
internal void 
RenderWeirdGradient(game_Off_Screen_Buffer *Buffer, int XOffset, int YOffset)
{	

	uint8 *Row = (uint8 *) Buffer->Memory;
	
	for(int Y = 0; Y < Buffer->Height; ++Y)
	{
		uint32 *Pixel = (uint32 *) Row;
		for(int X = 0; X < Buffer->Width; ++X)
		{
			uint8 Blue = 0;
			uint8 Green = 0 ;
			uint8 Red = 0;
			

			if (X > Buffer->Width/2){
				Blue = 0;
				Red = (uint8)(X + XOffset);
				Green = (uint8)(Y + YOffset);
			}else
			{
				Blue = (uint8)(X + XOffset);
				Green = (uint8)(Y + YOffset);
				Red = 0;
			}
			*Pixel++ = ((Red<<16)|(Green<<8)|Blue);
		}
		Row += Buffer->Pitch;
	}
}

#endif

internal void
invertTitleMap(uint32 TitleMap[18][33])
{
	for(int Row = 0; Row < 18; Row++)
	{
		for(int Column = 0; Column < 33; Column++)
		{
            if(TitleMap[Row][Column] == 1)
            {
                TitleMap[Row][Column]= 0;
            }
			else
			{
				TitleMap[Row][Column]= 1;
			}
		
		}
	}

}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{

    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
	 uint32 tempTileMap[18][33] =
		{
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 0},
        {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1, 1, 1, 1, 1,  1, 1, 0, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0, 1, 1, 1, 1,  1, 1, 0, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1, 1, 1, 1, 1,  1, 1, 0, 1,  0, 1, 1, 1,  1, 1, 1, 0},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1, 1, 1, 0, 1,  1, 1, 0, 1,  0, 1, 1, 1,  1, 1, 0, 0},
        {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1, 1, 1, 1, 1,  1, 1, 0, 1,  0, 1, 1, 1,  1, 1, 1, 0},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 0, 1,  0, 1, 1, 1,  1, 1, 0, 0},
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 0, 1,  0, 1, 1, 1,  1, 1, 0, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 0, 1},
        {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 0, 1},
        {0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 0, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 0},
    };
	
	static bool32 fistState = true;
	
	if(fistState){
		for(int Row = 0; Row < 18; Row++)
		{
			for(int Column = 0; Column < 33; Column++)
			{
			  GameState->TileMap[Row][Column] = tempTileMap[Row][Column];
			}
		}
		fistState = false;
	}
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
            GameState->PlayerX += Input->dtForFrame*dPlayerX;
            GameState->PlayerY += Input->dtForFrame*dPlayerY;
		}

		// Input.AButtonEndedDown;
		// Input.AButtonHalfTransitionCount;
		        // Input.AButtonEndedDown;
        // Input.AButtonHalfTransitionCount;

        GameState->PlayerX += (10.0f*Controller->StickAverageX);
        GameState->PlayerY -= (10.0f*Controller->StickAverageY);
		
		
		if( GameState->PlayerX > Buffer->Width)
		{
			GameState->PlayerX = 0;
			invertTitleMap(GameState->TileMap);
		}
		if( GameState->PlayerY > Buffer->Height)
		{
			GameState->PlayerY = 0;
			invertTitleMap(GameState->TileMap);
		}
		
		if( GameState->PlayerX < 0)
		{
			GameState->PlayerX = (real32)Buffer->Width;
			invertTitleMap(GameState->TileMap);
		}
		if( GameState->PlayerY < 0)
		{
			GameState->PlayerY = (real32)Buffer->Height;
			invertTitleMap(GameState->TileMap);
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

    real32 UpperLeftX = -30;
    real32 UpperLeftY = 0;
    real32 TileWidth = 30;
    real32 TileHeight = 30;
    
    DrawRectangle(Buffer, 0.0f, 0.0f, (real32)Buffer->Width, (real32)Buffer->Height,
                  1.0f, 0.0f, 0.1f);
				  
    for(int Row = 0;
        Row < 18;
        ++Row)
    {
        for(int Column = 0;
            Column < 33;
            ++Column)
        {
            uint32 TileID = GameState->TileMap[Row][Column];
            real32 Gray = 0.5f;
            if(TileID == 1)
            {
                Gray = 1.0f;
            }

            real32 MinX = UpperLeftX + ((real32)Column)*TileWidth;
            real32 MinY = UpperLeftY + ((real32)Row)*TileHeight;
            real32 MaxX = MinX + TileWidth;
            real32 MaxY = MinY + TileHeight;
            DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
        }
    }
    
    real32 PlayerR = 1.0f;
    real32 PlayerG = 1.0f;
    real32 PlayerB = 0.0f;
    real32 PlayerWidth = 0.75f*TileWidth;
    real32 PlayerHeight = TileHeight;
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

