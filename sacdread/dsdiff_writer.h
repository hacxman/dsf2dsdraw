/**
 * SACD Ripper - http://code.google.com/p/sacd-ripper/
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

#ifndef DSDIFF_WRITER_H_INCLUDED
#define DSDIFF_WRITER_H_INCLUDED

#include "scarletbook_types.h"

typedef struct {

	scarletbook_handle_t   *sb;

	uint8_t                *header;
	size_t					header_size;
	uint8_t                *footer;
	size_t					footer_size;
	int						fd;

} dsdiff_handle_t;

dsdiff_handle_t* dsdiff_open(scarletbook_handle_t *, char *, int, int, int);
void dsdiff_close(dsdiff_handle_t *);

#endif  /* DSDIFF_WRITER_H_INCLUDED */
