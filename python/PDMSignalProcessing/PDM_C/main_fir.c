// gcc main.c -o main.exe && main.exe

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define PATH "C:/Users/levyg/Documents/MEGA/Repositories/mems2sd_esp32/python/PDMSignalProcessing/PDM_C/"
#define SAMPLES 1024
#define BYTE_PER_SAMPLE 4
// #define APPLY_MASK(x,i,b) (long)((((x>>(b-1-i))&0x00000001) << 1)-1)
#define APPLY_MASK(x,i) (long)((((x>>(31-i))&0x00000001) << 1)-1)
// #define APPLY_MASK(x,i) ((x>>(31-i))&0x00000001)
#define QUEUE_SIZE 2

#define MAX_OVERFLOW  1073741823 // (long)(pow(2,30)-1)
#define MIN_OVERFLOW -1073741824 // (long)(-pow(2,30))

#define BYTE_MASK_LEFT(i) (0xFF<<((3-i)*8))
#define SHIFT_BYTE_RIGHT(x, i) (x>>((3-i)*8))
#define LONG_CASTING(x) ((long)(x))
#define CHAR_MASK(x, i) (unsigned char)SHIFT_BYTE_RIGHT((LONG_CASTING(x) & BYTE_MASK_LEFT(i)), i)

#define INPUT_SAMPLE_SIZE 32
#define OSR 16

#define FIR_ORDER 64

typedef short sample_t;

typedef struct 
{
    long acc_s1;
    long acc_s2;
    long prev_s1;
    long prev_s2;
} app_cic_t;

typedef struct {
    short kernel[FIR_ORDER];
} app_fir_t;


void init_app_cic(app_cic_t *cic);
void init_app_fir(app_fir_t *fir);
long swap_bytes_of_word(long x);
void integ_overflow(app_cic_t *cic);
void process_app_cic(app_cic_t *cic, size_t num_samples, long (*input_buffer)[SAMPLES], short (*output_buffer)[2*SAMPLES]);
void process_app_fir(app_fir_t *fir, short fir_coeffs[FIR_ORDER], short (*pcm_samples)[2*SAMPLES]);
long buffer[SAMPLES];
short pcm_samples[2*SAMPLES];

short fir_coeffs[FIR_ORDER] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, -9, 8, 24, -43, -38, 144, -1, -343, 224, 603, -838, -725, 2128, 234, -4795, 2229, 17687, 17687, 2229, -4795, 234, 2128, -725, -838, 603, 224, -343, -1, 144, -38, -43, 24, 8, -9, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


int main()
{
    app_cic_t cic;
    app_fir_t fir;
    long sample;
    size_t num_samples;

    init_app_cic(&cic);
    init_app_fir(&fir);
    
    FILE *fileb, *filetxt;

    fileb = fopen(PATH "clipped_data_bin", "rb");
    filetxt = fopen(PATH "c_fir_processed", "w"); fprintf(filetxt, ""); fclose(filetxt);
    filetxt = fopen(PATH "c_fir_processed", "a");

    if(fileb)
    {   
        // num_samples = fread(buffer, BYTE_PER_SAMPLE, SAMPLES, fileb);        
        while(num_samples = fread(buffer, sizeof(long), SAMPLES, fileb) == SAMPLES)
        {
            // firmware processing: start
            // clock_t begin=clock();
            process_app_cic(&cic, num_samples, &buffer, &pcm_samples);
            process_app_fir(&fir, fir_coeffs, &pcm_samples);
            // clock_t end=clock();
            // firmware processing: end
            // printf("Time taken:%lfms\n",1e3*(double)(end-begin)/CLOCKS_PER_SEC);
            for(int ii=0; ii<2*num_samples; ii++) fprintf(filetxt, "%d\n", pcm_samples[ii]);
        }
    } else {printf("File not read.\n");}
    fclose(fileb);
    fclose(filetxt);
    
    return 0;
}

long swap_bytes_of_word(long x)
{
    return ((x & 0x000000FF) << 24) | ((x & 0xFF000000) >> 24) | ((x & 0x0000FF00) << 8) | ((x & 0x00FF0000) >> 8);
}

