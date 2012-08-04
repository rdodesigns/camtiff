/**
 * @file ctiff.h
 * @description A TIFF + JSON image writing library.
 *
 * This is the opaque header that should be included in statically compiled
 * binaries.
 *
 * Created by Ryan Orendorff <ryan@rdodesigns.com> 28/10/11 21:00:27
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

#include "ctiff_types.h"

#if defined(LIB) && defined(WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
#endif

extern CTIFF CTIFFNew(const char*);
extern int CTIFFClose(CTIFF);
extern int CTIFFSetBasicMeta(CTIFF ctiff,
                      const char *artist,
                      const char *copyright,
                      const char *make,
                      const char *model,
                      const char *software,
                      const char *image_desc);
extern int CTIFFSetStyle(CTIFF ctiff,
                      unsigned  int width,
                      unsigned  int height,
                      unsigned  int pixel_type,
                               bool in_color);
extern int CTIFFSetRes(CTIFF ctiff, unsigned int x_res, unsigned int y_res);
extern int CTIFFAddNewPage(CTIFF, const void *page,
                                  const char *extended_metadata_name,
                                  const void *extended_metadata);
extern int CTIFFWrite(CTIFF);
extern int CTIFFClose(CTIFF);
extern void CTIFFWriteEvery(CTIFF ctiff, unsigned int num_pages);
extern int CTIFFSetStrict(CTIFF ctiff, bool strict);

#endif // end CTIFF header lock
