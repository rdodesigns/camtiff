/**
 * @file ctiff_types.h
 * @description Define the data structures in CamTIFF.
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk> 18/03/12 16:51:10
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

#ifndef CTIFF_TYPES_H

#define CTIFF_TYPES_H

#if defined(WIN32) && !defined(__WIN32)
#define __WIN32
#endif

// Windows corrections
#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __WIN32
  #define bool	BOOL
  #define true	1
  #define false 0
  #define __bool_true_false_are_defined   1
#else
  #include <stdbool.h> // bool type
#endif

// Alleviates need to import libTIFF here.
struct tiff;

#define CTIFF_PIXEL_TYPE_MIN 1
#define CTIFF_PIXEL_TYPE_MAX 3
/** The pixel types for CamTIFF.
 *
 *  CamTIFF pixel types combine TIFF pixel types (numbers 1-6) with the size
 *  of the pixel in question, where the TIFF pixel type is in 0xF0 and the
 *  size is in 0x0F.  Use the following equations to calculate either the
 *  pixel type or size.
 *
 *  Pixel size = ((pixel_type & 0x0F) + 0x01) << 3
 *  TIFF Pixel type = (pixel_type >> 4) & 0x0F
 */
enum pixel_type_e {           // LibTIFF tags
  CTIFF_PIXEL_UINT8   = 0x10, // SAMPLEFORMAT_UINT   = 1,
  CTIFF_PIXEL_UINT16  = 0x11,
  CTIFF_PIXEL_UINT32  = 0x13,
  CTIFF_PIXEL_INT8    = 0x20, // SAMPLEFORMAT_INT    = 2,
  CTIFF_PIXEL_INT16   = 0x21,
  CTIFF_PIXEL_INT32   = 0x23,
  CTIFF_PIXEL_FLOAT32 = 0x33, // SAMPLEFORMAT_IEEEFP = 3
  CTIFF_PIXEL_FLOAT64 = 0x37
};
/* TODO: Support the complex pixel data types.
 *   SAMPLEFORMAT_VOID          = 4 // Does not reflect real life signals
 *   SAMPLEFORMAT_COMPLEXINT    = 5
 *   SAMPLEFORMAT_COMPLEXIEEEFP = 6
 */


/** Structure for holding basic metadata about an image. */
typedef struct {
  const char *artist;
  const char *copyright;
  const char *make;
  const char *model;
  const char *software;
  const char *image_desc;
} CTIFF_basic_metadata;

/** Structure for holding the extended metadata about an image.
 *
 *  This structure is usually created dynamically, and should be freed with
 *  __CTIFFFreeExtMeta.
 * @see __CTIFFFreeExtMeta
 */
typedef struct {
  const char   *data;
} CTIFF_extended_metadata;

/** Structure for holding the style (width, height, etc) of a directory. */
typedef struct CTIFF_dir_style_s {
  unsigned  int width;
  unsigned  int height;
  unsigned  int bps;
  unsigned char pixel_data_type;
           bool in_color;
           bool black_is_min;
  unsigned  int x_res;
  unsigned  int y_res;
} CTIFF_dir_style;

/** Structure for holding an image and its associated metadata.
 *
 *  This structure is usually created dynamically, and should be freed with
 *  __CTIFFFreeDir
 * @see __CTIFFFreeDir
 */
typedef struct CTIFF_dir_s {
          CTIFF_dir_style  style;
     CTIFF_basic_metadata  basic_meta;
  CTIFF_extended_metadata  ext_meta;
               const char *timestamp;
               const void *data;
       struct CTIFF_dir_s *next_dir;
                      int  refs;
} CTIFF_dir;

// TODO: Implement node form of directory storage by 0.1.0a4
typedef struct node_s {
  CTIFF_dir *dir;
  CTIFF_dir *next_dir;
} node;

/** Structure for holding a set of CamTIFF directories.
 *
 *  This structure is usually created dynamically, and should be freed with
 *  __CTIFFFree.
 * @see __CTIFFFree
 */
typedef struct CTIFF_s {
  struct tiff  *tiff;
  const char   *output_file;
  unsigned int  num_dirs;
  unsigned int  num_page_styles;
  bool          strict;
  unsigned int  write_every_num;
  unsigned int  num_unwritten;

  CTIFF_dir    *def_dir;
  CTIFF_dir    *first_dir;
  CTIFF_dir    *last_dir;
  CTIFF_dir    *write_ptr;
} * CTIFF;

#endif /* end of include guard: CTIFF_TYPES_H */
