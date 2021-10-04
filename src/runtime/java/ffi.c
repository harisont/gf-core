#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include <jni.h>
#include "jni_utils.h"
#include <pgf/pgf.h>
#include "./ffi.h"


// error handling

JNIEXPORT PgfExnType handleError(JNIEnv *env, PgfExn err)
{
    if (err.type == PGF_EXN_SYSTEM_ERROR) {
        throw_string_exception(env, "java/io/IOException", err.msg);
    } else if (err.type == PGF_EXN_PGF_ERROR) {
        throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", err.msg);
    } 
    return err.type;
}

// string conversions

JPGF_INTERNAL jstring p2j_string(JNIEnv *env, PgfText *s) {
    const char* utf8 = s->text;
    // size_t len = s->size ;

	// TODO: convert to UTF-16 and use NewString(env, utf16, size) instead
    return (*env)->NewStringUTF(env, utf8);
}
