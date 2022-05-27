import numpy as np

def reverse_hword(x):
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
    return x

def reverse_byte(x):
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
    return x

def do_nothing(data): return data.copy()

def serialize(data, swap_bytes=True, reverse_stage_1=False, reverse_stage_2=False, to_print=False):
    if swap_bytes: bytes_swapped = data.copy().byteswap(inplace=False) 
    else: bytes_swapped = do_nothing(data)

    if reverse_stage_1: reverse1 = reverse_hword(bytes_swapped)
    else: reverse1 = do_nothing(bytes_swapped)

    byte_stream = reverse1.view(dtype=np.uint8)

    if reverse_stage_2: reverse2 = reverse_byte(byte_stream)
    else: reverse2 = do_nothing(byte_stream)

    bit_stream = np.unpackbits(reverse2.ravel(), axis=0)

    if to_print:
        print(data)
        print(bytes_swapped)
        print(byte_stream)
        print(bit_stream)

    return bit_stream