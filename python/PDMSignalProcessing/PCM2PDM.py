import numpy as np
import matplotlib.pyplot as plt

class PCM2PDM():
    def __init__(self, data, fs_pcm, os):
        self.data = data
        self.fs = fs_pcm
        self.os = os
        self.size = len(data)

        self.os_done = False
        self.pdm_done = False

        self.data_os = np.empty(self.size*self.os)
        self.data_pdm = np.empty(self.size*self.os)

        self.process()

    def set_data(self, data): 
        self.data = data
        self.os_done = False
        self.pdm_done = False

    def oversample(self): 
        # self.data_os =  self.data.repeat(self.os)
        self.data_os = self.data
        self.os_done = True

    def pdm(self, qe=0):
        if not self.os_done: self.oversample()

        data = self.data_os
        n = len(data)
        pdm_stream = np.zeros(n)

        for i in range(n):
            if qe>=0: pdm_stream[i]=1
            else: pdm_stream[i]=-1
            qe += data[i]-pdm_stream[i]

        self.data_pdm = pdm_stream
        self.pdm_done = True
    
    def process(self):
        if not self.os_done: self.oversample()
        if not self.pdm_done: self.pdm(qe=0)

    def mag_plot(self, short=False):
        fig, axs = plt.subplots(nrows=1, ncols=2, dpi=100, figsize=(20,4))

        n = self.size
        fs = self.fs*self.os
        data = self.data

        t = np.arange(n)/fs
        f = (np.arange(n/2)*fs/n)[:n // 2]
        
        mag = 20*np.log10(np.abs(np.fft.fft(data)) / n)[:n // 2]

        if short:
            m = len(t)//200
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
        fs = self.fs

        pdm_fs = os*fs

        data_os = self.data_os
        data_pdm = self.data_pdm

        n = len(data_os)
        t = np.arange(n)/pdm_fs
        f = (np.arange(n/2)*pdm_fs/n)[:n // 2]

        mag_os = 20*np.log10(np.abs(np.fft.fft(data_os)) / n)[:n // 2]
        mag_pdm = 20*np.log10(np.abs(np.fft.fft(data_pdm)) / n)[:n // 2]

        if short:
            axs[0,0].step(t[:n//200], data_os[:n//200], 'k')
            axs[0,1].step(t[:n//200], data_pdm[:n//200], 'k')
        else:
            axs[0,0].step(t, data_os, 'k')
            axs[0,1].step(t, data_pdm, 'k')
        
        axs[1,0].plot(f, mag_os, 'k')
        axs[1,1].plot(f, mag_pdm, 'k')

        axs[0,0].set_ylabel('Amplitude')
        axs[0,1].set_ylabel('Amplitude')
        axs[1,0].set_ylabel('Magnitude')
        axs[1,1].set_ylabel('Magnitude')

        axs[0,0].set_xlabel('Time (s)')
        axs[0,1].set_xlabel('Time (s)')
        axs[1,0].set_xlabel('Frequency (Hz)')
        axs[1,1].set_xlabel('Frequency (Hz)')

        axs[0,0].set_title('Time domain (PCM-OS)')
        axs[0,1].set_title('Time domain (PDM)')
        axs[1,0].set_title('Frequency domain (PCM-OS)')
        axs[1,1].set_title('Frequency domain (PDM)')

        plt.show()       