import numpy as np
import matplotlib.pyplot as plt

# def full_plot(data, fs, short=False, title=''):
#     np.seterr(divide = 'ignore') 
#     fig, axs = plt.subplots(nrows=1, ncols=2, dpi=100, figsize=(20,5))

#     n = len(data) - len(data)%2 # round to even size
#     data = data[:n]
#     t = np.arange(n)/fs
#     f = (np.arange(n/2)*fs/n)[:n // 2]

#     mag = 20*np.log10(np.abs(np.fft.fft(data)) / n)[:n // 2]

#     if short: 
#         axs[0].step(t[:n//10], data[:n//10], 'k')
#         # axs[1].plot(f, mag, 'k')
#         axs[1].semilogx(f, mag, 'k')
#     else: 
#         axs[0].step(t, data, 'k')
#         axs[1].semilogx(f, mag, 'k')

#     axs[0].set_ylabel('Amplitude')
#     axs[0].set_xlabel('Time (s)')
#     axs[0].set_title('Time domain')

#     axs[1].set_ylabel('Magnitude (dB)')
#     axs[1].set_xlabel('Frequency (Hz)')
#     axs[1].set_title('Frequency domain')

#     plt.grid(True, which="both")
#     plt.suptitle(title)
#     plt.show()


def full_plot(data, fs):
    np.seterr(divide = 'ignore') 
    fig, axs = plt.subplots(nrows=1, ncols=1, dpi=100, figsize=(6,4))

    n = len(data) - len(data)%2 # round to even size
    data = data[:n]
    t = np.arange(n)/fs
    f = (np.arange(n/2)*fs/n)[:n // 2]

    mag = 20*np.log10(np.abs(np.fft.fft(data)) / n)[:n // 2]

    axs.semilogx(f, mag, 'k')

    axs.set_ylabel('Magnitude (dB)')
    axs.set_xlabel('Frequency (Hz)')
    axs.set_title('Frequency domain')

    plt.grid(True, which="both")
    plt.show()


def get_mag(data, fs):
    n = len(data) - len(data)%2 # round to even size
    data = data[:n]
    f = (np.arange(n/2)*fs/n)[:n // 2]
    mag = 20*np.log10(np.abs(np.fft.fft(data)) / n)[:n // 2]
    return f, mag

def pdm_cic_fir_plot(pdm_stream, cic_processed, fir_processed, fs, os, lang='en'):
    np.seterr(divide = 'ignore') 
    fig, axs = plt.subplots(nrows=1, ncols=3, dpi=100, figsize=(15,3))

    f1, mag1 = get_mag(pdm_stream, fs)
    f2, mag2 = get_mag(cic_processed, fs/os)
    f3, mag3 = get_mag(fir_processed, fs/os)

    a = mag1[np.where(f1>40e3)][0]-mag1[np.where(f1>=100e3)][0]
    b = mag2[np.where(f2>=40e3)][0]-mag2[np.where(f2>=100e3)][0]
    c = mag3[np.where(f3>=40e3)][0]-mag3[np.where(f3>=100e3)][0]

    print(f"SNR_1 = %f, SNR_2 = %f, SNR_3 = %f" % (a, b, c))
    max_ = np.max(np.array([np.max(mag1), np.max(mag2), np.max(mag3)]))
    min_ = np.min(np.array([np.min(mag1), np.min(mag2), np.min(mag3)]))

    axs[0].semilogx(f1, mag1, 'k')
    axs[1].semilogx(f2, mag2, 'k')
    axs[2].semilogx(f3, mag3, 'k')

    if lang=='en':
        axs[0].set_ylabel('Magnitude (dB)')
        axs[0].set_xlabel('Frequency (Hz)'), axs[1].set_xlabel('Frequency (Hz)'), axs[2].set_xlabel('Frequency (Hz)')
        axs[0].set_title('PDM Stream'), axs[1].set_title('CIC Processed'), axs[2].set_title('FIR Processed')
    elif lang=='pt':
        axs[0].set_ylabel('Magnitude (dB)')
        axs[0].set_xlabel('Frequência (Hz)'), axs[1].set_xlabel('Frequência (Hz)'), axs[2].set_xlabel('Frequência (Hz)')
        axs[0].set_title('Fluxo de bits (PDM)'), axs[1].set_title('Saída do filtro CIC'), axs[2].set_title('Saída do filtro FIR')
    axs[0].grid(True, which="both"), axs[1].grid(True, which="both"), axs[2].grid(True, which="both")
    axs[0].set_ylim([min_-10,max_+5]), axs[1].set_ylim([min_-10,max_+5]), axs[2].set_ylim([min_-10,max_+5])
    plt.show()