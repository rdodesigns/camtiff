/**
 * @file ctiff_settings.h
 * @description Set parameters for a CamTIFF file.
 *
 * Created by Ryan Orendorff <ryan@rdodesigns.com> 18/03/12 16:52:58
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

#ifndef CTIFF_SETTINGS_H

#define CTIFF_SETTINGS_H

#include "ctiff_types.h"

int CTIFFWriteEvery(CTIFF ctiff, unsigned int num_pages);


int CTIFFSetStrict(CTIFF ctiff, bool strict);

int CTIFFSetBasicMeta(CTIFF ctiff,
                      const char *artist,
                      const char *copyright,
                      const char *make,
                      const char *model,
                      const char *software,
                      const char *image_desc);

int CTIFFSetStyle(CTIFF ctiff,
                      unsigned  int width,
                      unsigned  int height,
                      unsigned  int pixel_type,
                               bool in_color);

int CTIFFSetRes(CTIFF ctiff, unsigned int x_res, unsigned int y_res);
#endif /* end of include guard: CTIFF_SETTINGS_H */
