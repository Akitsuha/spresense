#ifndef MYGNSS_H
#define MYGNSS_H

//***************************************************************************
// gnss.cxx
// 
// Wearable running device projecet with Spresense for SONY Hackathon 
// this file is a gnss program
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
#include <arch/chip/gnss.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>

#include "myaudio.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define GNSS_POLL_FD_NUM          1
#define GNSS_POLL_TIMEOUT_FOREVER -1
#define MY_GNSS_SIG               18
#define GNSS_DEVNAME "/dev/gps"

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct cxd56_gnss_dms_s
{
  int8_t   sign;
  uint8_t  degree;
  uint8_t  minute;
  uint32_t frac;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static uint32_t                         posfixflag;
static uint32_t                         posfixflag_c;
static struct cxd56_gnss_positiondata_s posdat;

static void double_to_dmf(double x, struct cxd56_gnss_dms_s * dmf)
{
  int    b;
  int    d;
  int    m;
  double f;
  double t;

  if (x < 0)
    {
      b = 1;
      x = -x;
    }
  else
    {
      b = 0;
    }
  d = (int)x; /* = floor(x), x is always positive */
  t = (x - d) * 60;
  m = (int)t; /* = floor(t), t is always positive */
  f = (t - m) * 10000;

  dmf->sign   = b;
  dmf->degree = d;
  dmf->minute = m;
  dmf->frac   = f;
}


/****************************************************************************
 * Name: csv_outfile()
 *
 * Description:
 *   Write latitude, longitude, altitude in a csv file.
 *
 * Input Parameters:
 *   std::ofstream ofs - filestream.
 *   double latitude - latitude.
 *   double longitude - longitude.
 *   double altitude - altitude.
 *   std::string iso_time - ISO standard time.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

static int csv_outfile(std::ofstream& ofs, double latitude, double longitude, double altitude, std::string iso_time)
{
  ofs << std::fixed << std::setprecision(6) << posdat.receiver.longitude << "," << std::setprecision(6) << posdat.receiver.latitude << "," << "0" << std::endl;
  return 0;
}

/****************************************************************************
 * Name: gpx_outfile()
 *
 * Description:
 *   Write latitude, longitude, altitude in a csv file.
 *
 * Input Parameters:
 *   std::ofstream ofs - filestream.
 *   double latitude - latitude.
 *   double longitude - longitude.
 *   double altitude - altitude.(optional)
 *   std::string iso_time - ISO standard time.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

static int gpx_outfile(std::ofstream& ofs, double latitude, double longitude, double altitude, std::string iso_time)
{
  ofs << std::fixed << "<trkpt lat=\"" << std::setprecision(6) << latitude << "\" lon=\"" << std::setprecision(6) << longitude << "\">" << std::endl;
  ofs << "    <time>" << iso_time << "</time>" << std::endl;  // 時刻（ISO 8601形式）
  ofs << "</trkpt>" << std::endl;

  return 0;
}

/****************************************************************************
 * Name: read_and_print()
 *
 * Description:
 *   Read and print POS data.
 *
 * Input Parameters:
 *   fd - File descriptor.
 *   std::string data_path - a file path recording position data.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

static int read_and_print(int fd, std::string data_path)
{
  int ret;
  struct cxd56_gnss_dms_s dmf;

  std::stringstream ss;
  std::string iso_time;

  /* Read POS data. */

  ret = read(fd, &posdat, sizeof(posdat));
  if (ret < 0)
    {
      printf("read error\n");
      goto _err;
    }
  else if (ret != sizeof(posdat))
    {
      ret = ERROR;
      printf("read size error\n");
      goto _err;
    }
  else
    {
      ret = OK;
    }

  /* Print POS data. */

  /* Print time. */

  printf(">Hour:%d, minute:%d, sec:%d, usec:%ld\n",
         posdat.receiver.time.hour, posdat.receiver.time.minute,
         posdat.receiver.time.sec, posdat.receiver.time.usec);

  ss << std::setw(4) << std::setfill('0') << std::to_string(posdat.receiver.date.year)
    << "-" << std::setw(2) << std::setfill('0') << std::to_string(posdat.receiver.date.month)
    << "-" << std::setw(2) << std::setfill('0') << std::to_string(posdat.receiver.date.day)
    << "T" << std::setw(2) << std::setfill('0') << std::to_string(posdat.receiver.time.hour)
    << ":" << std::setw(2) << std::setfill('0') << std::to_string(posdat.receiver.time.minute)
    << ":" << std::setw(2) << std::setfill('0') << std::to_string(posdat.receiver.time.sec);

  iso_time = ss.str();

  std::cout << iso_time << std::endl;

  if (posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
    {
      /* 2D fix or 3D fix.
       * Convert latitude and longitude into dmf format and print it. */

      posfixflag = 1;

      std::ofstream ofs;
      ofs.open(data_path,std::ios::app); //出力ファイルストリーム

      double_to_dmf(posdat.receiver.latitude, &dmf);
      //printf(">LAT %d.%d.%04ld\n", dmf.degree, dmf.minute, dmf.frac);

      double_to_dmf(posdat.receiver.longitude, &dmf);
      //printf(">LNG %d.%d.%04ld\n", dmf.degree, dmf.minute, dmf.frac);

      //std::cout << std::fixed << std::setprecision(6) << posdat.receiver.longitude << "," << std::setprecision(6) << posdat.receiver.latitude << "," << "0" << std::endl;


      //ofs << std::fixed << std::setprecision(6) << posdat.receiver.longitude << "," << std::setprecision(6) << posdat.receiver.latitude << "," << "0" << std::endl;
      gpx_outfile(ofs,posdat.receiver.latitude,posdat.receiver.longitude,0.,iso_time);
      
    }
  else
    {
      /* No measurement. */
      posfixflag = 0;

      printf(">No Positioning Data\n");
    }

_err:
  return ret;
}

/****************************************************************************
 * Name: gnss_setparams()
 *
 * Description:
 *   Set gnss parameters use ioctl.
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

static int gnss_setparams(int fd)
{
  int      ret = 0;
  uint32_t set_satellite;
  struct cxd56_gnss_ope_mode_param_s set_opemode;

  /* Set the GNSS operation interval. */

  set_opemode.mode     = 1;     /* Operation mode:Normal(default). */
  set_opemode.cycle    = 1000;  /* Position notify cycle(msec step). */

  ret = ioctl(fd, CXD56_GNSS_IOCTL_SET_OPE_MODE, (uint32_t)&set_opemode);
  if (ret < 0)
    {
      printf("ioctl(CXD56_GNSS_IOCTL_SET_OPE_MODE) NG!!\n");
      goto _err;
    }

  /* Set the type of satellite system used by GNSS. */

  set_satellite = CXD56_GNSS_SAT_GPS | CXD56_GNSS_SAT_GLONASS;

  ret = ioctl(fd, CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM, set_satellite);
  if (ret < 0)
    {
      printf("ioctl(CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM) NG!!\n");
      goto _err;
    }

_err:
  return ret;
}

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

