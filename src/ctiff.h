/* ctiff.h - A TIFF image writing library for spectroscopic data.  This file
 * is part of camtiff.
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk> 28/10/11 21:00:27
 *
 * The Laser Analytics Tiff Writer University of Cambridge (camtiff) is a
 * library designed to, given an input 16 bit 3D array and some additional
 * comments, produce a TIFF image stack. It is designed to work with a piece of
 * LabVIEW software within the Laser Analytics group codenamed Apollo, a front
 * end for acquiring spectroscopic images.
 *
 * Copyright (GPL V3): This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Header Lock
#ifndef CTIFF_H
#define CTIFF_H

#if defined(WIN32) && !defined(__WIN32)
#define __WIN32
#endif

#include <stdint.h>  // uints
#include <tiffio.h>  // libTIFF (preferably 3.9.5+)
//#include "CTIFF_error.h"

#ifdef __WIN32
  #define bool	BOOL
  #define true	1
  #define false 0
  #define __bool_true_false_are_defined   1
#else
  #include <stdbool.h> // bool type
#endif

// Windows corrections
#if defined(__WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// Version info
#define CTIFF_MAJOR_VERSION 0
#define CTIFF_MINOR_VERSION 1
#define CTIFF_MAINT_VERSION 0

#define CTIFF_TESTING_VERSION "a0"



enum pixel_type {
  CTIFF_PIXEL_UINT = 1,
  CTIFF_PIXEL_INT,
  CTIFF_PIXEL_FLOAT,
  CTIFF_PIXEL_UNKNOWN,
  CTIFF_PIXEL_COMPLEXINT,
  CTIFF_PIXEL_COMPLEXFLOAT
};


#if defined(LIB) && defined(__WIN32)
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#else
#define __declspec(dllexport)
#endif
extern __declspec(dllexport)
int tiffWrite( uint32_t width,
               uint32_t height,
               uint32_t pages,
               uint8_t pixel_bit_depth,
               unsigned int pixel_type,
               const char* artist,
               const char* copyright,
               const char* make,
               const char* model,
               const char* software,
               const char* image_desc,
               const char* ext_metadata_name,
               const char* ext_metadata,
               bool strict,
               const char* output_path,
               const void* const buffer );


#endif // end CTIFF header lock
