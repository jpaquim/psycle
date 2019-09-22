// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "bitmap.h"

extern HINSTANCE appInstance;
static BOOL LoadBitmapFromBMPFile(LPTSTR szFileName, HBITMAP *phBitmap,
	HPALETTE *phPalette );

void ui_bitmap_init(ui_bitmap* self)
{
	self->hBitmap = 0;
}

void ui_bitmap_dispose(ui_bitmap* self)
{
	DeleteObject(self->hBitmap);
	self->hBitmap = 0;
}

void ui_bitmap_load(ui_bitmap* self, const char* path)
{
	int err;	
	if (appInstance) {		
		HPALETTE hPalette;
		LoadBitmapFromBMPFile((LPTSTR)path, &self->hBitmap, &hPalette);		
		err = GetLastError();
	}
}

BOOL LoadBitmapFromBMPFile(LPTSTR szFileName, HBITMAP *phBitmap,
	HPALETTE *phPalette)
{
	BITMAP  bm;

	*phBitmap = NULL;
	*phPalette = NULL;

	// Use LoadImage() to get the image loaded into a DIBSection
	*phBitmap = LoadImage( NULL, szFileName, IMAGE_BITMAP, 0, 0,
			 LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE );
	if( *phBitmap == NULL )
	return FALSE;

	// Get the color depth of the DIBSection
	GetObject(*phBitmap, sizeof(BITMAP), &bm );
	// If the DIBSection is 256 color or less, it has a color table
	if( ( bm.bmBitsPixel * bm.bmPlanes ) <= 8 )
	{
		HDC           hMemDC;
		HBITMAP       hOldBitmap;
		RGBQUAD       rgb[256];
		LPLOGPALETTE  pLogPal;
		WORD          i;

		// Create a memory DC and select the DIBSection into it
		hMemDC = CreateCompatibleDC( NULL );
		hOldBitmap = SelectObject( hMemDC, *phBitmap );
		// Get the DIBSection's color table
		GetDIBColorTable( hMemDC, 0, 256, rgb );
		// Create a palette from the color table
		pLogPal = malloc( sizeof(LOGPALETTE) + (256*sizeof(PALETTEENTRY)) );
		pLogPal->palVersion = 0x300;
		pLogPal->palNumEntries = 256;
		for(i=0;i<256;i++)
		{
			pLogPal->palPalEntry[i].peRed = rgb[i].rgbRed;
			pLogPal->palPalEntry[i].peGreen = rgb[i].rgbGreen;
			pLogPal->palPalEntry[i].peBlue = rgb[i].rgbBlue;
			pLogPal->palPalEntry[i].peFlags = 0;
		}
		*phPalette = CreatePalette( pLogPal );
		// Clean up
		free( pLogPal );
		SelectObject( hMemDC, hOldBitmap );
		DeleteDC( hMemDC );
	}
	else   // It has no color table, so use a halftone palette
	{
		HDC    hRefDC;

		hRefDC = GetDC( NULL );
		*phPalette = CreateHalftonePalette( hRefDC );
		ReleaseDC( NULL, hRefDC );
	}
	return TRUE;
}   


ui_size ui_bitmap_size(ui_bitmap* self)
{
	ui_size size;
	BITMAP bitmap ;
	
	GetObject(self->hBitmap, sizeof (BITMAP), &bitmap);
	size.width = bitmap.bmWidth;
	size.height = bitmap.bmHeight;
	return size;
}

void ui_bitmap_loadresource(ui_bitmap* self, int resourceid)
{
	self->hBitmap = LoadBitmap (appInstance, MAKEINTRESOURCE(resourceid));
}
