#include <cstring>
#include <jni.h>
#include <android/log.h>
#include "input.h"

#define D(s) __android_log_write(ANDROID_LOG_DEBUG, "input_jni.cpp", s)
#define E(s) __android_log_write(ANDROID_LOG_ERROR, "input_jni.cpp", s)

static float x;
static float y;
static float down;

static int argc;
static char **argv;

void initInput() {
    x = y = down = 0.0f;
}

int argCount() {
    return argc;
}
char *getArg(int n) {
    if (0 <= n && n < argc)
        return argv[n];
    return 0;
}

void readInputAxis(int axis, float *buffer, int size)
{
    float src;
    switch(axis)
    {
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
Java_com_iSynth_Audio_setArgs(JNIEnv *env, jobject obj, jobjectArray args) {
    argc = env->GetArrayLength(args);
    argv = new char*[argc];
    for (int i(0); i < argc; ++i) {
        jstring jstr = (jstring)env->GetObjectArrayElement(args, i);
        const char *str = env->GetStringUTFChars(jstr, NULL);
        argv[i] = new char[strlen(str)+1];
        strcpy(argv[i], str);
        env->ReleaseStringUTFChars(jstr, NULL);
    }
}

} //extern "C"

