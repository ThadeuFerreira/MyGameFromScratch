//This is my first game from scratch!!

#define HANDMADE_INTERNAL 1



#include "handmadehero.h"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <xinput.h>
#include <dsound.h>

#include "win32_handmade.h"
#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH

global_variable Win32_Off_Screen_Buffer GlobalBackBuffer;
global_variable bool32 GlobalPause;
global_variable LPDIRECTSOUNDBUFFER SecondaryBuffer;
global_variable bool32 GlobalRunning;
global_variable int64 GlobalPerfCountFrequency;


#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex,  XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name)  DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name)_Check_return_ HRESULT WINAPI name(_In_opt_ LPCGUID pcGuidDevice, _Outptr_ LPDIRECTSOUND *ppDS, _Pre_null_ LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}


float platformImpl(char *value)
{
	OutputDebugStringA("This is Windows32 \n");
	return 2;
}

internal void
CatStrings(size_t SourceACount, char *SourceA,
           size_t SourceBCount, char *SourceB,
           size_t DestCount, char *Dest)
{
    // TODO(casey): Dest bounds checking!
    
    for(int Index = 0;
        Index < SourceACount;
        ++Index)
    {
        *Dest++ = *SourceA++;
    }

    for(int Index = 0;
        Index < SourceBCount;
        ++Index)
    {
        *Dest++ = *SourceB++;
    }

    *Dest++ = 0;
}

internal int
StringLength(char *String)
{
    int Count = 0;
    while(*String++)
    {
        ++Count;
    }
    return(Count);
}

internal void
Win32BuildEXEPathFileName(win32_state *State, char *FileName,
                          int DestCount, char *Dest)
{
    CatStrings(State->OnePastLastEXEFileNameSlash - State->EXEFileName, State->EXEFileName,
               StringLength(FileName), FileName,
               DestCount, Dest);
}

internal real32
Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold)
{
    real32 Result = 0;

    if(Value < -DeadZoneThreshold)
    {
        Result = (real32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
    }
    else if(Value > DeadZoneThreshold)
    {
        Result = (real32)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
    }

    return(Result);
}

inline LARGE_INTEGER
Win32GetWallClock(void)
{    
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

internal void
Win32ProcessKeyboardMessage(game_button_state *NewState, bool32 IsDown)
{
    NewState->EndedDown = IsDown;
    ++NewState->HalfTransitionCount;
}


DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_read_file_result Result = {};
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(NULL, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, NULL) &&
                   (FileSize32 == BytesRead))
                {
                    // NOTE(casey): File read successfully
                    Result.ContentsSize = FileSize32;
                }
                else
                {                    
                    // TODO(casey): Logging
                    DEBUGPlatformFreeFileMemory(Thread, Result.Contents);
                    Result.Contents = 0;
                }
            }
            else
            {
                // TODO(casey): Logging
            }
        }
        else
        {
            // TODO(casey): Logging
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
    }

    return(Result);
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    bool32 Result = false;
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            // NOTE(casey): File read successfully
            Result = (BytesWritten == MemorySize);
        }
        else
        {
            // TODO(casey): Logging
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
    }

    return(Result);
}

internal void
Win32ProcessXInputDigitalButton(DWORD XInputButtonState,
                                game_button_state *OldState, DWORD ButtonBit,
                                game_button_state *NewState)
{
    NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

inline real32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    real32 Result = ((real32)(End.QuadPart - Start.QuadPart) /
                     (real32)GlobalPerfCountFrequency);
    return(Result);
}

inline FILETIME
Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }

    return(LastWriteTime);
}

internal win32_game_code
Win32LoadGameCode(char *SourceDLLName, char *TempDLLName)
{
    win32_game_code Result = {};

    // TODO(casey): Need to get the proper path here!
    // TODO(casey): Automatic determination of when updates are necessary.

    Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);

    CopyFile(SourceDLLName, TempDLLName, FALSE);
    
    Result.GameCodeDLL = LoadLibraryA(TempDLLName);
    if(Result.GameCodeDLL)
    {
        Result.UpdateAndRender = (game_update_and_render *)
            GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");
        
        Result.GetSoundSamples = (game_get_sound_samples *)
            GetProcAddress(Result.GameCodeDLL, "GameGetSoundSamples");

        Result.IsValid = (Result.UpdateAndRender &&
                          Result.GetSoundSamples);
    }

    if(!Result.IsValid)
    {
        Result.UpdateAndRender = 0;
        Result.GetSoundSamples = 0;
    }

    return(Result);
}

internal void
Win32UnloadGameCode(win32_game_code *GameCode)
{
	int a = 10;
	int b = 20;
    if(GameCode->GameCodeDLL)
    {
        FreeLibrary(GameCode->GameCodeDLL);
        GameCode->GameCodeDLL = 0;
    }
    GameCode->IsValid = false;
    GameCode->UpdateAndRender = 0;
    GameCode->GetSoundSamples = 0;
}


internal void
Win32GetInputFileLocation(win32_state *State, bool32 InputStream,
                          int SlotIndex, int DestCount, char *Dest)
{
    char Temp[64];
    wsprintf(Temp, "loop_edit_%d_%s.hmi", SlotIndex, InputStream ? "input" : "state");
    Win32BuildEXEPathFileName(State, Temp, DestCount, Dest);
}

internal win32_replay_buffer *
Win32GetReplayBuffer(win32_state *State, int unsigned Index)
{
    Assert(Index < ArrayCount(State->ReplayBuffers));
    win32_replay_buffer *Result = &State->ReplayBuffers[Index];
    return(Result);
}

