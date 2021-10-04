#include <math.h>
#include <stdbool.h>

#include <jni.h>
#include "jni_utils.h"
#include <pgf/pgf.h>
#include "./ffi.h"

JNIEXPORT PgfExnType handleError(JNIEnv *env, PgfExn err)
{
    if (err.type == PGF_EXN_SYSTEM_ERROR) {
        throw_string_exception(env, "java/io/IOException", err.msg);
    } else if (err.type == PGF_EXN_PGF_ERROR) {
        throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", err.msg);
    } 
    return err.type;
}