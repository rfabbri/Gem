////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "Base/GemConfig.h"

#include "GemPixImageSave.h"
#include "m_pd.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif /* WIN32 */

// for the various GL colorspaces
#include "Base/GemGL.h"

#if defined __APPLE__ && !defined __x86_64__
// with OSX10.6, apple has removed loads of Carbon functionality (in 64bit mode)
// LATER make this a real check in configure
# define HAVE_CARBONQUICKTIME
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "GemPixUtil.h"


#include "plugins/imagesaver.h"
namespace gem {
  class PixImageSaver : public gem::plugins::imagesaver {
  private:
    static PixImageSaver*s_instance;
    std::vector<gem::plugins::imagesaver*>m_savers;
    std::vector<std::string>m_ids;

    PixImageSaver(void) {
      gem::PluginFactory<gem::plugins::imagesaver>::loadPlugins("image");
      std::vector<std::string>available_ids=gem::PluginFactory<gem::plugins::imagesaver>::getIDs();
      if(available_ids.size()>0) {
	startpost("Image saving support:");
	int i;
	for(i=0; i<available_ids.size(); i++) {
	  startpost(" %s", available_ids[i].c_str());
	}
	endpost();
      }

      addSaver(available_ids, "magick");
      addSaver(available_ids);
    }
    bool addSaver( std::vector<std::string>available, std::string ID=std::string(""))
    {
      int i=0;
      int count=0;

      std::vector<std::string>id;
      if(!ID.empty()) {
	// if requested 'cid' is in 'available' add it to the list of 'id's
	if(std::find(available.begin(), available.end(), ID)!=available.end()) {
	  id.push_back(ID);
	} else {
	  // request for an unavailable ID
	  verbose(2, "backend '%s' unavailable", ID.c_str());
	  return false;
	}
      } else {
	// no 'ID' given: add all available IDs
	id=available;
      }

      for(i=0; i<id.size(); i++) {
	std::string key=id[i];
	verbose(2, "trying to add '%s' as backend", key.c_str());
	if(std::find(m_ids.begin(), m_ids.end(), key)==m_ids.end()) {
	  // not yet added, do so now!
	  gem::plugins::imagesaver*saver=
	    gem::PluginFactory<gem::plugins::imagesaver>::getInstance(key); 
	  if(NULL==saver)break;
	  m_ids.push_back(key);
	  m_savers.push_back(saver);
	  count++;
	  verbose(2, "added backend#%d '%s' @ 0x%x", m_savers.size()-1, key.c_str(), saver);
	}
      }
      return (count>0);
    }

  public:
    virtual ~PixImageSaver(void) {
      int i;
      for(i=0; i<m_savers.size(); i++) {
	delete m_savers[i];
	m_savers[i]=NULL;
      }
    }

    virtual bool save(std::string filename, const imageStruct&result) {
      int i;
      for(i=0; i<m_savers.size(); i++) {
	verbose(2, "trying saver[%d]=%s", i, m_ids[i].c_str());
	if(m_savers[i]->save(filename, result))
	  return true;
      }
      return false;
    }

    static PixImageSaver*getInstance(void) {
      if(NULL==s_instance) {
	s_instance=new PixImageSaver();
      }
      return s_instance;
    }

}; };

gem::PixImageSaver*gem::PixImageSaver::s_instance=NULL;





#ifdef HAVE_CARBONQUICKTIME
# include <Carbon/Carbon.h>
# include <QuickTime/QuickTime.h>
# include <QuickTime/ImageCompression.h>
# include <string.h>
# include <fcntl.h> 
int mem2QuickTimeImage(imageStruct* image, const char *filename, int type);
#endif /* HAVE_CARBONQUICKTIME */

/***************************************************************************
 *
 * mem2image - Save an image to a file
 *
 ***************************************************************************/
GEM_EXTERN int mem2image(imageStruct* image, const char *filename, const int type)
{
   gem::PixImageSaver*piximagesaver=gem::PixImageSaver::getInstance();
   if(piximagesaver) {
     std::string fname=filename;
     if(piximagesaver->save(filename, *image)) {
       return (1);
     }
   }
   return (0);

  switch (type) {
  case 0:
#ifdef HAVE_CARBONQUICKTIME
    if (mem2QuickTimeImage(image, filename, 0)) return(1);else
#endif
    break;
  default:
#ifdef HAVE_CARBONQUICKTIME
    if (mem2QuickTimeImage(image, filename, (type==1))) return(1);else
#endif
     break;
  }

  // unable to save image
  error("GEM: Unable to save image to '%s'", filename);
  return(0);
}

#ifdef HAVE_CARBONQUICKTIME
extern OSStatus FSPathMakeFSSpec(const UInt8 *path,FSSpec *spec,Boolean *isDirectory);
       
void GetAppFSSpec(FSSpec *fileSpec)
{
    FSRef processRef;
    FSCatalogInfo processInfo;
    ProcessSerialNumber psn = {0, kCurrentProcess};
    GetProcessBundleLocation(&psn, &processRef);
    FSGetCatalogInfo(&processRef, kFSCatInfoNodeFlags, &processInfo, NULL, fileSpec, NULL);
}

// MemAlloc
// Simple memory allocation wrapper
void* MemAlloc(unsigned long memsize)
{
    if(memsize <= 0)
	return NULL;
    else
	return (malloc(memsize));
}

unsigned char* CStringToPString(char *string)
{
    unsigned char *newString = (unsigned char*)MemAlloc(strlen(string) + 1);
    size_t i = 0;
    
    for(i = 0; i < strlen(string); i++)
	newString[i+1] = string[i];
    
    newString[0] = i;
    
    return newString;
}


