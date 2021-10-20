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

/* Throw an exceptions whose message is a Java string. */
JPGF_INTERNAL_DECL void
throw_jstring_exception(JNIEnv *env, const char* class_name, jstring msg);

/* Throw an exceptions whose message is a C-style string. */
JPGF_INTERNAL_DECL void
throw_string_exception(JNIEnv *env, const char* class_name, const char* msg);

/* Handle `PGFError`s by throwing exceptions if necessary and returning the
 * error type. `PGF_EXN_NONE` means no error. */
JPGF_INTERNAL_DECL PgfExnType 
handleError(JNIEnv *env, PgfExn err);

/* String conversions */

/* Decode standard UTF-8 strings. Only used inside `pgftext2jstring`. */
JPGF_INTERNAL_DECL uint32_t
pgf_utf8_decode(const uint8_t** src_inout);

/* Convert a `PgfText` to a Java string. */
JPGF_INTERNAL_DECL jstring 
pgftext2jstring(JNIEnv *env, PgfText* s);

/* Convert a Java string tp a `PgfText`. */
JPGF_INTERNAL_DECL PgfText* 
jstring2pgftext(JNIEnv *env, jstring s);

/* Java shorthands */

/* Create an empty Java `List`. */
JPGF_INTERNAL_DECL jobject
new_jlist(JNIEnv *env);


/* Get the ID of the method to add to Java lists. */
JPGF_INTERNAL_DECL jmethodID
get_jlist_add_method(JNIEnv *env);

/* List conversions */

/* Convert an array of `PgfTypeHypo`s to a Java list of `Hypo`s . */
JPGF_INTERNAL jobject
pgf_type_hypos2j_hypo_list(JNIEnv *env, int n_hypos, PgfTypeHypo *hypos);

#endif
