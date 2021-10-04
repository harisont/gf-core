#include <jni.h>
#include "jni_utils.h"
#include <pgf/pgf.h>

// error handling

JNIEXPORT PgfExnType handleError(JNIEnv *env, PgfExn err);

// string conversions
JPGF_INTERNAL_DECL jstring 
p2j_string(JNIEnv *env, PgfText* s);

JPGF_INTERNAL_DECL PgfText* 
j2p_string(JNIEnv *env, jstring s);

// marshalling/unmarshalling
//PgfUnmarshaller unmarshaller;
//PgfMarshaller marshaller;