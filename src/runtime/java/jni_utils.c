#include <jni.h>
#include <gu/utf8.h>
#include <gu/string.h>
#include <pgf/pgf.h>
#include "jni_utils.h"
#ifndef __MINGW32__
#include <alloca.h>
#else
#include <malloc.h>
#endif

#define l2p(x) ((void*) (intptr_t) (x))
#define p2l(x) ((jlong) (intptr_t) (x))

JPGF_INTERNAL void*
get_db(JNIEnv *env, jobject self) {
	jfieldID dbId = (*env)->GetFieldID(env, (*env)->GetObjectClass(env, self), "db", "J");
	return l2p((*env)->GetLongField(env, self, dbId));
}

JPGF_INTERNAL void*
get_rev(JNIEnv *env, jobject self) {
	jfieldID revId = (*env)->GetFieldID(env, (*env)->GetObjectClass(env, self), "rev", "J");
	return l2p((*env)->GetLongField(env, self, revId));
}

JPGF_INTERNAL void
throw_jstring_exception(JNIEnv *env, const char* class_name, jstring msg)
{
	jclass exception_class = (*env)->FindClass(env, class_name);
	if (!exception_class)
		return;
	jmethodID constrId = (*env)->GetMethodID(env, exception_class, "<init>", "(Ljava/lang/String;)V");
	if (!constrId)
		return;
	jobject exception = (*env)->NewObject(env, exception_class, constrId, msg);
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

