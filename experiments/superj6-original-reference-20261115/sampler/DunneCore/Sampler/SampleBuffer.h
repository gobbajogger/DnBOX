// Copyright AudioKit. All Rights Reserved.

#pragma once
#include <cstddef>
#include <cstdint>
namespace DunneCore
{

    // SampleBuffer represents an array of sample data, which can be addressed with a real-valued
    // "index" via linear interpolation.
    
    struct SampleBuffer
    {
        float *samples;
        const unsigned char *pcm16Data;
        std::size_t pcm16Bytes,ownedSampleWords;
        float sampleRate;
        int channelCount;
        int sampleCount;
        float startPoint, endPoint;
        bool isLooping;
        float loopStartPoint, loopEndPoint;
        float noteFrequency;
        
        SampleBuffer();
        ~SampleBuffer();
        
        void init(float sampleRate, int channelCount, int sampleCount);
        void deinit();
        // Cold bounded view only; immutable/readable byte owner must outlive voices.
        // Planar s16LE,1..2channels,8..192kHz,frames1..2^24,exact byte extent.
        // Rejects leave metadata unchanged. This is not storage mapping or lifetime discovery.
        bool initBorrowedPcm16(float sr,int channels,int frames,
                              const unsigned char *data,std::size_t bytes);
        inline float readValue(unsigned index) const {
            if(channelCount<=0||sampleCount<=0||std::size_t(index)>=std::size_t(channelCount)*std::size_t(sampleCount))return 0.f;
            if(!pcm16Data)return samples?samples[index]:0.f;
            if(std::size_t(index)>=pcm16Bytes/2)return 0.f;
            const unsigned value=unsigned(pcm16Data[index*2])|(unsigned(pcm16Data[index*2+1])<<8);
            const int signedValue=value>=32768?int(value)-65536:int(value);
            return float(signedValue)*.000030517578125f;
        }
        
        void setData(unsigned index, float data);
        
        // Use float for the real-valued index, because oscillators will need the extra precision.
        inline float interp(float fIndex, float gain)
        {
            if ((samples == 0 && pcm16Data == 0) || sampleCount == 0) return 0.0f;
            
            int ri = int(fIndex);
            float f = fIndex - ri;
            int rj = ri + 1;
            
            float si = ri < sampleCount ? readValue(unsigned(ri)) : 0.0f;
            float sj = rj < sampleCount ? readValue(unsigned(rj)) : 0.0f;
            return (float)(gain * ((1.0f - f) * si + f * sj));
        }
        
        inline void interp(float fIndex, float *leftOutput, float *rightOutput, float gain)
        {
            if ((samples == 0 && pcm16Data == 0) || sampleCount == 0)
            {
                *leftOutput = *rightOutput = 0.0f;
                return;
            }
            if (channelCount == 1)
            {
                *leftOutput = *rightOutput = interp(fIndex, gain);
                return;
            }
            
            int ri = int(fIndex);
            float f = fIndex - ri;
            int rj = ri + 1;
            
            float si = ri < sampleCount ? readValue(unsigned(ri)) : 0.0f;
            float sj = rj < sampleCount ? readValue(unsigned(rj)) : 0.0f;
            *leftOutput = (float)(gain * ((1.0f - f) * si + f * sj));
            si = ri < sampleCount ? readValue(unsigned(sampleCount + ri)) : 0.0f;
            sj = rj < sampleCount ? readValue(unsigned(sampleCount + rj)) : 0.0f;
            *rightOutput = (float)(gain * ((1.0f - f) * si + f * sj));
        }
    };
    
    // KeyMappedSampleBuffer is a derived version with added MIDI note-number and velocity ranges
    struct KeyMappedSampleBuffer : public SampleBuffer
    {
        // Any of these members may be negative, meaning "no value assigned"
        int noteNumber;     // closest MIDI note-number to this sample's frequency (noteFrequency)
        int minimumNoteNumber, maximumNoteNumber;     // bounding note numbers for mapping
        int minimumVelocity, maximumVelocity;       // min/max MIDI velocities for mapping
    };

}
