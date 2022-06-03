import numpy as np
import matplotlib.pyplot as plt

def full_plot(data, fs, short=False, title=''):
    np.seterr(divide = 'ignore') 
    fig, axs = plt.subplots(nrows=1, ncols=2, dpi=100, figsize=(20,5))

    n = len(data) - len(data)%2 # round to even size
    data = data[:n]
    t = np.arange(n)/fs
    f = (np.arange(n/2)*fs/n)[:n // 2]

    mag = 20*np.log10(np.abs(np.fft.fft(data)) / n)[:n // 2]

    if short: 
        axs[0].step(t[:n//10000], data[:n//10000], 'k')
        axs[1].plot(f[:n//20], mag[:n//20], 'k')
    else: 
        axs[0].step(t, data, 'k')
        axs[1].plot(f, mag, 'k')

    axs[0].set_ylabel('Amplitude')
    axs[0].set_xlabel('Time (s)')
    axs[0].set_title('Time domain')

    axs[1].set_ylabel('Magnitude (dB)')
    axs[1].set_xlabel('Frequency (Hz)')
    axs[1].set_title('Frequency domain')

    plt.grid(True, which="both")
    plt.suptitle(title)
    plt.show()