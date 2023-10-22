#ifndef MYAUDIO_H
#define MYAUDIO_H

//***************************************************************************
// myaudio.cxx
// 
// Wearable running device projecet with Spresense for SONY Hackathon 
// this file is an audio program
//
//***************************************************************************

/****************************************************************************
 * Included Files
 ****************************************************************************/


#include <nuttx/config.h>
#include <stdio.h>
#include <string>

#include <audiolite/audiolite.h>

#include "event_str.h"

/****************************************************************************
 * Privete Class
 ****************************************************************************/

/* For Event receiving */

class my_mp3listener : public audiolite_eventlistener
{
  public:
    volatile bool playing;

  public:
    my_mp3listener() : playing(false){};

    void on_event(int evt, audiolite_component *cmp,
                  unsigned long arg)
    {
      printf("Event %s is happened : %d\n", convert_evtid(evt),
                                            (int)arg);
      if (evt == AL_EVENT_DECODEDONE)
        {
          playing = false;
        }
    }
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

extern "C"
int myaudio_main(std::string filepath)
{
  int ret;
  my_mp3listener lsn;

  /* To Create below structure.
   *
   * +---------------------------------+
   * | my_mp3listener to listen events |
   * +---------------------------------+
   *     ^            ^           ^
   *     |            |           |
   * +--------+    +-----+    +-------+
   * | File   |    | MP3 |    | Audio |
   * | Stream | -> | Dec | -> | Output|
   * +--------+    +-----+    --------+
   *                ^   ^
   *                |   |
   *      +------+  |   |  +------+
   *      |Memroy|  |   |  |Memory|
   *      | Pool |--+   +--| Pool |
   *      +------+         +------+
   */

  audiolite_filestream *fstream = new audiolite_filestream;
  audiolite_mempoolapbuf *imempool = new audiolite_mempoolapbuf;
  audiolite_mempoolapbuf *omempool = new audiolite_mempoolapbuf;
  audiolite_outputcomp *aoutdev = new audiolite_outputcomp;
  audiolite_mp3dec *mp3 = new audiolite_mp3dec;

  /* File open on filestream as read mode */

  if (fstream->rfile(filepath.data()) != OK)
    {
      printf("File open error : %d\n", errno);
      //int err = errno;
      //std::cout << "Error: " << strerror(err) << std::endl;
      return -1;
    }

  /* Setup system parameter as
   *   Output Sampling rate : 48K
   *   Output bitwidth for each sample : 16 bits
   *   Output channels : 2 channel
   */

  audiolite_set_systemparam(48000, 16, 2);

  /* Set listener to listen system events */

  audiolite_set_evtlistener(&lsn);

  /* Setup memory pool to read MP3 data from the file
   * as 4096bytes x 8blocks.
   */

  imempool->create_instance(4096, 8);

  /* Setup memory pool to stored decoded data
   * as 4096bytes x 16blocks.
   */

  omempool->create_instance(4096, 16);

  /* Setup MP3 Decorder */

  /* Set memory pools on MP3 Decorder */

  mp3->set_mempool(imempool);
  mp3->set_outputmempool(omempool);

  /* Set file stream on MP3 Decorder */

  mp3->set_stream(fstream);

  /* Connect MP3 output to Audio Output device */

  mp3->bind(aoutdev);

  /* Let's play */

  printf("Start player : %s\n", filepath);
  lsn.playing = true;
  ret = mp3->start();
  if (ret != OK)
    {
      printf("Start error..: %d\n", ret);
      goto app_error;
    }

  /* Wait for finishing as receiving AL_EVENT_DECODEDONE
   * in my_mp3listener.
   */

  while (lsn.playing)
    {
      usleep(10 * 1000);
    }

  /* Stop playing */

  printf("Stop player\n");
  
  mp3->stop();

app_error:

  /* Clean up */

  mp3->unbindall();

  printf("Delete instances\n");

  delete fstream;
  delete mp3;
  delete aoutdev;
  delete imempool;
  delete omempool;

  printf("Delete system event handler\n");

  audiolite_eventdestroy();

  return 0;
}

#endif
