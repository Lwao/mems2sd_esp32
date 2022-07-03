/**
 * @file cic.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date June 26 2021
 */
/**
 * @file cic.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date June 26 2021
 */
// gcc main.c -o main.exe && main.exe

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

typedef short sample_t;

typedef struct {
    size_t head, tail, size;
    size_t capacity;
    long* data;
} fifo_t;

typedef struct 
{
    long acc;
} integrator_t;

typedef struct 
{
    long previous;
    long actual;
    long diff;
    char delay;
    fifo_t fifo;
} comb_t;

typedef struct 
{
    int N; // stages
    int R; // decimation factor
    int M; // differential delay
    int G; // gain
    integrator_t *integrators;
    comb_t *combs;
    int count;
} cic_t;



void init_fifo(fifo_t *fifo, size_t size);
void enqueue(fifo_t *fifo, long data_in);
void dequeue(fifo_t *fifo, long *data_out);

void init_integrator(integrator_t *integ);
void init_comb(comb_t *comb, char delay);
void init_cic(cic_t *cic, int N, int R, int M);

long process_integrator(integrator_t *integ, long data_in);
long process_comb(comb_t *comb, long data_in);
sample_t process_cic(cic_t *cic, long data_in, char *data_available);

long swap_bytes_of_word(long x);

#define PATH "C:/Users/levyg/Documents/MEGA/Repositories/mems2sd_esp32/python/PDMSignalProcessing/PDM_C/"
#define SAMPLES 1024
#define BYTE_PER_SAMPLE 4
#define APPLY_MASK(x,i) ((x>>(32-1-i))&0x00000001)
#define QUEUE_SIZE 2

#define MAX_OVERFLOW  1073741823 // (long)(pow(2,30)-1)
#define MIN_OVERFLOW -1073741824 // (long)(-pow(2,30))

long buffer[SAMPLES];

int main()
{
    cic_t cic;
    char data_available=0;
    long data_in=0;
    sample_t data_out=0;

    int N=2;
    int R=16;
    int M=1;

    init_cic(&cic, N, R, M);
    
    FILE *fileb, *filetxt;

    /////////////////////// use pdm stream in +/- 1 format
    fileb = fopen(PATH "data_bit", "r");
    filetxt = fopen(PATH "data_pcm", "w"); fprintf(filetxt, ""); fclose(filetxt);
    filetxt = fopen(PATH "data_pcm", "a");

    if(fileb)
    {
        while(fscanf(fileb, "%d", &data_in) != EOF)
        {
            data_out = (sample_t) process_cic(&cic, (long)data_in, &data_available);
            if(data_available==1) fprintf(filetxt, "%d\n", data_out);
        }
    }
    fclose(fileb);
    fclose(filetxt);
    ///////////////////////

    /////////////////////// use pdm stream in 32bit word format    
    // fileb = fopen(PATH "data_in", "rb");
    // filetxt = fopen(PATH "data_pcm", "w"); fprintf(filetxt, ""); fclose(filetxt);
    // filetxt = fopen(PATH "data_pcm", "a");

    // if(fileb)
    // {
    //     while(fread(buffer, BYTE_PER_SAMPLE, SAMPLES, fileb) == SAMPLES)
    //     {
    //         for(int ii=0; ii<SAMPLES; ii++) 
    //         {
    //             buffer[ii] = swap_bytes_of_word(buffer[ii]);
    //             for(int jj=0; jj<32; jj++) 
    //             {
    //                 data_in = (sample_t)((APPLY_MASK(buffer[ii], jj)<<1)-1);
    //                 data_out = process_cic(&cic, data_in, &data_available);
    //                 if(data_available==1) fprintf(filetxt, "%d\n", data_out);                    
    //             }
    //         }
    //     }
    // }
    // fclose(fileb);
    // fclose(filetxt);
    
    return 0;
}

long swap_bytes_of_word(long x)
{
    return ((x & 0x000000FF) << 24) | ((x & 0xFF000000) >> 24) | ((x & 0x0000FF00) << 8) | ((x & 0x00FF0000) >> 8);
}

void init_fifo(fifo_t *fifo, size_t size)
{
    fifo->capacity = size;
    fifo->tail = fifo->size = 0;
    fifo->head = fifo->capacity-1;
    fifo->data = (long*) malloc(fifo->capacity * sizeof(long));
}

void enqueue(fifo_t *fifo, long data_in)
{
    if(fifo->size == fifo->capacity) return;
    fifo->head = (fifo->head+1) % fifo->capacity;
    fifo->data[fifo->head] = data_in;
    fifo->size++;
}

void dequeue(fifo_t *fifo, long *data_out)
{
    if(fifo->size == 0) return;
    *data_out = fifo->data[fifo->tail];
    fifo->tail = (fifo->tail+1) % fifo->capacity;
    fifo->size--;
}

void init_integrator(integrator_t *integ)
{
    integ->acc = (long) 0;
}

void init_comb(comb_t *comb, char delay)
{
    comb->previous = (long) 0;
    comb->actual = (long) 0;
    comb->diff = (long) 0;
    comb->previous = (long) 0;
    comb->delay = (char) delay;
    init_fifo(&comb->fifo, delay);
    // comb->fifo = (fifo_t) {0, 0, (size_t)delay+1, malloc(sizeof(void*) * ((size_t)delay+1))};
    for(int ii=0; ii<delay; ii++) enqueue(&comb->fifo, (long)0);
}

void init_cic(cic_t *cic, int N, int R, int M)
{
    cic->N = N;
    cic->R = R;
    cic->M = M;
    cic->G = (cic->R*cic->M)^(cic->N);
    cic->integrators = malloc(cic->N * sizeof *cic->integrators);
    cic->combs = malloc(cic->N * sizeof *cic->combs);
    for(int ii=0; ii<cic->N; ii++)
    {
        init_integrator(&cic->integrators[ii]);
        init_comb(&cic->combs[ii], (char)cic->M);
    }
    cic->count = 0;
}

long process_integrator(integrator_t *integ, long data_in)
{
    if (data_in>=MAX_OVERFLOW) data_in -= MAX_OVERFLOW;
    if (data_in<=MIN_OVERFLOW) data_in -= MIN_OVERFLOW;
    
    integ->acc += data_in;

    if (integ->acc>=MAX_OVERFLOW) integ->acc -= MAX_OVERFLOW;
    if (integ->acc<=MIN_OVERFLOW) integ->acc -= MIN_OVERFLOW;

    return integ->acc;
}

long process_comb(comb_t *comb, long data_in)
{
    if (data_in>=MAX_OVERFLOW) data_in -= MAX_OVERFLOW;
    if (data_in<=MIN_OVERFLOW) data_in -= MIN_OVERFLOW;
    comb->actual = data_in;
    
    dequeue(&comb->fifo, &comb->previous);    
    comb->diff = comb->actual - comb->previous;
    enqueue(&comb->fifo, comb->actual);

    if (comb->diff>=MAX_OVERFLOW) comb->diff -= MAX_OVERFLOW;
    if (comb->diff<=MIN_OVERFLOW) comb->diff -= MIN_OVERFLOW;

    return comb->diff;
}

sample_t process_cic(cic_t *cic, long data_in, char *data_available)
{
    *data_available = 0;
    long acc = data_in;

    for(int ii=0; ii<cic->N; ii++) acc = process_integrator(&cic->integrators[ii], acc);
    
    if((cic->count % cic->R) == 0)
    {
        for(int cc=0; cc<cic->N; cc++) acc = process_comb(&cic->combs[cc], acc);
        *data_available = 1;
    }
    cic->count++;
    return (sample_t)acc;
}