internal void
Win32BeginRecordingInput(win32_state *State, int InputRecordingIndex)
{
    win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(State, InputRecordingIndex);
    if(ReplayBuffer->MemoryBlock)
    {
        State->InputRecordingIndex = InputRecordingIndex;

        char FileName[WIN32_STATE_FILE_NAME_COUNT];
        Win32GetInputFileLocation(State, true, InputRecordingIndex, sizeof(FileName), FileName);
        State->RecordingHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

#if HANDMADE_PROFILING
        LARGE_INTEGER FilePosition;
        FilePosition.QuadPart = State->TotalSize;
        SetFilePointerEx(State->RecordingHandle, FilePosition, 0, FILE_BEGIN);
#endif
        
        CopyMemory(ReplayBuffer->MemoryBlock, State->GameMemoryBlock, State->TotalSize);
    }
}

internal void
Win32EndRecordingInput(win32_state *State)
{
    CloseHandle(State->RecordingHandle);
    State->InputRecordingIndex = 0;
}

internal void
Win32BeginInputPlayBack(win32_state *State, int InputPlayingIndex)
{
    win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(State, InputPlayingIndex);
    if(ReplayBuffer->MemoryBlock)
    {
        State->InputPlayingIndex = InputPlayingIndex;

        char FileName[WIN32_STATE_FILE_NAME_COUNT];
        Win32GetInputFileLocation(State, true, InputPlayingIndex, sizeof(FileName), FileName);
        State->PlaybackHandle = CreateFileA(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

#if 0
        LARGE_INTEGER FilePosition;
        FilePosition.QuadPart = State->TotalSize;
        SetFilePointerEx(State->PlaybackHandle, FilePosition, 0, FILE_BEGIN);
#endif
        
        CopyMemory(State->GameMemoryBlock, ReplayBuffer->MemoryBlock, State->TotalSize);
    }
}


internal void
Win32EndInputPlayBack(win32_state *State)
{
    CloseHandle(State->PlaybackHandle);
    State->InputPlayingIndex = 0;
}

internal void
Win32RecordInput(win32_state *State, game_input *NewInput)    
{
    DWORD BytesWritten;
    WriteFile(State->RecordingHandle, NewInput, sizeof(*NewInput), &BytesWritten, 0);
}

internal void
Win32PlayBackInput(win32_state *State, game_input *NewInput)
{
    DWORD BytesRead = 0;
    if(ReadFile(State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0))
    {
        if(BytesRead == 0)
        {
            // NOTE(casey): We've hit the end of the stream, go back to the beginning
            int PlayingIndex = State->InputPlayingIndex;
            Win32EndInputPlayBack(State);
            Win32BeginInputPlayBack(State, PlayingIndex);
            ReadFile(State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0);
        }
    }
}

internal void
Win32ProcessPendingMessages(win32_state *State, game_controller_input *KeyboardController)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)Message.wParam;
                bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                if(WasDown != IsDown)
                {
                    if(VKCode == 'W')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
                    }
                    else if(VKCode == 'A')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                    }
                    else if(VKCode == 'S')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
                    }
                    else if(VKCode == 'D')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                    }
                    else if(VKCode == 'Q')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
                    }
                    else if(VKCode == 'E')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
                    }
                    else if(VKCode == VK_UP)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown);
                    }
                    else if(VKCode == VK_LEFT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown);
                    }
                    else if(VKCode == VK_DOWN)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown);
                    }
                    else if(VKCode == VK_RIGHT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown);
                    }
                    else if(VKCode == VK_ESCAPE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Start, IsDown);
                    }
                    else if(VKCode == VK_SPACE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Back, IsDown);
                    }
#if HANDMADE_INTERNAL
                    else if(VKCode == 'P')
                    {
                        if(IsDown)
                        {
                            GlobalPause = !GlobalPause;
                        }
                    }
                    else if(VKCode == 'L')
                    {
                        if(IsDown)
                        {
                            if(State->InputPlayingIndex == 0)
                            {
                                if(State->InputRecordingIndex == 0)
                                {
                                    Win32BeginRecordingInput(State, 1);
                                }
                                else
                                {
                                    Win32EndRecordingInput(State);
                                    Win32BeginInputPlayBack(State, 1);
                                }
                            }
                            else
                            {
                                Win32EndInputPlayBack(State);
                            }
                        }
                    }
#endif
                }

                bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
                if((VKCode == VK_F4) && AltKeyWasDown)
                {
                    GlobalRunning = false;
                }
            } break;

            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}
internal void
Win32ClearSoundBuffer(Win32_output_sound *soundOutput)
{
	VOID *region1;
	DWORD region1Size;
	VOID *region2;
	DWORD region2Size;
	if (SUCCEEDED(SecondaryBuffer->Lock(0, soundOutput->secondaryBufferSize, &region1, &region1Size, &region2, &region2Size, 0)))
	{
		uint8 *sampleOut = (uint8 *)region1;
		for (DWORD sampleIndex = 0; sampleIndex < region1Size; ++sampleIndex)
		{
			*sampleOut++ = 0;
		}

		sampleOut = (uint8*)region2;
		for (DWORD sampleIndex = 0; sampleIndex < region2Size; ++sampleIndex)
		{
			*sampleOut++ = 0;
		}
	}
	SecondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
}

