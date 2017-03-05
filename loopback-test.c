#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#define BUFSIZE 32

#define RATE 44100

int16_t buffer[BUFSIZE];
int error;
static pa_simple *s = NULL;
static pa_simple *s1 = NULL;
static char name_buf[] = "PulseAudio default device";

int pulseaudio_begin()
{
  int error;
  /* The sample type to use */
  static const pa_sample_spec ss = {
  	.format = PA_SAMPLE_S16LE,
  	.rate = RATE,
  	.channels = 2
  };
  /* Create the recording stream */
  if (!(s = pa_simple_new(NULL, "xyz", PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) {
    printf("Error: adin_pulseaudio: pa_simple_new() failed: %s\n", pa_strerror(error));
    return 1;
  }
  /* Create the playback stream */
  if (!(s1 = pa_simple_new(NULL, "abc", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
	fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
	return 1;
  }
  return 0;
}

int pulseaudio_end()
{
  if (s != NULL) {
    pa_simple_free(s);
    s = NULL;
  }
  if (s1 != NULL) {
    pa_simple_free(s1);
    s1 = NULL;
  }
  return 0;
}

int pulseaudio_read (int16_t *buf, int sampnum)
{
  int error;
  int cnt, bufsize;

  bufsize = sampnum * sizeof(int16_t);
  if (bufsize > BUFSIZE) bufsize = BUFSIZE;
  /* Record some data ... */
  if (pa_simple_read(s, buf, bufsize, &error) < 0) {
        printf("Error: pa_simple_read() failed: %s\n", pa_strerror(error));
  }
  cnt = bufsize / sizeof(int16_t);
  return (cnt);
}


int main()
{
	pulseaudio_begin();


while(1)
{
	pulseaudio_read(buffer, 32);
  /* Playback the recorded data ... */
	if (pa_simple_write(s1, buffer, 32, &error) < 0) 
  {
	  fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
	  goto finish;
	}
  /* And write it to STDOUT */    
  if (write(STDOUT_FILENO, buffer, 32) != 32) 
  {
    fprintf(stderr, __FILE__": write() failed: %s\n", strerror(errno));
    goto finish;
   }
	usleep(5);
}

finish:
	pulseaudio_end();

	return 0;
}
