#ifndef G2O_CONFIG_H
#define G2O_CONFIG_H

//#define G2O_HAVE_OPENGL 1
//#define G2O_OPENGL_FOUND 1
/* #undef G2O_OPENMP */
//#define G2O_SHARED_LIBS 1
//#define G2O_LGPL_SHARED_LIBS 1

// available sparse matrix libraries
/* #undef G2O_HAVE_CHOLMOD */
#define G2O_HAVE_CSPARSE 1

#define G2O_NO_IMPLICIT_OWNERSHIP_OF_OBJECTS

#ifdef G2O_NO_IMPLICIT_OWNERSHIP_OF_OBJECTS
#define G2O_DELETE_IMPLICITLY_OWNED_OBJECTS 0
#else
#define G2O_DELETE_IMPLICITLY_OWNED_OBJECTS 1
#endif

#define G2O_CXX_COMPILER "MSVC cl"

#ifdef __cplusplus
#include <g2o/core/eigen_types.h>
#endif

#endif

