/**
 * @file cic.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date June 26 2021
 */

#ifndef _CIC_H_ 
    #define _CIC_H_

    #include <stdlib.h>
    #include <assert.h>

    // FIFO from: https://gist.github.com/ryankurte/61f95dc71133561ed055ff62b33585f8
    typedef struct {
        size_t head;
        size_t tail;
        size_t size;
        void** data;
    } queue_t;

    void* queue_read(queue_t *queue);
    int queue_write(queue_t *queue, void* handle);


    typedef struct 
    {
        short acc;
    } Integrator_t;

    typedef struct 
    {
        short previous;
        short actual;
        short diff;
        short delay;
    } Comb_t;




#endif // _CIC_H_