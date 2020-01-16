//
// Created by s152717 on 4-1-2020.
//

#ifndef YADF_APIDEFINE_H
#define YADF_APIDEFINE_H

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef YADF_STATIC
#   define YADF_API extern
#else
#   if defined(__WINDOWS__)
#       ifdef YADF_BUILD
#           define YADF_API extern __declspec(dllexport)
#       else
#           define YADF_API extern __declspec(dllimport)
#       endif
#   else
#       define YADF_API extern
#   endif
#endif

#endif //YADF_APIDEFINE_H
