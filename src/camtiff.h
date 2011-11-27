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

#include <stdint.h>
#include <tiffio.h> // libTIFF (preferably 3.9.5+)

#if defined(WIN32) && !defined(__WIN32)
#define __WIN32
#endif

// Windows corrections
#if defined(__WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


// Error Codes
#define EWRITEDIR  1
#define EMALLOC    2
#define ETIFFOPEN  3
#define ETIFFWRITE 4

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

extern __declspec(dllexport)int tiffWrite(uint16_t width,
                              uint16_t height, uint16_t pages,
                              char* artist, char* copyright, char* make,
                              char* model, char* software, char* image_desc,
                              char* output_path, uint16_t* buffer);
#endif


int writeSubFile(TIFF *image, uint16_t width, uint16_t height,
                 char* artist, char* copyright, char* make, char* model,
                 char* software, char* image_desc,
                 uint16_t* buffer);

int tiffWrite(uint16_t width, uint16_t height, uint16_t pages,
              char* artist, char* copyright, char* make, char* model,
              char* software, char* image_desc,
              char* output_path, uint16_t* buffer);
