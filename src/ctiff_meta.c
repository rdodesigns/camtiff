/**
 * @file ctiff_meta.c
 * @description Functions for creating/validating metadata.
 *
 * Created using a pushdown automaton parser.
 *
 * Created by Ryan Orendorff <ryan@rdodesigns.com>
 * Date: 18/03/12 16:55:19
 *
 * Large parts copied from the JSON C validator.
 * Copyright (c) 2005 JSON.org
 *
 * Copyright (GPL V3):
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>  // sprintf
#include <string.h> // memset

#include "ctiff_meta.h"
#include "ctiff_util.h"
#include "ctiff_types.h"
#include "ctiff_vers.h"


typedef struct JSON_checker_struct {
    int state;
    int depth;
    int top;
    int* stack;
} * JSON_checker;

#define __   -1     /* the universal error code */

/** Define the type of a symbol in a JSON string.
 *
 *  Characters are mapped into these 31 character classes. This allows for
 *  a significant reduction in the size of the state transition table.
 */
enum classes {
    C_SPACE,  /* space */
    C_WHITE,  /* other whitespace */
    C_LCURB,  /* {  */
    C_RCURB,  /* } */
    C_LSQRB,  /* [ */
    C_RSQRB,  /* ] */
    C_COLON,  /* : */
    C_COMMA,  /* , */
    C_QUOTE,  /* " */
    C_BACKS,  /* \ */
    C_SLASH,  /* / */
    C_PLUS,   /* + */
    C_MINUS,  /* - */
    C_POINT,  /* . */
    C_ZERO ,  /* 0 */
    C_DIGIT,  /* 123456789 */
    C_LOW_A,  /* a */
    C_LOW_B,  /* b */
    C_LOW_C,  /* c */
    C_LOW_D,  /* d */
    C_LOW_E,  /* e */
    C_LOW_F,  /* f */
    C_LOW_L,  /* l */
    C_LOW_N,  /* n */
    C_LOW_R,  /* r */
    C_LOW_S,  /* s */
    C_LOW_T,  /* t */
    C_LOW_U,  /* u */
    C_ABCDF,  /* ABCDF */
    C_E,      /* E */
    C_ETC,    /* everything else */
    NR_CLASSES
};

/** Map characters to their symbol type.
 *
 *  This array maps the 128 ASCII characters into character classes.
 *  The remaining Unicode characters should be mapped to C_ETC.
 *  Non-whitespace control characters are errors.
 */
static int ascii_class[128] = {
    __,      __,      __,      __,      __,      __,      __,      __,
    __,      C_WHITE, C_WHITE, __,      __,      C_WHITE, __,      __,
    __,      __,      __,      __,      __,      __,      __,      __,
    __,      __,      __,      __,      __,      __,      __,      __,

    C_SPACE, C_ETC,   C_QUOTE, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_PLUS,  C_COMMA, C_MINUS, C_POINT, C_SLASH,
    C_ZERO,  C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT,
    C_DIGIT, C_DIGIT, C_COLON, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,

    C_ETC,   C_ABCDF, C_ABCDF, C_ABCDF, C_ABCDF, C_E,     C_ABCDF, C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_LSQRB, C_BACKS, C_RSQRB, C_ETC,   C_ETC,

    C_ETC,   C_LOW_A, C_LOW_B, C_LOW_C, C_LOW_D, C_LOW_E, C_LOW_F, C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_LOW_L, C_ETC,   C_LOW_N, C_ETC,
    C_ETC,   C_ETC,   C_LOW_R, C_LOW_S, C_LOW_T, C_LOW_U, C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_LCURB, C_ETC,   C_RCURB, C_ETC,   __
};


/** The state codes.
 *
 *  Used by the pushdown automaton to define its current state.
 */
enum states {
    GO,  /* start    */
    OK,  /* ok       */
    OB,  /* object   */
    KE,  /* key      */
    CO,  /* colon    */
    VA,  /* value    */
    AR,  /* array    */
    ST,  /* string   */
    ES,  /* escape   */
    U1,  /* u1       */
    U2,  /* u2       */
    U3,  /* u3       */
    U4,  /* u4       */
    MI,  /* minus    */
    ZE,  /* zero     */
    IT,  /* integer  */
    FR,  /* fraction */
    E1,  /* e        */
    E2,  /* ex       */
    E3,  /* exp      */
    T1,  /* tr       */
    T2,  /* tru      */
    T3,  /* true     */
    F1,  /* fa       */
    F2,  /* fal      */
    F3,  /* fals     */
    F4,  /* false    */
    N1,  /* nu       */
    N2,  /* nul      */
    N3,  /* null     */
    NR_STATES
};


