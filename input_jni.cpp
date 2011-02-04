#include <cstring>
#include <jni.h>
#include <android/log.h>
#include "input.h"
#include "synth.h"


#ifdef PROFILING
#include <sys/time.h>
#endif

using std::string;

#define D(s) __android_log_write(ANDROID_LOG_DEBUG, "input_jni.cpp", s)
#define E(s) __android_log_write(ANDROID_LOG_ERROR, "input_jni.cpp", s)

static float x;
static float y;
static float down;

void initInput(int argc, char **argv) {
    x = y = down = 0.0f;
}

void deinitInput() { }

void populateLogList(std::vector<std::string>& log_list) { }

double hires_time() {
#ifdef PROFILING
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec/1e6;
#else
    return 0.0;
#endif
}

char getKey() {
    return 0;
}

void readInputAxis(int axis, float *buffer, int size) {
    float src;
    switch(axis) {
        case 0:
            src = x;
            break;
        case 1:
            src = y;
            break;
        case 2:
            src = down;
            break;
        default:
            src = 0.0f;
            break;
    }
    
    for(int i=0; i<size; i++)
        buffer[i] = src;
}

string getPatchLocation(const char* patchname)
{
    return string("patches/") + patchname + string(".pat");
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_iSynth_Audio_inputXY(JNIEnv *env, jobject obj, jfloat X, jfloat Y)
{
    x = (float)X;
    y = (float)Y;
}

JNIEXPORT void JNICALL
Java_com_iSynth_Audio_inputDown(JNIEnv *env, jobject obj, jboolean d)
{
    down = d==JNI_TRUE? 1.0f: 0.0f;
}

JNIEXPORT void JNICALL
Java_com_iSynth_iSynth_setPatch(JNIEnv *env, jobject obj, jstring patch) {
    const char *cstr = env->GetStringUTFChars(patch, NULL);

    // assumes synth.cpp won't use this string after synthSetPatch retunrs
    synthSetPatch(cstr);

    env->ReleaseStringUTFChars(patch, NULL);
}

JNIEXPORT void JNICALL
Java_com_iSynth_iSynth_setScale(JNIEnv *env, jobject obj, jstring scale) {
    const char *cstr = env->GetStringUTFChars(scale, NULL);

    synthSetScale(cstr);

    env->ReleaseStringUTFChars(scale, NULL);
}

JNIEXPORT void JNICALL
Java_com_iSynth_iSynth_setKey(JNIEnv *env, jobject obj, jint key) {
    synthSetKey((int)key);
}

} //extern "C"

