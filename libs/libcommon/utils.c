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

#include "utils.h"

char *substr(const char *pstr, int start, int numchars)
{
    static char pnew[512];
    strncpy(pnew, pstr + start, numchars);
    pnew[numchars] = '\0';
    return pnew;
}

void rem_space(char *str)
{
    int len = strlen(str) - 1;
    int i = 0;
    int spaces = 0;

    if(!str || len < 0) 
        return;
        
    while (i < len)
    {
        while(str[i] == ' ') 
        {
            spaces++; 
            i++;
        }
        while(str[i] != ' ' && str[i] != '\0') 
        {
            str[i - spaces] = str[i]; 
            i++;
        }
        if(str[i + spaces - 1] != '\0') 
        {
            str[i - spaces] = ' '; 
            spaces--; 
        } 
        else 
        {
            break;
        }
    }
    str[i - spaces] = '\0';
}
