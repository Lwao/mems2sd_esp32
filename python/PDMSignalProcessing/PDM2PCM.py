import numpy as np
import matplotlib.pyplot as plt

from CIC import *

class PDM2PCM():
    def __init__(self, data, fs_pdm, os, stages=1, delay=1):
        self.data = data.astype(np.int32)
        self.fs = fs_pdm
        self.os = os
        self.size = len(data)
        self.delay = delay
        self.stages = stages
        self.cic = CIC(stages=self.stages, decimation_factor=self.os, differential_delay=self.delay)

        self.out_done = False

        self.data_pcm = np.empty(self.size//self.os)

        self.process()

    def set_data(self, data): 
        self.data = data
        self.out_done = False

    def process(self):
        if not self.out_done: self.data_pcm = self.cic.process(self.data)
        self.data_pcm = self.data_pcm[(2+self.delay):]-np.mean(self.data_pcm[(2+self.delay):])
        self.out_done = True

    def mag_plot(self, short=False):
        fig, axs = plt.subplots(nrows=1, ncols=2, dpi=100, figsize=(20,4))
        
        data = self.data_pcm
        n = len(data)
        fs = self.fs/self.os
        
        t = np.arange(n)/fs
        f = (np.arange(n/2)*fs/n)[:n // 2]

        mag = 20*np.log10(np.abs(np.fft.fft(data)) / n)[:n // 2]

        if short:
            m = len(t)//100
            axs[0].plot(t[:m], data[:m], 'k')
        else:
            axs[0].plot(t, data, 'k')

        axs[1].plot(f, mag, 'k')

        axs[0].set_title('Time domain')
        axs[0].set_ylabel('Amplitude')
        axs[0].set_xlabel('Time (s)')
        axs[1].set_title('Frequency domain')
        axs[1].set_ylabel('Magnitude')
        axs[1].set_xlabel('Frequency (Hz)')

        plt.show()

    def pdm_plot(self, short=False):
        np.seterr(divide = 'ignore') 

        fig, axs = plt.subplots(nrows=2, ncols=2, dpi=100, figsize=(20,10))

        os = self.os
        fs_pdm = self.fs
        fs_pcm = self.fs/self.os

        data_pcm = self.data_pcm
        data_pdm = self.data

        n_pcm = len(data_pcm)
        t_pcm = np.arange(n_pcm)/fs_pcm
        f_pcm = (np.arange(n_pcm/2)*fs_pcm/n_pcm)[:n_pcm // 2]

        n_pdm = len(data_pdm)
        t_pdm = np.arange(n_pdm)/fs_pdm
        f_pdm = (np.arange(n_pdm/2)*fs_pdm/n_pdm)[:n_pdm // 2]

        mag_pcm = 20*np.log10(np.abs(np.fft.fft(data_pcm)) / n_pcm)[:n_pcm // 2]
        mag_pdm = 20*np.log10(np.abs(np.fft.fft(data_pdm)) / n_pdm)[:n_pdm // 2]
        
        if short:
            axs[0,0].step(t_pcm[:n_pcm//100], data_pcm[:n_pcm//100], 'k')
            axs[0,1].step(t_pdm[:n_pdm//100], data_pdm[:n_pdm//100], 'k')
        else:
            axs[0,0].step(t_pcm, data_pcm, 'k')
            axs[0,1].step(t_pdm, data_pdm, 'k')

        axs[1,0].plot(f_pcm, mag_pcm, 'k')
        axs[1,1].plot(f_pdm, mag_pdm, 'k')

        axs[0,0].set_ylabel('Amplitude')
        axs[0,1].set_ylabel('Amplitude')
        axs[1,0].set_ylabel('Magnitude')
        axs[1,1].set_ylabel('Magnitude')

        axs[0,0].set_xlabel('Time (s)')
        axs[0,1].set_xlabel('Time (s)')
        axs[1,0].set_xlabel('Frequency (Hz)')
        axs[1,1].set_xlabel('Frequency (Hz)')

        axs[0,0].set_title('Time domain (PCM)')
        axs[0,1].set_title('Time domain (PDM)')
        axs[1,0].set_title('Frequency domain (PCM)')
        axs[1,1].set_title('Frequency domain (PDM)')

        plt.show()       