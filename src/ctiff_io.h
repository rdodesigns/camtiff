/* ctiff_io.h - A TIFF image writing library for spectroscopic data.
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk> 18/03/12 16:52:58
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

#ifndef CTIFF_IO_H

#define CTIFF_IO_H

#include "ctiff_types.h"

CTIFF CTIFFNewFile(const char* output_file);
int CTIFFCloseFile(CTIFF ctiff);

int tiffWrite(unsigned int width,
              unsigned int height,
              unsigned int pages,
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
              const void* const buffer);

#endif /* end of include guard: CTIFF_IO_H */
