// This file is part of SmallBASIC
//
// strings
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/str.h"
#include "common/fmt.h"

#define BUF_SIZE 256

/**
 * removes spaces and returns a new string
 */
char *trimdup(const char *str) {
  char *buf;
  char *p;

  buf = malloc(strlen(str) + 1);
  strcpy(buf, str);

  if (*str == '\0') {
    return buf;
  }

  p = (char *) str;
  while (is_wspace(*p)) {
    p++;
  }
  strcpy(buf, p);

  if (*p != '\0') {
    p = buf;
    while (*p) {
      p++;
    }
    p--;
    while (p > buf && is_wspace(*p)) {
      p--;
    }
    p++;
    *p = '\0';
  }
  return buf;
}

/**
 * whether the string contains any whitespace characters
 */
int has_wspace(const char *s) {
  int result = 0;
  int i;
  for (i = 0; s != NULL && s[i] && !result; i++) {
    result |= is_wspace(s[i]);
  }
  return result;
}

/**
 * removes spaces
 */
void str_alltrim(char *str) {
  if (str && has_wspace(str)) {
    char *buf = trimdup(str);
    strcpy(str, buf);
    free(buf);
  }
}

/**
 * caseless string compare
 */
int strcaselessn(const char *s1, int s1n, const char *s2, int s2n) {
  int result, i;
  for (i = 0;; i++) {
    if (i == s1n || i == s2n) {
      result = s1n < s2n ? -1 : s1n > s2n ? 1 : 0;
      break;
    }
    char c1 = s1[i];
    char c2 = s2[i];
    if (c1 != c2) {
      c1 = to_lower(c1);
      c2 = to_lower(c2);
      if (c1 != c2) {
        result = c1 < c2 ? -1 : 1;
        break;
      }
    }
  }
  return result;
}

/**
 * transdup
 */
char *transdup(const char *src, const char *what, const char *with, int ignore_case) {
  int lwhat = strlen(what);
  int lwith = strlen(with);
  int size = BUF_SIZE;
  char *p = (char *)src;
  char *dest = malloc(size);
  char *d = dest;

  *d = '\0';

  while (*p) {
    int eq;
    if (ignore_case) {
      eq = strncasecmp(p, what, lwhat);
    }
    else {
      eq = strncmp(p, what, lwhat);
    }
    if (eq == 0) {
      if ((d - dest) + lwith >= size - 1) {
        int len = d - dest;
        size += BUF_SIZE;
        dest = realloc(dest, size);
        d = dest + len;
      }
      memcpy(d, with, lwith);
      d += lwith;
      p += (lwhat - 1);
    } else {
      if ((d - dest) + 1 >= size - 1) {
        int len = d - dest;
        size += BUF_SIZE;
        dest = realloc(dest, size);
        d = dest + len;
      }
      *d = *p;
      d++;
    }
    p++;
  }

  *d = '\0';
  return dest;
}

/**
 * strstr with support for quotes
 */
char *q_strstr(const char *s1, const char *s2, const char *pairs) {
  char *z;
  int l2;
  int wait_q, open_q, level_q;

  char *p = (char *) s1;
  l2 = strlen(s2);
  wait_q = open_q = level_q = 0;

  while (*p) {
    if (*p == wait_q) {         // i am waiting that. level down
      level_q--;
      if (level_q <= 0) {       // level = 0
        level_q = 0;
        wait_q = 0;
      }
    } else if ((z = strchr(pairs, *p)) != NULL) { // character is a
      // delimiter;
      // level up
      open_q = ((z - pairs) + 1) % 2; // true, if its a 'begin'
      // delimiter

      if (wait_q && open_q) {
        // open_q of our pair?
        if (*(z + 1) == wait_q) {
          // increase level
          level_q++;
        }
      } else if (wait_q) {
        // do nothing, I am waitting something
      // else
      } else {
        // its a new section
        if (open_q) {
          level_q++;            // level = 1
          wait_q = *(z + 1);    // what to wait for
        }
      }
    } else if (wait_q == 0) {     // it is a regular character
      if (strncmp(p, s2, l2) == 0) {
        return p;
      }
    }
    // next
    p++;
  }

  return NULL;
}

