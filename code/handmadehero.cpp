
#include "handmadehero.h"

/*
Controler/keyboard input
sound output
bitmap output
*/

internal void
mainGameLoop(void)
{

}

internal void GameUserInput(ButtonActions *actionButt, uint8 *XOffset, uint8 *YOffset, uint16 *sTone)
{
	if (actionButt->Up)
	{
		(*YOffset)++;
	}
	if (actionButt->Down)
	{
		(*YOffset)--;
	}
	if (actionButt->Left)
	{
		(*XOffset)++;
	}
	if (actionButt->Right)
	{

		(*XOffset)--;

	}

	if (actionButt->ButB)
	{
		*sTone = 262;
	}
	if (actionButt->ButY)
	{
		*sTone = 880;
	}
	if (actionButt->ButX)
	{
		*sTone = 350;
	}
}
internal void GameOutputSound(game_sound_output_buffer *SoundBuffer, uint16 toneHz)
{
	local_persist real32 tSine;
		int16 toneVolume = 3000;
		if (toneHz == 0)
		{
			toneHz = 440;
		}
		int16 wavePeriod = SoundBuffer->samplesPerSecond/toneHz;
		int16 *sampleOut = SoundBuffer->samples;
		for (DWORD sampleIndex = 0; sampleIndex < SoundBuffer->sampleCount; ++sampleIndex)
		{
			real32 sineValue = sin(tSine);
			int16 sampleValue = (int16)(sineValue * toneVolume);
			*sampleOut++ = sampleValue;
			*sampleOut++ = sampleValue;
			tSine += 2.0f*Pi32*1.0f / (real32)wavePeriod;
		}
}

internal void 
RenderWierdGradient(game_Off_Screen_Buffer *Buffer, int XOffset, int YOffset)
{	
	int Pitch = Buffer->Width*(BYTES_PER_PIXEL);
	uint8 *Row = (uint8 *) Buffer->Memory;
	
	for(int Y = 0; Y < Buffer->Height; ++Y)
	{
		uint32 *Pixel = (uint32 *) Row;
		for(int X = 0; X < Buffer->Width; ++X)
		{
			uint8 Blue ;
			uint8 Green ;
			uint8 Red = 0;
			
			if (X > Buffer->Width/2){
				Blue = 0;
				Red = (X + XOffset);
			}else
			{
				Blue = (X + XOffset);
				Green = (Y + YOffset);
				Red = 0;
			}
			*Pixel++ = ((Red<<16)|(Green<<8)|Blue);
		}
		Row += Pitch;
	}
}

internal void GameUpdateAndRander(game_Off_Screen_Buffer *Buffer, 
									game_sound_output_buffer *SoundBuffer, 
									ButtonActions *actionButt, 
									uint8 *XOffset, uint8 *YOffset, uint16 *toneHz)
{

	GameUserInput(actionButt, XOffset, YOffset, toneHz);
	GameOutputSound(SoundBuffer, *toneHz);
	RenderWierdGradient(Buffer, *XOffset, *YOffset);
}