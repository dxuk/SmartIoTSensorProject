/*---------------------------------------------------------------
 * Extract data from a file, used to measure the transfer rates
 * for various stream formats.
 *
 * E.g. Use a gzipped file to measure the transfer rates for
 * compressed data
 * Use an MPEG file to measure the transfer rates of
 * Multimedia data formats
 * Use a plain BMP file to measure the transfer rates of
 * Uncompressed data
 *
 * This is beneficial especially in measuring bandwidth across WAN
 * links where data compression takes place before data transmission
 * ------------------------------------------------------------------- */

#ifndef _EXTRACTOR_H
#define _EXTRACTOR_H

#include <stdlib.h>
#include <stdio.h>
#include "Settings.h"

    /**
     * Constructor
     * @arg fileName   Name of the file
     * @arg size       Block size for reading
     */
    void Extractor_Initialize( char *fileName, int size, thread_Settings *mSettings );

    /**
     * Constructor
     * @arg fp         File Pointer
     * @arg size       Block size for reading
     */
    void Extractor_InitializeFile( FILE *fp, int size, thread_Settings *mSettings );


    /*
     * Fetches the next data block from
     * the file
     * @arg block     Pointer to the data read
     * @return        Number of bytes read
     */
    int Extractor_getNextDataBlock( char *block, thread_Settings *mSettings );


    /**
     * Function which determines whether
     * the file stream is still readable
     * @return true, if readable; false, if not
     */
    int Extractor_canRead( thread_Settings *mSettings );

    /**
     * This is used to reduce the read size
     * Used in UDP transfer to accomodate the
     * the header (timestamp)
     * @arg delta         Size to reduce
     */
    void Extractor_reduceReadSize( int delta, thread_Settings *mSettings );

    /**
     * Destructor
     */
    void Extractor_Destroy( thread_Settings *mSettings );

#endif

