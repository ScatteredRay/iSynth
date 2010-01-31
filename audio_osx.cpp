#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <unistd.h>
#include <AudioUnit/AudioUnit.h>

#include <math.h>

void produceStream(short *buffer, int samples);

AudioUnit gOutputUnit;
UInt32 FinishedFrames;

OSStatus AudRenderCallback(void* inRefCon,
                        AudioUnitRenderActionFlags* Flags,
                        const AudioTimeStamp* ioActionFlags,
                        UInt32 inBusNumber,
                        UInt32 inNumberFrames,
                        AudioBufferList* ioData)
{
    printf("Audio Render Callback: %d\n", ioData->mNumberBuffers);
    float FrameLength = 1.0f/44100.0f;
    
    produceStream((short*)ioData->mBuffers[0].mData, inNumberFrames);

    FinishedFrames += inNumberFrames;
    
    for(UInt32 channel = 1; channel < ioData->mNumberBuffers; channel++)
        memcpy(ioData->mBuffers[channel].mData, ioData->mBuffers[0].mData, ioData->mBuffers[0].mDataByteSize);

    //FinishedFrames += inNumberFrames;
    return noErr;
}

void streamSound()
{
    OSStatus err = noErr;

    err = AudioOutputUnitStart(gOutputUnit);
    verify(!err);
    CFRunLoopRun();
    
    verify_noerr(AudioOutputUnitStop(gOutputUnit));

    CloseComponent(gOutputUnit);
}

void setupSound()
{
    FinishedFrames = 0;
    OSStatus err = noErr;
    
    ComponentDescription desc;
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;
    
    Component comp = FindNextComponent(NULL, &desc);
    verify(comp);
    
    err = OpenAComponent(comp, &gOutputUnit);
    verify(comp);

    err = AudioUnitInitialize(gOutputUnit);
    verify(!err);

    AudioStreamBasicDescription streamFormat;
    streamFormat.mSampleRate = 44100.0;
    streamFormat.mFormatID = kAudioFormatLinearPCM;
    streamFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger 
        | kAudioFormatFlagsNativeEndian
        | kLinearPCMFormatFlagIsPacked
        | kAudioFormatFlagIsNonInterleaved;
    streamFormat.mBytesPerPacket = 2;
    streamFormat.mFramesPerPacket = 1;
    streamFormat.mBytesPerFrame = 2;
    streamFormat.mChannelsPerFrame = 2;
    streamFormat.mBitsPerChannel = 16;

    err = AudioUnitSetProperty(gOutputUnit,
                               kAudioUnitProperty_StreamFormat,
                               kAudioUnitScope_Input,
                               0,
                               &streamFormat,
                               sizeof(AudioStreamBasicDescription));
    verify(!err);

	err = AudioUnitInitialize(gOutputUnit);
    verify(!err);

    AURenderCallbackStruct input;
    input.inputProc = AudRenderCallback;
    input.inputProcRefCon = NULL;

	err = AudioUnitSetProperty (gOutputUnit, 
								kAudioUnitProperty_SetRenderCallback, 
								kAudioUnitScope_Input,
								0, 
								&input, 
								sizeof(input));
    verify(err);

    //err = AudioUnitAddRenderNotify(gOutputUnit, AudRenderCallback, NULL);
    //verify(!err);

}

void makeNoise()
{ 
  setupSound();
  streamSound();
}
