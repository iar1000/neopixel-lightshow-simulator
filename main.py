import math
from time import sleep

sine = [
    128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170,
    173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211,
    213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240,
    241, 243, 244, 245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254,
    254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251,
    250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232,
    230, 228, 226, 224, 222, 220, 218, 215, 213, 211, 208, 206, 203, 201, 198,
    196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155,
    152, 149, 146, 143, 140, 137, 134, 131, 128, 124, 121, 118, 115, 112, 109,
    106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 76, 73, 70, 67, 65,
    62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29,
    27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6,
    5, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11,
    12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37,
    40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76,
    79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121,
    124]

gamma = [
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3,
    3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6,
    6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10,
    11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17,
    17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
    25, 26, 27, 27, 28, 29, 29, 30, 31, 31, 32, 33, 34, 34, 35,
    36, 37, 38, 38, 39, 40, 41, 42, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 68, 69, 70, 71, 72, 73, 75, 76, 77, 78, 80, 81,
    82, 84, 85, 86, 88, 89, 90, 92, 93, 94, 96, 97, 99, 100, 102,
    103, 105, 106, 108, 109, 111, 112, 114, 115, 117, 119, 120, 122, 124, 125,
    127, 129, 130, 132, 134, 136, 137, 139, 141, 143, 145, 146, 148, 150, 152,
    154, 156, 158, 160, 162, 164, 166, 168, 170, 172, 174, 176, 178, 180, 182,
    184, 186, 188, 191, 193, 195, 197, 199, 202, 204, 206, 209, 211, 213, 215,
    218, 220, 223, 225, 227, 230, 232, 235, 237, 240, 242, 245, 247, 250, 252,
    255]

NUMPIXELS = 127
MIDDLE = 69
pixels = [0] * NUMPIXELS


# map from 0-9 for printing the strip
def map_dezi(strip):
    return [math.floor(x / 28) for x in strip]


# print out the values of the strip
def print_strip(strip):
    for pi in range(len(strip)):
        print(strip[pi], end="")
    print()


# sine wave passing through
def sine_traverse(t):
    new_pixels = [0] * NUMPIXELS
    for pi in range(NUMPIXELS):
        sine_pointer = (pi + t) % 255
        new_pixels[pi] = gamma[sine[sine_pointer]]
    return new_pixels


# two sine waves that traverse outwards
def sine_traverse_outwards(t):
    new_pixels = [0] * NUMPIXELS
    left_pixels = [0] * MIDDLE
    for pi in range(MIDDLE):
        sine_pointer = (pi + t) % 255
        left_pixels[pi] = gamma[sine[sine_pointer]]
    new_pixels[:MIDDLE] = left_pixels
    left_pixels.reverse()
    new_pixels[MIDDLE:] = left_pixels[:(NUMPIXELS-MIDDLE)]
    return new_pixels


def build_state_linear(goal_state, change):
    new_pixels = pixels
    done = True
    for pi in range(NUMPIXELS):
        # increase brightness linearly if necessary
        if pixels[pi] < goal_state[pi]:
            done = False
            new_pixels[pi] = pixels[pi] + change
            # handle overshoot
            if new_pixels[pi] > goal_state[pi]:
                new_pixels[pi] = goal_state[pi]
        # decrese brightness if necessary
        elif pixels[pi] > goal_state[pi]:
            done = False
            new_pixels[pi] = pixels[pi] - change
            # handle overshoot
            if new_pixels[pi] < goal_state[pi]:
                new_pixels[pi] = goal_state[pi]

    return new_pixels, done


# simulate time
start_program = False
for t in range(0, 80):
    while not start_program:
        init_setting = sine_traverse_outwards(t)
        new_pixels, ready = build_state_linear(init_setting, 1)
        pixels = new_pixels
        start_program = ready
        print_strip(map_dezi(pixels))

    pixels = sine_traverse_outwards(t)

    print_strip(map_dezi(pixels))
    sleep(0.1)

end_program = False
while not end_program:
    init_setting = [0] * NUMPIXELS
    new_pixels, ready = build_state_linear(init_setting, 1)
    pixels = new_pixels
    end_program = ready
    print_strip(map_dezi(pixels))
