import numpy as np
from scipy import signal
import matplotlib.pyplot as plt
from collections import deque

class FIFO:
    def __init__(self, size): self.constructor(size)
    def resize(self, size): self.constructor(size)    
    def constructor(self, size):
        self.size = size
        self.fifo = deque(np.zeros(size, dtype=np.int32))

    def push(self, data):
        self.fifo.appendleft(data)
        return self.fifo.pop()
    
    def seek_last(self): return self.fifo[-1]

def check_over_under_flow(val):
    if val>=(2**30-1): val -= (2**30-1)
    elif val<=(-2**30): val -= (-2**30)
    
    return val

class Integrator:
    def __init__(self):
        self.acc=0

    def process(self, data_in):
        data_in = check_over_under_flow(data_in)
        self.acc += data_in

        # check for overflow/underflow
        self.acc = check_over_under_flow(self.acc)

        return self.acc

class Comb:
    def __init__(self, delay=1):
        self.previous = 0
        self.actual = 0
        self.diff = 0
        self.delay = delay
        self.fifo = FIFO(size=self.delay)

    def process(self, data_in):
        data_in = check_over_under_flow(data_in)
        self.actual = data_in

        self.previous = self.fifo.seek_last() #self.actual
        self.diff = self.actual - self.previous
        _ = self.fifo.push(self.actual)
        
        # check for overflow/underflow
        self.diff = check_over_under_flow(self.diff)

        return self.diff

class CIC:
    def __init__(self, stages=1, decimation_factor=16, differential_delay=1):
        self.N = stages
        self.R = decimation_factor
        self.M = differential_delay
        self.gain = (self.R*self.M)**self.N

        self.integrator_stages = [Integrator() for _ in range(self.N)]
        self.comb_stages = [Comb(self.M) for _ in range(self.N)]

    def process(self, data_in): 
        data_out = []
        
        for (index, sample) in enumerate(data_in):
            acc = sample

            for i in range(self.N): # integration step
                acc = self.integrator_stages[i].process(acc)

            if (index % self.R) == 0: # decimation
                for c in range(self.N): # comb step
                    acc = self.comb_stages[c].process(acc)
                # data_out.append(acc/self.gain)
                data_out.append(acc)
            # data_out.append(acc)
        
        return data_out
