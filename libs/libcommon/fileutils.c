/**
 * SACD Ripper - http://code.google.com/write_ptr/sacd-ripper/
 *
 * Copyright (c) 2010-2011 by respective authors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include "logging.h"
#include "fileutils.h"
#include "utils.h"

// construct a filename from various parts
//
// path - the path the file is placed in (don't include a trailing '/')
// dir - the parent directory of the file (don't include a trailing '/')
// file - the filename
// extension - the suffix of a file (don't include a leading '.')
//
// NOTE: caller must free the returned string!
// NOTE: any of the parameters may be NULL to be omitted
char * make_filename(const char * path, const char * dir, const char * file, const char * extension)
{
    int  len   = 1;
    char * ret = NULL;
    int  pos   = 0;

    if (path)
    {
        len += strlen(path) + 1;
    }
    if (dir)
    {
        len += strlen(dir) + 1;
    }
    if (file)
    {
        len += strlen(file);
    }
    if (extension)
    {
        len += strlen(extension) + 1;
    }

    ret = malloc(sizeof(char) * len);
    if (ret == NULL)
        LOG(lm_main, LOG_ERROR, ("malloc(sizeof(char) * len) failed. Out of memory."));

    if (path)
    {
        strncpy(&ret[pos], path, strlen(path));
        pos     += strlen(path);
        ret[pos] = '/';
        pos++;
    }
    if (dir)
    {
        strncpy(&ret[pos], dir, strlen(dir));
        pos     += strlen(dir);
        ret[pos] = '/';
        pos++;
    }
    if (file)
    {
        strncpy(&ret[pos], file, strlen(file));
        pos += strlen(file);
    }
    if (extension)
    {
        ret[pos] = '.';
        pos++;
        strncpy(&ret[pos], extension, strlen(extension));
        pos += strlen(extension);
    }
    ret[pos] = '\0';

    //sanitize_filename(ret);

    return ret;
}

// substitute various items into a formatted string (similar to printf)
//
// format - the format of the filename
// tracknum - gets substituted for %N in format
// year - gets substituted for %Y in format
// artist - gets substituted for %A in format
// album - gets substituted for %L in format
// title - gets substituted for %T in format
//
// NOTE: caller must free the returned string!
char * parse_format(const char * format, int tracknum, const char * year, const char * artist, const char * album, const char * title)
{
    unsigned i     = 0;
    int      len   = 0;
    char     * ret = NULL;
    int      pos   = 0;

    for (i = 0; i < strlen(format); i++)
    {
        if ((format[i] == '%') && (i + 1 < strlen(format)))
        {
            switch (format[i + 1])
            {
            case 'A':
                if (artist)
                    len += strlen(artist);
                break;
            case 'L':
                if (album)
                    len += strlen(album);
                break;
            case 'N':
                if ((tracknum > 0) && (tracknum < 100))
                    len += 2;
                break;
            case 'Y':
                if (year)
                    len += strlen(year);
                break;
            case 'T':
                if (title)
                    len += strlen(title);
                break;
            case '%':
                len += 1;
                break;
            }

            i++;             // skip the character after the %
        }
        else
        {
            len++;
        }
    }

    ret = malloc(sizeof(char) * (len + 1));
    if (ret == NULL)
        LOG(lm_main, LOG_ERROR, ("malloc(sizeof(char) * (len+1)) failed. Out of memory."));

    for (i = 0; i < strlen(format); i++)
    {
        if ((format[i] == '%') && (i + 1 < strlen(format)))
        {
            switch (format[i + 1])
            {
            case 'A':
                if (artist)
                {
                    strncpy(&ret[pos], artist, strlen(artist));
                    pos += strlen(artist);
                }
                break;
            case 'L':
                if (album)
                {
                    strncpy(&ret[pos], album, strlen(album));
                    pos += strlen(album);
                }
                break;
            case 'N':
                if ((tracknum > 0) && (tracknum < 100))
                {
                    ret[pos]     = '0' + ((int) tracknum / 10);
                    ret[pos + 1] = '0' + (tracknum % 10);
                    pos         += 2;
                }
                break;
            case 'Y':
                if (year)
                {
                    strncpy(&ret[pos], year, strlen(year));
                    pos += strlen(year);
                }
                break;
            case 'T':
                if (title)
                {
                    strncpy(&ret[pos], title, strlen(title));
                    pos += strlen(title);
                }
                break;
            case '%':
                ret[pos] = '%';
                pos     += 1;
            }

            i++;             // skip the character after the %
        }
        else
        {
            ret[pos] = format[i];
            pos++;
        }
    }
    ret[pos] = '\0';

    return ret;
}

#ifdef __lv2ppu__
#include <sys/stat.h>
#include <sys/file.h>
#endif

// Uses mkdir() for every component of the path
// and returns if any of those fails with anything other than EEXIST.
int recursive_mkdir(char* path_and_name, mode_t mode)
{
    int  count;
    int  path_and_name_length = strlen(path_and_name);
    int  rc;
    char charReplaced;

    for (count = 0; count < path_and_name_length; count++)
    {
        if (path_and_name[count] == '/')
        {
            charReplaced             = path_and_name[count + 1];
            path_and_name[count + 1] = '\0';

            rc = mkdir(path_and_name, mode);
#ifdef __lv2ppu__
            sysFsChmod(path_and_name, S_IFMT | 0777); 
#endif
            path_and_name[count + 1] = charReplaced;

            if (rc != 0 && !(errno == EEXIST || errno == EISDIR || errno == EACCES || errno == EROFS))
                return rc;
        }
    }

    // in case the path doesn't have a trailing slash:
    rc = mkdir(path_and_name, mode);
#ifdef __lv2ppu__
    sysFsChmod(path_and_name, S_IFMT | 0777);
#endif
    return rc;
}

// Uses mkdir() for every component of the path except the last one,
// and returns if any of those fails with anything other than EEXIST.
int recursive_parent_mkdir(char* path_and_name, mode_t mode)
{
    int count;
    int have_component = 0;
    int rc             = 1; // guaranteed fail unless mkdir is called

    // find the last component and cut it off
    for (count = strlen(path_and_name) - 1; count >= 0; count--)
    {
        if (path_and_name[count] != '/')
            have_component = 1;

        if (path_and_name[count] == '/' && have_component)
        {
            path_and_name[count] = 0;
            rc                   = mkdir(path_and_name, mode);
            path_and_name[count] = '/';
        }
    }

    return rc;
}

void sanitize_filename(char *f)
{
    const char safe_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+._- ";
    char *c = f;

    if (!c || strlen(c) == 0)
        return;

    for (; *c; c++)
    {
        if (!strchr(safe_chars, *c))
            *c = ' ';
    }
    replace_double_space_with_single(f);
    trim_whitespace(f);
}
