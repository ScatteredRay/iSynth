# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

STLPORT_BASE	:= $(NDK_WRAPPERS_BASE)/stlport

LOCAL_MODULE    := iSynth
LOCAL_CFLAGS	+= -I$(STLPORT_BASE)/stlport \
		   -D__NEW__ \
		   -D__SGI_STL_INTERNAL_PAIR_H \
		   -DANDROID \
		   -DOS_ANDROID
LOCAL_LDLIBS	+= -L$(STLPORT_BASE)/build/lib/obj/arm-linux-gcc/so \
		   -lstlport \
		   -llog
LOCAL_SRC_FILES := input_jni.cpp audio_jni.cpp file_jni.cpp synth.cpp

include $(BUILD_SHARED_LIBRARY)
