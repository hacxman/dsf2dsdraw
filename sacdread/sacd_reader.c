/**
 * Copyright (c) 2011 Mr Wicked, http://code.google.com/p/sacd-ripper/
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__bsdi__)|| defined(__DARWIN__)
#define SYS_BSD 1
#endif

#if defined(__sun)
#include <sys/mnttab.h>
#elif defined(SYS_BSD)
#include <fstab.h>
#elif defined(__linux__)
#include <mntent.h>
#endif

#include "sacd_input.h"
#include "sacd_reader.h"

struct sacd_reader_s {
    /* Basic information. */
    int is_image_file;

    /* Information required for an image file. */
    sacd_input_t dev;
};

/**
 * Open a SACD image or block sacd file.
 */
static sacd_reader_t *sacd_open_image_file( const char *location )
{
    sacd_reader_t *sacd;
    sacd_input_t dev;

    dev = sacd_input_open( location );
    if( !dev ) {
	fprintf( stderr, "libsacdread: Can't open %s for reading\n", location );
	return NULL;
    }

    sacd = (sacd_reader_t *) malloc( sizeof( sacd_reader_t ) );
    if( !sacd ) {
        sacd_input_close(dev);
        return NULL;
    }
    sacd->is_image_file = 1;
    sacd->dev = dev;

    return sacd;
}

#if defined(__sun)
/* /dev/rdsk/c0t6d0s0 (link to /devices/...)
   /vol/dev/rdsk/c0t6d0/??
   /vol/rdsk/<name> */
static char *sun_block2char( const char *path )
{
    char *new_path;

    /* Must contain "/dsk/" */
    if( !strstr( path, "/dsk/" ) ) return (char *) _strdup( path );

    /* Replace "/dsk/" with "/rdsk/" */
    new_path = malloc( strlen(path) + 2 );
    strcpy( new_path, path );
    strcpy( strstr( new_path, "/dsk/" ), "" );
    strcat( new_path, "/rdsk/" );
    strcat( new_path, strstr( path, "/dsk/" ) + strlen( "/dsk/" ) );

    return new_path;
}
#endif

#if defined(SYS_BSD)
/* FreeBSD /dev/(r)(a)cd0c (a is for atapi), recomended to _not_ use r
   OpenBSD /dev/rcd0c, it needs to be the raw sacd
   NetBSD  /dev/rcd0[d|c|..] d for x86, c (for non x86), perhaps others
   Darwin  /dev/rdisk0,  it needs to be the raw sacd
   BSD/OS  /dev/sr0c (if not mounted) or /dev/rsr0c ('c' any letter will do) */
static char *bsd_block2char( const char *path )
{
    char *new_path;

    /* If it doesn't start with "/dev/" or does start with "/dev/r" exit */
    if( !strncmp( path, "/dev/",  5 ) || strncmp( path, "/dev/r", 6 ) )
      return (char *) _strdup( path );

    /* Replace "/dev/" with "/dev/r" */
    new_path = malloc( strlen(path) + 2 );
    strcpy( new_path, "/dev/r" );
    strcat( new_path, path + strlen( "/dev/" ) );

    return new_path;
}
#endif

