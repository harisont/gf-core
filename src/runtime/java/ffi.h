#include <jni.h>
#include "jni_utils.h"
#include <pgf/pgf.h>

JNIEXPORT PgfExnType handleError(JNIEnv *env, PgfExn err);