/**
 * @file pdm2pcm.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date July 1 2022
 */

#include "pdm2pcm.h"

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

void process_app_cic(app_cic_t *cic, long (*input_buffer)[SAMPLES], short (*output_buffer)[2*SAMPLES])
{
    long sample, temp, pcm_sample, comb_in[2];
    
    for(int ii=0; ii<SAMPLES; ii++)
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