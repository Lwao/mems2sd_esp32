/**
 * @file pdm2pcm.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date July 1 2022
 */

#ifndef _PDM2PCM_H_
#define _PDM2PCM_H_

#ifndef C_POSIX_LIB_INCLUDED
    #define C_POSIX_LIB_INCLUDED
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdint.h>
    #include <math.h>
#endif //C_POSIX_LIB_INCLUDED


#define SAMPLES 1024
#define BYTE_PER_SAMPLE 4
#define APPLY_MASK(x,i) (long)((((x>>(31-i))&0x00000001) << 1)-1)
#define INPUT_SAMPLE_SIZE 32
#define FIR_ORDER 64//256//64

#define MAX_OVERFLOW  1073741823 // (long)(pow(2,30)-1)
#define MIN_OVERFLOW -1073741824 // (long)(-pow(2,30))

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


/**
 * @brief Initialize FIFO structure with default parameters.
 *
 * @param fifo Pointer to FIFO strcuture to be initialized.
 * @param size FIFO size.
 */
void init_fifo(fifo_t *fifo, size_t size);

/**
 * @brief Enqueue data into FIFO.
 *
 * @param fifo Pointer to FIFO structure to be modified.
 * @param data_in Data to be enqueued.
 */
void enqueue(fifo_t *fifo, long data_in);

/**
 * @brief Dequeue data from FIFO.
 *
 * @param fifo Pointer to FIFO structure to be accessed.
 * @param data_in Data to be enqueued.
 */
void dequeue(fifo_t *fifo, long *data_out);

/**
 * @brief Initialize Integrator structure with default parameters.
 *
 * @param integ Pointer to Integrator structure to be initialized.
 */
void init_integrator(integrator_t *integ);

/**
 * @brief Initialize Comb structure with default parameters.
 *
 * @param comb Pointer to Comb structure to be initialized.
 * @param delay Comb differential delay (internal FIFO size).
 */
void init_comb(comb_t *comb, char delay);

/**
 * @brief Initialize CIC structure with default parameters.
 *
 * @param cic Pointer to CIC structure to be initialized.
 * @param N Number of Integrator/Comb stages.
 * @param R Decimation rate.
 * @param M Differential delay.
 */
void init_cic(cic_t *cic, int N, int R, int M);

/**
 * @brief Process one step in Integrator structure.
 *
 * @param integ Pointer to Integrator structure to be modified.
 * @param data_in Data to be processed.
 */
long process_integrator(integrator_t *integ, long data_in);

/**
 * @brief Process one step in Comb structure.
 *
 * @param comb Pointer to Comb structure to be modified.
 * @param data_in Data to be processed.
 */
long process_comb(comb_t *comb, long data_in);

/**
 * @brief Process one step in CIC structure.
 *
 * @param cic Pointer to CIC structure to be modified.
 * @param data_in Data to be processed.
 * @param data_available Pointer to flag indicating that there is data available.
 */
sample_t process_cic(cic_t *cic, long data_in, char *data_available);

/**
 * @brief Swap bytes of word (long).
 * 
 * @param x Long integer to have bytes swapped.
 */
long swap_bytes_of_word(long x);

/**
 * @brief Initialize App Specific CIC structure with default parameters.
 *
 * @param cic Pointer to App Specific CIC structure to be initialized.
 */
void init_app_cic(app_cic_t *cic);

/**
 * @brief Initialize App Specific FIR structure with default parameters.
 *
 * @param fir Pointer to App Specific FIR structure to be initialized.
 */
void init_app_fir(app_fir_t *fir);

/**
 * @brief Check for overflow in integrators' accumulators for App Specific CIC structure.
 *
 * @param cic Pointer to App Specific CIC structure to be used.
 */
void integ_overflow(app_cic_t *cic);

/**
 * @brief Process input buffer with App Specific CIC structure.
 *
 * @param cic Pointer to App Specific CIC structure to be used.
 */
void process_app_cic(app_cic_t *cic, long (*input_buffer)[SAMPLES], short (*output_buffer)[2*SAMPLES]);

/**
 * @brief Process input buffer with App Specific FIR structure.
 *
 * @param cic Pointer to App Specific FIR structure to be used.
 */
void process_app_fir(app_fir_t *fir, short fir_coeffs[FIR_ORDER], short (*pcm_samples)[2*SAMPLES]);

#endif // _PDM2PCM_H_