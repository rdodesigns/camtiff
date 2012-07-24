/**
 * ctiff_util.c - A TIFF image writing library for spectroscopic data.
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

#include <time.h>
#include <stdlib.h> // malloc

#include "ctiff_util.h"


/** Get the current time of the local machine in UTC.
 *
 *  @return New malloced time string on success, NULL on failure.
 */
const char* __CTIFFGetTime()
{
  time_t local_current_time;
  char  *time_str = (char*) malloc(20*sizeof(char));

#ifdef __WIN32
  int retval;
  struct tm gmt_current_time;

  // Get time of slice
  time(&local_current_time);

  retval = gmtime_s(&gmt_current_time, &local_current_time);
  if (retval) return NULL;

  strftime(time_str, 20, "%Y:%m:%d %H:%M:%S", &gmt_current_time);
#else
  struct tm* gmt_current_time;

  // Get time of slice
  time(&local_current_time);
  gmt_current_time = gmtime(&local_current_time);
  strftime(time_str, 20, "%Y:%m:%d %H:%M:%S", gmt_current_time);
#endif

  return time_str;
}
