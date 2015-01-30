
#include "handmadehero.h"


typedef unsigned long DWORD;



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
			real32 sineValue = sin(GameState->tSine);
			int16 sampleValue = (int16)(sineValue * toneVolume);
			*sampleOut++ = sampleValue;
			*sampleOut++ = sampleValue;
			GameState->tSine += 2.0f*Pi32*1.0f / (real32)wavePeriod;
			if(GameState->tSine > 2.0f*Pi32)
			{
				GameState->tSine -= 2.0f*Pi32;
			}
		}
}

internal void
RenderPlayer(game_Off_Screen_Buffer *Buffer, int PlayerX, int PlayerY, bool32 changeColor)
{
    uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
	uint32 Color;

	if (changeColor){
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

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{

    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
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
			 if(Controller->MoveLeft.EndedDown)
				{
					GameState->BlueOffset += 1;
					//GameState->GreenOffset += 1;
				}
			if(Controller->MoveRight.EndedDown)
				{
					GameState->BlueOffset -= 1;
					//GameState->GreenOffset += 1;
				}
				
				if(Controller->MoveDown.EndedDown)
				{
					//GameState->BlueOffset += 1;
					GameState->GreenOffset -= 1;
				}
				if(Controller->MoveUp.EndedDown)
				{
					//GameState->BlueOffset += 1;
					GameState->GreenOffset += 1;
				}
			  
		}

		// Input.AButtonEndedDown;
		// Input.AButtonHalfTransitionCount;
		        // Input.AButtonEndedDown;
        // Input.AButtonHalfTransitionCount;

        GameState->PlayerX += (int)(10.0f*Controller->StickAverageX);
        GameState->PlayerY -= (int)(10.0f*Controller->StickAverageY);
		
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
			GameState->PlayerX = Buffer->Width;
		}
		if( GameState->PlayerY < 0)
		{
			GameState->PlayerY = Buffer->Height;
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

	RenderWeirdGradient(Buffer, GameState->BlueOffset, GameState->GreenOffset);
	RenderPlayer(Buffer, GameState->PlayerX, GameState->PlayerY, GameState->PlayerColor);

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