extern "C" int mygnss_main()
{
  int      fd;
  int      ret;
  int      posperiod;
  sigset_t mask;
  struct cxd56_gnss_signal_setting_s setting;
  std::string data_folder_path = "/mnt/sd0/route";
  std::string extension = ".gpx";
  std::string datapath;
  std::ifstream datafile;
  int file_count = 0;

  std::cout << "Asakura" << std::endl;

  /* Get file descriptor to control GNSS. */

  fd = open(GNSS_DEVNAME, O_RDONLY);
  if (fd < 0)
    {
      printf("open error:%d,%d\n", fd, errno);
      
      return -ENODEV;
    }
  
  /* Configure mask to notify GNSS signal. */

  sigemptyset(&mask);
  sigaddset(&mask, MY_GNSS_SIG);
  ret = sigprocmask(SIG_BLOCK, &mask, NULL);
  if (ret != OK)
    {
      printf("sigprocmask failed. %d\n", ret);
      goto _err;
    }

  /* Set the signal to notify GNSS events. */

  setting.fd      = fd;
  setting.enable  = 1;
  setting.gnsssig = CXD56_GNSS_SIG_GNSS;
  setting.signo   = MY_GNSS_SIG;
  setting.data    = NULL;

  ret = ioctl(fd, CXD56_GNSS_IOCTL_SIGNAL_SET, (unsigned long)&setting);
  if (ret < 0)
    {
      printf("signal error\n");
      goto _err;
    }

  /* Set GNSS parameters. */

  ret = gnss_setparams(fd);
  if (ret != OK)
    {
      printf("gnss_setparams failed. %d\n", ret);
      goto _err;
    }

  /* Initial positioning measurement becomes cold start if specified hot
   * start, so working period should be long term to receive ephemeris. */

  posperiod  = 3000;
  posfixflag = 0;
  posfixflag_c = 0;

  /* Start GNSS. */

  myaudio_main("mnt/sd0/audio/helpMe.mp3");

  ret = ioctl(fd, CXD56_GNSS_IOCTL_START, CXD56_GNSS_STMOD_HOT);
  if (ret < 0)
    {
      printf("start GNSS ERROR %d\n", errno);
      goto _err;
    }
  else
    {
      printf("start GNSS OK\n");
    }

  while(file_count < 1000){
    datapath = data_folder_path + "/route_" + std::to_string(file_count) + extension;
    datafile = std::ifstream(datapath.c_str());

    if(datafile.is_open()){
      file_count++;
      continue;
    }
    else{
      std::cout << "filepath is " << datapath << std::endl;
      datafile.close();
      break;
    }
  }


  do
    {
      /* Wait for positioning to be fixed. After fixed,
       * idle for the specified seconds. */

      ret = sigwaitinfo(&mask, NULL);
      if (ret != MY_GNSS_SIG)
        {
          printf("sigwaitinfo error %d\n", ret);
          break;
        }

      /* Read and print POS data. */

      ret = read_and_print(fd,datapath);
      if (ret < 0)
        {
          break;
        }

      if (posfixflag)
        {
          /* Count down started after POS fixed. */

          if(posfixflag_c != posfixflag){

            myaudio_main("mnt/sd0/audio/start_gnss.mp3");
            posfixflag_c = posfixflag;
          }

          posperiod--;
        }
    }
  while (posperiod > 0);

  /* Stop GNSS. */

  myaudio_main("mnt/sd0/audio/error.mp3");

  ret = ioctl(fd, CXD56_GNSS_IOCTL_STOP, 0);
  if (ret < 0)
    {
      printf("stop GNSS ERROR\n");
    }
  else
    {
      printf("stop GNSS OK\n");
    }

_err:

  /* GNSS firmware needs to disable the signal after positioning. */

  setting.enable = 0;
  ret = ioctl(fd, CXD56_GNSS_IOCTL_SIGNAL_SET, (unsigned long)&setting);
  if (ret < 0)
    {
      printf("signal error\n");
    }

  sigprocmask(SIG_UNBLOCK, &mask, NULL);

  /* Release GNSS file descriptor. */

  ret = close(fd);
  if (ret < 0)
    {
      printf("close error %d\n", errno);
    }

  printf("End of GNSS Sample:%d\n", ret);

  return ret;

}

#endif