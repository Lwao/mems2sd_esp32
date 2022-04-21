/**
 * @file wav_header_big_endian.h
 * @brief Implementation of wav file header in big-endian architecture, 
 *    	  considering that the ESP32 has a little-endian architecture but 
 *        its I2S peripheral stores that in big-endian.
 *
 * @author Levy Gabriel da S. G.
 * @date April 21 2022
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "file.wav"

// wav file header https://forum.arduino.cc/t/creating-a-wav-file-header/314260/4
struct wav_header_struct {
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
} wav_header;

FILE* session_file = NULL;
char example_buffer[10] = {0,1,2,3,4,5,6,7,8,9};

void swap_byte_order_short(short* s);
void swap_byte_order_long(long* l);


int main()
{
	// wav file header initialization
    memcpy(wav_header.riff, "RIFX", 4);
    memcpy(wav_header.wave, "WAVE", 4);
    memcpy(wav_header.fmt,  "fmt ", 4);
    memcpy(wav_header.data, "data", 4);
  
    wav_header.chunk_size = 16;                         // size of
    wav_header.format_tag = 1;                          // PCM
    wav_header.num_chans = 1;                           // mono
    wav_header.srate = SAMPLE_RATE;                     // sample rate
    wav_header.bytes_per_sec = SAMPLE_RATE*BIT_DEPTH/8; // byte rate 
    wav_header.bytes_per_samp = BIT_DEPTH/8;            // 2-bytes
    wav_header.bits_per_samp = BIT_DEPTH;               // 16-bits

    // swap bytes to obey big-endian format 
    swap_byte_order_long(&wav_header.chunk_size);
    swap_byte_order_short(&wav_header.format_tag);
    swap_byte_order_short(&wav_header.num_chans);
    swap_byte_order_long(&wav_header.srate);
    swap_byte_order_long(&wav_header.bytes_per_sec);
    swap_byte_order_short(&wav_header.bytes_per_samp);
    swap_byte_order_short(&wav_header.bits_per_samp);
	
	// open file
	while(session_file==NULL) session_file = open_file(FILE_NAME, "a");
	
	// write .wav file header to session file
	fseek(session_file, 0L, SEEK_SET); 										// seek back to beginning of file
	fwrite(&wav_header, sizeof(struct wav_header_struct), 1, session_file); //write wav file header
	fsync(fileno(session_file)); 											// secure data writing
	
	// write data according to application specifics
	fwrite(example_buffer, sizeof(example_buffer), 1, session_file); // write buffer to sd card current file
	fsync(fileno(session_file)); 									 // secure data writing
	
	// finish .wav file format structure
	fseek(session_file, 0L, SEEK_END);                                    // seek to end of session file
	wav_header.flength = ftell(session_file);                             // get file size
	wav_header.dlength = wav_header.flength-44;                           // get data size (file size minus wav file header size)
	swap_byte_order_long(&wav_header.flength);                            // swap byte order to invert endianness
	swap_byte_order_long(&wav_header.dlength);                            // swap byte order to invert endianness
	close_file(&session_file);                                            // close file that was open in append mode
          
	while(session_file==NULL) session_file = open_file(FILE_NAME, "r+");  // re-open file in read-write mode
	fgets(text, 127, session_file);                                       // get initial sample text from file header

	fseek(session_file, 4, SEEK_SET);                                     // offset file pointer to end of "RIFX"            
	fputc((wav_header.flength >> 24) & 255, session_file);                // distribute bytes o file length to specific position
	fputc((wav_header.flength >> 16) & 255, session_file);                // distribute bytes o file length to specific position
	fputc((wav_header.flength >> 8) & 255, session_file);                 // distribute bytes o file length to specific position
	fputc((wav_header.flength >> 0) & 255, session_file);                 // distribute bytes o file length to specific position
	
	fseek(session_file, 40, SEEK_SET);                                    // offset file pointer to end of "data"            
	fputc((wav_header.dlength >> 24) & 255, session_file);                // distribute bytes o data length to specific position
	fputc((wav_header.dlength >> 16) & 255, session_file);                // distribute bytes o data length to specific position
	fputc((wav_header.dlength >> 8) & 255, session_file);                 // distribute bytes o data length to specific position
	fputc((wav_header.dlength >> 0) & 255, session_file);                 // distribute bytes o data length to specific position
            
	// close file in use for good
	close_file(&session_file);
	return 0;
}