void InvertGLImage( unsigned char *imageData, unsigned char * outData, long imageSize, long rowBytes )
{
    long i, j;
    // This is not an optimized routine!

    // FIXXME use a flip function in GemPixUtils for this

    // Copy rows into tmp buffer one at a time, reversing their order
    for (i = 0, j = imageSize - rowBytes; i < imageSize; i += rowBytes, j -= rowBytes) {
      memcpy( &outData[j], &imageData[i], static_cast<size_t>(rowBytes) );
    }
}

int mem2QuickTimeImage(imageStruct* image, const char *filename, const int type)
{
    OSErr			err;
    ComponentResult		cErr 	= 0;
    
    GWorldPtr 			img	= NULL;
    GraphicsExportComponent 	geComp 	= NULL;
    Rect			r;
    
    OSType			osFileType; 	//= kQTFileTypeJPEG; //kQTFileTypeTIFF fot Tiff kQTFileTypeSGIImage for sgi
    FSSpec			spec;
    FSRef			ref;

    unsigned char *data = NULL;

    const UInt8*filename8=reinterpret_cast<const UInt8*>(filename);


    if(!image->upsidedown) { // the image is openGL-oriented, not quicktime-oriented! flip it!
      int rowBytes = image->xsize * image->csize;
      int imageSize = image->ysize * rowBytes;

      data = new unsigned char[imageSize];
      
      InvertGLImage(image->data, data, imageSize, rowBytes);
    }

    switch (type){

        case 0:
            osFileType 	=kQTFileTypeTIFF;
            break;
        case 1:
            osFileType 	= kQTFileTypeJPEG;
            break;
        default:
           osFileType 	=kQTFileTypeTIFF;
            break;
    }
    
    err = ::FSPathMakeRef(filename8, &ref, NULL );

    if (err == fnfErr) {
      // if the file does not yet exist, then let's create the file
      int fd;
      fd = open(filename, O_CREAT | O_RDWR, 0600);
      if (fd < 0)
	return 0;
      write(fd, " ", 1);
      close(fd);
      err = FSPathMakeRef(filename8, &ref, NULL);
    }
    
    if (err != noErr)
    {
        error("ERROR: %d in FSPathMakeRef()", err);
    }

    err = ::FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags, NULL, NULL, &spec, NULL);

    if (err != noErr)
    {
        error("ERROR: %d in FSGetCatalogInfo()", err);
    }

  //  err = FSMakeFSSpec(spec.vRefNum, 0, filename, &spec); //spits out -37 error but still works?
    err = FSMakeFSSpec(spec.vRefNum, spec.parID, filename8, &spec);  //this always gives an error -37 ???

    if (err != noErr && err != -37)
    {
        error("ERROR: %d in FSMakeFSSpec()", err);
    }
        
    err = OpenADefaultComponent(GraphicsExporterComponentType, osFileType, &geComp);
    if (err != noErr)
    {
	error("ERROR: %d in OpenADefaultComponent()", err);
	return 0; // FIXME:
    }
    
    r.top = 0;
    r.left = 0;
    r.bottom = image->ysize;
    r.right = image->xsize;
    
   // ::OffsetRect(&r, -r.left, -r.top);
   //SetRect(&r,r.left,r.bottom,r.right,r.top);
     err = QTNewGWorldFromPtr(&img,  
			      //k32RGBAPixelFormat,
			      k32ARGBPixelFormat,
			      &r, NULL, NULL, 0,
			      // keepLocal,	
			      //useDistantHdwrMem, 
			      (data?data:image->data),
			      static_cast<long>(image->xsize * image->csize));

     // is this the right place to free the "data" buffer (if used)?
     // i don't know, whether quicktime still needs the buffer...
                
    if (err != noErr)
    {
      error("ERROR: %d in QTNewGWorldFromPtr()", err);
      if(data)delete[]data;
      return 0; // FIXME:
    }
    
    
    // Set the input GWorld for the exporter
	cErr = GraphicsExportSetInputGWorld(geComp, img);
	if (cErr != noErr)
	{
	    error("ERROR: %d in GraphicsExportSetInputGWorld()", cErr);
      if(data)delete[]data;
	    return 0; // FIXME:
	}
    
    // Set the output file to our FSSpec
	cErr = GraphicsExportSetOutputFile(geComp, &spec);
	if (cErr != noErr)
	{
	    error("ERROR: %i in GraphicsExportSetOutputFile()", cErr);
      if(data)delete[]data;
	    return 0; // FIXME:
	}
        
        // Set the compression quality (needed for JPEG, not necessarily for other formats)
        if (osFileType 	== kQTFileTypeJPEG){
         //   cErr = GraphicsExportSetCompressionQuality(geComp, codecLosslessQuality);
            cErr = GraphicsExportSetCompressionQuality(geComp,  codecHighQuality);

            if (cErr != noErr)
            {
                error("ERROR: %i in GraphicsExportSetCompressionQuality()", cErr);
                if(data)delete[]data;
                return 0; // FIXME:
            }
         }
        
        // Export it
	cErr = GraphicsExportDoExport(geComp, NULL);
	if (cErr != noErr)
	{
	    error("ERROR: %i in GraphicsExportDoExport()", cErr);
      if(data)delete[]data;
	    return 0; // FIXME:
	}
        
  // finally, close the component
  if (geComp != NULL)
    CloseComponent(geComp);

  if(data)delete[]data;

  return 1;
}
#endif /* HAVE_CARBONQUICKTIME */
