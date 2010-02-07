#include <stdio.h>
#include <unistd.h>
#include <AssertMacros.h>
#include <CoreFoundation/CoreFoundation.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioUnit/AUComponent.h>
#include <AudioUnit/AudioComponent.h>

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

    AudioComponentInstanceDispose(gOutputUnit);
}

void setupSound()
{
    FinishedFrames = 0;
    OSStatus err = noErr;
    
    AudioComponentDescription desc;
    desc.componentType = kAudioUnitType_Output;
#if TARGET_OS_IPHONE
    desc.componentSubType = kAudioUnitSubType_RemoteIO;
#else
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
#endif
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;
    
    AudioComponent comp = AudioComponentFindNext(NULL, &desc);
    verify(comp);
    
    err = AudioComponentInstanceNew(comp, &gOutputUnit);
    verify(!err);

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
