// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uix11bitmapimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11

#include "uiapp.h"
#include "uigraphics.h"
#include "uix11app.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xdbe.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../../detail/portable.h"

typedef struct BmpHeader {
   uint16_t type;       /* Magic identifier            */
   uint32_t size;       /* File size in bytes          */
   uint16_t reserved1;
   uint16_t reserved2;
   uint32_t offset;     /* Offset to image data, bytes */
} BmpHeader;

typedef struct BmpInfo {
   uint32_t size;             /* Header size in bytes      */
   int32_t width;             /* Width and height of image */
   int32_t height;                
   uint16_t planes;           /* Number of colour planes   */
   uint16_t bits;             /* Bits per pixel            */
   uint32_t compression;      /* Compression type          */
   uint32_t imagesize;        /* Image size in bytes       */
   int32_t xresolution;
   int32_t yresolution;       /* Pixels per meter          */
   uint32_t ncolours;         /* Number of colours         */
   uint32_t importantcolours; /* Important colours         */
} BmpInfo;

typedef struct BmpColourIndex {
   uint8_t r,g,b,junk;
} BmpColourIndex;

static Bool
bigendian (void)
{
  union { int i; char c[sizeof(int)]; } u;
  u.i = 1;
  return !u.c[0];
}

/* prototypes */
static void dispose(psy_ui_x11_BitmapImp*);
static int load(psy_ui_x11_BitmapImp*, const char* path);
static int loadresource(psy_ui_x11_BitmapImp*, int resourceid);
static psy_ui_RealSize dev_size(psy_ui_x11_BitmapImp*);
static int empty(psy_ui_x11_BitmapImp*);
static void dev_settransparency(psy_ui_x11_BitmapImp*, psy_ui_Colour colour);
static void dev_preparemask(psy_ui_x11_BitmapImp*, psy_ui_Colour clrtrans);

/* vtable */
static psy_ui_BitmapImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_x11_BitmapImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose =
			(psy_ui_bitmap_imp_fp_dispose)
			dispose;
		imp_vtable.dev_load =
			(psy_ui_bitmap_imp_fp_load)
			load;
		imp_vtable.dev_loadresource =
			(psy_ui_bitmap_imp_fp_loadresource)
			loadresource;
		imp_vtable.dev_size =
			(psy_ui_bitmap_imp_fp_size)
			dev_size;
		imp_vtable.dev_empty =
			(psy_ui_bitmap_imp_fp_empty)
			empty;
		imp_vtable.dev_settransparency =
			(psy_ui_bitmap_imp_fp_settransparency)
			dev_settransparency;
		imp_vtable_initialized = TRUE;
	}
}

/* implementation */
void psy_ui_x11_bitmapimp_init(psy_ui_x11_BitmapImp* self,
	psy_ui_RealSize size)
{
	psy_ui_bitmap_imp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	self->mask = 0;	
	if (size.width == 0 && size.height == 0) {
		self->pixmap = 0;
	} else {
		psy_ui_X11App* x11app;
		int screen;
				
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		screen = DefaultScreen(x11app->dpy);		
		self->pixmap = XCreatePixmap(x11app->dpy,
			DefaultRootWindow(x11app->dpy), (int)size.width, (int)size.height,
			DefaultDepth(x11app->dpy, screen));		
	}
}

void dispose(psy_ui_x11_BitmapImp* self)
{
	if (self->pixmap) {
		psy_ui_X11App* x11app;		
				
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		XFreePixmap(x11app->dpy, self->pixmap);
		self->pixmap = 0;
	}
	if (self->mask) {
		psy_ui_X11App* x11app;		
				
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		XFreePixmap(x11app->dpy, self->mask);
		self->mask = 0;
	}
}

