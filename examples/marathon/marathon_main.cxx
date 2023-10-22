//***************************************************************************
// marathon_main.cxx
// 
// Wearable running device projecet with Spresense for SONY Hackathon 
// this file is a main program
//
//***************************************************************************


/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "mygnss.h"
#include "myaudio.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/



/****************************************************************************
 * Private Types
 ****************************************************************************/


/****************************************************************************
 * Private Data
 ****************************************************************************/



/****************************************************************************
 * Name: read_and_print()
 *
 * Description:
 *   Read and print POS data.
 *
 * Input Parameters:
 *   fd - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/




/****************************************************************************
 * Name: gnss_main()
 *
 * Description:
 *   Set parameters and run positioning.
 *
 * Input Parameters:
 *   argc - Does not use.
 *   argv - Does not use.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/


extern "C" int marathon_main(int argc, FAR char *argv[])
{
  sleep(3);

  std::cout << "main" << std::endl;

  myaudio_main("mnt/sd0/audio/hello.mp3");

  mygnss_main();

  return 0;
}
