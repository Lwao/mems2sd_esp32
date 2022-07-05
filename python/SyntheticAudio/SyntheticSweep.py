
import numpy as np
import sounddevice as sd
import sys, time, argparse

def gen_audio(fs=44100):
    fs = 44100
    t_end = 3
    freq = np.concatenate([np.arange(start=0, stop=22000, step=2500),np.zeros(1)])

    t = np.arange(0, t_end, 1/fs)
    n = len(t)
    d = len(freq)
    m = n//d

    data = np.zeros(n)

    for (i,f) in enumerate(freq): 
        if i!=d-1: range_ = np.arange(start=i*m, stop=(i+1)*m+1, step=1)
        else: range_ = np.arange(start=i*m, stop=n, step=1)
        data[range_] = np.sin(2*np.pi*f*t[range_])
    
    return data, fs

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate and play synthetic audio.')
    parser.add_argument('-f','--fs', help='Sampling frequency', required=False)
    args = vars(parser.parse_args())

    if args['fs']: data, fs = gen_audio(int(args['fs']))
    else: data, fs = gen_audio()

    sd.play(data, fs, loop=True) # loop through sample

    try:
        while True:
            time.sleep(.1)
    except KeyboardInterrupt:
        print('Exiting...')
        sd.stop() # stop sound device play
        print('Processes successfully closed!')
        sys.exit()