sacd_reader_t *sacd_open( const char *ppath )
{
    struct stat fileinfo;
    int ret;
	sacd_reader_t *ret_val = NULL;
    char *dev_name = NULL;
	char *path;

#ifdef _MSC_VER
	int len;
#endif

    if( ppath == NULL )
      return 0;

	path = _strdup(ppath);
    if( path == NULL )
      return 0;

#ifdef _MSC_VER
	/* Strip off the trailing \ if it is not a drive */
	len = strlen(path);
	if ((len > 1) &&
		(path[len - 1] == '\\')  &&
		(path[len - 2] != ':'))
	{
		path[len-1] = '\0';
	}
#endif

    ret = stat( path, &fileinfo );

    if( ret < 0 ) {
        /* maybe "host:port" url? try opening it with acCeSS library */
        if( strchr(path,':') ) {
			ret_val = sacd_open_image_file( path );
			free(path);
	        return ret_val;
        }
	/* If we can't stat the file, give up */
	fprintf( stderr, "libsacdread: Can't stat %s\n", path );
	perror("");
	free(path);
	return NULL;
    }

    /* First check if this is a block/char sacd or a file*/
    if( S_ISBLK( fileinfo.st_mode ) ||
	S_ISCHR( fileinfo.st_mode ) ||
	S_ISREG( fileinfo.st_mode ) ) {

	/**
	 * Block devices and regular files are assumed to be SACD-Video images.
	 */
#if defined(__sun)
	ret_val = sacd_open_image_file( sun_block2char( path ) );
#elif defined(SYS_BSD)
	ret_val = sacd_open_image_file( bsd_block2char( path ) );
#else
	ret_val = sacd_open_image_file( path );
#endif

	free(path);
	return ret_val;

    } else if( S_ISDIR( fileinfo.st_mode ) ) {
	sacd_reader_t *auth_drive = 0;
	char *path_copy;
#if defined(SYS_BSD)
	struct fstab* fe;
#elif defined(__sun) || defined(__linux__)
	FILE *mntfile;
#endif

	/* XXX: We should scream real loud here. */
	if( !(path_copy = _strdup( path ) ) ) {
		free(path);
		return NULL;
	}

#ifndef WIN32 /* don't have fchdir, and getcwd( NULL, ... ) is strange */
              /* Also WIN32 does not have symlinks, so we don't need this bit of code. */

	/* Resolve any symlinks and get the absolut dir name. */
	{
	    char *new_path;
	    int cdir = open( ".", O_RDONLY );

	    if( cdir >= 0 ) {
		chdir( path_copy );
		new_path = malloc(PATH_MAX+1);
		if(!new_path) {
		  free(path);
		  return NULL;
		}
		getcwd(new_path, PATH_MAX );
		fchdir( cdir );
		close( cdir );
		    free( path_copy );
		    path_copy = new_path;
	    }
	}
#endif
	/**
	 * If we're being asked to open a directory, check if that directory
	 * is the mountpoint for a SACD-ROM which we can use instead.
	 */

	if( strlen( path_copy ) > 1 ) {
	    if( path_copy[ strlen( path_copy ) - 1 ] == '/' )
		path_copy[ strlen( path_copy ) - 1 ] = '\0';
	}

	if(path_copy[0] == '\0') {
	    path_copy[0] = '/';
	    path_copy[1] = '\0';
	}

#if defined(SYS_BSD)
	if( ( fe = getfsfile( path_copy ) ) ) {
	    dev_name = bsd_block2char( fe->fs_spec );
	    fprintf( stderr,
		     "libsacdread: Attempting to use sacd %s"
		     " mounted on %s\n",
		     dev_name,
		     fe->fs_file );
	    auth_drive = sacd_open_image_file( dev_name );
	}
#elif defined(__sun)
	mntfile = fopen( MNTTAB, "r" );
	if( mntfile ) {
	    struct mnttab mp;
	    int res;

	    while( ( res = getmntent( mntfile, &mp ) ) != -1 ) {
		if( res == 0 && !strcmp( mp.mnt_mountp, path_copy ) ) {
		    dev_name = sun_block2char( mp.mnt_special );
		    fprintf( stderr,
			     "libsacdread: Attempting to use sacd %s"
			     " mounted on %s\n",
			     dev_name,
			     mp.mnt_mountp );
		    auth_drive = sacd_open_image_file( dev_name );
		    break;
		}
	    }
	    fclose( mntfile );
	}
#elif defined(__linux__)
        mntfile = fopen( MOUNTED, "r" );
        if( mntfile ) {
            struct mntent *me;

            while( ( me = getmntent( mntfile ) ) ) {
                if( !strcmp( me->mnt_dir, path_copy ) ) {
		    fprintf( stderr,
			     "libsacdread: Attempting to use sacd %s"
			     " mounted on %s\n",
			     me->mnt_fsname,
			     me->mnt_dir );
                    auth_drive = sacd_open_image_file( me->mnt_fsname );
		    dev_name = _strdup(me->mnt_fsname);
                    break;
                }
            }
            fclose( mntfile );
	}
#elif defined(_MSC_VER)
    auth_drive = sacd_open_image_file( path );
#endif

#ifndef _MSC_VER
	if( !dev_name ) {
	  fprintf( stderr, "libsacdread: Couldn't find sacd name.\n" );
	} else if( !auth_drive ) {
		fprintf( stderr, "libsacdread: Device %s inaccessible.\n", dev_name );
	}
#else
	if( !auth_drive ) {
	    fprintf( stderr, "libsacdread: Device %s inaccessible.\n", dev_name );
	}
#endif

	free( dev_name );
	free( path_copy );

        /**
         * If we've opened a drive, just use that.
         */
	if( auth_drive ) {
		free(path);
		return auth_drive;
	}
    }

    /* If it's none of the above, screw it. */
    fprintf( stderr, "libsacdread: Could not open %s\n", path );
	free( path );
    return NULL;
}

void sacd_close( sacd_reader_t *sacd )
{
    if( sacd ) {
        if( sacd->dev ) sacd_input_close( sacd->dev );
        free( sacd );
    }
}

ssize_t sacd_read_block_raw( sacd_reader_t *sacd, uint32_t lb_number,
			 size_t block_count, unsigned char *data)
{
   ssize_t ret;
   if( !sacd->dev ) {
     	fprintf( stderr, "libsacdread: Fatal error in block read.\n" );
	return 0;
   }

   ret = sacd_input_seek( sacd->dev, (int) lb_number );
   if( ret != (int) lb_number ) {
     	fprintf( stderr, "libsacdread: Can't seek to block %u\n", lb_number );
	return 0;
   }

   ret = sacd_input_read( sacd->dev, (char *) data,
			 (int) block_count );
   return ret;
}
