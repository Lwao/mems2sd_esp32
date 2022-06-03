import numpy as np
def reverse_hword(x: np.int16) -> np.int16:
    """
    in-> 0123 4567 89AB CDEF
    
    masks:
        0000 0000 1111 1111 
        1111 1111 0000 0000

    in-> 89AB CDEF 0123 4567

    masks:
        0000 1111 0000 1111
        1111 0000 1111 0000

    in-> CDEF 89AB 4567 0123

    masks:
        0011 0011 0011 0011
        1100 1100 1100 1100

    in-> EFCD AB89 6745 2301

    masks:
        0101 0101 0101 0101
        1010 1010 1010 1010

    in-> FEDC BA98 7654 3210
    """
    x = ((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8)
    x = ((x & 0x0F0F) << 4) | ((x & 0xF0F0) >> 4)
    x = ((x & 0x3333) << 2) | ((x & 0xCCCC) >> 2)
    x = ((x & 0x5555) << 1) | ((x & 0xAAAA) >> 1)
    return x.astype(np.int16)

def reverse_byte(x: np.uint8) -> np.uint8:
    """
    in-> 0123 4567
    
    masks:
        0000 1111 
        1111 0000

    in-> 4567 0123

    masks:
        0011 0011
        1100 1100

    in-> 6745 2301

    masks:
        0101 0101
        1010 1010
    
    in-> 7654 3210
    """
    x = ((x & 0x0F) << 4) | ((x & 0xF0) >> 4)
    x = ((x & 0x33) << 2) | ((x & 0xCC) >> 2)
    x = ((x & 0x55) << 1) | ((x & 0xAA) >> 1)
    return x.astype(np.uint8)

def do_nothing(data): return data.copy()

def serialize(data, swap_bytes=False, reverse_bit_hword_stage=False, reverse_bit_byte_stage=False, to_print=False):
    if reverse_bit_hword_stage: reverse1 = reverse_hword(data.copy())
    else: reverse1 = do_nothing(data.copy())

    if swap_bytes: bytes_swapped = reverse1.copy().byteswap(inplace=False) 
    else: bytes_swapped = do_nothing(reverse1.copy())

    byte_stream = bytes_swapped.view(dtype=np.uint8)

    if reverse_bit_byte_stage: reverse2 = reverse_byte(byte_stream)
    else: reverse2 = do_nothing(byte_stream)

    bit_stream = np.unpackbits(reverse2.ravel(), axis=0)

    if to_print:
        print("Input data:", end=' ')
        print(data, end=' ')
        print(type(data[0]))

        if reverse_bit_hword_stage:
            print("Half-word bits reversed:", end=' ')
            print(reverse1, end=' ')
            print(type(reverse1[0]))

        if swap_bytes:
            print("Bytes swapped:", end=' ')
            print(bytes_swapped, end=' ')
            print(type(bytes_swapped[0]))

        print("Byte stream:", end=' ')
        print(byte_stream, end=' ')
        print(type(byte_stream[0]))

        if reverse_bit_byte_stage:
            print("Byte bits reversed:", end=' ')
            print(reverse2, end=' ')
            print(type(reverse2[0]))

        print("Bit stream:", end=' ')
        print(bit_stream, end=' ')
        print(type(bit_stream[0]))

    return bit_stream


def serialize2(data):
    data = reverse_hword(data)
    data = data.byteswap(inplace=False) 
    data = data.view(dtype=np.uint8)
    data = reverse_byte(data)
    data = np.unpackbits(data.ravel(), axis=0)
    return data