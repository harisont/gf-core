#include <stdlib.h>
#include <alloca.h>
#include <string.h>

#include <pgf/pgf.h>

#include <jni.h>
#include "jni_utils.h"


#define l2p(x) ((void*) (intptr_t) (x))
#define p2l(x) ((jlong) (intptr_t) (x))

/* Error handling */

JPGF_INTERNAL void
throw_jstring_exception(JNIEnv *env, const char* class_name, jstring msg)
{
	jclass exception_class = (*env)->FindClass(env, class_name);
	if (!exception_class)
		return;
	jmethodID cid = (*env)->GetMethodID(env, exception_class, "<init>", "(Ljava/lang/String;)V");
	if (!cid)
		return;
	jobject exception = (*env)->NewObject(env, exception_class, cid, msg);
	if (!exception)
		return;
	(*env)->Throw(env, exception);
}

JPGF_INTERNAL void
throw_string_exception(JNIEnv *env, const char* class_name, const char* msg)
{
	jstring jmsg = (*env)->NewStringUTF(env, msg);
	if (!jmsg)
		return;
	throw_jstring_exception(env, class_name, jmsg);
}

JPGF_INTERNAL PgfExnType 
handleError(JNIEnv *env, PgfExn err)
{
    if (err.type == PGF_EXN_SYSTEM_ERROR) {
        throw_string_exception(env, "java/io/IOException", err.msg);
    } else if (err.type == PGF_EXN_PGF_ERROR) {
        throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", err.msg);
    } 
    return err.type;
}

/* String conversions */

JPGF_INTERNAL uint32_t
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

JPGF_INTERNAL jstring 
pgftext2jstring(JNIEnv *env, PgfText *s) 
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
jstring2pgftext(JNIEnv *env, jstring s)
{
	const char* text = (*env)->GetStringUTFChars(env, s, 0);
	jsize size = (*env)->GetStringUTFLength(env, s);
	PgfText *pgfText = (PgfText*)malloc(sizeof(PgfText)+size+1);
	memcpy(pgfText->text, text, size);
	pgfText->size = (size_t)size;
	(*env)->ReleaseStringUTFChars(env, s, text);
	return pgfText;
}

/* Java shorthands */

JPGF_INTERNAL jobject
new_jlist(JNIEnv *env)
{
	jclass lcls = (*env)->FindClass(env, "java/util/ArrayList");
	if (!lcls)
		return NULL;
	jmethodID cid = (*env)->GetMethodID(env, lcls, "<init>", "()V");
	if (!cid)
		return NULL;
	jobject list = (*env)->NewObject(env, lcls, cid);
	if (!list)
		return NULL;
	return list;
}

JPGF_INTERNAL jmethodID
get_jlist_add_method(JNIEnv *env)
{
	jclass lcls = (*env)->FindClass(env, "java/util/ArrayList");
	jmethodID add_id = (*env)->GetMethodID(env, lcls, "add", "(Ljava/lang/Object;)Z");
	if (!add_id)
		return NULL;
	return add_id;
}

/* List conversions */

JPGF_INTERNAL jobject
pgf_type_hypos2j_hypo_list(JNIEnv *env, size_t n_hypos, PgfTypeHypo *phypos)
{
	jobject jhypos = new_jlist(env);
	jmethodID add_id = get_jlist_add_method(env);

	jclass hcls = (*env)->FindClass(env, "org/grammaticalframework/pgf/Hypo");
	jmethodID cid = (*env)->GetMethodID(env, hcls, "<init>", "(ZLjava/lang/String;Lorg/grammaticalframework/pgf/Type;)V");		

	for (size_t i = 0; i < n_hypos; i++) {
		// get bindType, var and type from current Hypo
		jboolean bindType = phypos[i].bind_type == 0 ? JNI_TRUE : JNI_FALSE;
		jstring var = pgftext2jstring(env,phypos[i].cid);
		jobject type = (jobject)phypos[i].type;

		jobject hObj = (*env)->NewObject(env, hcls, cid, bindType, var, type);

		(*env)->CallBooleanMethod(env, jhypos, add_id, hObj);
	}

	return jhypos;
}

JPGF_INTERNAL_DECL PgfTypeHypo*
j_hypo_list2pgf_type_hypos(JNIEnv *env, jsize n_hypos, jobject hypos)
{	
	// find Hypo class
	jclass hcls = (*env)->FindClass(env, "org/grammaticalframework/pgf/Hypo");

	// allocate array of PgfTypeHypos 
	PgfTypeHypo *phypos = malloc(sizeof(PgfTypeHypo) * n_hypos);

	for (jsize i = 0; i < n_hypos; i++) {
		// get current Hypo
		jobject h = (*env)->GetObjectArrayElement(env, hypos, i);

		// h.bindType
		jfieldID bid = (*env)->GetFieldID(env, hcls, "bindType", "Z");
		jboolean bt = (*env)->GetBooleanField(env, h, bid);
		phypos[i].bind_type = bt == JNI_TRUE ? PGF_BIND_TYPE_EXPLICIT : PGF_BIND_TYPE_IMPLICIT;

		// h.var
		jfieldID vid = (*env)->GetFieldID(env, hcls, "var", "Ljava/lang/String");
		jobject v = (*env)->GetObjectField(env, h, vid);
		phypos[i].cid = jstring2pgftext(env, v);

		// h.type
		jfieldID tid = (*env)->GetFieldID(env, hcls, "type", "Lorg/grammaticalframework/pgf/Type");
		jobject t = (*env)->GetObjectField(env, h, tid);
		phypos[i].type = (PgfType)t;
	}

	return phypos;
}