//////////////////////////////////////////////////////////////////////
// Canvas class

class Canvas
{
public:
    // operator cast to HDC
    // (used when passing Canvas to Windows API)
    operator HDC () { return _hdc; }

    void Point (int x, int y, COLORREF color)
    {
        ::SetPixel (_hdc, x, y, color);
    }

    void MoveTo (int x, int y)
    {
        ::MoveToEx (_hdc, x, y, 0);
    }

    void Line ( int x1, int y1, int x2, int y2 )
    {
        MoveToEx (_hdc, x1, y1, 0);
        LineTo (_hdc, x2, y2);
    }

    void Rectangle (int left, int top, int right, int bottom)
    {
        // draw rectangle using current pen
        // and fill it using current brush
        ::Rectangle (_hdc, left, top, right, bottom);
    }

    void GetTextSize (int & cxChar, int & cyChar)
    {
        TEXTMETRIC tm;
        GetTextMetrics (_hdc, & tm);
        cxChar = tm.tmAveCharWidth;
        cyChar = tm.tmHeight + tm.tmExternalLeading;
    }

    void Text (int x, int y, char const * buf, int cBuf)
    {
        ::TextOut ( _hdc, x, y, buf, cBuf );
    }

    void Char (int x, int y, char c)
    {
        TextOut (_hdc, x, y, & c, 1);
    }

    void SelectObject (void* pObj)
    {
        ::SelectObject (_hdc, pObj);
    }

protected:

    Canvas (HDC hdc): _hdc (hdc) {}

    HDC  _hdc;
};

class MemCanvas: public Canvas
{
public:
    MemCanvas (HDC hdc) 
        : Canvas (::CreateCompatibleDC (hdc))
    {}

    ~MemCanvas ()
    {
        ::DeleteDC(_hdc); 
    }
};

class PaintCanvas: public Canvas
{
public:
    // Constructor obtains the DC
    PaintCanvas (HWND hwnd)
    : Canvas (BeginPaint (hwnd, & _paint)),
      _hwnd (hwnd)
    {}

    // Destructor releases the DC
    ~PaintCanvas ()
    {
        EndPaint (_hwnd, & _paint);
    }

protected:

    PAINTSTRUCT _paint;
    HWND        _hwnd;
};
class UpdateCanvas: public Canvas
{
public:
    UpdateCanvas (HWND hwnd)
    :   Canvas (GetDC (hwnd)),
        _hwnd(hwnd)
    {}

    ~UpdateCanvas ()
    {
        ReleaseDC (_hwnd, _hdc);
    }

protected:

    HWND _hwnd;
};


//////////////////////////////////////////////////////////////////////
// Bitmap class

class Bitmap
{
public:
    Bitmap ()
		: _hBitmap (0)
    {}
    // Transfer semantics
    Bitmap (Bitmap & bmp)
        : _hBitmap (bmp.Release ())
    {}

    void operator = (Bitmap & bmp)
    {
        if (bmp._hBitmap != _hBitmap)
        {
            Free ();
            _hBitmap = bmp.Release ();
        }
    }

    HBITMAP Release ()
    {
        HBITMAP h = _hBitmap;
        _hBitmap = 0;
        return h;
    }
    
	
	Bitmap::Bitmap (Canvas & canvas, int dx, int dy)
   : _hBitmap (0)
	{
    CreateCompatible (canvas, dx, dy);
	}

	void Load (HINSTANCE hInst, char const * resName)
	{
    Free ();
    _hBitmap = (HBITMAP) ::LoadImage (hInst, 
        resName,
        IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    
		//if (_hBitmap == 0)
        //throw WinException ("Cannot load bitmap from resources", resName);
	}

	void Load (HINSTANCE hInst, int id)
	{
    Free ();
    _hBitmap = (HBITMAP) ::LoadImage (hInst, 
        MAKEINTRESOURCE (id),
        IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	//   if (_hBitmap == 0)
    //  throw WinException ("Cannot load bitmap from resources");
	}

	void Load (char * path)
	{
    Free ();
    _hBitmap = (HBITMAP) ::LoadImage (0, path, 
        IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //if (_hBitmap == 0)
    //    throw WinException ("Cannot load bitmap from file", path);
	}
   
	void GetSize (int & width, int & height)
	{
    BITMAP bm;
    ::GetObject (_hBitmap, sizeof (bm), & bm);
    width = bm.bmWidth;
    height = bm.bmHeight;
	}

	void Bitmap::CreateCompatible (Canvas & canvas, int width, int height)
	{
    Free ();
    _hBitmap = ::CreateCompatibleBitmap (canvas, width, height);
	}

	~Bitmap ()
    {
        Free ();
    }
	
	// implicit conversion for use with Windows API
    operator HBITMAP () { return _hBitmap; }

protected:
    Bitmap (HBITMAP hBitmap)
        : _hBitmap (hBitmap)
    {}
    void Free ()
    {
        if (_hBitmap) 
            ::DeleteObject (_hBitmap);
    }

    HBITMAP    _hBitmap;
};

//////////////////////////////////////////////////////////////////////
// Blitter class

class Blitter
{
public:
    Blitter (Bitmap & bmp)
        : _bmp (bmp), _mode (SRCCOPY),
          _xDst (0), _yDst (0),
          _xSrc (0), _ySrc (0)
    {
        bmp.GetSize (_width, _height);
    }
    void SetMode (DWORD mode)
    {
        _mode = mode;
    }
    void SetDest (int x, int y)
    {
        _xDst = x;
        _yDst = y;
    }
    void SetSrc (int x, int y)
    {
        _xSrc = x;
        _ySrc = y;
    }
    void SetArea (int width, int height)
    {
        _width = width;
        _height = height;
    }

    // transfer bitmap to canvas
    void BlitTo (Canvas & canvas);

private:
    Bitmap & _bmp;
    int     _xDst, _yDst;
    int     _xSrc, _ySrc;
    int     _width, _height;
    DWORD   _mode;
};

////
//
// Implementation
//
////

//////////////////////////////////////////////////////////////////////
// Blitter implementation

void Blitter::BlitTo (Canvas & canvas)
{
    // Create canvas in memory using target canvas as template
    MemCanvas memCanvas (canvas);
    // Convert bitmap to the format appropriate for this canvas
    memCanvas.SelectObject (_bmp);
    // Transfer bitmap from memory canvas to target canvas
    ::BitBlt (canvas, _xDst, _yDst, _width, _height, 
              memCanvas, _xSrc, _ySrc, _mode);
}
