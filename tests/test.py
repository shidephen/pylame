from pylame import MP3Encoder
from pydub import AudioSegment
import io
import numpy as np
from time import time

w = AudioSegment.from_wav("test.wav")
encoder = MP3Encoder(w.frame_rate, 320, 2)

samples = np.array(w.get_array_of_samples())

start = time()
for i in range(50):
    o = encoder.encode_interleaved(samples)
print(f'pylame costs {time() - start}')

start = time()
buf = io.BytesIO()
for i in range(50):
    buf.seek(0)
    w.export(buf, format='mp3')
print(f'Pydub costs: {time() - start}s')

with open('out.mp3', 'wb') as f:
    f.write(o)
