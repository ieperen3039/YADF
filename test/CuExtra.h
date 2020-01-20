//
// Created by s152717 on 16-1-2020.
//

#ifndef YADF_CUEXTRA_H
#define YADF_CUEXTRA_H

#include "Structs.h"

#define CuAssertVec3iEquals(tc, ex, ac) CuAssertVec3iEquals_LineMsg((tc),__FILE__,__LINE__,NULL,(ex),(ac))

bool vectori_equals(Vector3ic* expected, Vector3ic* actual) { return (*expected).x == (*actual).x && (*expected).y == (*actual).y && (*expected).z == (*actual).z; }

void CuAssertVec3iEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message, Vector3i expected,
                                 Vector3i actual) {
    char buf[STRING_MAX];
    if (vectori_equals(&expected, &actual)) return;
    sprintf(buf,
            "expected (%d, %d, %d) but was (%d, %d, %d)",
            expected.x, expected.y, expected.z, actual.x, actual.y, actual.z
    );
    CuFail_Line(tc, file, line, message, buf);
}

#endif //YADF_CUEXTRA_H
