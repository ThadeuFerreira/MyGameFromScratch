internal void
DrawBitmapInvert(game_Off_Screen_Buffer *Buffer, loaded_bitmap *Bitmap, 
			real32 RealX, real32 RealY,
           int32 AlignX = 0, int32 AlignY = 0)
{
    RealX -= (real32)AlignX;
    RealY -= (real32)AlignY;
    
    int32 MinX = RoundReal32ToInt32(RealX);
    int32 MinY = RoundReal32ToInt32(RealY);
    int32 MaxX = RoundReal32ToInt32(RealX + (real32)Bitmap->Width);
    int32 MaxY = RoundReal32ToInt32(RealY + (real32)Bitmap->Height);

    int32 SourceOffsetX = 0;
    if(MinX < 0)
    {
        SourceOffsetX = MinX;
        MinX = 0;
    }

    int32 SourceOffsetY = 0;
    if(MinY < 0)
    {
        SourceOffsetY = -MinY;
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

    // TODO(casey): SourceRow needs to be changed based on clipping.
    uint32 *SourceRow = Bitmap->Pixels;
    SourceRow += SourceOffsetY*Bitmap->Width - SourceOffsetX;
    uint8 *DestRow = ((uint8 *)Buffer->Memory +
                      MinX*Buffer->BytesPerPixel +
                      MinY*Buffer->Pitch);
	
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
        uint32 *Dest = (uint32 *)DestRow;
        uint32 *Source = SourceRow;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {
            real32 A = (real32)((*Source >> 24) & 0xFF) / 255.0f;
			if(A != 0) A = A/2;
            real32 SR = (real32)((*Source >> 16) & 0xFF);
            real32 SG = (real32)((*Source >> 8) & 0xFF);
            real32 SB = (real32)((*Source >> 0) & 0xFF);

            real32 DR = (real32)((*Dest >> 16) & 0xFF);
            real32 DG = (real32)((*Dest >> 8) & 0xFF);
            real32 DB = (real32)((*Dest >> 0) & 0xFF);
			
			//SR = (real32)(DR + SR)/1.5f;
			//SG = (real32)(DG + SG)/1.5f;
			//SB = (real32)(DB + SB)/1.5f;
			

            // TODO(casey): Someday, we need to talk about premultiplied alpha!
            // (this is not premultiplied alpha)
            real32 R = (1.0f-A)*DR + A*SR;
            real32 G = (1.0f-A)*DG + A*SG;
            real32 B = (1.0f-A)*DB + A*SB;

            *Dest = (((uint32)(R + 0.5f) << 16) |
                     ((uint32)(G + 0.5f) << 8) |
                     ((uint32)(B + 0.5f) << 0));
            
            ++Dest;
            ++Source;
        }

        DestRow += Buffer->Pitch;
        SourceRow += Bitmap->Width;
    }
}