/**
 * is_alpha
 */
int is_alpha(int ch) {
  if (ch == 0) {
    return 0;
  }
  if ((ch > 64 && ch < 91) || (ch > 96 && ch < 123)) {
    return -1;
  }
  return (ch & 0x80);           // +foreign
}

/**
 * returns true if the ch is alphanumeric
 */
int is_alnum(int ch) {
  if (ch == 0) {
    return 0;
  }
  return (is_alpha(ch) || is_digit(ch));
}

/**
 * returns true if the ch is an 'empty' character
 */
int is_space(int ch) {
  return (ch == ' ' || ch == '\t' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\v') ? -1 : 0;
}

/**
 * returns true if 'text' contains digits only
 */
int is_all_digits(const char *text) {
  const char *p = text;

  if (p == NULL) {
    return 0;
  }
  if (*p == '\0') {
    return 0;
  }
  while (*p) {
    if (!is_digit(*p)) {
      return 0;
    }
    p++;
  }
  return 1;
}

/**
 * converts the 'str' string to uppercase
 */
char *strupper(char *str) {
  char *p = str;

  if (p == NULL) {
    return 0;
  }
  while (*p) {
    *p = to_upper(*p);
    p++;
  }
  return str;
}

/**
 * converts the 'str' string to lowercase
 */
char *strlower(char *str) {
  char *p = str;

  if (p == NULL) {
    return 0;
  }
  while (*p) {
    *p = to_lower(*p);
    p++;
  }
  return str;
}

/**
 * Returns the number of a string (constant numeric expression)
 *
 * type  <=0 = error
 *         1 = int32_t
 *         2 = double
 *
 * Warning: octals are different from C (QB compatibility: 009 = 9)
 */
char *get_numexpr(char *text, char *dest, int *type, var_int_t *lv, var_num_t *dv) {
  char *p = text;
  char *d = dest;
  char *epos = NULL;
  byte base = 10;
  byte dpc = 0, e_fmt = 0, eop = '+';
  int sign = 1;
  var_num_t power = 1.0;
  var_num_t num;

  *type = 0;
  *lv = 0;
  *dv = 0.0;

  if (p == NULL) {
    *dest = '\0';
    return NULL;
  }
  // spaces
  while (is_space(*p)) {
    p++;
  }
  // sign
  if ((*p == '-' || *p == '+') && strchr("0123456789.", *(p + 1)) &&
      *(p + 1) != '\0'){if (*p == '-') {
      sign = -1;
    }
    // don't copy it
    p++;
  }

  //
  // resolve the base (hex, octal and binary)
  //
  if ((*p == '&') || (*p == '0' && (*(p + 1) != '\0' && strchr("HXBO", to_upper(*(p + 1))) != NULL))) {
    p++;
    switch (*p) {
    case 'H':
    case 'h':
    case 'X':
    case 'x':
      base = 16;
      break;
    case 'O':
    case 'o':
      base = 8;
      break;
    case 'B':
    case 'b':
      base = 2;
      break;
    default:
      // Unknown base
      *type = -1;
      return p;
    }

    p++;
  }
  //
  // copy parts of number
  //
  if (base == 16) {
    // copy hex
    while (is_hexdigit(*p)) {
      *d = to_upper(*p);
      d++;
      p++;
    }
  } else if (base != 10) {
    // copy octal | bin
    while (is_digit(*p)) {
      *d++ = *p++;
    }
  } else if (is_digit(*p) || *p == '.') {
    // copy number (first part)
    while (is_digit(*p) || *p == '.') {
      if (*p == '.') {
        dpc++;
        if (dpc > 1) {
          // DP ERROR
          *type = -2;
          break;
        }
      }
      *d++ = *p++;
    }

    // check second part
    if ((*p == 'E' || *p == 'e') && (*type == 0)) {
      epos = d;
      // E
      *d++ = *p++;

      if (*p == '+' || *p == '-' || is_digit(*p) || *p == '.') {
        dpc = 0;

        // copy second part (power)
        if (*p == '+' || *p == '-') {
          *d++ = *p++;
          if (strchr("+-*/\\^", *p) != 0) {
            // stupid E format
            // (1E--9 || 1E++9)
            e_fmt = 1;
            eop = *p;
            *d++ = *p++;
            if (*p == '+' || *p == '-') {
              *d++ = *p++;
            }
          }
        }
        // power
        while (is_digit(*p) || *p == '.') {
          if (*p == '.') {
            dpc++;
            if (dpc > 1) {
              // DP ERROR (second part)
              *type = -4;
              break;
            }
          }
          *d++ = *p++;
          // after E
        }
      }
      else {
        // E+- ERROR
        *type = -3;
      }
    }
  } else {
    // NOT A NUMBER
    *type = -9;
  }
  *d = '\0';

  //
  // finaly, calculate the number
  //
  if (*type == 0) {
    switch (base) {
    case 10:
      if (dpc || (epos != NULL) || (strlen(dest) > 8)) {
        // double
        *type = 2;
        if (epos) {
          if (e_fmt) {
            int r_type = 1;

            *epos = '\0';
            num = sb_strtof(dest) * ((double) sign);
            // restore E
            *epos = 'E';

            power = sb_strtof(epos + 3);

            if (r_type > 0) {
              switch (eop) {
              case '+':
                *dv = num + power;
                break;
              case '-':
                *dv = num - power;
                break;
              case '*':
                *dv = num * power;
                break;
              case '/':
                if (ABS(power) != 0.0) {
                  *dv = num / power;
                } else {
                  *dv = 0;
                }
                break;
              case '\\':
                if ((long) power != 0) {
                  *type = 1;
                  *lv = num / (long) power;
                } else {
                  *type = 1;
                  *lv = 0;
                }
                break;
              case '^':
                *dv = pow(num, power);
                break;
              }
            }
          } else {
            *epos = '\0';
            power = pow(10, sb_strtof(epos + 1));
            *dv = sb_strtof(dest) * ((double) sign) * power;
            *epos = 'E';
          }
        } else {
          *dv = sb_strtof(dest) * ((double) sign);
        }
      } else {
        // dpc = 0 && epos = 0
        *type = 1;
        *lv = xstrtol(dest) * sign;
      }
      break;
    case 16:
      *type = 1;
      *lv = hextol(dest);
      break;
    case 8:
      *type = 1;
      *lv = octtol(dest);
      break;
    case 2:
      *type = 1;
      *lv = bintol(dest);
      break;
    }
  }
  if (is_alpha(*p)) {
    // its not a number
    *type = -9;
  }
  while (is_space(*p)) {
    p++;
  }
  return p;
}

/**
 * numexpr_sb_strtof
 */
var_num_t numexpr_sb_strtof(char *source) {
  char buf[BUF_SIZE];
  int type;
  var_int_t lv;
  var_num_t dv;

  get_numexpr(source, buf, &type, &lv, &dv);

  if (type == 1) {
    return (var_num_t) lv;
  } else if (type == 2) {
    return dv;
  }
  return 0.0;
}

/**
 * numexpr_strtol
 */
var_int_t numexpr_strtol(char *source) {
  char buf[BUF_SIZE], *np;
  int type;
  var_int_t lv;
  var_num_t dv;

  np = get_numexpr(source, buf, &type, &lv, &dv);

  if (type == 1 && *np == '\0') {
    return lv;
  } else if (type == 2 && *np == '\0') {
    return (var_int_t) dv;
  }
  return 0;
}

/**
 * convertion: binary to decimal
 */
long bintol(const char *str) {
  long r = 0;
  char *p = (char *) str;

  if (p == NULL) {
    return 0;
  }
  while (*p) {
    if (*p == 48 || *p == 49) {
      // 01
      r = (r << 1) + ((*p) - 48);
    }
    p++;
  }
  return r;
}

/**
 * convertion: octal to decimal
 */
long octtol(const char *str) {
  long r = 0;
  char *p = (char *) str;

  if (p == NULL) {
    return 0;
  }
  while (*p) {
    if (*p >= 48 && *p <= 55) {
      // 01234567
      r = (r << 3) + ((*p) - 48);
    }
    p++;
  }
  return r;
}

/**
 * convertion: hexadecimal to decimal
 */
long hextol(const char *str) {
  long r = 0;
  char *p = (char *) str;

  if (p == NULL) {
    return 0;
  }
  while (*p) {
    if (is_digit(*p)) {
      // 0123456789
      r = (r << 4) + ((*p) - 48);
    } else if (*p >= 65 && *p <= 70) {
      // ABCDEF
      r = (r << 4) + ((*p) - 55);
    } else if (*p >= 97 && *p <= 102) {
      // abcdef
      r = (r << 4) + ((*p) - 87);
    }
    p++;
  }
  return r;
}

/**
 * string to double
 */
var_num_t sb_strtof(const char *str) {
  char *p = (char *) str;
  var_num_t r = 0.0;
  int negate = 0;
  int places = 0;

  if (p != NULL) {
    if (*p == '-') {
      negate = 1;
      p++;
    } else if (*p == '+') {
      p++;
    }
    int dot = 0;
    while (*p) {
      if (is_digit(*p)) {
        r = r * 10.0f + (*p - '0');
        if (dot) {
          places++;
        }
      } else if (*p == '.') {
        dot = 1;
      } else if (*p == ' ') {
        break;
      } else {
        r = 0;
        break;
      }
      p++;
    }
  }
  if (places) {
    r /= pow(10, places);
  }
  return negate ? -r : r;
}

/**
 * xstrtol
 */
long xstrtol(const char *str) {
  if (str == NULL) {
    return 0;
  }
  return atoi(str);
}

/**
 * whether the string is a number
 */
int is_number(const char *str) {
  char *p = (char *) str;
  int dpc = 0, cnt = 0;

  if (str == NULL) {
    return 0;
  }
  if (*p == '+' || *p == '-') {
    p++;
  }
  while (*p) {
    if (strchr("0123456789.", *p) == NULL) {
      return 0;
    } else {
      cnt++;
    }
    if (*p == '.') {
      dpc++;
      if (dpc > 1) {
        return 0;
      }
    }
    p++;
  }
  if (cnt) {
    return 1;
  }
  return 0;
}

/**
 * double to string
 */
char *ftostr(var_num_t num, char *dest) {
  bestfta(num, dest);
  return dest;
}

/**
 * ltostr
 */
char *ltostr(var_int_t num, char *dest) {
  sprintf(dest, VAR_INT_FMT, num);
  return dest;
}

/**
 * returns whether the character is whitespace
 */
int is_wspace(int c) {
  return (c != 0 && (c == ' ' || c == '\t' || c == '\r' ||
                     c == '\n' || c == '\v' || c == '\f'));
}

/**
 * squeeze (& strdup)
 */
char *sqzdup(const char *source) {
  char *rp, *p, *d;
  int lc = 0;

  rp = malloc(strlen(source) + 1);
  p = (char *) source;
  d = rp;

  while (*p != '\0' && is_wspace(*p))
    p++;

  while (*p) {
    if (is_wspace(*p)) {
      if (!lc) {
        lc = 1;
        if (p > source) {
          if (is_alpha(*(p - 1)) || is_digit(*(p - 1)))
            *d++ = ' ';
          else {
            char *nc;

            nc = p;
            while (*nc != '\0' && is_wspace(*nc))
              nc++;
            if (is_alpha(*nc) || is_digit(*nc)
                )
              *d++ = ' ';
          }
        }
      }
    } else {
      (lc = 0, *d++ = *p);
    }
    p++;
  }

  *d = '\0';
  if (d > rp) {
    if (is_wspace(*(d - 1))) {
      *(d - 1) = '\0';
    }
  }

  return rp;
}

/**
 * enclose, returns a newly created string
 */
char *encldup(const char *source, const char *pairs) {
  int l = strlen(source);
  char *rp = malloc(l + 3);

  memcpy(rp + 1, source, l);
  *(rp) = pairs[0];
  if (pairs[1]) {
    *(rp + l + 1) = pairs[1];
  } else {
    *(rp + l + 1) = pairs[0];
  }
  *(rp + l + 2) = '\0';
  return rp;
}

/**
 * disclose, returns a newly created string
 */
char *discldup(const char *source, const char *pairs, const char *ignpairs) {
  char *np, *z;
  int wait_p = 0, level_p = 0;
  int wait_q = 0, level_q = 0;
  int record = 0;

  char *rp = strdup(source);
  char *r = rp;
  char *p = (char *) source;

  while (*p) {
    // ignore pairs
    if (*p == wait_q) {
      // ignore pair - level down
      level_q--;
      if (level_q <= 0) {
        level_q = 0;
        wait_q = 0;
      }
    } else if ((z = strchr(ignpairs, *p)) != NULL) {
      int open_q = ((z - ignpairs) + 1) % 2;

      if (wait_q && open_q) {
        if (*(z + 1) == wait_q) {
          // open_q of our pair?
          level_q++;
        }
      } else if (wait_q) {
        // do nothing, I am waitting something
      } else {
        // new pair
        if (open_q) {
          level_q++;
          wait_q = *(z + 1);
        }
      }
    }
    // primary pairs
    else if (*p == wait_p && wait_q == 0) { // primary pair - level
      // down
      level_p--;
      if (level_p <= 0) {
        // store and exit
        record = 0;
        break;
      }
    } else if ((z = strchr(pairs, *p)) != NULL && wait_q == 0) {
      int open_p = ((z - pairs) + 1) % 2;

      if (wait_p && open_p) {
        if (*(z + 1) == wait_p) {
          // open_q of our pair?
          level_p++;
        }
      } else if (wait_p) {
        // do nothing, I am waitting something
      } else {
        // new pair
        if (open_p) {
          level_p++;
          wait_p = *(z + 1);
          record = 1;
        }
      }
    }
    // next
    if (record == 1) {
      // ignore the first
      record++;
    } else if (record == 2) {
      *r++ = *p;
    }
    p++;
  }

  *r = '\0';
  // actually, resize down
  np = strdup(rp);
  free(rp);

  return np;
}

/**
 * C-Style control codes
 */
char *cstrdup(const char *source) {
  char *buf, *p, *d;

  buf = malloc(strlen(source) + 1);
  p = (char *) source;
  d = buf;
  while (*p) {
    if (*p == '\\') {
      p++;
      switch (*p) {
      case 'e':
        *d++ = '\033';
        break;
      case 'v':
        *d++ = '\v';
        break;
      case 't':
        *d++ = '\t';
        break;
      case 'r':
        *d++ = '\r';
        break;
      case 'n':
        *d++ = '\n';
        break;
      case 'b':
        *d++ = '\b';
        break;
      case '\'':
        *d++ = '\'';
        break;
      case '\"':
        *d++ = '\"';
        break;
      case 'a':
        *d++ = '\a';
        break;
      case 'f':
        *d++ = '\f';
        break;
      case '\\':
        *d++ = '\\';
        break;
      case 'x':                // hex
        if (is_hexdigit(*(p + 1)) && is_hexdigit(*(p + 2))) {
          int c = 0;

          p++;
          if (is_digit(*p))
            c |= ((*p - '0') << 4);
          else
            c |= (((to_upper(*p) - 'A') + 10) << 4);
          p++;
          if (is_digit(*p))
            c |= *p - '0';
          else
            c |= (to_upper(*p) - 'A') + 10;

          *d++ = c;
        } else
          *d++ = '\0';
        break;
      case '0':                // oct
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
        if (is_octdigit(*(p + 1)) && is_octdigit(*(p + 2))) {
          int c = 0;

          c |= ((*p - '0') << 6);
          p++;
          c |= ((*p - '0') << 3);
          p++;
          c |= (*p - '0');

          *d++ = c;
        } else
          *d++ = '\0';
        break;

      default:
        *d++ = *p;
      }

      p++;
    } else
      *d++ = *p++;
  }

  *d = '\0';
  return buf;
}

/**
 * basic-string to c-string convertion
 */
char *bstrdup(const char *source) {
  char *buf, *p, *d;

  buf = malloc(strlen(source) * 4 + 1);
  p = (char *) source;
  d = buf;
  while (*p) {
    if (*p < 32 && *p >= 0) {
      switch (*p) {
      case '\033':
        *d++ = '\\';
        *d++ = 'e';
        break;
      case '\v':
        *d++ = '\\';
        *d++ = 'v';
        break;
      case '\t':
        *d++ = '\\';
        *d++ = 't';
        break;
      case '\r':
        *d++ = '\\';
        *d++ = 'r';
        break;
      case '\n':
        *d++ = '\\';
        *d++ = 'n';
        break;
      case '\b':
        *d++ = '\\';
        *d++ = 'b';
        break;
      case '\'':
        *d++ = '\\';
        *d++ = '\'';
        break;
      case '\"':
        *d++ = '\\';
        *d++ = '\"';
        break;
      case '\a':
        *d++ = '\\';
        *d++ = 'a';
        break;
      case '\f':
        *d++ = '\\';
        *d++ = 'f';
        break;
      case '\\':
        *d++ = '\\';
        *d++ = '\\';
        break;
      default:
        *d++ = '\\';
        *d++ = 'x';
        *d++ = to_hexdigit((*p & 0xF0) >> 4);
        *d++ = to_hexdigit(*p & 0xF);
      }

      p++;
    } else
      *d++ = *p++;
  }

  *d = '\0';
  return buf;
}

/**
 * baseof
 */
const char *baseof(const char *source, int delim) {
  const char *p;

  p = strrchr(source, delim);
  if (p) {
    return p + 1;
  }
  return source;
}

/**
 * memory dump
 */
void hex_dump(const unsigned char *block, int size) {
#if defined(_UnixOS) || defined(_DOS)
  int i, j;

  printf("\n---HexDump---\n\t");
  for (i = 0; i < size; i++) {
    printf("%02X ", block[i]);
    if (((i + 1) % 16) == 0 || (i == size - 1)) {
      printf(" %04x ", i);
      for (j = ((i - 15 <= 0) ? 0 : i - 15); j <= i; j++) {
        if (block[j] < 32) {
          printf(".");
        } else {
          printf("%c", block[j]);
        }
      }
      printf("\n\t");
    }
  }

  printf("\n");
#endif
}

void cstr_init(cstr *cs, int size) {
  cs->length = 0;
  cs->size = size < 1 ? 1 : size;
  cs->buf = malloc(size);
  cs->buf[0] = '\0';
}

void cstr_append(cstr *cs, const char *str) {
  cstr_append_i(cs, str, strlen(str));
}

void cstr_append_i(cstr *cs, const char *str, int len) {
  if (len > 0) {
    if (cs->size - cs->length < len + 1) {
      cs->size += len + 1;
      cs->buf = realloc(cs->buf, cs->size);
    }
    strlcat(cs->buf, str, cs->size);
    cs->length += len;
    cs->buf[cs->length] = '\0';
  }
}
