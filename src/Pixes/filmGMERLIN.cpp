////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file 
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) G�nther Geiger.
//    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include <string.h>
#include "Pixes/filmGMERLIN.h"


/////////////////////////////////////////////////////////
//
// filmGMERLIN
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

filmGMERLIN :: filmGMERLIN(int format) : film(format)
#ifdef HAVE_GMERLIN
				       ,
					 m_file(NULL),
					 m_opt(NULL),
					 m_gformat(NULL),
					 m_track(0),
					 m_stream(0),
					 m_gframe(NULL)
#endif /* GMERLIN */
{
  static bool first_time=true;
  if (first_time) {
#ifdef HAVE_GMERLIN
    post("pix_film:: gmerlin support");
#endif
    first_time = false;
  }
#ifdef HAVE_GMERLIN
  m_file = bgav_create();
  m_opt = bgav_get_options(m_file);
  /*
  bgav_options_set_connect_timeout(m_opt,   connect_timeout);
  bgav_options_set_read_timeout(m_opt,      read_timeout);
  bgav_options_set_network_bandwidth(m_opt, network_bandwidth);
  */
  bgav_options_set_seek_subtitles(m_opt, 0);
  bgav_options_set_sample_accurate(m_opt, 1);

#endif
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
filmGMERLIN :: ~filmGMERLIN()
{
  close();
}

#ifdef HAVE_GMERLIN
void filmGMERLIN :: close(void)
{
  if(m_file)bgav_close(m_file);
  m_file=NULL;
}

/////////////////////////////////////////////////////////
// really open the file ! (OS dependent)
//
/////////////////////////////////////////////////////////
bool filmGMERLIN :: open(char *filename, int format)
{
  close();
  m_track=0;

  if(!strncmp(filename, "vcd://", 6))
    {
    if(!bgav_open_vcd(m_file, filename + 5))
      {
	error("Could not open VCD Device %s",
              filename + 5);
      return false;
      }
    }
  else if(!strncmp(filename, "dvd://", 6))
    {
    if(!bgav_open_dvd(m_file, filename + 5))
      {
	error("Could not open DVD Device %s",
              filename + 5);
      return false;
      }
    }
  else if(!strncmp(filename, "dvb://", 6))
    {
    if(!bgav_open_dvb(m_file, filename + 6))
      {
	error("Could not open DVB Device %s",
              filename + 6);
      return false;
      }
    }
  else if(!bgav_open(m_file, filename))
    {
      error("Could not open file %s",
            filename);
      close();
      return false;
    }

  if(bgav_is_redirector(m_file))
    {
      int i=0;
      int num_urls=bgav_redirector_get_num_urls(m_file);
      post("Found redirector:");
      for(i = 0; i < num_urls; i++)
      {
	post("#%d: '%s' -> %s", i, bgav_redirector_get_name(m_file, i), bgav_redirector_get_url(m_file, i));
      }
      if(true){
	filename=(char*)bgav_redirector_get_url(m_file, 0);
	close();
	return open(filename);
      }
    }

  /*
   * ok, we have been able to open the "file"
   * now get some information from it...
   */
  
  m_numTracks = bgav_num_tracks(m_file);
  // LATER: check whether this track has a video-stream...
  bgav_select_track(m_file, m_track);

  m_seekable=bgav_can_seek_sample(m_file);

  bgav_set_video_stream(m_file, m_stream, BGAV_STREAM_DECODE);
  if(!bgav_start(m_file)) {
    close();
    return false;
  }

  m_gformat = (gavl_video_format_t*)bgav_get_video_format (m_file, m_stream);
  m_gformat->pixelformat=GAVL_RGBA_32;
  m_gframe = gavl_video_frame_create_nopad(m_gformat);

  m_image.image.xsize=m_gformat->frame_width;
  m_image.image.ysize=m_gformat->frame_height;
  m_image.image.setCsizeByFormat(GL_RGBA);
  m_image.image.notowned=true;
  m_image.newfilm=true;

  return true;
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
pixBlock* filmGMERLIN :: getFrame(){
  if(!m_file)return NULL;

  bgav_read_video(m_file, m_gframe, m_stream);
  m_image.newimage=true;
  m_image.image.data=m_gframe->planes[0];
  return &m_image;
}

int filmGMERLIN :: changeImage(int imgNum, int trackNum){
  if(!m_file)return FILM_ERROR_FAILURE;
  if(imgNum>m_numFrames || imgNum<0)return FILM_ERROR_FAILURE;
  if  (imgNum>0)m_curFrame=imgNum;
  if(trackNum>0)m_curTrack=trackNum;

  int i=-1;
  //  if ((i=gmerlin_set_video_position(m_quickfile, m_curFrame, m_curTrack))){  }
  if(bgav_can_seek(m_file)) {
    int64_t seekpos = imgNum;
    bgav_seek_scaled(m_file, &seekpos, m_gformat->timescale);
    return FILM_ERROR_SUCCESS;
  }
  return FILM_ERROR_FAILURE;
}
#endif // GMERLIN