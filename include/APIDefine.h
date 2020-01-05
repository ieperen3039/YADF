//
// Created by s152717 on 4-1-2020.
//

#ifndef YADF_APIDEFINE_H
#define YADF_APIDEFINE_H

#ifdef YADF_STATIC
#  define YADF_API extern
#else
#  ifdef YADF_BUILD
#    define YADF_API extern __declspec(dllexport)
#  else
#    define YADF_API extern __declspec(dllimport)
#  endif
#endif

#endif //YADF_APIDEFINE_H
