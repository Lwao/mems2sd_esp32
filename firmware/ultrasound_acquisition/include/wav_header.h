/**
 * @file wav_header.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date June 27 2021
 */

#ifndef _WAV_HEADER_H_ 
#define _WAV_HEADER_H_ 

#ifndef ESP_MANAGEMENT_LIBS_INCLUDED
    #define ESP_MANAGEMENT_LIBS_INCLUDED
    #include "esp_err.h" // error codes and helper functions
    #include "esp_log.h" // logging library
    #include "esp_vfs_fat.h" // FAT filesystem support
#endif

#ifndef C_POSIX_LIB_INCLUDED
    #define C_POSIX_LIB_INCLUDED
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdint.h>
    #include <math.h>
    #include <sys/unistd.h>
    #include <sys/stat.h>
#endif //C_POSIX_LIB_INCLUDED

#include "sd_driver.h"

// wav file header https://forum.arduino.cc/t/creating-a-wav-file-header/314260/4
typedef struct {
    char  riff[4];        /* "RIFF" little endian and "RIFX" big endian */
    long  flength;        /* file length in bytes                       */
    char  wave[4];        /* "WAVE"                                     */
    char  fmt[4];         /* "fmt "                                     */
    long  chunk_size;     /* size of FMT chunk in bytes (usually 16)    */
    short format_tag;     /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM    */
    short num_chans;      /* 1=mono, 2=stereo                           */
    long  srate;          /* Sampling rate in samples per second        */
    long  bytes_per_sec;  /* bytes per second = srate*bytes_per_samp    */
    short bytes_per_samp; /* 2=16-bit mono, 4=16-bit stereo             */
    short bits_per_samp;  /* Number of bits per sample                  */
    char  data[4];        /* "data"                                     */
    long  dlength;        /* data length in bytes (filelength - 44)     */
} wav_header_t;

/**
 * @brief Byte swap to compensate reversed endianness of ESP32 I2S peripheral (short).
 * 
 * @param s short integer to revert.
 */
void swap_byte_order_short(short* s);

/**
 * @brief Byte swap to compensate reversed endianness of ESP32 I2S peripheral (long).
 * 
 * @param l long integer to revert.
 */
void swap_byte_order_long(long* l);

/**
 * @brief Initialize .wav file header.
 * 
 * @param file file to which initialize header.
 * @param wav_header .wav header structure with standard data.
 * @param sample_rate sample rate used in the recording.
 * @param bit_depth bit depth used for samples in recording.
 * @param endianness endianness of format (1) for little endian and (0) for big endian
 */
void init_wav_header(FILE **file, wav_header_t *wav_header, long sample_rate, long bit_depth, int endianness);

/**
 * @brief Finish .wav file header.
 * 
 * @param file file to which finish header.
 * @param wav_header .wav header structure with standard data.
 * @param fname name of file to be manipulated.
 * @param endianness endianness of format (1) for little endian and (0) for big endian
 */
void finish_wav_header(FILE **file, wav_header_t *wav_header, char* fname, int endianness);

#endif // _WAV_HEADER_H_ 