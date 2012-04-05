/**
 * ctiff_util.h - A TIFF image writing library for spectroscopic data.
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk> 18/03/12 16:52:58
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

#ifndef CTIFF_UTIL_H

#define CTIFF_UTIL_H

#ifdef WIN32
#define inline __inline // Microsoft, I hate you (uses C89).
#endif

#define FREE(p)    do { free((void*) (p)); (p) = NULL; } while(0)
#define RETNONZERO(f) if ((retval = (f)) == 0) return retval

static inline const void* __movePtr(const void* const ptr,
                                    unsigned int dist, unsigned int size)
{
  return (void *) (((char *) ptr)+(dist*size/8));
}

const char* __CTIFFGetTime();

#endif /* end of include guard: CTIFF_UTIL_H */
