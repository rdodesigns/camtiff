/* camtiff.h - A TIFF image writing library for spectroscopic data.  This file
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

#include <stdint.h>  // uints
#include <tiffio.h>  // libTIFF (preferably 3.9.5+)
#include <stdbool.h> // bool type

#if defined(WIN32) && !defined(__WIN32)
#define __WIN32
#endif

// Windows corrections
#if defined(__WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// Version info
#define CAMTIFF_VERSION 1

// Error Codes
#define EWRITEDIR  1
#define EMALLOC    2
#define ETIFFOPEN  3
#define ETIFFWRITE 4
#define EBITDEPTH  5
#define EPAGEZERO  6

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

extern __declspec(dllexport)
int tiffWrite(uint32_t width, uint32_t height,
              uint32_t pages, uint8_t pixel_bit_depth,
              const char* artist, const char* copyright, const char* make,
              const char* model, const char* software, const char* image_desc,
              const char* ext_metadata, bool strict,
              const char* output_path, const void* const buffer);
#endif

// TODO: Fill out metadata struct
struct basic_metadata {
  const char* artist;
  const char* copyright;
  const char* make;
  const char* model;
  const char* software;
  const char* image_desc;
};

//struct ext_metadata {

//};

struct stack {
  uint32_t width;
  uint32_t height;
  uint32_t pages;
  uint8_t pixel_bit_depth;
  struct basic_metadata b_meta;
  void* buffer;
};

struct page {
  uint32_t width;
  uint32_t height;
  uint8_t pixel_bit_depth;
  struct basic_metadata b_meta;
  const void* buffer;
};


// TODO: Fill out image struct
struct image {
  TIFF* tiff;
  struct stack stack;
  struct basic_metadata meta;
};

int writeSubFile(TIFF *image, struct page page, struct basic_metadata b_meta,
                 const char* ext_meta, bool strict);

int tiffWrite(uint32_t width, uint32_t height,
              uint32_t pages, uint8_t pixel_bit_depth,
              const char* artist, const char* copyright, const char* make,
              const char* model, const char* software, const char* image_desc,
              const char* ext_metadata, bool strict,
              const char* output_path, const void* const buffer);

