#ifndef JNI_UTILS
#define JNI_UTILS

#if defined(_MSC_VER)

#define JPGF_INTERNAL_DECL
#define JPGF_INTERNAL

#else

#define JPGF_INTERNAL_DECL  __attribute__ ((visibility ("hidden")))
#define JPGF_INTERNAL       __attribute__ ((visibility ("hidden")))

#endif


#define l2p(x) ((void*) (intptr_t) (x))
#define p2l(x) ((jlong) (intptr_t) (x))

/* Error handling */

JPGF_INTERNAL_DECL PgfExnType 
handleError(JNIEnv *env, PgfExn err);

/* String conversions */

JPGF_INTERNAL_DECL uint32_t
pgf_utf8_decode(const uint8_t** src_inout);

JPGF_INTERNAL_DECL jstring 
pgftext2jstring(JNIEnv *env, PgfText* s);

JPGF_INTERNAL_DECL PgfText* 
jstring2pgftext(JNIEnv *env, jstring s);



JPGF_INTERNAL_DECL void*
get_db(JNIEnv *env, jobject self);

JPGF_INTERNAL_DECL void*
get_rev(JNIEnv *env, jobject self);

JPGF_INTERNAL_DECL void
throw_jstring_exception(JNIEnv *env, const char* class_name, jstring msg);

JPGF_INTERNAL_DECL void
throw_string_exception(JNIEnv *env, const char* class_name, const char* msg);

#endif
