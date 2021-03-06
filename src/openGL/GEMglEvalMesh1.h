 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2002-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  *	zmoelnig@iem.kug.ac.at
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  *
  *  this file has been generated...
  * ------------------------------------------------------------------
  */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLEVALMESH__H_
#define _INCLUDE__GEM_OPENGL_GEMGLEVALMESH__H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglEvalMesh1
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glEvalMesh1( GLenum mode, GLint i1, GLint i2)"
 */

class GEM_EXTERN GEMglEvalMesh1 : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglEvalMesh1, GemGLBase);

	public:
	  // Constructor
	  GEMglEvalMesh1 (t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglEvalMesh1 ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	mode;		// VAR
	  virtual void	modeMess(t_float);	// FUN

	  GLint	i1;		// VAR
	  virtual void	i1Mess(t_float);	// FUN

	  GLint	i2;		// VAR
	  virtual void	i2Mess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[3];

	// static member functions
	  static void	 modeMessCallback (void*, t_float);
	  static void	 i1MessCallback (void*, t_float);
	  static void	 i2MessCallback (void*, t_float);
};
#endif // for header file
