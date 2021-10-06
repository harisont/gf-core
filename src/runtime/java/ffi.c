#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <alloca.h>

#include <jni.h>
#include "jni_utils.h"
#include <pgf/pgf.h>
#include "./ffi.h"

JNIEnv *env;

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

// marshalling/unmarshalling

static PgfExpr
eabs(PgfUnmarshaller *this, PgfBindType btype, PgfText *name, PgfExpr body)
{
	//TODO:
	return NULL;
}

static PgfExpr
eapp(PgfUnmarshaller *this, PgfExpr fun, PgfExpr arg)
{
    //TODO:
	return NULL;
}

static PgfExpr
elit(PgfUnmarshaller *this, PgfLiteral lit)
{
    //TODO:
	return NULL;
}

static PgfExpr
emeta(PgfUnmarshaller *this, PgfMetaId meta)
{
    //TODO:
	return NULL;
}

static PgfExpr
efun(PgfUnmarshaller *this, PgfText *name)
{
    //TODO:
	return NULL;
}

static PgfExpr
evar(PgfUnmarshaller *this, int index)
{
    //TODO:
	return NULL;
}

static PgfExpr
etyped(PgfUnmarshaller *this, PgfExpr expr, PgfType typ)
{
    //TODO:
	return NULL;
}

static PgfExpr
eimplarg(PgfUnmarshaller *this, PgfExpr expr)
{
    //TODO:
	return NULL;
}

static PgfLiteral
lint(PgfUnmarshaller *this, size_t size, uintmax_t *v)
{
    //TODO:
	return NULL;
}

static PgfLiteral
lflt(PgfUnmarshaller *this, double v)
{
    //TODO:
	return NULL;
}

static PgfLiteral
lstr(PgfUnmarshaller *this, PgfText *v)
{
    //TODO:
	return NULL;
}

JPGF_INTERNAL PgfType
dtyp(PgfUnmarshaller *this, int n_hypos, PgfTypeHypo *hypos, PgfText *cat, int n_exprs, PgfExpr *exprs)
{
	// construct empty list (TODO: handle failures?)
	jclass lClass = (*env)->FindClass(env, "java/util/ArrayList");
	jmethodID lConstr = (*env)->GetMethodID(env, lClass, "<init>", "()V");
	jobject lHypos = (*env)->NewObject(env, lClass, lConstr);

	// get id of the method to add elements to such list
	jmethodID lAdd = (*env)->GetMethodID(env, lClass, "add", "(Ljava/lang/Object;)Z");

	// get constructor id of Hypo class
	jclass hClass = (*env)->FindClass(env, "org/grammaticalframework/pgf/Hypo");
	jmethodID hConstr = (*env)->GetMethodID(env, hClass, "<init>", "(Ljava/lang/Object;J)V");		

	for (int i = 0; i < n_hypos; i++) {
		// get bindType, var and type from current Hypo
		jboolean bindType = (jboolean)hypos[i].bind_type; // will this cast work? lo scopriremo solo vivendo
		jstring var = pgf_text2jstring(env,hypos[i].cid); // and does this function actually work?
		jobject type = (jobject)hypos[i].type;

		// construct Hypo object
		jobject hObj = (*env)->NewObject(env, hClass, hConstr, bindType, var, type);

		// add it to the list of Hypos
		(*env)->CallBooleanMethod(env, lHypos, lAdd, hObj);
	}

	// get cat as jstring
	jstring cString = pgf_text2jstring(env,cat); 

	jobject lExprs = (*env)->NewObject(env, lClass, lConstr);
	// TODO: fill it with Exprs

	// construct a Java Type object
	jclass tClass = (*env)->FindClass(env, "org/grammaticalframework/pgf/Type");
	jmethodID tConstr = (*env)->GetMethodID(env, tClass, "<init>", "()V");
	jobject type = (*env)->NewObject(env, tClass, tConstr, lHypos, cString, lExprs);

	// return it casted to PgfType 
	return (PgfType) type ;
}

static void
free_ref(PgfUnmarshaller *this, object x)
{
    //TODO: 
}

static PgfUnmarshallerVtbl unmarshallerVtbl =
{
    eabs,
    eapp,
    elit,
    emeta,
    efun,
    evar,
    etyped,
    eimplarg,
    lint,
    lflt,
    lstr,
    dtyp,
    free_ref
};

PgfUnmarshaller unmarshaller = { &unmarshallerVtbl };