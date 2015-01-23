#if !defined(WIN32_HANDMADE_H)


struct Win32_Off_Screen_Buffer
{
	 BITMAPINFO Info;
	 void *Memory;
	 int Width;
	 int Height;
	 int Pitch;
	 int BytesPerPixel;
};


struct Win32_Window_Dimension
{
	int Width;
	int Height;
};


struct Win32_output_sound
{
	int samplesPerSecond = 48000;
	int waveCounter = 0;
	int sTone = 120;
	int wavePeriod = samplesPerSecond / sTone;
	int bytesPerSample = sizeof(int16) * 2;
	DWORD secondaryBufferSize = samplesPerSecond*bytesPerSample;
	DWORD safetyBytes;
	uint32 runningSampleIndex = 0;
	real32 tSine;
	int latencySampleCount = samplesPerSecond / 60;

};

struct win32_debug_time_marker
{
    DWORD OutputPlayCursor;
    DWORD OutputWriteCursor;
    DWORD OutputLocation;
    DWORD OutputByteCount;
    DWORD ExpectedFlipPlayCursor;
    
    DWORD FlipPlayCursor;
    DWORD FlipWriteCursor;
};

struct win32_game_code
{
    HMODULE GameCodeDLL;
	FILETIME DLLLastWriteTime;
    game_update_and_render *UpdateAndRender;
    game_get_sound_samples *GetSoundSamples;

    bool32 IsValid;
};
#define WIN32_HANDMADE_H
#endif