internal void
Win32FillSoundBuffer(Win32_output_sound *soundOutput, game_sound_output_buffer *SourceBuffer,   DWORD bytesToLock, DWORD bytesToWrite)
{
	VOID *region1;
	DWORD region1Size;
	VOID *region2;
	DWORD region2Size;
	
	if (SUCCEEDED(SecondaryBuffer->Lock(bytesToLock, bytesToWrite, &region1, &region1Size, &region2, &region2Size, 0)))
	{
		DWORD region1SampleCounter = region1Size / soundOutput->bytesPerSample;
		int16 *sampleOut = (int16 *)region1;
		int16 *sampleIn = SourceBuffer->samples;
		for (DWORD sampleIndex = 0; sampleIndex < region1SampleCounter; ++sampleIndex)
		{
			*sampleOut++ = *sampleIn++;
			*sampleOut++ = *sampleIn++;
			soundOutput->runningSampleIndex++;

		}
	
		DWORD region2SampleCounter = region2Size / soundOutput->bytesPerSample;
		sampleOut = (int16 *)region2;

		for (DWORD sampleIndex = 0; sampleIndex < region2SampleCounter; ++sampleIndex)
		{
			*sampleOut++ = *sampleIn++;
			*sampleOut++ = *sampleIn++;
			soundOutput->runningSampleIndex++;
		}
	}
	SecondaryBuffer->Unlock(region1, region1Size, region2, region2Size);

}

internal void
Win32LoadXInput(void)    
{
    // TODO(casey): Test this on Windows 8
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary)
    {
        // TODO(casey): Diagnostic
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    
    if(!XInputLibrary)
    {
        // TODO(casey): Diagnostic
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) {XInputGetState = XInputGetStateStub;}

        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetState) {XInputSetState = XInputSetStateStub;}

        // TODO(casey): Diagnostic

    }
    else
    {
        // TODO(casey): Diagnostic
    }
}

internal Win32_Window_Dimension 
Win32GetWindowDimension(HWND Window)
{
	Win32_Window_Dimension Result;
	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return Result;
}

internal void
Win32InitDSound(HWND Window, int32 samplesPerSecond, int32 BufferSize)
{
	//Load Direct Sound Library
	HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

	if (DSoundLibrary)
	{
		//Get a DirectSound object!
		direct_sound_create *DirectSoundCreate = (direct_sound_create *) GetProcAddress(DSoundLibrary, "DirectSoundCreate");

		LPDIRECTSOUND DirectSound = {};
		HRESULT Error = DirectSoundCreate(0, &DirectSound, 0);
		if (DirectSoundCreate && SUCCEEDED(Error))
		{
			WAVEFORMATEX WaveFormat = {};
			WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormat.nChannels = 2;
			WaveFormat.nSamplesPerSec = samplesPerSecond;
			WaveFormat.wBitsPerSample = 16;
			WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
			WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;

			if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
			{
				DSBUFFERDESC BufferDescription = {};
				BufferDescription.dwSize = sizeof(BufferDescription);
				BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
				LPDIRECTSOUNDBUFFER PrimaryBuffer;
				
				if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
				{
					HRESULT Error_cd = PrimaryBuffer->SetFormat(&WaveFormat);
					if (SUCCEEDED(Error_cd))
					{
						OutputDebugStringA("PrimaryBuffer set");
					}
					else
					{
						OutputDebugStringA("PrimaryBuffer not set");
					}
				}
			}

			DSBUFFERDESC BufferDescription = {};
			BufferDescription.dwSize = sizeof(BufferDescription);
			BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
			BufferDescription.dwBufferBytes = BufferSize;
			BufferDescription.lpwfxFormat = &WaveFormat;

			HRESULT Error_cd = DirectSound->CreateSoundBuffer(&BufferDescription, &SecondaryBuffer, 0);
			if (SUCCEEDED(Error_cd))
			{
				OutputDebugStringA("Secondary set");
			}
			else
			{
				OutputDebugStringA("Secondary not set");
			}

		}
		else
		{
			//TODO log errors here
		}
	}
}

internal void 
Win32UpdateWindow( Win32_Off_Screen_Buffer *Buffer, HDC DeviceContext,
					int WindowWidth, int WindowHeight)
{

	StretchDIBits(	DeviceContext,
		0, 0, Buffer->Width, Buffer->Height,
		0, 0, Buffer->Width, Buffer->Height,
		Buffer->Memory,
		&Buffer->Info,
		DIB_RGB_COLORS,
		SRCCOPY
		);

}

internal void 
Win32ResizeDIBSection(Win32_Off_Screen_Buffer *Buffer, int Width, int Height)
{
    int BytesPerPixel = 4;
	Buffer->BytesPerPixel = BytesPerPixel;
	if(Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, NULL, MEM_RELEASE);
	}
	Buffer->Width = Width;
	Buffer->Height = Height;
	
	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biHeight = - Buffer->Height;
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = PIXEL_BIT_COUNT; //24 RGB and 8 for pading 
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
	
	Buffer->Memory = VirtualAlloc(NULL, BitmapMemorySize,  MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	Buffer->Pitch = Width*BytesPerPixel;
	
}

