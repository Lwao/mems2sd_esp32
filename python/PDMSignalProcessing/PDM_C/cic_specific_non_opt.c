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

typedef short sample_t;

typedef struct 
{
    long acc_s1;
    long acc_s2;
    long prev_s1;
    long prev_s2;
} app_cic_t;


void init_app_cic(app_cic_t *cic);
long swap_bytes_of_word(long x);

long buffer[SAMPLES];

long integ_out[INPUT_SAMPLE_SIZE*SAMPLES] = {0};
short pcm_samples[INPUT_SAMPLE_SIZE*SAMPLES/OSR] = {0};

int main()
{
    app_cic_t cic;
    long sample;// = 0xd5591e4e;
    unsigned long samples[2] = {0xD5591E4E, 0x4EC5597C};
    size_t num_samples;


    init_app_cic(&cic);
    
    FILE *fileb, *filetxt;
    long temp;

    fileb = fopen(PATH "clipped_data_bin", "r");
    // filetxt = fopen(PATH "c_cic_processed", "w"); fprintf(filetxt, ""); fclose(filetxt);
    // filetxt = fopen(PATH "c_cic_processed", "a");
    filetxt = fopen(PATH "pcm_c", "w"); fprintf(filetxt, ""); fclose(filetxt);
    filetxt = fopen(PATH "pcm_c", "a");

    if(fileb)
    {   
        num_samples = fread(buffer, BYTE_PER_SAMPLE, SAMPLES, fileb);

        // num_samples = 2;
        if(num_samples>0)
        // while(num_samples = fread(buffer, BYTE_PER_SAMPLE, SAMPLES, fileb) == SAMPLES)
        {
            // for(int ii=0; ii<num_samples; ii++) buffer[ii] = swap_bytes_of_word(buffer[ii]);
            // firmware processing: start
            clock_t begin=clock();
            for(int ii=0; ii<num_samples; ii++)
            {
                sample = buffer[ii];
                // sample = samples[ii];

                // 2 stages integration

                for(int jj=INPUT_SAMPLE_SIZE*ii; jj<INPUT_SAMPLE_SIZE*(ii+1); jj++) 
                {
                    integ_out[jj] = cic.acc_s1 + APPLY_MASK(sample,jj%INPUT_SAMPLE_SIZE);
                    cic.acc_s1 = integ_out[jj];

                    integ_out[jj] += cic.acc_s2;
                    cic.acc_s2 = integ_out[jj];

                    cic.acc_s1 = cic.acc_s1>=MAX_OVERFLOW ? cic.acc_s1-MAX_OVERFLOW : cic.acc_s1;
                    cic.acc_s1 = cic.acc_s1<=MIN_OVERFLOW ? cic.acc_s1-MIN_OVERFLOW : cic.acc_s1;
                    cic.acc_s2 = cic.acc_s1>=MAX_OVERFLOW ? cic.acc_s2-MAX_OVERFLOW : cic.acc_s2;
                    cic.acc_s2 = cic.acc_s2<=MIN_OVERFLOW ? cic.acc_s2-MIN_OVERFLOW : cic.acc_s2;
                }

                for(int jj=2*ii; jj<(2*ii+2); jj++)
                {
                    temp = integ_out[INPUT_SAMPLE_SIZE/2*(jj)] - cic.prev_s1;
                    cic.prev_s1 = integ_out[INPUT_SAMPLE_SIZE/2*(jj)];

                    pcm_samples[jj] = temp - cic.prev_s2;
                    cic.prev_s2 = temp;

                    pcm_samples[jj] = pcm_samples[jj]>=MAX_OVERFLOW ? pcm_samples[jj]-MAX_OVERFLOW : pcm_samples[jj];
                    pcm_samples[jj] = pcm_samples[jj]<=MIN_OVERFLOW ? pcm_samples[jj]-MIN_OVERFLOW : pcm_samples[jj];
                }
            }
            // firmware processing: end

            clock_t end=clock();
            printf("Time taken:%lf\n",1e3*(double)(end-begin)/CLOCKS_PER_SEC);
            for(int ii=0; ii<INPUT_SAMPLE_SIZE*num_samples/OSR; ii++) fprintf(filetxt, "%d\n", pcm_samples[ii]);
            // for(int ii=0; ii<INPUT_SAMPLE_SIZE*num_samples/OSR; ii++) printf("%d, ", pcm_samples[ii]);
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