int load(psy_ui_x11_BitmapImp* self, const char* path)
{	
	FILE* fp;
	BmpHeader header;
    BmpInfo infoheader;
    BmpColourIndex colourindex[256];
    psy_ui_X11App* x11app;
    uintptr_t bytesRead;
	int32_t i,j;
	int32_t gotindex = FALSE;
	uint8_t grey,r,g,b;
	GC gc;
	psy_ui_Colour colour;
	uint32_t padding;
	bool verbose;
	
	verbose = 0;
	if ((fp = fopen(path,"rb")) == NULL) {
		return PSY_ERRFILE;
	}
	/* Magic identifier            */
	bytesRead = fread(&header.type, sizeof(char), 2, fp);
	if (header.type != 'M'*256+'B') {
		/* todo warning */
	}
	/* File size in bytes          */
	bytesRead = fread(&header.size, sizeof(char), 4, fp);
	/* reserved					   */
	bytesRead = fread(&header.reserved1, sizeof(char), 2, fp);
	bytesRead = fread(&header.reserved2, sizeof(char), 2, fp);
	/* Offset to image data, bytes */
	bytesRead = fread(&header.offset, sizeof(char), 4, fp);
	 /* Read and check the information header */
	if (fread(&infoheader,sizeof(BmpInfo),1,fp) != 1) {
		return PSY_ERRFILE;      
	}	
	if (verbose) {	
	 fprintf(stderr,"Image size = %d x %d\n",infoheader.width,infoheader.height);
	 fprintf(stderr,"Number of colour planes is %d\n",infoheader.planes);
     fprintf(stderr,"Bits per pixel is %d\n",infoheader.bits);
     fprintf(stderr,"Compression type is %d\n",infoheader.compression);
     fprintf(stderr,"Number of colours is %d\n",infoheader.ncolours);
     fprintf(stderr,"Number of required colours is %d\n",
        infoheader.importantcolours);
	}
	/* Read the lookup table if there is one */
	for (i=0;i<255;i++) {
		colourindex[i].r = rand() % 256;
		colourindex[i].g = rand() % 256;
		colourindex[i].b = rand() % 256;
		colourindex[i].junk = rand() % 256;
	}
	if (infoheader.ncolours > 0) {
      for (i=0;i<infoheader.ncolours;i++) {
         if (fread(&colourindex[i].b,sizeof(unsigned char),1,fp) != 1) {
            fprintf(stderr,"Image read failed\n");
            exit(-1);
         }
         if (fread(&colourindex[i].g,sizeof(unsigned char),1,fp) != 1) {
            fprintf(stderr,"Image read failed\n");
            exit(-1);
         }
         if (fread(&colourindex[i].r,sizeof(unsigned char),1,fp) != 1) {
            fprintf(stderr,"Image read failed\n");
            exit(-1);
         }
         if (fread(&colourindex[i].junk,sizeof(unsigned char),1,fp) != 1) {
            fprintf(stderr,"Image read failed\n");
            exit(-1);
         }
         fprintf(stderr,"%3d\t%3d\t%3d\t%3d\n",i,
            colourindex[i].r,colourindex[i].g,colourindex[i].b);
      }
      gotindex = TRUE;
   }
    /* Seek to the start of the image data */
   fseek(fp,header.offset,SEEK_SET);
   if (verbose) {
     fprintf(stderr,"Image size = %d x %d\n",infoheader.width,infoheader.height);
   }
   psy_ui_x11_bitmapimp_init(self,
   psy_ui_realsize_make(
		infoheader.width,
		infoheader.height)); 		
   x11app = (psy_ui_X11App*)psy_ui_app()->imp;	
   gc = XCreateGC(x11app->dpy, self->pixmap, 0, NULL);
   psy_ui_colour_init_rgb(&colour, 255, 255, 255);
            XSetForeground(x11app->dpy, gc,
				psy_ui_x11app_colourindex(x11app, colour));	
   XFillRectangle(x11app->dpy, self->pixmap, gc,
		0, 0, infoheader.width,
		infoheader.height);
   /* Read the image */
   padding = (infoheader.width * 3) % 4;
   for (j=0;j<infoheader.height;j++) {
      for (i=0;i<infoheader.width;i++) {

         switch (infoheader.bits) {
         case 1:
            break;
         case 4:
            break;
         case 8:
            if (fread(&grey,sizeof(unsigned char),1,fp) != 1) {
               fprintf(stderr,"Image read failed\n");
               exit(-1);
            }
            if (gotindex) {
               putchar(colourindex[grey].r);
               putchar(colourindex[grey].g);
               putchar(colourindex[grey].b);
            } else {
               putchar(grey);
            }
            break;
         case 24:
            if (fread(&b,sizeof(unsigned char),1,fp) != 1) {
               fprintf(stderr,"Image read failed\n");
               exit(-1);
            }
            if (fread(&g,sizeof(unsigned char),1,fp) != 1) {
               fprintf(stderr,"Image read failed\n");
               exit(-1);
            }
            if (fread(&r,sizeof(unsigned char),1,fp) != 1) {
               fprintf(stderr,"Image read failed\n");
               exit(-1);
            }                                    
			psy_ui_colour_init_rgb(&colour, r, g, b);
            XSetForeground(x11app->dpy, gc,
				psy_ui_x11app_colourindex(x11app, colour));									
            XDrawPoint(x11app->dpy, self->pixmap, gc, i, infoheader.height - j);      
            if (verbose) {      			
              fprintf(stderr,"Point = %d , %d, %d, %d, %d\n",i, infoheader.height - j,
              (int)r, (int)g, (int)b);
			}
           // putchar(r);
          //  putchar(g);
          //  putchar(b);
            break;
         }

      } /* i */
      for (i = 0; i < padding; ++i) {
		  fread(&r,sizeof(unsigned char),1,fp);
	  }      
   } /* j */
   XFreeGC(x11app->dpy, gc);	
   fclose(fp);
	return PSY_OK;
}

int loadresource(psy_ui_x11_BitmapImp* self, int resourceid)
{
	/*HBITMAP bitmap;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	bitmap = LoadBitmap(winapp->instance, MAKEINTRESOURCE(resourceid));
	if (bitmap != NULL) {
		dispose(self);
		self->bitmap = bitmap;
	}
	return bitmap == 0;*/
	return 0;
}

psy_ui_RealSize dev_size(psy_ui_x11_BitmapImp* self)
{
	psy_ui_RealSize size;    

	if (self->pixmap) {
        Window root;
        unsigned int temp;
        unsigned int width = 0;
        unsigned int height = 0;
        psy_ui_X11App* xtapp;		

        xtapp = (psy_ui_X11App*)psy_ui_app()->imp;
        XGetGeometry(xtapp->dpy, self->pixmap, &root, &temp, &temp,
            &width, &height, &temp, &temp);
		size.width = width;
		size.height = height;
	} else {
		size.width = 0.0;
		size.height = 0.0;
	}
	return size;
}

int empty(psy_ui_x11_BitmapImp* self)
{
	return self->pixmap == 0;
}

void dev_settransparency(psy_ui_x11_BitmapImp* self, psy_ui_Colour colour)
{
	dev_preparemask(self, colour);
}

void dev_preparemask(psy_ui_x11_BitmapImp* self, psy_ui_Colour clrtrans)
{	
	
}	

#endif