LRESULT CALLBACK Win32MainWindowCallBack(
	HWND	Window,
	UINT	Message,
	WPARAM	WParam,
	LPARAM	LParam
	)
{
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    // NOTE(casey): Set the Windows scheduler granularity to 1ms
    // so that our Sleep() can be more granular.
    UINT DesiredSchedulerMS = 1;
    bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
	
	Win32LoadXInput();
	LRESULT Result = 0;
	Win32_Window_Dimension Dimension = Win32GetWindowDimension(Window);
	Win32ResizeDIBSection(&GlobalBackBuffer, 960, 540);
	
	switch(Message)
	{
		case WM_PAINT:
		{
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
			
			Win32_Window_Dimension Dimension = Win32GetWindowDimension(Window);
			Win32UpdateWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
			EndPaint(Window, &Paint);
		} break;
		case WM_SIZE:
		{
			//OutputDebugStringA("WM_SIZE\n");
		} break;
		case WM_DESTROY:
		{
			//TODO: This is a ERRO situation, deal with it!!
			GlobalRunning = false;
			//OutputDebugStringA("WM_DESTROY\n");
		} break;
		case WM_CLOSE:
		{
			GlobalRunning = false;
			//OutputDebugStringA("WM_CLOSE\n");
		} break;
        case WM_ACTIVATEAPP:
        {
#if 0
            if(WParam == TRUE)
            {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 255, LWA_ALPHA);
            }
            else
            {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 64, LWA_ALPHA);
            }
#endif
        } break;
		default:
		{
			Result =  DefWindowProc(Window, Message,WParam,LParam);
		} break;

	}

	return Result;

}

internal void
Win32DebugDrawVertical(Win32_Off_Screen_Buffer *Backbuffer,
                       int X, int Top, int Bottom, uint32 Color)
{
    if(Top <= 0)
    {
        Top = 0;
    }

    if(Bottom > Backbuffer->Height)
    {
        Bottom = Backbuffer->Height;
    }
    
    if((X >= 0) && (X < Backbuffer->Width))
    {
        uint8 *Pixel = ((uint8 *)Backbuffer->Memory +
                        X*Backbuffer->BytesPerPixel +
                        Top*Backbuffer->Pitch);
        for(int Y = Top;
            Y < Bottom;
            ++Y)
        {
            *(uint32 *)Pixel = Color;
			Pixel += Backbuffer->Pitch;
        }
    }
}

inline void
Win32DrawSoundBufferMarker(Win32_Off_Screen_Buffer *Backbuffer,
                           Win32_output_sound *soundOutput,
                           real32 C, int PadX, int Top, int Bottom,
                           DWORD Value, uint32 Color)
{
    real32 XReal32 = (C * (real32)Value);
    int X = PadX + (int)XReal32;
    Win32DebugDrawVertical(Backbuffer, X, Top, Bottom, Color);
}

#if HANDMADE_DEBUG_SOUND
internal void
Win32DebugSyncDisplay(Win32_Off_Screen_Buffer *Backbuffer,
                      int MarkerCount, win32_debug_time_marker *Markers,
                      int CurrentMarkerIndex,
                      Win32_output_sound *SoundOutput, real32 TargetSecondsPerFrame)
{
    int PadX = 16;
    int PadY = 16;

    int LineHeight = 64;
    
    real32 C = (real32)(Backbuffer->Width - 2*PadX) / (real32)SoundOutput->secondaryBufferSize;
    for(int MarkerIndex = 0;
        MarkerIndex < MarkerCount;
        ++MarkerIndex)
    {
        win32_debug_time_marker *ThisMarker = &Markers[MarkerIndex];
        Assert(ThisMarker->OutputPlayCursor < SoundOutput->secondaryBufferSize);
        Assert(ThisMarker->OutputWriteCursor < SoundOutput->secondaryBufferSize);
        Assert(ThisMarker->OutputLocation < SoundOutput->secondaryBufferSize);
        Assert(ThisMarker->OutputByteCount < SoundOutput->secondaryBufferSize);
        Assert(ThisMarker->FlipPlayCursor < SoundOutput->secondaryBufferSize);
        Assert(ThisMarker->FlipWriteCursor < SoundOutput->secondaryBufferSize);

        DWORD PlayColor = 0xFFFFFFFF;
        DWORD WriteColor = 0xFFFF0000;
        DWORD ExpectedFlipColor = 0xFFFFFF00;
        DWORD PlayWindowColor = 0xFFFF00FF;

        int Top = PadY;
        int Bottom = PadY + LineHeight;
        if(MarkerIndex == CurrentMarkerIndex)
        {
            Top += LineHeight+PadY;
            Bottom += LineHeight+PadY;

            int FirstTop = Top;
            
            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputPlayCursor, PlayColor);
            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputWriteCursor, WriteColor);

            Top += LineHeight+PadY;
            Bottom += LineHeight+PadY;

            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputLocation, PlayColor);
            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputLocation + ThisMarker->OutputByteCount, WriteColor);

            Top += LineHeight+PadY;
            Bottom += LineHeight+PadY;

            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, FirstTop, Bottom, ThisMarker->ExpectedFlipPlayCursor, ExpectedFlipColor);
        }        
        
        Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipPlayCursor, PlayColor);
        Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipPlayCursor + 480*SoundOutput->bytesPerSample, PlayWindowColor);
        Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipWriteCursor, WriteColor);
    }

}
#endif

internal void
Win32GetEXEFileName(win32_state *State)
{
    // NOTE(casey): Never use MAX_PATH in code that is user-facing, because it
    // can be dangerous and lead to bad results.
    DWORD SizeOfFilename = GetModuleFileNameA(0, State->EXEFileName, sizeof(State->EXEFileName));
    State->OnePastLastEXEFileNameSlash = State->EXEFileName;
    for(char *Scan = State->EXEFileName;
        *Scan;
        ++Scan)
    {
        if(*Scan == '\\')
        {
            State->OnePastLastEXEFileNameSlash = Scan + 1;
        }
    }
}

