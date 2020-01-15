//
// Created by s152717 on 17-12-2019.
//

#include "CuTest.h"
#include <stdio.h>

#define TYPE long
TYPE elements[] = {00, 11, 22, 33, 44, 55, 66, 77, 88, 99};

#include "testList.h"
#include "testMap.h"
#include "testWorld.h"

int main(int argc, char** argv) {
    CuString* output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    CuSuiteAddSuite(suite, list_suite());
    CuSuiteAddSuite(suite, map_suite());
    CuSuiteAddSuite(suite, world_suite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);

    list_cleanup();
    map_cleanup();
}