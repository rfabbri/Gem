 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2005 James Tittle. tigital @ mac.com
  *  Copyright (c) 2002 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM
  *	zmoelnig@iem.kug.ac.at
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  *
  * ------------------------------------------------------------------
  */

#ifndef INCLUDE_GEM_GLUNIFORM1FARB_H_
#define INCLUDE_GEM_GLUNIFORM1FARB_H_

#include "Base/GemGLUtil.h"
#include "Base/GemBase.h"

/*
 CLASS
	GEMglUniform1fARB
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glUniform1fARB( GLint location, GLfloat val)"
 */

class GEM_EXTERN GEMglUniform1fARB : public GemBase
{
	CPPEXTERN_HEADER(GEMglUniform1fARB, GemBase)

	public:
	  // Constructor
	  GEMglUniform1fARB (t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglUniform1fARB ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLint	location;		// VAR
	  virtual void	locMess(t_float);	// FUN

	  GLfloat	val;		// VAR
	  virtual void	valMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[2];

	// static member functions
	  static void	 locMessCallback (void*, t_floatarg);
	  static void	 valMessCallback (void*, t_floatarg);
};
#endif // for header file
