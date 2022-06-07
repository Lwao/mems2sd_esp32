
import numpy as np
import sounddevice as sd
import sys, time, argparse

def gen_audio(fs=44100):
    t_end = 1
    fm_ini = 0
    fm_end = fs/2

    t = np.arange(0, t_end, 1/fs)
    n = len(t)
    f = 5000
    data = np.sin(2*np.pi*f*t)
    
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