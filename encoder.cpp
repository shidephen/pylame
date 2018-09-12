#include<algorithm>
#include<stdexcept>
#include<iostream>
#include<cstdio>
#include<pybind11/pybind11.h>
#include<pybind11/numpy.h>

using namespace std;

extern "C"
{
#include<lame/lame.h>
}

#define MP3_BUF_SIZE 8192

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
    {
        int ret;
        m_settings = lame_init();

        lame_set_num_channels(m_settings, channels);
        lame_set_in_samplerate(m_settings, samplerate);
        lame_set_brate(m_settings, bitrate);
        lame_set_quality(m_settings, 2);

        // Debug output
        lame_set_errorf(m_settings, &error_out);
        lame_set_debugf(m_settings, &debug_out);
        lame_set_msgf(m_settings, &info_out);

        ret = lame_init_params(m_settings);
        if(ret < 0)
            throw new invalid_argument("Some parameter is invalid");
    }


    ~MP3Encoder(){
        lame_close(m_settings);
    }
    
    pybind11::array_t<uint8_t>
    EncodeInterleaved(pybind11::array_t<int16_t> input)
    {
        auto in_buf = input.request();
        auto in_ptr = (int16_t*)in_buf.ptr;
        size_t num_frame = in_buf.size/2;
        auto *encode_buf = 
            new uint8_t[(size_t)(in_buf.size * 0.625) + 7200];
        
        // =============== Lame Proc ===================

        size_t sample_num = num_frame * 2;
        size_t input_pos = 0, encoded_size = 0;
        uint8_t* tmp_buf = new uint8_t[MP3_BUF_SIZE*2];

        for(size_t input_pos=0, i=0; input_pos < sample_num;
            input_pos += MP3_BUF_SIZE*2, i+=1){
            
            size_t input_size;

            if(input_pos + MP3_BUF_SIZE*2 < sample_num)
                input_size = MP3_BUF_SIZE;
            else
                input_size = num_frame - input_pos / 2;
            
            int ret = lame_encode_buffer_interleaved(m_settings,
                in_ptr + input_pos, input_size,
                tmp_buf, MP3_BUF_SIZE*2);
            
            if(ret > 0){
                copy_n(tmp_buf, ret, encode_buf+encoded_size);
                encoded_size += ret;
            }else{
                cout << "Warning encoded ret: " << ret << endl;
            }
        }
        
        // Get last buffer bytes
        int ret = lame_encode_flush_nogap(m_settings, tmp_buf, MP3_BUF_SIZE*2);
        if(ret > 0){
            copy_n(tmp_buf, ret, encode_buf+encoded_size);
            encoded_size += ret;
        }

        delete[] tmp_buf;

        // =========================================
        
        pybind11::array_t<uint8_t> result{encoded_size};
        auto out_buf = result.request(true);
        auto out_ptr = (uint8_t*)out_buf.ptr;

        copy_n(encode_buf, encoded_size, out_ptr);
        delete[] encode_buf;

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
            throw new invalid_argument("Invalid bit rate");
    }
    size_t ChannelNum() {
        return lame_get_num_channels(m_settings);
    }
private:
    size_t m_samplerate, m_bitrate, m_channels;
    lame_t m_settings;
};

PYBIND11_MODULE(pylame, m)
{
    namespace py = pybind11;

    py::class_<MP3Encoder>(m, "MP3Encoder")
        .def(py::init<size_t, size_t, size_t>())
        .def("encode_interleaved", &MP3Encoder::EncodeInterleaved);
}
