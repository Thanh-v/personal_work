import math
import cmath

def bit_reveerse(index, num_bits):
    result = 0
    for _ in range(num_bits):
        result = (result << 1) | (index & 1)
        index >>= 1

    return result

def fft(x):
    x = [complex(value) for value in x]

    num_bits = int(math.log2(N))
    X = [0j] * N

    for i in range(N):
        reversed_index = bit_reveerse(i, num_bits)
        X[reversed_index] = x[i]

    length = 2

    while length <= N:
        half = length // 2

        w_length = cmath.exp(-2j * math.pi / length)

        for start in range(0, N, length):
            w = 1 + 0j

            for j in range(half):
                u = X[start + j]
                v = X[start + j + half]

                t = w * v

                X[start + j] = u + t
                X[start + j + half] = u - t

                w *= w_length

        length *= 2

    return X


fs = 1000 # Sampling freq
N = 1024 # Number of samples

x = []

for n in range(N):
    t = n / fs

    value = (
        math.cos(2 * math.pi * 50 * t)
        + 0.5 * math.cos(2 * math.pi *120 *t)
        + 0.25 * math.cos(2 * math.pi * 200 * t)
    )

    x.append(complex(value))

X = fft(x)

freq = []
magnitude = []

for k in range(N):
    f = k * fs / N
    mag = abs(X[k])

    freq.append(f)
    magnitude.append(mag)

half_N = N // 2

peaks = []

for k in range(half_N):
    peaks.append((freq[k], magnitude[k]))

peaks.sort(
    key=lambda x: x[1],
    reverse=True
)

print("Top 10 frequency peaks:")
for i in range(10):
    freq, mag = peaks[i]
    print(
        f"{i + 1:2d}"
        f"f = {freq:10.4f} Hz, "
        f"|X| = {mag:.6f}"
    )