void init_app_cic(app_cic_t *cic)
{
    cic->acc_s1 = 0;
    cic->acc_s2 = 0;
    cic->prev_s1 = 0;
    cic->prev_s2 = 0;
}

void init_app_fir(app_fir_t *fir)
{
    for(int ii=0; ii<FIR_ORDER; ii++) fir->kernel[ii] = (short) 0;
}

void integ_overflow(app_cic_t *cic)
{
    cic->acc_s1 = cic->acc_s1>=MAX_OVERFLOW ? cic->acc_s1-MAX_OVERFLOW : cic->acc_s1;
    cic->acc_s1 = cic->acc_s1<=MIN_OVERFLOW ? cic->acc_s1-MIN_OVERFLOW : cic->acc_s1;
    cic->acc_s2 = cic->acc_s1>=MAX_OVERFLOW ? cic->acc_s2-MAX_OVERFLOW : cic->acc_s2;
    cic->acc_s2 = cic->acc_s2<=MIN_OVERFLOW ? cic->acc_s2-MIN_OVERFLOW : cic->acc_s2;
}

void process_app_cic(app_cic_t *cic, size_t num_samples, long (*input_buffer)[SAMPLES], short (*output_buffer)[2*SAMPLES])
{
    long sample, temp, pcm_sample, comb_in[2];
    
    for(int ii=0; ii<num_samples; ii++)
    {
        sample = (*input_buffer)[ii];
        
        // 2 stages integration

        // decimation for first comb sample (avoid if statement)
        cic->acc_s1 += APPLY_MASK(sample,INPUT_SAMPLE_SIZE*ii%INPUT_SAMPLE_SIZE);
        cic->acc_s2 += cic->acc_s1;
        integ_overflow(cic);
        comb_in[0] = cic->acc_s2;

        for(int jj=INPUT_SAMPLE_SIZE*ii+1; jj<INPUT_SAMPLE_SIZE/2*(2*ii+1); jj++) 
        {
            cic->acc_s1 += APPLY_MASK(sample,jj%INPUT_SAMPLE_SIZE);
            cic->acc_s2 += cic->acc_s1;
            integ_overflow(cic);
        }

        // decimation for second comb sample (avoid if statement)
        cic->acc_s1 += APPLY_MASK(sample,(INPUT_SAMPLE_SIZE/2*(2*ii+1))%INPUT_SAMPLE_SIZE);
        cic->acc_s2 += cic->acc_s1;
        integ_overflow(cic);
        comb_in[1] = cic->acc_s2;

        for(int jj=INPUT_SAMPLE_SIZE/2*(2*ii+1)+1; jj<INPUT_SAMPLE_SIZE*(ii+1); jj++) 
        {
            cic->acc_s1 += APPLY_MASK(sample,jj%INPUT_SAMPLE_SIZE);
            cic->acc_s2 += cic->acc_s1;
            integ_overflow(cic);
        }

        for(int jj=0; jj<2; jj++)
        {
            temp = comb_in[jj] - cic->prev_s1;
            cic->prev_s1 = comb_in[jj];

            pcm_sample = temp - cic->prev_s2;
            cic->prev_s2 = temp;

            pcm_sample = pcm_sample>=MAX_OVERFLOW ? pcm_sample-MAX_OVERFLOW : pcm_sample;
            pcm_sample = pcm_sample<=MIN_OVERFLOW ? pcm_sample-MIN_OVERFLOW : pcm_sample;

            (*output_buffer)[2*ii+jj] = (short) pcm_sample;
        }
    }
}

void process_app_fir(app_fir_t *fir, short fir_coeffs[FIR_ORDER], short (*pcm_samples)[2*SAMPLES])
{
    int size_fir = FIR_ORDER, size_pcm = 2*SAMPLES;
    for(int ii=0; ii<size_pcm; ii++)
    {
        // update FIR filter kernel with newest sample and clean it
        fir->kernel[ii % size_fir] = (*pcm_samples)[ii];
        (*pcm_samples)[ii] = 0;

        // compute next sample with filter kernel's samples
        for(int jj=0; jj<size_fir; jj++) (*pcm_samples)[ii] += (fir->kernel[jj]*fir_coeffs[( ( (size_fir-1) - jj) + ii+1) % size_fir]);
    }
}