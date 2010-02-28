#include "audio.h"

#include <dsound.h>
#include <windows.h>
#include <winerror.h>
#include <dxerr.h>

#include "exception.h"
#include "input.h"
#include "synth.h"

IDirectSoundBuffer *buffer;
HANDLE notification_event;

EXCEPTION_D(DsoundExcept, Exception, "DirectSound Exception")

void setupSound(unsigned int buffer_size)
{
  IDirectSound8 *dsound;
  if(FAILED(DirectSoundCreate8(0, &dsound, 0)))
    throw DsoundExcept("couldn't create dsound interface");
  if(FAILED(dsound->SetCooperativeLevel(GetConsoleWindow(), DSSCL_PRIORITY)))
    throw DsoundExcept("couldn't set dsound cooperative level");
    
  WAVEFORMATEX wave_format =
  {
    WAVE_FORMAT_PCM, // format tag
    2,               // channel count
    44100,           // sample rate
    44100*4,         // bytes per second
    4,               // block alignment (bytes per sample * channel count)
    16,              // bits per sample
    0                // size of additional data in this descriptor
  };
  
  DSBUFFERDESC buffer_descriptor = 
  {
    sizeof(DSBUFFERDESC), // size of this descriptor
    DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY,
    buffer_size*8,        // size of buffer
    0,                    // "reserved"
    &wave_format,         // wave format descriptor
    DS3DALG_DEFAULT       // 3d virtualization algorithm
  };
  
  
  if(FAILED(dsound->CreateSoundBuffer(&buffer_descriptor, &buffer, 0)))
    throw DsoundExcept("couldn't create dsound buffer");
  
  IDirectSoundNotify *notify;
  if(FAILED(buffer->QueryInterface(IID_IDirectSoundNotify, 
                                   (void **)&notify)))
    throw DsoundExcept("couldn't create dsound notify interface");
  
  notification_event = CreateEvent(0, false, false, 0);
  if(!notification_event)
    throw DsoundExcept("couldn't create notification event");
  
  const int notify_count = 8;
  DSBPOSITIONNOTIFY notify_positions[notify_count];
  for(int i=0; i<8; i++)
  {
    notify_positions[i].dwOffset     = i * buffer_size * 8 / notify_count;
    notify_positions[i].hEventNotify = notification_event;
  }
  
  if(FAILED(notify->SetNotificationPositions(notify_count, notify_positions)))
    throw DsoundExcept("couldn't set dsound notification positions");
  
  void *write_buffer;
  DWORD write_buffer_size;
  if(FAILED(buffer->Lock(0, 0, &write_buffer, &write_buffer_size,
                         0, 0, DSBLOCK_ENTIREBUFFER)))
    throw DsoundExcept("couldn't lock dsound buffer");

  memset(write_buffer, 0, write_buffer_size);

  if(FAILED(buffer->Unlock(write_buffer, write_buffer_size, 0, 0)))
    throw DsoundExcept("couldn't unlock dsound buffer");

  if(FAILED(buffer->Play(0, 0, DSBPLAY_LOOPING)))
    throw DsoundExcept("couldn't play dsound buffer");
}

void streamSound(unsigned int buffer_size)
{
  unsigned int next_write_position = 0;
  
  for(;;)
  {
    if(getKey() == 27) return;
    
    DWORD event = MsgWaitForMultipleObjects(1, &notification_event, false,
                                            INFINITE,  QS_ALLEVENTS);
    if(event == WAIT_OBJECT_0) // time to stream more audio
    {
      DWORD write_window_start, write_window_end;
      buffer->GetCurrentPosition(&write_window_end, &write_window_start);
      write_window_start /= 4;
      write_window_end   /= 4;

      if(write_window_end == 0) write_window_end = buffer_size*2-1;
      else write_window_end--;

      next_write_position %= (buffer_size*2);
//      printf("nwp:%d, wws:%d, wwe:%d\n", next_write_position, write_window_start, write_window_end);
        
      if(write_window_end < write_window_start) write_window_end +=
        buffer_size*2;
      
      if(next_write_position < write_window_start) next_write_position +=
        buffer_size*2;
        
      if(next_write_position < write_window_end)
      {
        int write_length = write_window_end-next_write_position;
        next_write_position %= (buffer_size*2);
        
        void  *write_buffers     [2];
        DWORD  write_buffer_sizes[2];
        if(FAILED(buffer->Lock(next_write_position*4, write_length*4,
                               write_buffers,         write_buffer_sizes,
                               write_buffers+1,       write_buffer_sizes+1, 0)))
          throw DsoundExcept("couldn't lock dsound buffer");
        
//        printf("[%p, %d], [%p, %d]\n", write_buffers[0], write_buffer_sizes[0],
//                                       write_buffers[1], write_buffer_sizes[1]);

        for(int i=0; i<2; i++)
          if(write_buffers[i])
            produceStream((short *)(write_buffers[i]),
                                    write_buffer_sizes[i]/4);
   
        if(FAILED(buffer->Unlock(write_buffers[0], write_buffer_sizes[0], 
                                 write_buffers[1], write_buffer_sizes[1])))
          throw DsoundExcept("couldn't unlock dsound buffer");
          
        next_write_position += write_length;
      }      
    }
    else // other crap
    {
      MSG message;
      while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) 
      { 
        TranslateMessage(&message); 
        DispatchMessage (&message); 
      }
    }
  }
}

void makeNoise()
{ 
  setupSound (1024);
  streamSound(1024);
}