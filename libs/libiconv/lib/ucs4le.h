/*
 * Copyright (C) 1999-2000 Free Software Foundation, Inc.
 * This file is part of the GNU LIBICONV Library.
 *
 * The GNU LIBICONV Library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * The GNU LIBICONV Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the GNU LIBICONV Library; see the file COPYING.LIB.
 * If not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * UCS-4LE = UCS-4 little endian
 */

static int
ucs4le_mbtowc (conv_t conv, ucs4_t *pwc, const unsigned char *s, int n)
{
  if (n >= 4) {
    *pwc = s[0] + (s[1] << 8) + (s[2] << 16) + (s[3] << 24);
    return 4;
  }
  return RET_TOOFEW(0);
}

static int
ucs4le_wctomb (conv_t conv, unsigned char *r, ucs4_t wc, int n)
{
  if (n >= 4) {
    r[0] = (unsigned char) wc;
    r[1] = (unsigned char) (wc >> 8);
    r[2] = (unsigned char) (wc >> 16);
    r[3] = (unsigned char) (wc >> 24);
    return 4;
  } else
    return RET_TOOSMALL;
}
