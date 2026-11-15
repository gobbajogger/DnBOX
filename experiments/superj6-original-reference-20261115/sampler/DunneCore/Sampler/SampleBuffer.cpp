// Copyright AudioKit. All Rights Reserved.

#include "SampleBuffer.h"
#include <cmath>

namespace DunneCore
{

    SampleBuffer::SampleBuffer()
    : samples(0)
    , pcm16Data(nullptr),pcm16Bytes(0),ownedSampleWords(0)
    , channelCount(0)
    , sampleCount(0)
    , startPoint(0.0f)
    , endPoint(0.0f)
    , isLooping(false)
    , loopStartPoint(0.0f)
    , loopEndPoint(0.0f)
    {
    }
    
    SampleBuffer::~SampleBuffer()
    {
        deinit();
    }
    
    void SampleBuffer::init(float sampleRate, int channelCount, int sampleCount)
    {
        this->sampleRate = sampleRate;
        this->sampleCount = sampleCount;
        this->channelCount = channelCount;
        if (samples) delete[] samples;
        samples = new float[channelCount * sampleCount];
        pcm16Data=nullptr;pcm16Bytes=0;ownedSampleWords=std::size_t(channelCount)*std::size_t(sampleCount);
        loopStartPoint = startPoint = 0.0f;
        loopEndPoint = endPoint = (float)(sampleCount - 1);
    }
    
    void SampleBuffer::deinit()
    {
        if (samples) delete[] samples;
        samples = 0;pcm16Data=nullptr;pcm16Bytes=0;ownedSampleWords=0;
    }
    
    void SampleBuffer::setData(unsigned index, float data)
    {
        if (channelCount>0 && sampleCount>0 && std::size_t(index)<std::size_t(channelCount)*std::size_t(sampleCount))
        {
            if(pcm16Data){
                const std::size_t words=std::size_t(channelCount)*std::size_t(sampleCount);
                float *owned=new float[words];
                for(std::size_t i=0;i<words;++i)owned[i]=readValue(unsigned(i));
                samples=owned;ownedSampleWords=words;pcm16Data=nullptr;pcm16Bytes=0;
            }
            samples[index] = data;
        }
    }
    
    bool SampleBuffer::initBorrowedPcm16(float sr, int channels, int frames,
                                        const unsigned char *data, std::size_t bytes)
    {
        if (!std::isfinite(sr) || sr < 8000.f || sr > 192000.f ||
            channels < 1 || channels > 2 || frames < 1 || frames > 16777216 || !data)
            return false;
        const std::size_t words=std::size_t(channels)*std::size_t(frames);
        if (bytes != words*2) return false;
        const std::uintptr_t begin=reinterpret_cast<std::uintptr_t>(data);
        if (bytes > UINTPTR_MAX-begin) return false;
        const std::uintptr_t end=begin+bytes;
        auto overlap=[&](const void *p,std::size_t n) {
            if(!p||!n)return false;
            const std::uintptr_t a=reinterpret_cast<std::uintptr_t>(p);
            if(n>UINTPTR_MAX-a)return true;
            return begin<a+n && a<end;
        };
        if(overlap(this,sizeof(*this)) || overlap(samples,ownedSampleWords*sizeof(float)))
            return false;
        if(samples)delete[] samples;
        samples=nullptr;ownedSampleWords=0;
        pcm16Data=data;pcm16Bytes=bytes;
        sampleRate=sr;channelCount=channels;sampleCount=frames;
        loopStartPoint=startPoint=0.f;loopEndPoint=endPoint=float(frames-1);
        return true;
    }

}
