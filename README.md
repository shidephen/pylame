# pylame
Python wrapped of mp3lame encoder

## build status
[![CircleCI](https://circleci.com/gh/shidephen/pylame.svg?style=svg)](https://circleci.com/gh/shidephen/pylame)

## Example

```python
from pylame import MP3Encoder
from numpy import *

samples = random.rand(100)
# MP3Encoder(samplerate, bitrate, channels)
enc = MP3Encoder(44100, 320, 1)
out = encoder.encode_interleaved(samples)
with open('out.mp3', 'wb') as f:
    f.write(o)

```
