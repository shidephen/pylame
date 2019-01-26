#include<algorithm>
#include<stdexcept>
#include<iostream>
#include<cstdio>
#include<cstring>
#include<pybind11/pybind11.h>
#include<pybind11/numpy.h>

using namespace std;

extern "C"
{
#include<lame/lame.h>
}

// 256KB
#define MP3_BUF_SIZE 1024 * 256

namespace{
void debug_out(const char* format, va_list ap){
    printf(format, ap);
}

void error_out(const char* format, va_list ap){
    printf(format, ap);
}

void info_out(const char* format, va_list ap){
    printf(format, ap);
}
}


class MP3Encoder
{
public:
    MP3Encoder(size_t samplerate, size_t bitrate, size_t channels)
    : m_channels(channels), m_samplerate(samplerate), m_bitrate(bitrate)
    {
        int ret;
        m_settings = lame_init();

        lame_set_num_channels(m_settings, channels);
        lame_set_in_samplerate(m_settings, samplerate);
        lame_set_brate(m_settings, bitrate);
        lame_set_quality(m_settings, 5);

        // Debug output
        lame_set_errorf(m_settings, &error_out);
        lame_set_debugf(m_settings, &debug_out);
        lame_set_msgf(m_settings, &info_out);

        ret = lame_init_params(m_settings);
        if(ret < 0)
            throw new invalid_argument("Some parameter is invalid");
    }

    ~MP3Encoder(){
        if(m_encoded_buffer != nullptr)
            delete[] m_encoded_buffer;
        lame_close(m_settings);
    }
    
    pybind11::array_t<uint8_t>
    EncodeInterleaved(pybind11::array_t<int16_t> input)
    {
        auto in_buf = input.request();
        auto in_ptr = (int16_t*)in_buf.ptr;
        size_t num_frame = in_buf.size/m_channels;
        size_t outbuf_size = (size_t)(in_buf.size * 1.25 / m_channels) + 7200;
        realloc_buffer(outbuf_size);
        
        // =============== Lame Proc ===================

        size_t sample_num = in_buf.size;
        size_t input_pos = 0, encoded_size = 0;

        int ret = lame_encode_buffer_interleaved(m_settings,
                in_ptr, num_frame,
                m_encoded_buffer, outbuf_size);
        assert(ret >= 0);
        
        int rest = lame_encode_flush_nogap(m_settings, m_encoded_buffer+ret, outbuf_size - ret);
        assert(rest >= 0);

        // =========================================
        
        pybind11::array_t<uint8_t> result{(size_t)(ret + rest)};
        auto out_buf = result.request(true);
        auto out_ptr = (uint8_t*)out_buf.ptr;

        memcpy(out_ptr, m_encoded_buffer, ret + rest);

        return result;
    }

    // Properties ----------------------
    void SampleRate(size_t samplerate) {
        lame_set_in_samplerate(m_settings, samplerate);
        int ret = lame_init_params(m_settings);
        if(ret < 0)
            throw new invalid_argument("Invalid sample rate");
    }
    size_t SampleRate(){
        return lame_get_in_samplerate(m_settings);
    }
    void BitRate(size_t bitrate){
        lame_set_brate(m_settings, bitrate);
        int ret = lame_init_params(m_settings);
        if(ret < 0)
            throw new invalid_argument("Invalid bit rate");
    }
    size_t BitRate(){
        return lame_get_brate(m_settings);
    }
    void ChannelNum(size_t channels){
        lame_set_num_channels(m_settings, channels);
        int ret = lame_init_params(m_settings);
        if(ret < 0)
            throw new invalid_argument("Invalid channel number");

        m_channels = channels;
    }
    size_t ChannelNum() {
        return lame_get_num_channels(m_settings);
    }
private:
    size_t m_samplerate, m_bitrate, m_channels;
    lame_t m_settings;

    uint8_t *m_encoded_buffer;
    size_t m_encoded_buffer_size = MP3_BUF_SIZE;

    void realloc_buffer(size_t outsize) {
        if(outsize > m_encoded_buffer_size) {
            if(m_encoded_buffer != nullptr) {
                delete[] m_encoded_buffer;
                m_encoded_buffer = nullptr;

            }
            m_encoded_buffer = new uint8_t[outsize];
            m_encoded_buffer_size = outsize;
        }
    }
};

PYBIND11_MODULE(pylame, m)
{
    namespace py = pybind11;

    py::class_<MP3Encoder>(m, "MP3Encoder")
        .def(py::init<size_t, size_t, size_t>())
        .def("encode_interleaved", &MP3Encoder::EncodeInterleaved);
}
