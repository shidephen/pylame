from pylame import MP3Encoder
from pydub import AudioSegment
import numpy as np
from time import time

w = AudioSegment.from_wav("../test.wav")
encoder = MP3Encoder(w.frame_rate, 320, 2)

buf = np.array(w.get_array_of_samples())

start = time()
o = encoder.encode_interleaved(buf)
es = time() - start

print("Encoding costs: {}s".format(es))

with open('out.mp3', 'wb') as f:
    f.write(o)


