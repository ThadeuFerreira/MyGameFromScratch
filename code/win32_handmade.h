#if !defined(WIN32_HANDMADE_H)


struct Win32_Off_Screen_Buffer
{
    // NOTE(casey): Pixels are always 32-bits wide, Memory Order BB GG RR XX
	 BITMAPINFO Info;
	 void *Memory;
	 int Width;
	 int Height;
	 int Pitch;
	 int BytesPerPixel;
};

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct win32_replay_buffer
{
    HANDLE FileHandle;
    HANDLE MemoryMap;
    char FileName[WIN32_STATE_FILE_NAME_COUNT];
    void *MemoryBlock;
};

struct win32_state
{
    uint64 TotalSize;
    void *GameMemoryBlock;
    win32_replay_buffer ReplayBuffers[4];
    
    HANDLE RecordingHandle;
    int InputRecordingIndex;

    HANDLE PlaybackHandle;
    int InputPlayingIndex;
    
    char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
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