#include <stdio.h>
#include "file2.h"
static void fun() {
    printf("fun\n");
}

void pr() {
    printf("pr\n");
    fun();
}