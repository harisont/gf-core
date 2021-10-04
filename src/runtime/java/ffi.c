#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <alloca.h>

#include <jni.h>
#include "jni_utils.h"
#include <pgf/pgf.h>
#include "./ffi.h"

PGF_API uint32_t
pgf_utf8_decode(const uint8_t** src_inout)
{
	const uint8_t* src = *src_inout;
	uint8_t c = src[0];
	if (c < 0x80) {
		*src_inout = src + 1;
		return c;
	}
	size_t len = (c < 0xe0 ? 1 :
	              c < 0xf0 ? 2 :
	              c < 0xf8 ? 3 :
	              c < 0xfc ? 4 :
	                         5
	             );
	uint64_t mask = 0x0103070F1f7f;
	uint32_t u = c & (mask >> (len * 8));
	for (size_t i = 1; i <= len; i++) {
		c = src[i];
		u = u << 6 | (c & 0x3f);
	}
	*src_inout = &src[len + 1];
	return u;
}


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

JPGF_INTERNAL jstring 
pgf_text2jstring(JNIEnv *env, PgfText *s) 
{
	const char* utf8s = s->text;
    const char* utf8 = s->text;
    size_t len = s->size ;

	jchar* utf16 = alloca(len*sizeof(jchar));
	jchar* dst   = utf16;
	while (utf8s-utf8 < len) {
		uint32_t ucs = pgf_utf8_decode((const uint8_t**) &utf8s);

		if (ucs <= 0xFFFF) {
			*dst++ = ucs;
		} else {
			ucs -= 0x10000;
			*dst++ = 0xD800+((ucs >> 10) & 0x3FF);
			*dst++ = 0xDC00+(ucs & 0x3FF);
		}
	}

	return (*env)->NewString(env, utf16, dst-utf16);
}


JPGF_INTERNAL PgfText* 
jstring2pgf_text(JNIEnv *env, jstring s)
{
	const char* text = (*env)->GetStringUTFChars(env, s, 0);
	jsize size = (*env)->GetStringLength(env, s);
	PgfText *pgfText = (PgfText*)malloc(sizeof(PgfText));
	memcpy(pgfText->text, text, size);
	pgfText->size = (size_t)s;
	(*env)->ReleaseStringUTFChars(env, s, text);
	return pgfText;
}