LRESULT 
Win32CreateInitialWindow(HINSTANCE Instance)
{
    win32_state Win32State = {};
	Win32GetEXEFileName(&Win32State);

    char SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "handmadehero.dll",
                              sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);

    char TempGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "handmade_temp.dll",
                              sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);
	WNDCLASSA WindowClass = {};
	
	 // NOTE(casey): Set the Windows scheduler granularity to 1ms
    // so that our Sleep() can be more granular.
    UINT DesiredSchedulerMS = 1;
    bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);

	WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallBack;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	LARGE_INTEGER performanceFrequencyResult;
	QueryPerformanceFrequency(&performanceFrequencyResult);

	int64 perfFreqCount = performanceFrequencyResult.QuadPart;
	
    // TODO(casey): How do we reliably query on this on Windows?
    
    // TODO(casey): Let's think about running non-frame-quantized for audio latency...
    // TODO(casey): Let's use the write cursor delta from the play cursor to adjust
    // the target audio latency.
    

	// TODO(casey): How do we reliably query on this on Windows?



	if (RegisterClassA(&WindowClass))
	{
		HWND Window = CreateWindowExA(
                0, // WS_EX_TOPMOST|WS_EX_LAYERED,
			WindowClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			Instance,
			0
			);
		if (Window)
		{
			 int MonitorRefreshHz = 60;
            HDC RefreshDC = GetDC(Window);
            int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
            ReleaseDC(Window, RefreshDC);
            if(Win32RefreshRate > 1)
            {
                MonitorRefreshHz = Win32RefreshRate;
            }
			
			real32 GameUpdateHz = (MonitorRefreshHz / 2.0f);
			real32 TargetSecondsPerFrame = 1.0f / (real32)GameUpdateHz;
			uint8 XOffset = 0;
			uint8 YOffset = 0;
			uint16 sTone = 440;
			Win32_output_sound soundOutput = {};
			
			soundOutput.samplesPerSecond = 48000;
            soundOutput.bytesPerSample = sizeof(int16)*2;
            soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond*soundOutput.bytesPerSample;

            // TODO(casey): Actually compute this variance and see
            // what the lowest reasonable value is.
            soundOutput.safetyBytes = (int)(((real32)soundOutput.samplesPerSecond*(real32)soundOutput.bytesPerSample / GameUpdateHz)/3.0f);
			
			Win32InitDSound(Window, soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);
			Win32ClearSoundBuffer(&soundOutput);
			
			GlobalRunning = true;

			
			int16 *Samples = (int16 *)VirtualAlloc(0, soundOutput.secondaryBufferSize,
                                                   MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			
#if HANDMADE_INTERNAL
            LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
            LPVOID BaseAddress = 0;
#endif
            
            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes(64);
            GameMemory.TransientStorageSize = Megabytes(100);
			GameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
            GameMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
            GameMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
			
			// TODO(casey): Handle various memory footprints (USING SYSTEM METRICS)
            // TODO(casey): Use MEM_LARGE_PAGES and call adjust token
            // privileges when not on Windows XP?
            Win32State.TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
            Win32State.GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t)Win32State.TotalSize,
                                                 MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            GameMemory.PermanentStorage = Win32State.GameMemoryBlock;
            GameMemory.TransientStorage = ((uint8 *)GameMemory.PermanentStorage +
                                           GameMemory.PermanentStorageSize);
			            for(int ReplayIndex = 0;
                ReplayIndex < ArrayCount(Win32State.ReplayBuffers);
                ++ReplayIndex)
            {
                win32_replay_buffer *ReplayBuffer = &Win32State.ReplayBuffers[ReplayIndex];

                // TODO(casey): Recording system still seems to take too long
                // on record start - find out what Windows is doing and if
                // we can speed up / defer some of that processing.
                
                Win32GetInputFileLocation(&Win32State, false, ReplayIndex,
                                          sizeof(ReplayBuffer->FileName), ReplayBuffer->FileName);

                ReplayBuffer->FileHandle =
                    CreateFileA(ReplayBuffer->FileName,
                                GENERIC_WRITE|GENERIC_READ, 0, 0, CREATE_ALWAYS, 0, 0);

                LARGE_INTEGER MaxSize;
                MaxSize.QuadPart = Win32State.TotalSize;
                ReplayBuffer->MemoryMap = CreateFileMapping(
                    ReplayBuffer->FileHandle, 0, PAGE_READWRITE,
                    MaxSize.HighPart, MaxSize.LowPart, 0);

                ReplayBuffer->MemoryBlock = MapViewOfFile(
                    ReplayBuffer->MemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, Win32State.TotalSize);
                if(ReplayBuffer->MemoryBlock)
                {
                }
                else
                {
                    // TODO(casey): Diagnostic
                }
            }
										   
            if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
            {			
				game_input Input[2] = {};
				game_input *NewInput = &Input[0];
				game_input *OldInput = &Input[1];
				
				NewInput->SecondsToAdvanceOverUpdate = TargetSecondsPerFrame;
				
                LARGE_INTEGER LastCounter = Win32GetWallClock();
                LARGE_INTEGER FlipWallClock = Win32GetWallClock();

                int DebugTimeMarkerIndex = 0;
                win32_debug_time_marker DebugTimeMarkers[30] = {0};

                DWORD AudioLatencyBytes = 0;
                real32 AudioLatencySeconds = 0;
                bool32 SoundIsValid = false;
				
				LARGE_INTEGER begCounter; // Starting the clock
				QueryPerformanceCounter(&begCounter);
				SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
				int64 LastCycleCount =  __rdtsc();
				
				DWORD LastPlayCursor = 0;
				
                win32_game_code Game = Win32LoadGameCode(SourceGameCodeDLLFullPath,
                                                         TempGameCodeDLLFullPath);
                uint32 LoadCounter = 0;
				
				while (GlobalRunning)
				{
				
                    FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
                    if(CompareFileTime(&NewDLLWriteTime, &Game.DLLLastWriteTime) != 0)
                    {
                        Win32UnloadGameCode(&Game);
                        Game = Win32LoadGameCode(SourceGameCodeDLLFullPath,
                                                 TempGameCodeDLLFullPath);
                        LoadCounter = 0;
                    }
					
					begCounter.QuadPart;
                    game_controller_input *OldKeyboardController = GetController(OldInput, 0);
                    game_controller_input *NewKeyboardController = GetController(NewInput, 0);
                    *NewKeyboardController = {};
                    NewKeyboardController->IsConnected = true;
                    for(int ButtonIndex = 0;
                        ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
                        ++ButtonIndex)
                    {
                        NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                            OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                    }                  

                    Win32ProcessPendingMessages(&Win32State, NewKeyboardController);            

					//XINPUT_VIBRATION Vibration;
					//ButtonActions actionButt;				
					
					if(!GlobalPause)
					{
					    POINT MouseP;
                        GetCursorPos(&MouseP);
                        ScreenToClient(Window, &MouseP);
                        NewInput->MouseX = MouseP.x;
                        NewInput->MouseY = MouseP.y;
                        NewInput->MouseZ = 0; // TODO(casey): Support mousewheel?
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[0],
                                                    GetKeyState(VK_LBUTTON) & (1 << 15));
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[1],
                                                    GetKeyState(VK_MBUTTON) & (1 << 15));
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[2],
                                                    GetKeyState(VK_RBUTTON) & (1 << 15));
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[3],
                                                    GetKeyState(VK_XBUTTON1) & (1 << 15));
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[4],
                                                    GetKeyState(VK_XBUTTON2) & (1 << 15));
                        
                        // TODO(casey): Need to not poll disconnected controllers to avoid
                        // xinput frame rate hit on older libraries...
                        // TODO(casey): Should we poll this more frequently
                        DWORD MaxControllerCount = XUSER_MAX_COUNT;
						if(MaxControllerCount > ArrayCount(NewInput->Controllers) -1)
						{
							MaxControllerCount = ArrayCount(NewInput->Controllers) -1;
						}
						
						for (DWORD ControllerIndex = 0; ControllerIndex< XUSER_MAX_COUNT; ControllerIndex++ )
						{
							DWORD OurControllerIndex = ControllerIndex + 1;
							game_controller_input *OldController = GetController(OldInput, OurControllerIndex);
							game_controller_input *NewController = GetController(NewInput, OurControllerIndex);
							
							XINPUT_STATE ControllerState;
							if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
							{                            
							
								NewController->IsConnected = true;
								NewController->IsAnalog = OldController->IsAnalog;
							   
								// NOTE(casey): This controller is plugged in
								// TODO(casey): See if ControllerState.dwPacketNumber increments too rapidly
								XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

								// TODO(casey): This is a square deadzone, check XInput to
								// verify that the deadzone is "round" and show how to do
								// round deadzone processing.
								NewController->StickAverageX = Win32ProcessXInputStickValue(
									Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
								NewController->StickAverageY = Win32ProcessXInputStickValue(
									Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
									
									
								NewController->RightStickAverageX = Win32ProcessXInputStickValue(
									Pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
								NewController->RightStickAverageY = Win32ProcessXInputStickValue(
									Pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
									
								if((NewController->StickAverageX != 0.0f) ||
								   (NewController->StickAverageY != 0.0f))
								{
									NewController->IsAnalog = true;
								}

								if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
								{
									NewController->StickAverageY = 1.0f;
									NewController->IsAnalog = false;
								}
								
								if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
								{
									NewController->StickAverageY = -1.0f;
									NewController->IsAnalog = false;
								}
								
								if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
								{
									NewController->StickAverageX = -1.0f;
									NewController->IsAnalog = false;
								}
								
								if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
								{
									NewController->StickAverageX = 1.0f;
									NewController->IsAnalog = false;
								}

								real32 Threshold = 0.5f;
								Win32ProcessXInputDigitalButton(
									(NewController->StickAverageX < -Threshold) ? 1 : 0,
									&OldController->MoveLeft, 1,
									&NewController->MoveLeft);
								Win32ProcessXInputDigitalButton(
									(NewController->StickAverageX > Threshold) ? 1 : 0,
									&OldController->MoveRight, 1,
									&NewController->MoveRight);
								Win32ProcessXInputDigitalButton(
									(NewController->StickAverageY < -Threshold) ? 1 : 0,
									&OldController->MoveDown, 1,
									&NewController->MoveDown);
								Win32ProcessXInputDigitalButton(
									(NewController->StickAverageY > Threshold) ? 1 : 0,
									&OldController->MoveUp, 1,
									&NewController->MoveUp);

								Win32ProcessXInputDigitalButton(Pad->wButtons,
																&OldController->ActionDown, XINPUT_GAMEPAD_A,
																&NewController->ActionDown);
								Win32ProcessXInputDigitalButton(Pad->wButtons,
																&OldController->ActionRight, XINPUT_GAMEPAD_B,
																&NewController->ActionRight);
								Win32ProcessXInputDigitalButton(Pad->wButtons,
																&OldController->ActionLeft, XINPUT_GAMEPAD_X,
																&NewController->ActionLeft);
								Win32ProcessXInputDigitalButton(Pad->wButtons,
																&OldController->ActionUp, XINPUT_GAMEPAD_Y,
																&NewController->ActionUp);
								Win32ProcessXInputDigitalButton(Pad->wButtons,
																&OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER,
																&NewController->LeftShoulder);
								Win32ProcessXInputDigitalButton(Pad->wButtons,
																&OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER,
																&NewController->RightShoulder);

								Win32ProcessXInputDigitalButton(Pad->wButtons,
																&OldController->Start, XINPUT_GAMEPAD_START,
																&NewController->Start);
								Win32ProcessXInputDigitalButton(Pad->wButtons,
																&OldController->Back, XINPUT_GAMEPAD_BACK,
																&NewController->Back);
								
							}
							else
							{
							// Controller is not connected 
							// NOTE(casey): The controller is not available
							
							/*char strBuffer[264];
							 wsprintfA(strBuffer, "Controle %d nao conectado\n", ControllerIndex);
								NewController->IsConnected = false;
								OutputDebugStringA(strBuffer);*/
							}
						}

						    thread_context Thread = {};
							game_Off_Screen_Buffer Buffer = {};
							Buffer.Memory = GlobalBackBuffer.Memory;
							Buffer.Width = GlobalBackBuffer.Width; 
							Buffer.Height = GlobalBackBuffer.Height;
							Buffer.Pitch = GlobalBackBuffer.Pitch; 
							Buffer.BytesPerPixel = GlobalBackBuffer.BytesPerPixel;
							
							if(Win32State.InputRecordingIndex)
							{
								Win32RecordInput(&Win32State, NewInput);
							}

							if(Win32State.InputPlayingIndex)
							{
								Win32PlayBackInput(&Win32State, NewInput);
							}
							
							NewInput->SecondsToAdvanceOverUpdate = TargetSecondsPerFrame;
							
							Game.UpdateAndRender(&Thread, &GameMemory, NewInput, &Buffer);
							LARGE_INTEGER AudioWallClock = Win32GetWallClock();
							real32 FromBeginToAudioSeconds = Win32GetSecondsElapsed(FlipWallClock, AudioWallClock);

							DWORD PlayCursor;
							DWORD WriteCursor;
							if(SecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
							{
								/* NOTE(casey):

								   Here is how sound output computation works.

								   We define a safety value that is the number
								   of samples we think our game update loop
								   may vary by (let's say up to 2ms)
						   
								   When we wake up to write audio, we will look
								   and see what the play cursor position is and we
								   will forecast ahead where we think the play
								   cursor will be on the next frame boundary.

								   We will then look to see if the write cursor is
								   before that by at least our safety value.  If
								   it is, the target fill position is that frame
								   boundary plus one frame.  This gives us perfect
								   audio sync in the case of a card that has low
								   enough latency.

								   If the write cursor is _after_ that safety
								   margin, then we assume we can never sync the
								   audio perfectly, so we will write one frame's
								   worth of audio plus the safety margin's worth
								   of guard samples.
								*/
								if(!SoundIsValid)
								{
									soundOutput.runningSampleIndex = WriteCursor / soundOutput.bytesPerSample;
									SoundIsValid = true;
								}

								DWORD ByteToLock = ((soundOutput.runningSampleIndex*soundOutput.bytesPerSample) %
													soundOutput.secondaryBufferSize);

								DWORD ExpectedSoundBytesPerFrame =
									(int)((real32)(soundOutput.samplesPerSecond*soundOutput.bytesPerSample) /
                                      GameUpdateHz);
									  
								real32 SecondsLeftUntilFlip = (TargetSecondsPerFrame - FromBeginToAudioSeconds);
								DWORD ExpectedBytesUntilFlip = (DWORD)((SecondsLeftUntilFlip/TargetSecondsPerFrame)*(real32)ExpectedSoundBytesPerFrame);

								DWORD ExpectedFrameBoundaryByte = PlayCursor + ExpectedBytesUntilFlip;
							
								DWORD SafeWriteCursor = WriteCursor;
								if(SafeWriteCursor < PlayCursor)
								{
									SafeWriteCursor += soundOutput.secondaryBufferSize;
								}
								Assert(SafeWriteCursor >= PlayCursor);
								SafeWriteCursor += soundOutput.safetyBytes;
							
								bool32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);                        

								DWORD TargetCursor = 0;
								if(AudioCardIsLowLatency)
								{
									TargetCursor = (ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame);
								}
								else
								{
									TargetCursor = (WriteCursor + ExpectedSoundBytesPerFrame +
										soundOutput.safetyBytes);
								}
								TargetCursor = (TargetCursor % soundOutput.secondaryBufferSize);
							
								DWORD BytesToWrite = 0;
								if(ByteToLock > TargetCursor)
								{
									BytesToWrite = (soundOutput.secondaryBufferSize - ByteToLock);
									BytesToWrite += TargetCursor;
								}
								else
								{
									BytesToWrite = TargetCursor - ByteToLock;
								}

								game_sound_output_buffer SoundBuffer = {};
								SoundBuffer.samplesPerSecond = soundOutput.samplesPerSecond;
								SoundBuffer.sampleCount = BytesToWrite / soundOutput.bytesPerSample;
								SoundBuffer.samples = Samples;
								
								Game.GetSoundSamples(&Thread, &GameMemory, &SoundBuffer);


	#if HANDMADE_INTERNAL
								win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
								Marker->OutputPlayCursor = PlayCursor;
								Marker->OutputWriteCursor = WriteCursor;
								Marker->OutputLocation = ByteToLock;
								Marker->OutputByteCount = BytesToWrite;
								Marker->ExpectedFlipPlayCursor = ExpectedFrameBoundaryByte;

								DWORD UnwrappedWriteCursor = WriteCursor;
								if(UnwrappedWriteCursor < PlayCursor)
								{
									UnwrappedWriteCursor += soundOutput.secondaryBufferSize;
								}
								AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
								AudioLatencySeconds =
									(((real32)AudioLatencyBytes / (real32)soundOutput.bytesPerSample) /
									 (real32)soundOutput.samplesPerSecond);
							
								char TextBuffer[256];
								_snprintf_s(TextBuffer, sizeof(TextBuffer),
											"BTL:%u TC:%u BTW:%u - PC:%u WC:%u DELTA:%u (%fs)\n",
											ByteToLock, TargetCursor, BytesToWrite,
											PlayCursor, WriteCursor, AudioLatencyBytes, AudioLatencySeconds);
								//OutputDebugStringA(TextBuffer);
	#endif   
								Win32FillSoundBuffer(&soundOutput,&SoundBuffer, ByteToLock, BytesToWrite);
							}
							else
							{
								SoundIsValid = false;
							}
						
							LARGE_INTEGER WorkCounter = Win32GetWallClock();
							real32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);

							// TODO(casey): NOT TESTED YET!  PROBABLY BUGGY!!!!!
							real32 SecondsElapsedForFrame = WorkSecondsElapsed;
							if(SecondsElapsedForFrame < TargetSecondsPerFrame)
							{                        
								if(SleepIsGranular)
								{
									DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame -
																	   SecondsElapsedForFrame));
									if(SleepMS > 0)
									{
										Sleep(SleepMS);
									}
								}
							
								real32 TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
																						   Win32GetWallClock());
								if(TestSecondsElapsedForFrame < TargetSecondsPerFrame)
								{
									// TODO(casey): LOG MISSED SLEEP HERE
								}
							
								while(SecondsElapsedForFrame < TargetSecondsPerFrame)
								{                            
									SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
																					Win32GetWallClock());
								}
							}
							else
							{
								// TODO(casey): MISSED FRAME RATE!
								// TODO(casey): Logging
							}

						
						LARGE_INTEGER EndCounter = Win32GetWallClock();
						real32 MSPerFrame = 1000.0f*Win32GetSecondsElapsed(LastCounter, EndCounter);                    
						LastCounter = EndCounter;
						
						
	#if HANDMADE_DEBUG_SOUND
							// TODO(casey): Note, current is wrong on the zero'th index
							Win32DebugSyncDisplay(&GlobalBackBuffer, ArrayCount(DebugTimeMarkers), DebugTimeMarkers,
												  DebugTimeMarkerIndex - 1, &soundOutput, TargetSecondsPerFrame);
	#endif
	                    Win32_Window_Dimension Dimensiton = Win32GetWindowDimension(Window);
                        HDC DeviceContext = GetDC(Window);
						Win32UpdateWindow(&GlobalBackBuffer, DeviceContext, Dimensiton.Width, Dimensiton.Height);
                        ReleaseDC(Window, DeviceContext);
							FlipWallClock = Win32GetWallClock();
	#if HANDMADE_INTERNAL
							// NOTE(casey): This is debug code
							{
								DWORD PlayCursor;
								DWORD WriteCursor;
								if(SecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
								{
									Assert(DebugTimeMarkerIndex < ArrayCount(DebugTimeMarkers));
									win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
									Marker->FlipPlayCursor = PlayCursor;
									Marker->FlipWriteCursor = WriteCursor;
								}
							
							}
	#endif
		
						
						game_input *Temp = NewInput;
						NewInput = OldInput;
						OldInput = Temp;
						

						uint64 EndCycleCount = __rdtsc();
						uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
						LastCycleCount = EndCycleCount;
						
						real64 FPS = 0.0f;
						real64 MCPF = ((real64)CyclesElapsed / (1000.0f * 1000.0f));

						char FPSBuffer[256];
						_snprintf_s(FPSBuffer, sizeof(FPSBuffer),
									"%.02fms/f,  %.02ff/s,  %.02fmc/f\n", MSPerFrame, FPS, MCPF);
						OutputDebugStringA(FPSBuffer);
						
	#if HANDMADE_INTERNAL
							++DebugTimeMarkerIndex;
							if(DebugTimeMarkerIndex == ArrayCount(DebugTimeMarkers))
							{
								DebugTimeMarkerIndex = 0;
							}
	#endif
					}
				}

			}
			else
			{
				//TODO : LOGING MEMORY not enough
				OutputDebugStringA("NAO CONSEGUIU ALOCAR MEMORIA SUFICIENTE");
			}
		}
		else
		{
			//TODO() LOGIN
		}
	}
	else
	{
		//TODO() LOGING
	}

	return 0;
}	


int CALLBACK WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine,
	int ShowCode
	)
{
	Win32CreateInitialWindow(Instance);

	return 0;
}
