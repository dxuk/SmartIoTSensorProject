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
 * -------------------------------------------------------------------
 */

#include "pch.h"

#define HEADERS()
#include "headers.h"

#include "Extractor.h"

/**
 * Constructor
 * @arg fileName   Name of the file
 * @arg size       Block size for reading
 * Open the file and set the block size
 */
void Extractor_Initialize ( char *fileName, int inSize, thread_Settings *mSettings ) {

    if ( (mSettings->Extractor_file = fopen (fileName, "rb")) == NULL ) {
        fprintf( stderr, "Unable to open the file stream\n");
        fprintf( stderr, "Will use the default data stream\n");
        return;
    }
    mSettings->Extractor_size =  inSize;
}


/**
 * Constructor
 * @arg fp         File Pointer
 * @arg size       Block size for reading
 * Set the block size,file pointer
 */
void Extractor_InitializeFile ( FILE *fp, int inSize, thread_Settings *mSettings ) {
    mSettings->Extractor_file = fp;
    mSettings->Extractor_size =  inSize;
}


/**
 * Destructor - Close the file
 */
void Extractor_Destroy ( thread_Settings *mSettings ) {
    if ( mSettings->Extractor_file != NULL )
        fclose( mSettings->Extractor_file );
}


/*
 * Fetches the next data block from
 * the file
 * @arg block     Pointer to the data read
 * @return        Number of bytes read
 */
int Extractor_getNextDataBlock ( char *data, thread_Settings *mSettings ) {
    if ( Extractor_canRead( mSettings ) ) {
        return (int)(fread( data, 1, mSettings->Extractor_size,
                      mSettings->Extractor_file ));
    }
    return 0;
}

/**
 * Function which determines whether
 * the file stream is still readable
 * @return boolean    true, if readable; false, if not
 */
int Extractor_canRead ( thread_Settings *mSettings ) {
    return(( mSettings->Extractor_file != NULL )
           && !(feof( mSettings->Extractor_file )));
}

/**
 * This is used to reduce the read size
 * Used in UDP transfer to accomodate the
 * the header (timestamp)
 * @arg delta         Size to reduce
 */
void Extractor_reduceReadSize ( int delta, thread_Settings *mSettings ) {
    mSettings->Extractor_size -= delta;
}



















































