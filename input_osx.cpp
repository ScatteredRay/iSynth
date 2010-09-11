#include <assert.h>
#include <CoreFoundation/CFBundle.h>
#include "input.h"
#include <mach/mach.h>
#include <mach/mach_time.h>

float osx_x;
float osx_y;
float osx_down;

void inputDown(float down)
{
    osx_down = down;
}

void inputXY(float X, float Y)
{
    osx_x = X;
    osx_y = Y;
}

void initInput(int _argc, char **_argv)
{
    osx_x = 0.0f;
    osx_y = 0.0f;
    osx_down = 0.0f;
}

void readInputAxis(int axis, float *buffer, int size)
{
    float src;
    switch(axis)
    {
        case 0:
            src = osx_x;
            break;
        case 1:
            src = osx_y;
            break;
        case 2:
            src = osx_down;
            break;
        default:
            src = 0.0f;
            break;
    }

    for(int i=0; i<size; i++)
        buffer[i] = src;
}

using namespace std;

void populatePatchList(vector<string>& patches)
{
    patches.push_back(getPatchLocation("rubberbass"));
}

std::string getPatchLocation(const char* patchname)
{
   CFBundleRef Bundle = CFBundleGetMainBundle();
    assert(Bundle);
    
    CFURLRef patch = CFBundleCopyResourceURL(
                      Bundle,
                      CFStringCreateWithCString(kCFAllocatorDefault,
                                                patchname,
                                                kCFStringEncodingMacRoman),
                      CFSTR("pat"),
                      NULL);
                      
    CFStringRef path = CFURLCopyFileSystemPath(patch, kCFURLPOSIXPathStyle);
    assert(path);
    size_t pathlen = (CFStringGetLength(path) + 1) * sizeof(char);
    char* cpath = (char*)malloc(pathlen);
    CFStringGetCString(path, cpath, pathlen, kCFStringEncodingMacRoman);
    assert(cpath);
    string ret = string(cpath);
    
    
    // The ptr is owned by the CFStringRef, but since we've copied it onto a
    // std::string we can just free everything! Yeah!
    free(cpath);
    CFRelease(patch);
    CFRelease(path);

    return ret;
}

void populateLogList(vector<string>& log_list)
{
}

double hires_time()
{
  mach_timebase_info_data_t info;
  mach_timebase_info(&info);
  uint64_t time = mach_absolute_time();
  uint64_t nanoseconds = time * info.numer / info.denom;
  return 
    (double)nanoseconds / 1000000000.0;
}