/** State table for pushdown automaton.
 *
 *  The state transition table takes the current state and the current symbol,
 *  and returns either a new state or an action. An action is represented as a
 *  negative number. A JSON text is accepted if at the end of the text the
 *  state is OK and if the mode is MODE_DONE.
 */
static int state_transition_table[NR_STATES][NR_CLASSES] = {
/*               white                                      1-9                                   ABCDF  etc
             space |  {  }  [  ]  :  ,  "  \  /  +  -  .  0  |  a  b  c  d  e  f  l  n  r  s  t  u  |  E  |*/
/*start  GO*/ {GO,GO,-6,__,-5,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*ok     OK*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*object OB*/ {OB,OB,__,-9,__,__,__,__,ST,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*key    KE*/ {KE,KE,__,__,__,__,__,__,ST,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*colon  CO*/ {CO,CO,__,__,__,__,-2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*value  VA*/ {VA,VA,-6,__,-5,__,__,__,ST,__,__,__,MI,__,ZE,IT,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
/*array  AR*/ {AR,AR,-6,__,-5,-7,__,__,ST,__,__,__,MI,__,ZE,IT,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
/*string ST*/ {ST,__,ST,ST,ST,ST,ST,ST,-4,ES,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST},
/*escape ES*/ {__,__,__,__,__,__,__,__,ST,ST,ST,__,__,__,__,__,__,ST,__,__,__,ST,__,ST,ST,__,ST,U1,__,__,__},
/*u1     U1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U2,U2,U2,U2,U2,U2,U2,U2,__,__,__,__,__,__,U2,U2,__},
/*u2     U2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U3,U3,U3,U3,U3,U3,U3,U3,__,__,__,__,__,__,U3,U3,__},
/*u3     U3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U4,U4,U4,U4,U4,U4,U4,U4,__,__,__,__,__,__,U4,U4,__},
/*u4     U4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,ST,ST,ST,ST,ST,ST,ST,ST,__,__,__,__,__,__,ST,ST,__},
/*minus  MI*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,ZE,IT,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*zero   ZE*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,FR,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*int    IT*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,FR,IT,IT,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*frac   FR*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,FR,FR,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*e      E1*/ {__,__,__,__,__,__,__,__,__,__,__,E2,E2,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*ex     E2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*exp    E3*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*tr     T1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T2,__,__,__,__,__,__},
/*tru    T2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T3,__,__,__},
/*true   T3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__},
/*fa     F1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F2,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*fal    F2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F3,__,__,__,__,__,__,__,__},
/*fals   F3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F4,__,__,__,__,__},
/*false  F4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__},
/*nu     N1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N2,__,__,__},
/*nul    N2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N3,__,__,__,__,__,__,__,__},
/*null   N3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__},
};


/** These modes can be pushed on the stack.  */
enum modes {
    MODE_ARRAY,
    MODE_DONE,
    MODE_KEY,
    MODE_OBJECT,
};

/** Delete the JSON_checker object.
 *
 *  Frees the memory associated with the JSON parser if the JSON string is
 *  invalid.
 *
 * @param jc The JSON_checker automaton structure.
 * @return false always.
 */
static int
reject(JSON_checker jc)
{
    free((void*)jc->stack);
    free((void*)jc);
    return false;
}


/** Push a mode onto the stack.
 *
 * @param jc The JSON_checker automaton structure.
 * @return false if there is overflow.
 */
static int
push(JSON_checker jc, int mode)
{
    jc->top += 1;
    if (jc->top >= jc->depth) {
        return false;
    }
    jc->stack[jc->top] = mode;
    return true;
}


/** Pop from the stack.
 *
 *  Pop the stack, assuring that the current mode matches the expectation.
 *
 * @param jc The JSON_checker automaton structure.
 * @return True on success, false if there is underflow or the modes mismatch.
 */
static int
pop(JSON_checker jc, int mode)
{
    if (jc->top < 0 || jc->stack[jc->top] != mode) {
        return false;
    }
    jc->top -= 1;
    return true;
}


/** Create new JSON checker object.
 *
 *  new_JSON_checker starts the checking process by constructing a JSON_checker
 *  object. It takes a depth parameter that restricts the level of maximum
 *  nesting.
 *
 *  To continue the process, call JSON_checker_char for each character in the
 *  JSON text, and then call JSON_checker_done to obtain the final result.
 *  These functions are fully reentrant.
 *
 *  The JSON_checker object will be deleted by JSON_checker_done.
 *  JSON_checker_char will delete the JSON_checker object if it sees an error.
 *
 *  @param depth The size of the mode stack.
 *  @return A JSON_checker struct.
 */
JSON_checker
new_JSON_checker(int depth)
{
    JSON_checker jc = (JSON_checker)malloc(sizeof(struct JSON_checker_struct));
    jc->state = GO;
    jc->depth = depth;
    jc->top = -1;
    jc->stack = (int*)calloc(depth, sizeof(int));
    push(jc, MODE_DONE);
    return jc;
}


/** Check if next charater in a string is valid for the current JSON state.
 *
 *  After calling new_JSON_checker, call this function for each character (or
 *  partial character) in your JSON text. It can accept UTF-8, UTF-16, or
 *  UTF-32. It returns true if things are looking ok so far. If it rejects the
 *  text, it deletes the JSON_checker object and returns false.
 *
 * @param jc        The JSON_checker automaton structure.
 * @param next_char The next character in the string to test.
 * @return          next_char if it is not whitespace. -1 if it is white
 *                  space. 0/false on failure.
 */
char
JSON_checker_char(JSON_checker jc, int next_char)
{
    int retchar = next_char;
    int next_class, next_state;

    // Determine the character's class.
    if (next_char < 0) {
        return reject(jc);
    }
    if (next_char >= 128) {
        next_class = C_ETC;
    } else {
        next_class = ascii_class[next_char];
        if (next_class <= __) {
            return reject(jc);
        }
    }

    next_state = state_transition_table[jc->state][next_class];

    // If white space character, return -1.
    if (!(next_state == ST) &&
         (next_class == C_SPACE || next_class == C_WHITE))
      retchar = -1;

    // Get the next state from the state transition table.
    if (next_state >= 0) {
        // Change the state
        jc->state = next_state;
    } else {
    // Or perform one of the actions.
        switch (next_state) {
/* empty } */
        case -9:
            if (!pop(jc, MODE_KEY)) {
                return reject(jc);
            }
            jc->state = OK;
            break;

/* } */ case -8:
            if (!pop(jc, MODE_OBJECT)) {
                return reject(jc);
            }
            jc->state = OK;
            break;

/* ] */ case -7:
            if (!pop(jc, MODE_ARRAY)) {
                return reject(jc);
            }
            jc->state = OK;
            break;

/* { */ case -6:
            if (!push(jc, MODE_KEY)) {
                return reject(jc);
            }
            jc->state = OB;
            break;

/* [ */ case -5:
            if (!push(jc, MODE_ARRAY)) {
                return reject(jc);
            }
            jc->state = AR;
            break;

/* " */ case -4:
            switch (jc->stack[jc->top]) {
            case MODE_KEY:
                jc->state = CO;
                break;
            case MODE_ARRAY:
            case MODE_OBJECT:
                jc->state = OK;
                break;
            default:
                return reject(jc);
            }
            break;

/* , */ case -3:
            switch (jc->stack[jc->top]) {
            case MODE_OBJECT:

            // A comma causes a flip from object mode to key mode.
                if (!pop(jc, MODE_OBJECT) || !push(jc, MODE_KEY)) {
                    return reject(jc);
                }
                jc->state = KE;
                break;
            case MODE_ARRAY:
                jc->state = VA;
                break;
            default:
                return reject(jc);
            }
            break;

/* : */ case -2:

            // A colon causes a flip from key mode to object mode.
            if (!pop(jc, MODE_KEY) || !push(jc, MODE_OBJECT)) {
                return reject(jc);
            }
            jc->state = VA;
            break;
// Bad action.
        default:
            return reject(jc);
        }
    }
    return retchar;
}


/** Check that in correct ending mode after string is parsed.
 *
 *  The JSON_checker_done function should be called after all of the characters
 *  have been processed, but only if every call to JSON_checker_char returned
 *  true. This function deletes the JSON_checker and returns true if the JSON
 *  text was accepted.
 *
 * @param jc The JSON_checker automaton structure.
 * @return   True is the JSON is valid, false if not.
 */
int
JSON_checker_done(JSON_checker jc)
{
    int result = jc->state == OK && pop(jc, MODE_DONE);
    reject(jc);
    return result;
}

/** Validate metadata.
 *
 *  This function validates a metadata string.
 *
 * @param json The metadata string.
 * @return     True if valid string, false if invalid.
 */
int __CTIFFIsValidJSON(const char* json)
{
  int retval = 1;
  const char *pt = json;
  JSON_checker jc = new_JSON_checker(20);

  do {
    char next_char = *pt;
    if (next_char <= 0) {
      break;
    }
    if (!JSON_checker_char(jc, next_char)) {
      return 0;
    }
  } while (*(pt++) != '\0');

  if (!JSON_checker_done(jc)) {
    return 0;
  }

  return retval;
}

void __CTIFFTarValidExtMetaError(char** ret, const char* json, bool strict)
{
  char* buf;

  if (ret == NULL) return;

  if (strict){
    FREE(*ret);
  } else {
    buf = *ret + strlen(json);
    memcpy(*ret, json, sizeof(char)*strlen(json));
    *buf = '\0';
  }
}

/** Validate metadata and return a compressed version.
 *
 *  This function removes the white space in between the keys and objects in
 *  order to create the minimal representation of a JSON object.
 *
 * @param json The metadata string.
 * @return     Compressed JSON string.
 */
const char* __CTIFFTarValidExtMeta(const char* json, bool strict)
{
  char *ret;
  char *buf;
  char tmp_char;
  const char *pt;
  JSON_checker jc;

  if (!((json != NULL) && (strlen(json) != 0))) return NULL;

  ret = (char*) malloc(sizeof(char)*(strlen(json)+1));
  buf = ret;
  pt = json;
  jc = new_JSON_checker(20);

  do {
    char next_char = *pt;
    if (next_char <= 0) {
      break;
    }
    if (!(tmp_char = JSON_checker_char(jc, next_char))) {
      __CTIFFTarValidExtMetaError(&ret,json,strict);
      return ret;
    }
    if (tmp_char != -1) *buf++ = tmp_char;

  } while (*(pt++) != '\0');

  if (!JSON_checker_done(jc)) {
    __CTIFFTarValidExtMetaError(&ret,json,strict);
    return ret;
  }

  *buf = '\0';
  return ret;
}

/** Validate metadata and add CTIFF information header.
 *
 *  This function removes the white space in between the keys and objects in
 *  order to create the minimal representation of a JSON object. Additionally
 *  it adds information about the CamTIFF file.
 * @see __CTIFFTarValidExtMeta
 *
 * @param json The metadata string.
 * @return     Compressed JSON string.
 */
const char* __CTIFFCreateValidExtMeta(bool strict, const char* name,
                                      const char* ext_meta)
{
  char *buf;
  char *head_buf = (char*) malloc(sizeof(char)*128);
  const char* tar_ext_meta;
  const char *CTIFF_ext_head = "\"ctiff\":\"%s\",\"libctiff\":\"%d.%d.%d%s\","
                               "\"strict\":%s";

  // Make the head first, can be of variable size
  sprintf(head_buf, CTIFF_ext_head, CTIFF_SPECIFICATION,
                                    CTIFFLIB_MAJOR_VERSION,
                                    CTIFFLIB_MINOR_VERSION,
                                    CTIFFLIB_MAINT_VERSION,
                                    CTIFFLIB_TESTING_VERSION,
                                    strict ? "true" : "false");

  // This must be freed if it is not NULL!
  tar_ext_meta = __CTIFFTarValidExtMeta(ext_meta, strict);

  // If we have valid metadata and a valid name
  if ( (tar_ext_meta != NULL) && (name != NULL && strlen(name) != 0) ){
    buf = (char*) malloc(6 + strlen(head_buf) +
                             strlen(name) + strlen(tar_ext_meta) + 1);
    sprintf(buf, "{%s,\"%s\":%s}", head_buf,
                                   name,
                                   tar_ext_meta);
  } else {
    buf = (char *) malloc(2 + strlen(head_buf) + 1);
    sprintf(buf, "{%s}", head_buf);
  }


  if (tar_ext_meta != NULL) FREE(tar_ext_meta);
  FREE(head_buf);
  return buf;
}
