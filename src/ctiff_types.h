/* ctiff_types.h - A TIFF image writing library for spectroscopic data.
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk> 18/03/12 16:51:10
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

#ifndef CTIFF_TYPES_H

#define CTIFF_TYPES_H

typedef struct {
  const char *artist;
  const char *copyright;
  const char *make;
  const char *model;
  const char *software;
  const char *image_desc;
} CTIFF_basic_metadata;

typedef struct {
  const char   *data;
  const char   *white_space;
} CTIFF_extended_metadata;

typedef struct CTIFF_dir_style_s {
  unsigned  int width;
  unsigned  int height;
  unsigned  int bps;
  unsigned char pixel_data_type;
           bool in_color;
           bool black_is_min;
  unsigned  int x_resolution;
  unsigned  int y_resolution;
} CTIFF_dir_style;

typedef struct CTIFF_dir_s {
          CTIFF_dir_style  style;
     CTIFF_basic_metadata  basic_meta;
  CTIFF_extended_metadata  ext_meta;
               const char *timestamp;
               const void *data;
       struct CTIFF_dir_s *next_dir;
                      int  refs;
} CTIFF_dir;

typedef struct CTIFF_s {
  TIFF         *tiff;
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
