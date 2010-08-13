#include <jni.h>
#include "synth.h"

extern "C" {

JNIEXPORT void JNICALL
Java_com_iSynth_Audio_produceStream(JNIEnv *env, jobject obj, jshortArray buffer, jint samples)
{
    jshort *buf;
    buf = env->GetShortArrayElements(buffer, 0);
    if (!buf)
        return;;

    synthProduceStream(buf, samples);

    env->ReleaseShortArrayElements(buffer, buf, 0);
}

JNIEXPORT void JNICALL
Java_com_iSynth_Audio_nextPatch(JNIEnv *env, jobject obj, jint d)
{
    synthNextPatch(d);
}

}

