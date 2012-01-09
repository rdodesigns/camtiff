/* error.h - Defines the error macros used by camtiff
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk>
 * Date: 08/01/12 19:20:46
 *
 * Copyright GPL V3
 */

#ifndef ERROR_H

#define ERROR_H

#include <stdio.h>

// Conditional debug messages.
#ifdef DEBUG
  #define DEBUGP(e) printf("%s\n", e);
#else
  #define DEBUGP(e)
#endif

// Simplified try something and return error functions.
#define TRYFUNC(e, msg)    \
  { int retval;            \
    if ((retval = e)) {    \
      printf("%s\n", msg); \
      return retval;       \
    }                      \
  }

#define TRYRETURN(e, msg, retcode) \
    if (e) {                       \
      printf("%s\n", msg);         \
      return retcode;              \
    }

#endif /* end of include guard: ERROR_H */
