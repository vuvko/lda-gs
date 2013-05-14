#ifndef MUNKRES_GLOBAL_H
#define MUNKRES_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MUNKRES_LIBRARY)
#  define MUNKRESSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MUNKRESSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MUNKRES_GLOBAL_H
