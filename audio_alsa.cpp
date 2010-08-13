#include <alsa/asoundlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "synth.h"

snd_pcm_uframes_t period_size = 512;
snd_pcm_uframes_t buffer_size = 128;
unsigned int      periods     = 4;

static snd_pcm_t *init_alsa(snd_pcm_t **pcm_handle,
							snd_pcm_stream_t stream_type) {
    const char          *pcm_name = "default";
    snd_pcm_hw_params_t *hwparams;
    unsigned int         rate = 11025;
    int                  status;


    // Open the device.  Experimenting with duplex mode.
    fprintf(stderr, "Opening PCM device\n");
    status = snd_pcm_open(pcm_handle, pcm_name,
                          stream_type, 0);
    if(status < 0) {
        fprintf(stderr, "Unable to open audio device: %s\n",
                snd_strerror(status));
        goto error;
    }


    // Determine what the hardware can do.
    fprintf(stderr, "Allocating hw params\n");
    snd_pcm_hw_params_malloc(&hwparams);
    status = snd_pcm_hw_params_any(*pcm_handle, hwparams);
    if(status < 0) {
        fprintf(stderr, "Unable to get hardware parameters: %s\n",
                snd_strerror(status));
        goto error;
    }


    // Set up interleaved audio.
    fprintf(stderr, "Setting up interleaved audio\n");
    status = snd_pcm_hw_params_set_access(*pcm_handle, hwparams,
                                          SND_PCM_ACCESS_RW_INTERLEAVED);
    if(status < 0) {
        fprintf(stderr, "Unable to set interleaved mode: %s\n",
                snd_strerror(status));
        goto error;
    }


    // Set the audio format.
    fprintf(stderr, "Setting format\n");
    status = snd_pcm_hw_params_set_format(*pcm_handle, hwparams,
                                          SND_PCM_FORMAT_S16_LE);
    if(status<0) {
        fprintf(stderr, "Unable to set audio format: %s\n",
                snd_strerror(status));
        goto error;
    }


    // Set stereo audio.
    fprintf(stderr, "Setting channels\n");
    status = snd_pcm_hw_params_set_channels(*pcm_handle, hwparams, 2);
    if(status<0) {
        fprintf(stderr, "Unable to set stereo mode: %s\n",
                snd_strerror(status));
        goto error;
    }


    // Set sample rate.
    fprintf(stderr, "Setting audio rate\n");
    status = snd_pcm_hw_params_set_rate_near(*pcm_handle, hwparams,
                                             &rate, NULL);
    if(status<0) {
        fprintf(stderr, "Unable to set sample rate to 44100: %s\n",
                snd_strerror(status));
        goto error;
    }


	if((status = snd_pcm_hw_params_set_periods_near(*pcm_handle, hwparams, &periods, 0)) < 0) {
		fprintf(stderr, "Unable to set periods: %s\n", snd_strerror(status));
		goto error;
	}
	fprintf(stderr, "Set periods to: %d\n", periods);


	status = snd_pcm_hw_params_set_period_size_near(*pcm_handle, hwparams, &period_size, NULL);
	if(status<0) {
		fprintf(stderr, "Unable to set period size: %s\n",
				snd_strerror(status));
	}
	snd_pcm_hw_params_get_periods(hwparams, &periods, NULL);
	fprintf(stderr, "Set period size to %d\n", (int)period_size);

	buffer_size = ((period_size * periods) >> 2);
	if((status = snd_pcm_hw_params_set_buffer_size_near(*pcm_handle, hwparams, &buffer_size)) < 0) {
		fprintf(stderr, "Unable to set buffer size: %s\n",
				snd_strerror(status));
	}
	fprintf(stderr, "Set buffer size to: %d\n", (int)buffer_size);


	// Figure out latency, as period_size * periods / (rate * bytes_per_frame)
	fprintf(stderr, "Have a latency of %d * %d / (%d * %d) = %d msecs\n",
			(int)period_size, periods, rate, 4,
			(int)(period_size * 1000 * periods / (rate * 4.0)));


    // Write the settings out to the audio card.
    fprintf(stderr, "Setting hw params\n");
    status = snd_pcm_hw_params(*pcm_handle, hwparams);
    if(status<0) {
        fprintf(stderr, "Unable to set audio parameters: %s\n",
                snd_strerror(status));
        goto error;
    }

    snd_pcm_hw_params_free(hwparams);

    status = snd_pcm_prepare(*pcm_handle);
    if(status<0) {
        fprintf(stderr, "Unable to prepare audio device: %s\n",
                snd_strerror(status));
        goto error;
    }

	fprintf(stderr, "Audio initialized\n");
    return *pcm_handle;

error:
    if(*pcm_handle) {
        snd_pcm_drain(*pcm_handle);
        snd_pcm_close(*pcm_handle);
    }
    return NULL;
}


