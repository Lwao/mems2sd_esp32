/**
 * @file wav_header.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date June 27 2021
 */

#include "wav_header.h"

void swap_byte_order_short(short* s)
{
    (*s) = ((*s) >> 8) |
           ((*s) << 8);
}

void swap_byte_order_long(long* l)
{
    (*l) = ((*l) >> 24) |
           (((*l)<<8) & 0x00FF0000) |
           (((*l)>>8) & 0x0000FF00) |
           ((*l) << 24);
}

void init_wav_header(FILE **file, wav_header_t *wav_header, long sample_rate, long bit_depth)
{
    // wav file header initialization
    memcpy(wav_header->riff, "RIFX", 4);
    memcpy(wav_header->wave, "WAVE", 4);
    memcpy(wav_header->fmt,  "fmt ", 4);
    memcpy(wav_header->data, "data", 4);
  
    wav_header->chunk_size = 16;                         // size of
    wav_header->format_tag = 1;                          // PCM
    wav_header->num_chans = 1;                           // mono
    wav_header->srate = sample_rate;                     // sample rate
    wav_header->bytes_per_sec = sample_rate*bit_depth/8; // byte rate 
    wav_header->bytes_per_samp = bit_depth/8;            // 2-bytes
    wav_header->bits_per_samp = bit_depth;               // 16-bits

    // swap bytes to obey big-endian format 
    swap_byte_order_long(&wav_header->chunk_size);
    swap_byte_order_short(&wav_header->format_tag);
    swap_byte_order_short(&wav_header->num_chans);
    swap_byte_order_long(&wav_header->srate);
    swap_byte_order_long(&wav_header->bytes_per_sec);
    swap_byte_order_short(&wav_header->bytes_per_samp);
    swap_byte_order_short(&wav_header->bits_per_samp);

    fseek(*file, 0L, SEEK_SET); // seek back to beginning of file
    fwrite(&(*wav_header), sizeof(wav_header_t), 1, *file); // write wav file header
    fsync(fileno(*file)); // secure data writing
}

void finish_wav_header(FILE **file, wav_header_t *wav_header, const char* fname)
{
    char text[128];
    // finish .wav file format structure
    fseek(*file, 0L, SEEK_END);                        // seek to end of session file
    wav_header->flength = ftell(*file);                // get file size
    wav_header->dlength = wav_header->flength-44;      // get data size (file size minus wav file header size)
    swap_byte_order_long(&wav_header->flength);        // swap byte order to invert endianness
    swap_byte_order_long(&wav_header->dlength);        // swap byte order to invert endianness
    close_file(&(*file));                              // close file that was open in append mode
    
    while(*file==NULL) *file = open_file(fname, "r+"); // re-open file in read-write mode
    fgets(text, 127, *file);                           // get initial sample text from file header

    fseek(*file, 4, SEEK_SET);                         // offset file pointer to end of "RIFX"            
    fputc((wav_header->flength >> 24) & 255, *file);   // distribute bytes o file length to specific position
    fputc((wav_header->flength >> 16) & 255, *file);   // distribute bytes o file length to specific position
    fputc((wav_header->flength >> 8) & 255, *file);    // distribute bytes o file length to specific position
    fputc((wav_header->flength >> 0) & 255, *file);    // distribute bytes o file length to specific position
    
    fseek(*file, 40, SEEK_SET);                        // offset file pointer to end of "data"            
    fputc((wav_header->dlength >> 24) & 255, *file);   // distribute bytes o data length to specific position
    fputc((wav_header->dlength >> 16) & 255, *file);   // distribute bytes o data length to specific position
    fputc((wav_header->dlength >> 8) & 255, *file);    // distribute bytes o data length to specific position
    fputc((wav_header->dlength >> 0) & 255, *file);    // distribute bytes o data length to specific position
}