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
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "rotateXYZ.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME(rotateXYZ);

/////////////////////////////////////////////////////////
//
// rotateXYZ
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
rotateXYZ :: rotateXYZ(int argc, t_atom *argv)
{
    if (argc == 3)
    {
        m_vector[0] = atom_getfloat(&argv[0]);
        m_vector[1] = atom_getfloat(&argv[1]);
        m_vector[2] = atom_getfloat(&argv[2]);
    }
    else if (argc == 0)
    {
        m_vector[0] = 0.f;
        m_vector[1] = 0.f;
        m_vector[2] = 0.f;
    }
    else
    {
      throw(GemException("needs 0 or 3 arguments"));
    }

    // create the new inlets
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("xVal"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("yVal"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("zVal"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
rotateXYZ :: ~rotateXYZ()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void rotateXYZ :: render(GemState *)
{
    glRotatef(m_vector[0], 1.f, 0.f, 0.f);
    glRotatef(m_vector[1], 0.f, 1.f, 0.f);
    glRotatef(m_vector[2], 0.f, 0.f, 1.f);
}

/////////////////////////////////////////////////////////
// xMess
//
/////////////////////////////////////////////////////////
void rotateXYZ :: xMess(float val)
{
    m_vector[0] = val;
    setModified();
}

/////////////////////////////////////////////////////////
// yMess
//
/////////////////////////////////////////////////////////
void rotateXYZ :: yMess(float val)
{
    m_vector[1] = val;
    setModified();
}

/////////////////////////////////////////////////////////
// zMess
//
/////////////////////////////////////////////////////////
void rotateXYZ :: zMess(float val)
{
    m_vector[2] = val;
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void rotateXYZ :: obj_setupCallback(t_class *classPtr)
{
  CPPEXTERN_MSG1(classPtr, "xVal", xMess, float);
  CPPEXTERN_MSG1(classPtr, "yVal", yMess, float);
  CPPEXTERN_MSG1(classPtr, "zVal", zMess, float);
}