int alsa_recover(snd_pcm_t *stream, int print_message) {
    snd_pcm_status_t *status;
    int res;

    snd_pcm_status_alloca(&status);
    if ((res = snd_pcm_status(stream, status))<0) {
        fprintf(stderr, "status error: %s", snd_strerror(res));
        return -1;
    }

    if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
        struct timeval now, diff, tstamp;
        gettimeofday(&now, 0);
        snd_pcm_status_get_trigger_tstamp(status, &tstamp);
        timersub(&now, &tstamp, &diff);
		if(print_message)
			fprintf(stderr, "underrun!!! (at least %.3f ms long)\n",
					diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
        if ((res = snd_pcm_prepare(stream))<0) {
            fprintf(stderr, "xrun: prepare error: %s", snd_strerror(res));
            return -1;
        }
        return 0;     /* ok, data should be accepted again */
    }
    fprintf(stderr, "read/write error, state = %s",
		snd_pcm_state_name(snd_pcm_status_get_state(status)));
    return -1;
}


void makeNoise() {
	snd_pcm_t *pcm_handle;
	int reported_underflow = 0;
	if(!init_alsa(&pcm_handle, SND_PCM_STREAM_PLAYBACK))
		return;

	{
		int frames_avail = snd_pcm_avail_update(pcm_handle);
		fprintf(stderr, "At the start, there are %d frames\n", frames_avail);
		snd_pcm_uframes_t output_buffer[frames_avail];
		bzero(output_buffer, sizeof(output_buffer));
		snd_pcm_writei(pcm_handle, output_buffer, frames_avail);
	}

	while(1) {
		int frames_avail = snd_pcm_avail_update(pcm_handle);
		int status;
		if(!frames_avail)
			continue;

		// Clip the number of frames, because synthProduceStream
		// doesn't like more than, say, 4000 bytes at a time.
		if(frames_avail > 256)
			frames_avail = 256;

		snd_pcm_uframes_t output_buffer[frames_avail];

		synthProduceStream((long *)output_buffer, frames_avail);
		/*
		printf("Sample:\n");
		for(int i=0; i<frames_avail/2; i++) {
			printf(" %08x", output_buffer[i]);
			if(!(i+1&7))
				printf("\n");
		}
		printf("\n");
		*/
		status = snd_pcm_writei(pcm_handle, output_buffer, frames_avail);
		if(status == -EPIPE) {
			if(!reported_underflow)
				fprintf(stderr, "Write error: (Over/Under)run\n");
			if(alsa_recover(pcm_handle, !reported_underflow))
				return;

			// Don't report another underflow until we've been
			// underflow-free for 8 loops;
			reported_underflow = 8;
		}

		else if(status <= 0) {
			fprintf(stderr, "Write error (%d): %s\n",
					status, snd_strerror(status));
		}
		else if(reported_underflow > 1)
			reported_underflow--;
		else if(reported_underflow == 1) {
			reported_underflow = 0;
			fprintf(stderr, "Cleared underflow condition\n");
		}
	}
}
