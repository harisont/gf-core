#include <stdlib.h>
#include <string.h>

#include <math.h>

#include <pgf/pgf.h>

#include <jni.h>
#include "jni_utils.h"


static JavaVM* cachedJVM;
static JNIEnv *env;

typedef struct {
	PgfItor fn;
	JNIEnv *env;
	jobject grammar;
	jobject object;
	jmethodID method_id;
} JPGFClosure;

//////////////////////////////////////////////////////////////////////////////
// Marshalling and unmarshalling

static PgfExpr
eabs(PgfUnmarshaller *this, PgfBindType btype, PgfText *name, PgfExpr body)
{
	//TODO:
}

static PgfExpr
eapp(PgfUnmarshaller *this, PgfExpr fun, PgfExpr arg)
{
    //TODO:
}

static PgfExpr
elit(PgfUnmarshaller *this, PgfLiteral lit)
{
    //TODO:
}

static PgfExpr
emeta(PgfUnmarshaller *this, PgfMetaId meta)
{
    //TODO:
}

static PgfExpr
efun(PgfUnmarshaller *this, PgfText *name)
{
    //TODO:
}

static PgfExpr
evar(PgfUnmarshaller *this, int index)
{
    //TODO:
}

static PgfExpr
etyped(PgfUnmarshaller *this, PgfExpr expr, PgfType typ)
{
    //TODO:
}

static PgfExpr
eimplarg(PgfUnmarshaller *this, PgfExpr expr)
{
    //TODO:
}

static PgfLiteral
lint(PgfUnmarshaller *this, size_t size, uintmax_t *v)
{
    //TODO:
}

static PgfLiteral
lflt(PgfUnmarshaller *this, double v)
{
    //TODO:
}

static PgfLiteral
lstr(PgfUnmarshaller *this, PgfText *v)
{
    //TODO:
}

JPGF_INTERNAL PgfType
dtyp(PgfUnmarshaller *this, int n_hypos, PgfTypeHypo *phypos, PgfText *pcat, int n_exprs, PgfExpr *pexprs)
{	
	JNIEnv *env;
    (*cachedJVM)->AttachCurrentThread(cachedJVM, (void **)&env, NULL);

	jobject jhypos = pgf_type_hypos2j_hypo_list(env, n_hypos, phypos);
	jstring cname = pgftext2jstring(env, pcat); 
	jobject jexprs = new_jlist(env); // TODO: fill it with Exprs

	jclass tcls = (*env)->FindClass(env, "org/grammaticalframework/pgf/Type");
	jmethodID cid = (*env)->GetMethodID(env, tcls, "<init>", "([Lorg/grammaticalframework/pgf/Hypo;Ljava/lang/String;[Lorg/grammaticalframework/pgf/Expr;)V");
	jobject t = (*env)->NewObject(env, tcls, cid, jhypos, cname, jexprs);

	return (PgfType)t ;
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

static object
match_lit(PgfMarshaller *this, PgfUnmarshaller *u, PgfLiteral lit)
{
    // TODO:
}

static object
match_expr(PgfMarshaller *this, PgfUnmarshaller *u, PgfExpr expr)
{
    // TODO:
}

static object
match_type(PgfMarshaller *this, PgfUnmarshaller *u, PgfType ty)
{
	// convert PgfType to java Type
    jobject t = (jobject)ty;

	// get corresponding class
	jclass tcls = (*env)->GetObjectClass(env, t); 

	// convert Type.hypos (:: Hypo[]) to PgfTypeHypo*
	jfieldID hid = (*env)->GetFieldID(env, tcls, "hypos", "[Lorg/grammaticalframework/pgf/Hypo");
	jobject hypos = (*env)->GetObjectField(env, t, hid);
	jsize n_hypos = (*env)->GetArrayLength(env,hypos);
	PgfTypeHypo* phypos = j_hypo_list2pgf_type_hypos(env, n_hypos, hypos);
	if (phypos == NULL) {
        return 0;
    }

	// convert Type.cat (:: String) to C-style string
	jfieldID cid = (*env)->GetFieldID(env, tcls, "cat", "Ljava/lang/String");
	PgfText* pcat = jstring2pgftext(env, (*env)->GetObjectField(env, t, cid));
	if (pcat == NULL) {
        return 0;
    }

	// convert Type.exprs (:: Expr[]) to PgfExpr[]
	jfieldID eid = (*env)->GetFieldID(env, tcls, "exprs", "[Lorg/grammaticalframework/pgf/Hypo");
	jobject exprs = (*env)->GetObjectField(env, t, eid);
	jsize n_exprs = (*env)->GetArrayLength(env,exprs); 
	PgfExpr pexprs[n_exprs];
	for (jsize i = 0; i < n_exprs; i++) {
		pexprs[i] = (PgfExpr)(*env)->GetObjectArrayElement(env, exprs, n_exprs);
	}

	object res = u->vtbl->dtyp(u, n_hypos, phypos, pcat, n_exprs, pexprs);

	// free phypos
	for (jsize i = 0; i < n_hypos; i++) {
		free(phypos[i].cid);
		//TODO: free(phypos[i].type);
	}
	free(phypos);

	// free pcat
	free(pcat);

	// TODO: free pexprs
	//for (jsize i = 0; i < n_exprs; i++) {
    //    free(pexprs[i]);
	//}

	return res;
}

static PgfMarshallerVtbl marshallerVtbl =
{
    match_lit,
    match_expr,
    match_type
};

PgfMarshaller marshaller = { &marshallerVtbl };

static void
pgf_collect_names(PgfItor* fn, PgfText* key, void* value, PgfExn* err)
{
	PgfText *name = key;
    JPGFClosure* clo = (JPGFClosure*) fn;

	jstring jname = pgftext2jstring(clo->env, name);

	(*clo->env)->CallBooleanMethod(clo->env, clo->object, clo->method_id, jname);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    cachedJVM = jvm;
    return JNI_VERSION_1_6;
}

////////////////////////////////////////////////////////////////////////////// 
// Native methods

/* PGF */

// PGF "getters"

JPGF_INTERNAL void*
get_db(JNIEnv *env, jobject self) {
	jfieldID db_id = (*env)->GetFieldID(env, (*env)->GetObjectClass(env, self), "db", "J");
	return l2p((*env)->GetLongField(env, self, db_id));
}

JPGF_INTERNAL void*
get_rev(JNIEnv *env, jobject self) {
	jfieldID rev_id = (*env)->GetFieldID(env, (*env)->GetObjectClass(env, self), "rev", "J");
	return l2p((*env)->GetLongField(env, self, rev_id));
}

JNIEXPORT jobject JNICALL 
Java_org_grammaticalframework_pgf_PGF_readPGF__Ljava_lang_String_2(JNIEnv *env, jclass cls, jstring s)
{ 	
	long rev = 0;
	PgfExn err;

	const char *fpath = (*env)->GetStringUTFChars(env, s, 0);
	PgfDB* db = pgf_read_pgf(fpath, &rev, &err);
	(*env)->ReleaseStringUTFChars(env, s, fpath);

	if (handleError(env,err) == PGF_EXN_NONE) {
		jmethodID cid = (*env)->GetMethodID(env, cls, "<init>", "(JJ)V");
		return (*env)->NewObject(env, cls, cid, db, rev);
	} else {
		jmethodID fid = (*env)->GetMethodID(env, cls, "finalize", "()V");
		(*env)->CallVoidMethod(env, cls, fid);
		return NULL;
	}
}

JNIEXPORT jstring JNICALL
Java_org_grammaticalframework_pgf_PGF_bootNGF(JNIEnv* env, jclass cls, jstring p, jstring n)
{
	long rev = 0;
	PgfExn err;

	const char *ppath = (*env)->GetStringUTFChars(env, p, 0);
	const char *npath = (*env)->GetStringUTFChars(env, n, 0);
	PgfDB* db = pgf_boot_ngf(ppath, npath, &rev, &err);

	if (handleError(env,err) == PGF_EXN_NONE) {
		jmethodID cid = (*env)->GetMethodID(env, cls, "<init>", "(JJ)V");
		return (*env)->NewObject(env, cls, cid, db, rev);
	} else {
		jmethodID fid = (*env)->GetMethodID(env, cls, "finalize", "()V");
		(*env)->CallVoidMethod(env, cls, fid);
		return NULL;
	}
}

JNIEXPORT jobject JNICALL 
Java_org_grammaticalframework_pgf_PGF_readNGF(JNIEnv *env, jclass cls, jstring s)
{ 	
	long rev = 0;
	PgfExn err;

	const char *fpath = (*env)->GetStringUTFChars(env, s, 0);
	PgfDB* db = pgf_read_ngf(fpath, &rev, &err);
	(*env)->ReleaseStringUTFChars(env, s, fpath);

	if (handleError(env,err) == PGF_EXN_NONE) {
		jmethodID cid = (*env)->GetMethodID(env, cls, "<init>", "(JJ)V");
		return (*env)->NewObject(env, cls, cid, db, rev);
	} else {
		jmethodID fid = (*env)->GetMethodID(env, cls, "finalize", "()V");
		(*env)->CallVoidMethod(env, cls, fid);
		return NULL;
	}
}

JNIEXPORT jobject JNICALL 
Java_org_grammaticalframework_pgf_PGF_newNGF__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jclass cls, jstring n, jstring p)
{
	long rev = 0;
	PgfExn err;
	const char *fpath;
	
	if (p != NULL) {
		fpath = (*env)->GetStringUTFChars(env, p, 0);
	} else {
		fpath = NULL;
	}

	PgfText *anamePGF = jstring2pgftext(env,n);
	PgfDB* db = pgf_new_ngf(anamePGF, fpath, &rev, &err);

	if (p != NULL) {
		(*env)->ReleaseStringUTFChars(env, p, fpath);
	}

	if (handleError(env,err) == PGF_EXN_NONE) {
		jmethodID cid = (*env)->GetMethodID(env, cls, "<init>", "(JJ)V");
		return (*env)->NewObject(env, cls, cid, db, rev);
	} else {
		jmethodID fid = (*env)->GetMethodID(env, cls, "finalize", "()V");
		(*env)->CallVoidMethod(env, cls, fid);
		return NULL;
	}
}

JNIEXPORT void JNICALL
Java_org_grammaticalframework_pgf_PGF_writePGF(JNIEnv* env, jobject self, jstring p)
{
	PgfExn err;

	const char *fpath = (*env)->GetStringUTFChars(env, p, 0);
	pgf_write_pgf(fpath, get_db(env, self),(long)get_rev(env, self), &err);

    handleError(env,err);
}

JNIEXPORT jstring JNICALL 
Java_org_grammaticalframework_pgf_PGF_getAbstractName(JNIEnv* env, jobject self)
{
	PgfExn err;

	PgfText* txt = pgf_abstract_name(get_db(env, self),(long)get_rev(env, self),&err);

	if (err.type != PGF_EXN_NONE) {
		jclass cls = (*env)->GetObjectClass(env, self);
		jmethodID fid = (*env)->GetMethodID(env, cls, "finalize", "()V");
		(*env)->CallVoidMethod(env, cls, fid);
		return NULL;
	}

	jstring aname = pgftext2jstring(env, txt);
	free(txt);

	return aname;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_PGF_getCategories(JNIEnv* env, jobject self)
{	
	PgfExn err;
	jobject cats = new_jlist(env); 
	jmethodID add_id = get_jlist_add_method(env);

	JPGFClosure clo = { { pgf_collect_names }, env, self, cats, add_id };
	pgf_iter_categories(get_db(env, self),(long)get_rev(env, self),&clo.fn,&err);

	if (err.type != PGF_EXN_NONE) {
		// should not be necessary to do any explicit cleanup as cats is just a regular Java object (?)
		return NULL;
	}

	return cats;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_PGF_categoryContext(JNIEnv* env, jobject self, jstring c)
{	
	PgfExn err;
	size_t n_hypos;

	PgfText *cname = jstring2pgftext(env,c);
	PgfTypeHypo *phypos = pgf_category_context(get_db(env, self),(long)get_rev(env, self), cname, &n_hypos, &unmarshaller, &err);
	
	if (handleError(env,err) != PGF_EXN_NONE || phypos == NULL) {
        return NULL;
    }

	jobject ctxs = pgf_type_hypos2j_hypo_list(env, n_hypos, phypos);

	for (size_t i = 0; i < n_hypos; i++) {
        free(phypos[i].cid);
        // should not be necessary to clean hypos[i].type as it is just a regular Java object (?)
    }
    free(phypos);

    return ctxs;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_PGF_getStartCat(JNIEnv* env, jobject self)
{
	PgfExn err;
    PgfType t = pgf_start_cat(get_db(env, self),(long)get_rev(env, self), &unmarshaller, &err);

    if (t == 0) {
        throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "start category cannot be found");
        return NULL;
    }
    else if (handleError(env,err) != PGF_EXN_NONE) {
        return NULL;
    }

    return (jobject)t;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_PGF_getFunctions(JNIEnv* env, jobject self)
{	
	PgfExn err;
	jobject funs = new_jlist(env);
	jmethodID add_id = get_jlist_add_method(env);

	JPGFClosure clo = { { pgf_collect_names }, env, self, funs, add_id };
	pgf_iter_functions(get_db(env, self),(long)get_rev(env, self),&clo.fn,&err);

	if (err.type != PGF_EXN_NONE) {
		// should not be necessary to do any explicit cleanup as funs is just a regular Java object (?)
		return NULL;
	}

	return funs;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_PGF_getFunctionsByCat(JNIEnv* env, jobject self, jstring c)
{
	PgfExn err;
	jobject funs = new_jlist(env);
	jmethodID add_id = get_jlist_add_method(env);

	PgfText *cname = jstring2pgftext(env,c);
	JPGFClosure clo = { { pgf_collect_names }, env, self, funs, add_id };
	pgf_iter_functions_by_cat(get_db(env, self),(long)get_rev(env, self),cname,&clo.fn,&err);

	if (err.type != PGF_EXN_NONE) {
		// should not be necessary to do any explicit cleanup as funs is just a regular Java object (?)
		return NULL;
	}

	return funs;
}

JNIEXPORT jboolean JNICALL
Java_org_grammaticalframework_pgf_PGF_functionIsConstructor(JNIEnv* env, jobject self, jstring f)
{
	PgfExn err;

	PgfText *fname = jstring2pgftext(env, f);
	int is_constr = pgf_function_is_constructor(get_db(env, self),(long)get_rev(env, self), fname, &err);

	free(fname);

	handleError(env, err);

    return is_constr == 0 ? JNI_FALSE : JNI_TRUE;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_PGF_getFunctionType(JNIEnv* env, jobject self, jstring f)
{
	PgfExn err;

	PgfText *fname = jstring2pgftext(env, f);
	PgfType t = pgf_function_type(get_db(env, self),(long)get_rev(env, self), fname, &unmarshaller, &err);

	free(fname);

	if (t == 0) {
        throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "function is not defined");
        return NULL;
	} else if (handleError(env, err) != PGF_EXN_NONE) {
		return NULL;
	}

	return (jobject)t;
}

JNIEXPORT jdouble JNICALL
Java_org_grammaticalframework_pgf_PGF_getFunctionProbability(JNIEnv* env, jobject self, jstring f)
{
	PgfExn err;

	PgfText *fname = jstring2pgftext(env, f);
	prob_t p = pgf_function_prob(get_db(env, self),(long)get_rev(env, self), fname, &err);

	free(fname);

	handleError(env,err);
	double dp = (double) p;
    if (dp == INFINITY) {
        throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "function is not defined");
    }

    return (jdouble)dp;
}

JNIEXPORT void JNICALL 
Java_org_grammaticalframework_pgf_PGF_finalize(JNIEnv *env, jobject self)
{	
	jfieldID dbID = (*env)->GetFieldID(env, self , "db", "J");
    jfieldID revID = (*env)->GetFieldID(env, self , "rev", "J");
	PgfDB* db = (PgfDB*)(*env)->GetObjectField(env,self,dbID);
	jlong rev = (*env)->GetLongField(env,self,revID);
	pgf_free_revision(db, rev);
}

/* Type */

JNIEXPORT jobject JNICALL 
Java_org_grammaticalframework_pgf_Type_readType(JNIEnv* env, jclass cls, jstring s)
{
	PgfText* txt = jstring2pgftext(env, s);
	PgfType t = pgf_read_type(txt, &unmarshaller);

	free(txt);

	if (t == 0) {
        throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "type cannot be parsed");
        return NULL;
    }

	return (jobject)t;
}

JNIEXPORT jstring JNICALL
Java_org_grammaticalframework_pgf_Type_toString___3Ljava_lang_String_2(JNIEnv* env, jobject self, jobjectArray vs)
{
	// TODO: build ctx from vs
	PgfText *p = pgf_print_type((PgfType)&self, NULL, 0, &marshaller);
	// TODO: free ctx
	jstring s = pgftext2jstring(env, p);
	free(p);
	return s;
}

/*
static void
pgf_collect_langs(GuMapItor* fn, const void* key, void* value, GuExn* err)
{
	PgfCId name = (PgfCId) key;
    PgfConcr* concr = *((PgfConcr**) value);
    JPGFClosure* clo = (JPGFClosure*) fn;

	jstring jname = gu2j_string(clo->env, name);

	jclass concr_class = (*clo->env)->FindClass(clo->env, "org/grammaticalframework/pgf/Concr");
	jmethodID cid = (*clo->env)->GetMethodID(clo->env, concr_class, "<init>", "(Lorg/grammaticalframework/pgf/PGF;J)V");
	jobject jconcr = (*clo->env)->NewObject(clo->env, concr_class, cid, clo->grammar, p2l(concr));

	(*clo->env)->CallObjectMethod(clo->env, clo->object, clo->method_id, jname, jconcr);
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_PGF_getLanguages(JNIEnv* env, jobject self)
{
	jclass map_class = (*env)->FindClass(env, "java/util/HashMap");
	if (!map_class)
		return NULL;
	jmethodID cid = (*env)->GetMethodID(env, map_class, "<init>", "()V");
	if (!cid)
		return NULL;
	jobject languages = (*env)->NewObject(env, map_class, cid);
	if (!languages)
		return NULL;
	jmethodID put_method = (*env)->GetMethodID(env, map_class, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
	if (!put_method)
		return NULL;

	PGF* pgf = get_ref(env, self);

	GuPool* tmp_pool = gu_local_pool();

	// Create an exception frame that catches all errors.
	GuExn* err = gu_exn(tmp_pool);

	JPGFClosure clo = { { pgf_collect_langs }, env, self, languages, put_method };
	pgf_iter_languages(pgf, &clo.fn, err);
	if (!gu_ok(err)) {
		gu_pool_free(tmp_pool);
		return NULL;
	}
	
	gu_pool_free(tmp_pool);

	return languages;
}

JNIEXPORT jstring JNICALL
Java_org_grammaticalframework_pgf_PGF_graphvizAbstractTree(JNIEnv* env, jobject self, jobject jexpr)
{
	GuPool* tmp_pool = gu_local_pool();

	GuExn* err = gu_exn(tmp_pool);
	GuStringBuf* sbuf = gu_new_string_buf(tmp_pool);
	GuOut* out = gu_string_buf_out(sbuf);

	pgf_graphviz_abstract_tree(get_ref(env,self),
	                           gu_variant_from_ptr(l2p(get_ref(env,jexpr))),
	                           pgf_default_graphviz_options,
	                           out, err);

	jstring jstr = gu2j_string_buf(env, sbuf);

	gu_pool_free(tmp_pool);
	return jstr;
}

JNIEXPORT jstring JNICALL 
Java_org_grammaticalframework_pgf_Concr_getName(JNIEnv* env, jobject self)
{
	return gu2j_string(env, pgf_concrete_name(get_ref(env, self)));
}

JNIEXPORT void JNICALL
Java_org_grammaticalframework_pgf_Concr_load__Ljava_io_InputStream_2(JNIEnv* env, jobject self, jobject java_stream)
{
	GuPool* tmp_pool = gu_local_pool();

	GuInStream* jstream =
		jpgf_new_java_stream(env, java_stream, tmp_pool);
	if (!jstream) {
		gu_pool_free(tmp_pool);
		return;
	}

	GuIn* in = gu_new_in(jstream, tmp_pool);

	// Create an exception frame that catches all errors.
	GuExn* err = gu_exn(tmp_pool);

	pgf_concrete_load(get_ref(env, self), in, err);
	if (!gu_ok(err)) {
		if (gu_exn_caught(err, PgfExn)) {
			GuString msg = (GuString) gu_exn_caught_data(err);
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
		} else {
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The language cannot be loaded");
		}
	}

	gu_pool_free(tmp_pool);
}

JNIEXPORT void JNICALL
Java_org_grammaticalframework_pgf_Concr_unload(JNIEnv* env, jobject self)
{
	pgf_concrete_unload(get_ref(env, self));
}

JNIEXPORT jlong JNICALL 
Java_org_grammaticalframework_pgf_Parser_newCallbacksMap
  (JNIEnv* env, jclass clazz, jobject jconcr, jobject jpool)
{
	return p2l(pgf_new_callbacks_map(get_ref(env, jconcr), get_ref(env, jpool)));
}

typedef struct {
	PgfLiteralCallback callback;
	jobject jcallback;
	jmethodID match_methodId;
	jmethodID predict_methodId;
	GuFinalizer fin;
} JPgfLiteralCallback;

typedef struct {
	GuEnum en;
	jobject jiterator;
	GuFinalizer fin;
} JPgfTokenProbEnum;

static PgfExprProb*
jpgf_literal_callback_match(PgfLiteralCallback* self, PgfConcr* concr,
                            GuString ann,
                            GuString sentence, size_t* poffset,
                            GuPool *out_pool)
{
	JPgfLiteralCallback* callback = gu_container(self, JPgfLiteralCallback, callback);

	JNIEnv *env;
    (*cachedJVM)->AttachCurrentThread(cachedJVM, (void**)&env, NULL);

	jstring jann    = gu2j_string(env, ann);
	size_t  joffset = gu2j_string_offset(sentence, *poffset);
	jobject result = (*env)->CallObjectMethod(env, callback->jcallback, callback->match_methodId, jann, joffset);
	if (result == NULL)
		return NULL;

	jclass result_class = (*env)->GetObjectcls(env, result);
	
	jfieldID epId = (*env)->GetFieldID(env, result_class, "ep", "Lorg/grammaticalframework/pgf/ExprProb;");
	jobject jep = (*env)->GetObjectField(env, result, epId);
	jclass ep_class = (*env)->GetObjectcls(env, jep);
	jfieldID exprId = (*env)->GetFieldID(env, ep_class, "expr", "Lorg/grammaticalframework/pgf/Expr;");
	jobject jexpr = (*env)->GetObjectField(env, jep, exprId);
	jfieldID probId = (*env)->GetFieldID(env, ep_class, "prob", "D");
	double prob = (*env)->GetDoubleField(env, jep, probId);

	jfieldID offsetId = (*env)->GetFieldID(env, result_class, "offset", "I");
	*poffset = j2gu_string_offset(sentence, (*env)->GetIntField(env, result, offsetId));

	PgfExprProb* ep = gu_new(PgfExprProb, out_pool);
	ep->expr = gu_variant_from_ptr(get_ref(env, jexpr));
	ep->expr = pgf_clone_expr(ep->expr, out_pool);
	ep->prob = prob;
	return ep;
}

static void 
jpgf_token_prob_enum_fin(GuFinalizer* self)
{
	JPgfTokenProbEnum* en = gu_container(self, JPgfTokenProbEnum, fin);
	
	JNIEnv *env;
    (*cachedJVM)->AttachCurrentThread(cachedJVM, (void**)&env, NULL);

	(*env)->DeleteGlobalRef(env, en->jiterator);
}

static GuEnum*
jpgf_literal_callback_predict(PgfLiteralCallback* self, PgfConcr* concr,
	                          GuString ann,
	                          GuString prefix,
	                          GuPool *out_pool)
{
	JPgfLiteralCallback* callback = gu_container(self, JPgfLiteralCallback, callback);

	JNIEnv *env;
    (*cachedJVM)->AttachCurrentThread(cachedJVM, (void**)&env, NULL);

	jstring jann    = gu2j_string(env, ann);
	jstring jprefix = gu2j_string(env, prefix);
	jobject jiterator = (*env)->CallObjectMethod(env, callback->jcallback, callback->predict_methodId, jann, jprefix);
	if (jiterator == NULL)
		return NULL;

	JPgfTokenProbEnum* en = gu_new(JPgfTokenProbEnum, out_pool);
	en->en.next = NULL;
	en->jiterator = (*env)->NewGlobalRef(env, jiterator);
	en->fin.fn = jpgf_token_prob_enum_fin;

	gu_pool_finally(out_pool, &en->fin);

	return &en->en;
}

static void 
jpgf_literal_callback_fin(GuFinalizer* self)
{
	JPgfLiteralCallback* callback = gu_container(self, JPgfLiteralCallback, fin);
	
	JNIEnv *env;
    (*cachedJVM)->AttachCurrentThread(cachedJVM, (void**)&env, NULL);

	(*env)->DeleteGlobalRef(env, callback->jcallback);
}

JNIEXPORT void JNICALL Java_org_grammaticalframework_pgf_Parser_addLiteralCallback
  (JNIEnv* env, jclass clazz, jobject jconcr, jlong callbacksRef, jstring jcat, jobject jcallback, jobject jpool)
{
	PgfConcr* concr = get_ref(env, jconcr);
	GuPool* pool = get_ref(env, jpool);

	JPgfLiteralCallback* callback = gu_new(JPgfLiteralCallback, pool);
	callback->callback.match   = jpgf_literal_callback_match;
	callback->callback.predict = jpgf_literal_callback_predict;
	callback->jcallback = (*env)->NewGlobalRef(env, jcallback);
	callback->fin.fn = jpgf_literal_callback_fin;

	jclass callback_class = (*env)->GetObjectcls(env, jcallback);
	callback->match_methodId = (*env)->GetMethodID(env, callback_class, "match", "(Ljava/lang/String;I)Lorg/grammaticalframework/pgf/LiteralCallback$CallbackResult;");
	callback->predict_methodId = (*env)->GetMethodID(env, callback_class, "predict", "(Ljava/lang/String;Ljava/lang/String;)Ljava/util/Iterator;");

	gu_pool_finally(pool, &callback->fin);
	
	pgf_callbacks_map_add_literal(concr, l2p(callbacksRef),
                                  j2gu_string(env, jcat, pool), &callback->callback);
}

static void
throw_parse_error(JNIEnv *env, PgfParseError* err)
{
	jstring jtoken;
	if (err->incomplete)
		jtoken = NULL;
	else {
		jtoken = gu2j_string_len(env, err->token_ptr, err->token_len);
		if (!jtoken)
			return;
	}

	jclass exception_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/ParseError");
	if (!exception_class)
		return;
	jmethodID cid = (*env)->GetMethodID(env, exception_class, "<init>", "(Ljava/lang/String;IZ)V");
	if (!cid)
		return;
	jobject exception = (*env)->NewObject(env, exception_class, cid, jtoken, err->offset, err->incomplete);
	if (!exception)
		return;
	(*env)->Throw(env, exception);
}

JNIEXPORT jobject JNICALL 
Java_org_grammaticalframework_pgf_Parser_parseWithHeuristics
  (JNIEnv* env, jclass clazz, jobject jconcr, jstring jstartCat, jstring js, jdouble heuristics, jlong callbacksRef, jobject jpool)
{
	GuPool* pool = get_ref(env, jpool);
	GuPool* out_pool = gu_new_pool();

    GuString startCat = j2gu_string(env, jstartCat, pool);
    GuString s = j2gu_string(env, js, pool);
    GuExn* parse_err = gu_new_exn(pool);

	PgfType* type = gu_new_flex(pool, PgfType, exprs, 0);
	type->hypos   = gu_empty_seq();
	type->cid     = startCat;
	type->n_exprs = 0;

	GuEnum* res =
		pgf_parse_with_heuristics(get_ref(env, jconcr), type, s, heuristics, l2p(callbacksRef), parse_err, pool, out_pool);

	if (!gu_ok(parse_err)) {
		if (gu_exn_caught(parse_err, PgfExn)) {
			GuString msg = (GuString) gu_exn_caught_data(parse_err);
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
		} else if (gu_exn_caught(parse_err, PgfParseError)) {
			throw_parse_error(env, (PgfParseError*) gu_exn_caught_data(parse_err));
		}

		gu_pool_free(out_pool);
		return NULL;
	}

	jfieldID refId = (*env)->GetFieldID(env, (*env)->GetObjectcls(env, jconcr), "gr", "Lorg/grammaticalframework/pgf/PGF;");
	jobject jpgf = (*env)->GetObjectField(env, jconcr, refId);

	jclass expiter_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/ExprIterator");
	jmethodID cid = (*env)->GetMethodID(env, expiter_class, "<init>", "(Lorg/grammaticalframework/pgf/PGF;Lorg/grammaticalframework/pgf/Pool;JJ)V");
	jobject jexpiter = (*env)->NewObject(env, expiter_class, cid, jpgf, jpool, p2l(out_pool), p2l(res));

	return jexpiter;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_Completer_complete(JNIEnv* env, jclass clazz, jobject jconcr, jstring jstartCat, jstring js, jstring jprefix)
{
	GuPool* pool = gu_new_pool();

    GuString startCat = j2gu_string(env, jstartCat, pool);
    GuString s = j2gu_string(env, js, pool);
    GuString prefix = j2gu_string(env, jprefix, pool);
    GuExn* parse_err = gu_new_exn(pool);

	PgfType* type = gu_new_flex(pool, PgfType, exprs, 0);
	type->hypos   = gu_empty_seq();
	type->cid     = startCat;
	type->n_exprs = 0;

	GuEnum* res =
		pgf_complete(get_ref(env, jconcr), type, s, prefix, parse_err, pool);

	if (!gu_ok(parse_err)) {
		if (gu_exn_caught(parse_err, PgfExn)) {
			GuString msg = (GuString) gu_exn_caught_data(parse_err);
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
		} else if (gu_exn_caught(parse_err, PgfParseError)) {
			throw_parse_error(env, (PgfParseError*) gu_exn_caught_data(parse_err));
		}

		gu_pool_free(pool);
		return NULL;
	}

	jclass tokiter_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/TokenIterator");
	jmethodID cid = (*env)->GetMethodID(env, tokiter_class, "<init>", "(JJ)V");
	jobject jtokiter = (*env)->NewObject(env, tokiter_class, cid, p2l(pool), p2l(res));

	return jtokiter;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_SentenceExtractor_lookupSentence
  (JNIEnv* env, jclass clazz, jobject jconcr, jstring jstartCat, jstring js, jobject jpool)
{
	GuPool* pool = get_ref(env, jpool);
	GuPool* out_pool = gu_new_pool();

    GuString startCat = j2gu_string(env, jstartCat, pool);
    GuString s = j2gu_string(env, js, pool);

	PgfType* type = gu_new_flex(pool, PgfType, exprs, 0);
	type->hypos   = gu_empty_seq();
	type->cid     = startCat;
	type->n_exprs = 0;

	GuEnum* res =
		pgf_lookup_sentence(get_ref(env, jconcr), type, s, pool, out_pool);

	jfieldID refId = (*env)->GetFieldID(env, (*env)->GetObjectcls(env, jconcr), "gr", "Lorg/grammaticalframework/pgf/PGF;");
	jobject jpgf = (*env)->GetObjectField(env, jconcr, refId);

	jclass expiter_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/ExprIterator");
	jmethodID cid = (*env)->GetMethodID(env, expiter_class, "<init>", "(Lorg/grammaticalframework/pgf/PGF;Lorg/grammaticalframework/pgf/Pool;JJ)V");
	jobject jexpiter = (*env)->NewObject(env, expiter_class, cid, jpgf, jpool, p2l(out_pool), p2l(res));

	return jexpiter;
}

JNIEXPORT jobject JNICALL 
Java_org_grammaticalframework_pgf_TokenIterator_fetchTokenProb(JNIEnv* env, jclass clazz, jlong enumRef, jobject jpool)
{
	GuEnum* res = (GuEnum*) l2p(enumRef);

	PgfTokenProb* tp = gu_next(res, PgfTokenProb*, get_ref(env, jpool));
	if (tp == NULL)
		return NULL;

	jclass tp_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/TokenProb");
	jmethodID tp_cid = (*env)->GetMethodID(env, tp_class, "<init>", "(DLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	jobject jtp = (*env)->NewObject(env, tp_class, tp_cid, (double) tp->prob, gu2j_string(env,tp->tok), gu2j_string(env,tp->cat), gu2j_string(env,tp->fun));

	return jtp;
}

JNIEXPORT jobject JNICALL 
Java_org_grammaticalframework_pgf_ExprIterator_fetchExprProb
  (JNIEnv* env, jclass clazz, jlong enumRef, jobject jpool, jobject gr)
{
	GuEnum* res = (GuEnum*) l2p(enumRef);
	GuPool* pool = get_ref(env, jpool);

	PgfExprProb* ep = gu_next(res, PgfExprProb*, pool);
	if (ep == NULL)
		return NULL;

	jclass expprob_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/ExprProb");
	jmethodID methodId = (*env)->GetStaticMethodID(env, expprob_class, "mkExprProb", 
	           "(Lorg/grammaticalframework/pgf/Pool;Lorg/grammaticalframework/pgf/PGF;JD)Lorg/grammaticalframework/pgf/ExprProb;");
	jobject jexpprob = (*env)->CallStaticObjectMethod(env, expprob_class, methodId, 
	           jpool, gr, p2l(gu_variant_to_ptr(ep->expr)), (double) ep->prob);

	return jexpprob;
}

JNIEXPORT jstring JNICALL
Java_org_grammaticalframework_pgf_Concr_linearize(JNIEnv* env, jobject self, jobject jexpr)
{
	GuPool* tmp_pool = gu_local_pool();
	GuExn* err = gu_exn(tmp_pool);
	GuStringBuf* sbuf = gu_new_string_buf(tmp_pool);
	GuOut* out = gu_string_buf_out(sbuf);
	
	pgf_linearize(get_ref(env, self), gu_variant_from_ptr((void*) get_ref(env, jexpr)), out, err);
	if (!gu_ok(err)) {
		if (gu_exn_caught(err, PgfLinNonExist)) {
			gu_pool_free(tmp_pool);
			return NULL;
		} else if (gu_exn_caught(err, PgfExn)) {
			GuString msg = (GuString) gu_exn_caught_data(err);
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
		} else {
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The expression cannot be linearized");
		}
		gu_pool_free(tmp_pool);
		return NULL;
	}

	jstring jstr = gu2j_string_buf(env, sbuf);

	gu_pool_free(tmp_pool);
	
	return jstr;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_Concr_linearizeAll(JNIEnv* env, jobject self, jobject jexpr)
{
	PgfConcr* concr = get_ref(env, self);
	
	jclass list_class = (*env)->FindClass(env, "java/util/ArrayList");
	jmethodID list_cid = (*env)->GetMethodID(env, list_class, "<init>", "()V");
	jobject strings = (*env)->NewObject(env, list_class, list_cid);
	
	jmethodID add_id = (*env)->GetMethodID(env, list_class, "add", "(Ljava/lang/Object;)Z");

	GuPool* tmp_pool = gu_local_pool();
	GuExn* err = gu_exn(tmp_pool);
	
	GuEnum* cts =
		pgf_lzr_concretize(concr,
		                   gu_variant_from_ptr((void*) get_ref(env, jexpr)),
		                   err, tmp_pool);
	if (!gu_ok(err)) {
		gu_pool_free(tmp_pool);
		return NULL;
	}

	for (;;) {
		PgfCncTree ctree = gu_next(cts, PgfCncTree, tmp_pool);
		if (gu_variant_is_null(ctree))
			break;

		GuPool* step_pool = gu_local_pool();
		GuStringBuf* sbuf = gu_new_string_buf(step_pool);
		GuOut* out = gu_string_buf_out(sbuf);

		ctree = pgf_lzr_wrap_linref(ctree, step_pool);
		pgf_lzr_linearize_simple(concr, ctree, 0, out, err, step_pool);
		if (!gu_ok(err)) {
			gu_pool_free(step_pool);

			if (gu_exn_caught(err, PgfLinNonExist)) {				
				continue;
			} else if (gu_exn_caught(err, PgfExn)) {
				GuString msg = (GuString) gu_exn_caught_data(err);
				throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
			} else {
				throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The expression cannot be linearized");
			}
			gu_pool_free(tmp_pool);
			return NULL;
		}

		jstring jstr = gu2j_string_buf(env, sbuf);

		(*env)->CallBooleanMethod(env, strings, add_id, jstr);
		
		gu_pool_free(step_pool);
	}

	gu_pool_free(tmp_pool);

	return strings;
}

JNIEXPORT jobject JNICALL 
Java_org_grammaticalframework_pgf_Concr_tabularLinearize(JNIEnv* env, jobject self, jobject jexpr)
{
	jclass map_class = (*env)->FindClass(env, "java/util/HashMap");
	if (!map_class)
		return NULL;
	jmethodID cid = (*env)->GetMethodID(env, map_class, "<init>", "()V");
	if (!cid)
		return NULL;
	jobject table = (*env)->NewObject(env, map_class, cid);
	if (!table)
		return NULL;

	jmethodID put_method = (*env)->GetMethodID(env, map_class, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
	if (!put_method)
		return NULL;
		
	PgfConcr* concr = get_ref(env, self);

	GuPool* tmp_pool = gu_local_pool();
	GuExn* err = gu_exn(tmp_pool);

	GuEnum* cts = 
		pgf_lzr_concretize(concr,
		                   gu_variant_from_ptr((void*) get_ref(env, jexpr)),
		                   err,
		                   tmp_pool);
	if (!gu_ok(err)) {
		if (gu_exn_caught(err, PgfExn)) {
			GuString msg = (GuString) gu_exn_caught_data(err);
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
		} else {
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The expression cannot be concretized");
		}
		gu_pool_free(tmp_pool);
		return NULL;
	}

	PgfCncTree ctree = gu_next(cts, PgfCncTree, tmp_pool);
	if (gu_variant_is_null(ctree)) {
		gu_pool_free(tmp_pool);
		return NULL;
	}

	size_t n_lins;
	GuString* labels;
	pgf_lzr_get_table(concr, ctree, &n_lins, &labels);

	for (size_t lin_idx = 0; lin_idx < n_lins; lin_idx++) {
		GuStringBuf* sbuf = gu_new_string_buf(tmp_pool);
		GuOut* out = gu_string_buf_out(sbuf);

		pgf_lzr_linearize_simple(concr, ctree, lin_idx, out, err, tmp_pool);

		jstring jstr = NULL;
		if (gu_ok(err)) {
			jstr = gu2j_string_buf(env, sbuf);
		} else {
			gu_exn_clear(err);
		}

		jstring jname = gu2j_string(env, labels[lin_idx]);

		(*env)->CallObjectMethod(env, table, put_method, jname, jstr);

		(*env)->DeleteLocalRef(env, jname);
		
		if (jstr != NULL)
			(*env)->DeleteLocalRef(env, jstr);
	}
	
	gu_pool_free(tmp_pool);

	return table;
}

typedef struct {
	PgfLinFuncs* funcs;
	JNIEnv* env;
	GuPool* tmp_pool;
	GuBuf* stack;
	GuBuf* list;
	bool bind;
	PgfCapitState capit;
	jobject bind_instance;
	jclass object_class;
	jclass bracket_class;
	jmethodID bracket_cid;
} PgfBracketLznState;

static void
pgf_bracket_lzn_symbol_token(PgfLinFuncs** funcs, PgfToken tok)
{
	PgfBracketLznState* state = gu_container(funcs, PgfBracketLznState, funcs);
	JNIEnv* env = state->env;

	if (state->bind) {
		jobject bind_instance = (*env)->NewLocalRef(env, state->bind_instance);
		gu_buf_push(state->list, jobject, bind_instance);
		state->bind = false;
	} else {
		if (state->capit == PGF_CAPIT_NEXT)
			state->capit = PGF_CAPIT_NONE;
	}

	if (state->capit == PGF_CAPIT_ALL)
		state->capit = PGF_CAPIT_NEXT;

	jstring jtok = gu2j_string_capit(env, tok, state->capit);
	gu_buf_push(state->list, jobject, jtok);
	
	if (state->capit == PGF_CAPIT_FIRST)
		state->capit = PGF_CAPIT_NONE;
}

static void
pgf_bracket_lzn_begin_phrase(PgfLinFuncs** funcs, PgfCId cat, int fid, GuString ann, PgfCId fun)
{
	PgfBracketLznState* state = gu_container(funcs, PgfBracketLznState, funcs);
	
	gu_buf_push(state->stack, GuBuf*, state->list);
	state->list = gu_new_buf(jobject, state->tmp_pool);
}

static void
pgf_bracket_lzn_end_phrase(PgfLinFuncs** funcs, PgfCId cat, int fid, GuString ann, PgfCId fun)
{
	PgfBracketLznState* state = gu_container(funcs, PgfBracketLznState, funcs);
	JNIEnv* env = state->env;

	GuBuf* parent = gu_buf_pop(state->stack, GuBuf*);

	if (gu_buf_length(state->list) > 0) {
		jstring jcat = gu2j_string(env, cat);
		jstring jfun = gu2j_string(env, fun);
		jstring jann = gu2j_string(env, ann);

		size_t len = gu_buf_length(state->list);
		jobjectArray jchildren = (*env)->NewObjectArray(env, len, state->object_class, NULL);
		for (int i = 0; i < len; i++) {
			jobject obj = gu_buf_get(state->list, jobject, i);
			(*env)->SetObjectArrayElement(env, jchildren, i, obj);
			(*env)->DeleteLocalRef(env, obj);
		}

		jobject jbracket = (*env)->NewObject(env, 
		                                     state->bracket_class,
		                                     state->bracket_cid, 
		                                     jcat,
		                                     jfun,
		                                     fid,
		                                     jann,
		                                     jchildren);

		(*env)->DeleteLocalRef(env, jchildren);
		(*env)->DeleteLocalRef(env, jann);
		(*env)->DeleteLocalRef(env, jfun);
		(*env)->DeleteLocalRef(env, jcat);

		gu_buf_push(parent, jobject, jbracket);
	}

	state->list = parent;
}

static void
pgf_bracket_lzn_symbol_bind(PgfLinFuncs** funcs)
{
	PgfBracketLznState* state = gu_container(funcs, PgfBracketLznState, funcs);
	state->bind = true;
}

static void
pgf_bracket_lzn_symbol_capit(PgfLinFuncs** funcs, PgfCapitState capit)
{
	PgfBracketLznState* state = gu_container(funcs, PgfBracketLznState, funcs);
	state->capit = capit;
}

static void
pgf_bracket_lzn_symbol_meta(PgfLinFuncs** funcs, PgfMetaId id)
{
	pgf_bracket_lzn_symbol_token(funcs, "?");
}

static PgfLinFuncs pgf_bracket_lin_funcs = {
	.symbol_token  = pgf_bracket_lzn_symbol_token,
	.begin_phrase  = pgf_bracket_lzn_begin_phrase,
	.end_phrase    = pgf_bracket_lzn_end_phrase,
	.symbol_ne     = NULL,
	.symbol_bind   = pgf_bracket_lzn_symbol_bind,
	.symbol_capit  = pgf_bracket_lzn_symbol_capit,
	.symbol_meta   = pgf_bracket_lzn_symbol_meta
};

JNIEXPORT jobjectArray JNICALL 
Java_org_grammaticalframework_pgf_Concr_bracketedLinearize(JNIEnv* env, jobject self, jobject jexpr)
{
	jclass object_class = (*env)->FindClass(env, "java/lang/Object");
	if (!object_class)
		return NULL;

	jclass bracket_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/Bracket");
	if (!bracket_class)
		return NULL;
	jmethodID bracket_cid = (*env)->GetMethodID(env, bracket_class, "<init>", "(Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;[Ljava/lang/Object;)V");
	if (!bracket_cid)
		return NULL;
		
	jclass bind_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/BIND");
	if (!bind_class)
		return NULL;
	jfieldID bind_instance_id = (*env)->GetStaticFieldID(env, bind_class, "instance", "Lorg/grammaticalframework/pgf/BIND;");
	if (!bind_instance_id)
		return NULL;
	jobject bind_instance = (*env)->GetStaticObjectField(env, bind_class, bind_instance_id);
	if (!bind_instance)
		return NULL;

	GuPool* tmp_pool = gu_local_pool();
	GuExn* err = gu_exn(tmp_pool);

	PgfConcr* concr = get_ref(env, self);

	GuEnum* cts = 
		pgf_lzr_concretize(concr, gu_variant_from_ptr((void*) get_ref(env, jexpr)), err, tmp_pool);
	if (!gu_ok(err)) {
		if (gu_exn_caught(err, PgfExn)) {
			GuString msg = (GuString) gu_exn_caught_data(err);
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
		} else {
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The expression cannot be concretized");
		}
		gu_pool_free(tmp_pool);
		return NULL;
	}

	PgfCncTree ctree = gu_next(cts, PgfCncTree, tmp_pool);
	if (gu_variant_is_null(ctree)) {
		throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The expression cannot be concretized");
		gu_pool_free(tmp_pool);
		return NULL;
	}

	ctree = pgf_lzr_wrap_linref(ctree, tmp_pool);

	PgfBracketLznState state;
	state.funcs = &pgf_bracket_lin_funcs;
	state.env   = env;
	state.tmp_pool = tmp_pool;
	state.stack = gu_new_buf(GuBuf*, tmp_pool);
	state.list  = gu_new_buf(jobject, tmp_pool);
	state.bind  = true;
	state.capit = PGF_CAPIT_NONE;
	state.bind_instance = bind_instance;
	state.object_class = object_class;
	state.bracket_class = bracket_class;
	state.bracket_cid = bracket_cid;
	pgf_lzr_linearize(concr, ctree, 0, &state.funcs, tmp_pool);

	size_t len = gu_buf_length(state.list);
	jobjectArray array = (*env)->NewObjectArray(env, len, object_class, NULL);
	for (int i = 0; i < len; i++) {
		jobject obj = gu_buf_get(state.list, jobject, i);
		(*env)->SetObjectArrayElement(env, array, i, obj);
		(*env)->DeleteLocalRef(env, obj);
	}

	gu_pool_free(tmp_pool);

	return array;
}

typedef struct {
	PgfMorphoCallback fn;
	jobject analyses;
	prob_t prob;
	JNIEnv* env;
	jmethodID add_id;
	jclass an_class;
	jmethodID an_cid;
} JMorphoCallback;

static void
jpgf_collect_morpho(PgfMorphoCallback* self,
                    PgfCId lemma, GuString analysis, prob_t prob,
	                GuExn* err)
{
	JMorphoCallback* callback = (JMorphoCallback*) self;
	JNIEnv* env = callback->env;
	
	jstring jlemma = gu2j_string(env,lemma);
	jstring janalysis = gu2j_string(env,analysis);
	jobject jan = (*env)->NewObject(env, 
	                                callback->an_class,
	                                callback->an_cid, 
	                                jlemma,
	                                janalysis,
	                                (double) prob);
	(*env)->CallBooleanMethod(env, callback->analyses, callback->add_id, jan);
	(*env)->DeleteLocalRef(env, jan);
	(*env)->DeleteLocalRef(env, janalysis);
	(*env)->DeleteLocalRef(env, jlemma);
	
	callback->prob += exp(-prob);
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_Concr_lookupMorpho(JNIEnv* env, jobject self, jstring sentence)
{
	jclass list_class = (*env)->FindClass(env, "java/util/ArrayList");
	jmethodID list_cid = (*env)->GetMethodID(env, list_class, "<init>", "()V");
	jobject analyses = (*env)->NewObject(env, list_class, list_cid);
	
	jmethodID add_id = (*env)->GetMethodID(env, list_class, "add", "(Ljava/lang/Object;)Z");

	jclass an_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/MorphoAnalysis");
	jmethodID an_cid = (*env)->GetMethodID(env, an_class, "<init>", "(Ljava/lang/String;Ljava/lang/String;D)V");

	GuPool* tmp_pool = gu_new_pool();
	
	GuExn* err = gu_exn(tmp_pool);

	JMorphoCallback callback = { { jpgf_collect_morpho }, analyses, 0, env, add_id, an_class, an_cid };
	pgf_lookup_morpho(get_ref(env, self), j2gu_string(env, sentence, tmp_pool),
	                  &callback.fn, err);
	if (!gu_ok(err)) {
		if (gu_exn_caught(err, PgfExn)) {
			GuString msg = (GuString) gu_exn_caught_data(err);
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
		} else {
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The lookup failed");
		}
		analyses = NULL;
	}

	gu_pool_free(tmp_pool);

	return analyses;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_Lexicon_lookupWordPrefix
   (JNIEnv* env, jclass clazz, jobject jconcr, jstring prefix)
{
	GuPool* pool = gu_new_pool();	
	GuExn* err = gu_new_exn(pool);

	GuEnum* en = 
		(prefix == NULL) ? pgf_fullform_lexicon(get_ref(env, jconcr), 
	                                            pool)
	                     : pgf_lookup_word_prefix(get_ref(env, jconcr), j2gu_string(env, prefix, pool),
	                                              pool, err);
	if (!gu_ok(err)) {
		if (gu_exn_caught(err, PgfExn)) {
			GuString msg = (GuString) gu_exn_caught_data(err);
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
		} else {
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The lookup failed");
		}
		return NULL;
	}

	jclass iter_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/FullFormIterator");
	jmethodID iter_cid = (*env)->GetMethodID(env, iter_class, "<init>", "(Lorg/grammaticalframework/pgf/Concr;JJ)V");
	jobject iter = (*env)->NewObject(env, iter_class, iter_cid, jconcr, p2l(pool), p2l(en));

	return iter;
}

JNIEXPORT jobject JNICALL 
Java_org_grammaticalframework_pgf_FullFormIterator_fetchFullFormEntry
  (JNIEnv* env, jobject clazz, jlong enumRef, jobject jpool, jobject jconcr)
{
	GuEnum* res = (GuEnum*) l2p(enumRef);

	PgfFullFormEntry* entry = gu_next(res, PgfFullFormEntry*, get_ref(env, jpool));
	if (entry == NULL)
		return NULL;

	GuString form = pgf_fullform_get_string(entry);

	jclass list_class = (*env)->FindClass(env, "java/util/ArrayList");
	jmethodID list_cid = (*env)->GetMethodID(env, list_class, "<init>", "()V");
	jobject analyses = (*env)->NewObject(env, list_class, list_cid);

	jmethodID add_id = (*env)->GetMethodID(env, list_class, "add", "(Ljava/lang/Object;)Z");

	jclass an_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/MorphoAnalysis");
	jmethodID an_cid = (*env)->GetMethodID(env, an_class, "<init>", "(Ljava/lang/String;Ljava/lang/String;D)V");

	GuPool* tmp_pool = gu_local_pool();
	GuExn* err = gu_exn(tmp_pool);

	JMorphoCallback callback = { { jpgf_collect_morpho }, analyses, 0, env, add_id, an_class, an_cid };
	pgf_fullform_get_analyses(entry, &callback.fn, err);
	if (!gu_ok(err)) {
		if (gu_exn_caught(err, PgfExn)) {
			GuString msg = (GuString) gu_exn_caught_data(err);
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
		} else {
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The lookup failed");
		}
		analyses = NULL;
	}

	gu_pool_free(tmp_pool);

	jclass entry_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/FullFormEntry");
	jmethodID entry_cid = (*env)->GetMethodID(env, entry_class, "<init>", "(Ljava/lang/String;DLjava/util/List;)V");
	jobject jentry = (*env)->NewObject(env, entry_class, entry_cid, gu2j_string(env,form), - log(callback.prob), analyses);

	return jentry;
}

JNIEXPORT jboolean JNICALL
Java_org_grammaticalframework_pgf_Concr_hasLinearization(JNIEnv* env, jobject self, jstring jid)
{
	PgfConcr* concr = get_ref(env, self);
	GuPool* tmp_pool = gu_local_pool();
	PgfCId id = j2gu_string(env, jid, tmp_pool);
	bool res = pgf_has_linearization(concr, id);
	gu_pool_free(tmp_pool);
	return res;
}

JNIEXPORT jstring JNICALL
Java_org_grammaticalframework_pgf_Concr_getPrintName(JNIEnv* env, jobject self, jstring jid)
{
	PgfConcr* concr = get_ref(env, self);
	GuPool* tmp_pool = gu_local_pool();
	PgfCId id = j2gu_string(env, jid, tmp_pool);
	GuString name = pgf_print_name(concr, id);
	jstring jname = (name == NULL) ? NULL : gu2j_string(env, name);
	gu_pool_free(tmp_pool);

	return jname;
}

JNIEXPORT jstring JNICALL
Java_org_grammaticalframework_pgf_Concr_graphvizParseTree(JNIEnv* env, jobject self, jobject jexpr)
{
	GuPool* tmp_pool = gu_local_pool();

	GuExn* err = gu_exn(tmp_pool);
	GuStringBuf* sbuf = gu_new_string_buf(tmp_pool);
	GuOut* out = gu_string_buf_out(sbuf);

	pgf_graphviz_parse_tree(get_ref(env,self),
	                        gu_variant_from_ptr(l2p(get_ref(env,jexpr))),
	                        pgf_default_graphviz_options,
	                        out, err);

	jstring jstr = gu2j_string_buf(env, sbuf);

	gu_pool_free(tmp_pool);
	return jstr;
}

JNIEXPORT jlong JNICALL
Java_org_grammaticalframework_pgf_Pool_alloc(JNIEnv* env, jclass clazz)
{
	return p2l(gu_new_pool());
}

JNIEXPORT void JNICALL 
Java_org_grammaticalframework_pgf_Pool_free(JNIEnv* env, jclass clazz, jlong ref)
{
	gu_pool_free((GuPool*) l2p(ref));
}

JNIEXPORT jstring JNICALL
Java_org_grammaticalframework_pgf_Expr_showExpr(JNIEnv* env, jclass clazz, jlong ref)
{
	GuPool* tmp_pool = gu_local_pool();

	GuExn* err = gu_exn(tmp_pool);
	GuStringBuf* sbuf = gu_new_string_buf(tmp_pool);
	GuOut* out = gu_string_buf_out(sbuf);

	pgf_print_expr(gu_variant_from_ptr(l2p(ref)), NULL, 0, out, err);

	jstring jstr = gu2j_string_buf(env, sbuf);

	gu_pool_free(tmp_pool);
	return jstr;
}

JNIEXPORT jobject JNICALL 
Java_org_grammaticalframework_pgf_Expr_readExpr(JNIEnv* env, jclass clazz, jstring s)
{
	GuPool* pool = gu_new_pool();
	
	GuPool* tmp_pool = gu_local_pool();
	GuString buf = j2gu_string(env, s, tmp_pool);
	GuIn* in = gu_data_in((uint8_t*) buf, strlen(buf), tmp_pool);
	GuExn* err = gu_exn(tmp_pool);

	PgfExpr e = pgf_read_expr(in, pool, tmp_pool, err);
	if (!gu_ok(err) || gu_variant_is_null(e)) {
		throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The expression cannot be parsed");
		gu_pool_free(tmp_pool);
		gu_pool_free(pool);
		return NULL;
	}

	gu_pool_free(tmp_pool);

	jclass pool_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/Pool");
	jmethodID pool_cid = (*env)->GetMethodID(env, pool_class, "<init>", "(J)V");
	jobject jpool = (*env)->NewObject(env, pool_class, pool_cid, p2l(pool));

	jmethodID cid = (*env)->GetMethodID(env, clazz, "<init>", "(Lorg/grammaticalframework/pgf/Pool;Ljava/lang/Object;J)V");
	return (*env)->NewObject(env, clazz, cid, jpool, NULL, p2l(gu_variant_to_ptr(e)));
}

JNIEXPORT jlong JNICALL 
Java_org_grammaticalframework_pgf_Expr_initStringLit(JNIEnv* env, jclass clazz, jstring jstr, jlong jpool)
{
	GuPool* pool = l2p(jpool);
	PgfExpr expr;

	PgfExprLit* e =
		gu_new_variant(PGF_EXPR_LIT,
					   PgfExprLit,
					   &expr, pool);

	GuString str = (*env)->GetStringUTFChars(env, jstr, 0);
	PgfLiteralStr* slit =
		gu_new_flex_variant(PGF_LITERAL_STR,
		                    PgfLiteralStr,
		                    val, strlen(str)+1,
		                    &e->lit, pool);
	strcpy(slit->val, str);
	(*env)->ReleaseStringUTFChars(env, jstr, str);

	return expr;
}

JNIEXPORT jlong JNICALL 
Java_org_grammaticalframework_pgf_Expr_initIntLit(JNIEnv* env, jclass clazz, jint jd, jlong jpool)
{
	GuPool* pool = l2p(jpool);
	PgfExpr expr;

	PgfExprLit* e =
		gu_new_variant(PGF_EXPR_LIT,
					   PgfExprLit,
					   &expr, pool);

	PgfLiteralInt* nlit =
		gu_new_variant(PGF_LITERAL_INT,
					   PgfLiteralInt,
					   &e->lit, pool);
	nlit->val = jd;

	return expr;
}

JNIEXPORT jlong JNICALL 
Java_org_grammaticalframework_pgf_Expr_initFloatLit(JNIEnv* env, jclass clazz, jdouble jf, jlong jpool)
{
	GuPool* pool = l2p(jpool);
	PgfExpr expr;

	PgfExprLit* e =
		gu_new_variant(PGF_EXPR_LIT,
					   PgfExprLit,
					   &expr, pool);

	PgfLiteralFlt* flit =
		gu_new_variant(PGF_LITERAL_FLT,
					   PgfLiteralFlt,
					   &e->lit, pool);
	flit->val = jf;

	return expr;
}

JNIEXPORT jlong JNICALL
Java_org_grammaticalframework_pgf_Expr_initApp__Lorg_grammaticalframework_pgf_Expr_2_3Lorg_grammaticalframework_pgf_Expr_2J
(JNIEnv* env, jclass clazz, jobject jfun, jobjectArray args, jlong jpool)
{
	GuPool* pool = l2p(jpool);
	PgfExpr expr = gu_variant_from_ptr(get_ref(env, jfun));

	size_t n_args = (*env)->GetArrayLength(env, args);
	for (size_t i = 0; i < n_args; i++) {
		PgfExpr fun = expr;
		PgfExpr arg = gu_variant_from_ptr(get_ref(env, (*env)->GetObjectArrayElement(env, args, i)));

		PgfExprApp* e =
			gu_new_variant(PGF_EXPR_APP,
						   PgfExprApp,
						   &expr, pool);
		e->fun = fun;
		e->arg = arg;
	}

	return expr;
}

JNIEXPORT jlong JNICALL 
Java_org_grammaticalframework_pgf_Expr_initApp__Ljava_lang_String_2_3Lorg_grammaticalframework_pgf_Expr_2J
(JNIEnv* env, jclass clazz, jstring jfun, jobjectArray args, jlong jpool)
{
	GuPool* pool = l2p(jpool);
	PgfExpr expr;

	GuString fun = (*env)->GetStringUTFChars(env, jfun, 0);
	PgfExprFun* e =
		gu_new_flex_variant(PGF_EXPR_FUN,
					        PgfExprFun,
					        fun, strlen(fun)+1,
					        &expr, pool);
	strcpy(e->fun, fun);
	(*env)->ReleaseStringUTFChars(env, jfun, fun);

	size_t n_args = (*env)->GetArrayLength(env, args);
	for (size_t i = 0; i < n_args; i++) {
		PgfExpr fun = expr;
		PgfExpr arg = gu_variant_from_ptr(get_ref(env, (*env)->GetObjectArrayElement(env, args, i)));

		PgfExprApp* e =
			gu_new_variant(PGF_EXPR_APP,
						   PgfExprApp,
						   &expr, pool);
		e->fun = fun;
		e->arg = arg;
	}

	return expr;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_Expr_unApp(JNIEnv* env, jobject self)
{
	jclass expr_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/Expr");
	if (!expr_class)
		return NULL;
	jmethodID expr_cid = (*env)->GetMethodID(env, expr_class, "<init>", "(Lorg/grammaticalframework/pgf/Pool;Ljava/lang/Object;J)V");
	jclass app_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/ExprApplication");
	if (!app_class)
		return NULL;
	jmethodID app_cid = (*env)->GetMethodID(env, app_class, "<init>", "(Ljava/lang/String;[Lorg/grammaticalframework/pgf/Expr;)V");
	if (!app_cid)
		return NULL;

	PgfExpr expr = gu_variant_from_ptr(get_ref(env, self));
	
	GuPool* tmp_pool = gu_local_pool();
	PgfApplication* app = pgf_expr_unapply(expr, tmp_pool);
	
	jobject japp = NULL;
	if (app != NULL) {
		jstring jfun = gu2j_string(env, app->fun);
		jobject jargs = (*env)->NewObjectArray(env, app->n_args, expr_class, NULL);
		for (size_t i = 0; i < app->n_args; i++) {
			jobject jarg = (*env)->NewObject(env, expr_class, expr_cid, NULL, self, p2l(app->args[i]));
			(*env)->SetObjectArrayElement(env, jargs, i, jarg);
			(*env)->DeleteLocalRef(env, jarg);
		}
		japp = (*env)->NewObject(env, app_class, app_cid, jfun, jargs);
	}

	gu_pool_free(tmp_pool);

	return japp;
}

JNIEXPORT jint JNICALL
Java_org_grammaticalframework_pgf_Expr_unMeta(JNIEnv* env, jobject self)
{
	PgfExpr expr = gu_variant_from_ptr(get_ref(env, self));

	PgfExprMeta* pmeta = pgf_expr_unmeta(expr);
	if (pmeta != NULL) {
		return pmeta->id;
	}

	return -1;
}

JNIEXPORT jstring JNICALL
Java_org_grammaticalframework_pgf_Expr_unStr(JNIEnv* env, jobject self)
{
	PgfExpr expr = gu_variant_from_ptr(get_ref(env, self));

	PgfLiteralStr* pstr = pgf_expr_unlit(expr, PGF_LITERAL_STR);
	if (pstr != NULL) {
		return gu2j_string(env, pstr->val);
	}

	return NULL;
}

JNIEXPORT void JNICALL
Java_org_grammaticalframework_pgf_Expr_visit(JNIEnv* env, jobject self, jobject visitor)
{
	PgfExpr e = gu_variant_from_ptr(l2p(get_ref(env, self)));

	GuPool* tmp_pool = gu_local_pool();

	PgfApplication* app = pgf_expr_unapply(e, tmp_pool);
	if (app != NULL) {
		char* method_name = gu_malloc(tmp_pool, strlen(app->fun)+4);
		strcpy(method_name, "on_");
		strcat(method_name, app->fun);
		
		GuExn* err = gu_exn(tmp_pool);
		GuStringBuf* sbuf = gu_new_string_buf(tmp_pool);
		GuOut* out = gu_string_buf_out(sbuf);

		gu_putc('(', out, err);
		for (size_t i = 0; i < app->n_args; i++) {
			gu_puts("Lorg/grammaticalframework/pgf/Expr;", out, err);
		}
		gu_puts(")V", out, err);
		gu_putc('\0', out, err);

		char* sig = gu_string_buf_data(sbuf);

		jclass visitor_class = (*env)->GetObjectcls(env, visitor);
		jmethodID methodID = (*env)->GetMethodID(env, visitor_class, method_name, sig);
		
		if (methodID != NULL) {
			jclass expr_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/Expr");
			jmethodID expr_cid = (*env)->GetMethodID(env, expr_class, "<init>", "(Lorg/grammaticalframework/pgf/Pool;Ljava/lang/Object;J)V");

			jvalue* args = gu_malloc(tmp_pool, sizeof(jvalue)*app->n_args);
			for (size_t i = 0; i < app->n_args; i++) {
				args[i].l = (*env)->NewObject(env, expr_class, expr_cid, NULL, self, p2l(app->args[i]));
			}
			(*env)->CallVoidMethodA(env, visitor, methodID, args);
		} else {
			(*env)->ExceptionClear(env);

			methodID = (*env)->GetMethodID(env, visitor_class, "defaultCase", "(Lorg/grammaticalframework/pgf/Expr;)V");
			if (methodID != NULL) {
				(*env)->CallVoidMethod(env, visitor, methodID, self);
			} else {
				(*env)->ExceptionClear(env);
			}
		}
	}

	gu_pool_free(tmp_pool);
}

JNIEXPORT jboolean JNICALL
Java_org_grammaticalframework_pgf_Expr_equals(JNIEnv* env, jobject self, jobject other)
{
	jclass self_class  = (*env)->GetObjectcls(env, self);
	jclass other_class = (*env)->GetObjectcls(env, other);

	if (!(*env)->IsAssignableFrom(env, other_class, self_class))
		return JNI_FALSE;

	PgfExpr e_self  = gu_variant_from_ptr(l2p(get_ref(env, self)));
	PgfExpr e_other = gu_variant_from_ptr(l2p(get_ref(env, other)));
	
	if (pgf_expr_eq(e_self, e_other))
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

JNIEXPORT jint JNICALL
Java_org_grammaticalframework_pgf_Expr_hashCode(JNIEnv* env, jobject self)
{
	PgfExpr e = gu_variant_from_ptr(l2p(get_ref(env, self)));
	return pgf_expr_hash(0, e);
}

JNIEXPORT jint JNICALL
Java_org_grammaticalframework_pgf_Expr_size(JNIEnv* env, jobject self)
{
	PgfExpr e = gu_variant_from_ptr(l2p(get_ref(env, self)));
	return pgf_expr_size(e);
}

JNIEXPORT jobjectArray JNICALL
Java_org_grammaticalframework_pgf_Type_getHypos(JNIEnv* env, jobject self)
{
	PgfType* tp = get_ref(env, self);

	jclass hypo_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/Hypo");
	jmethodID cid = (*env)->GetMethodID(env, hypo_class, "<init>", "(Ljava/lang/Object;J)V");

	size_t n_hypos = gu_seq_length(tp->hypos);
	jobjectArray jhypos = (*env)->NewObjectArray(env, n_hypos, hypo_class, NULL);
	for (size_t i = 0; i < n_hypos; i++) {
		PgfHypo *hypo = gu_seq_index(tp->hypos, PgfHypo, i);
		jobject jhypo = (*env)->NewObject(env, hypo_class, cid, self, p2l(hypo));
		(*env)->SetObjectArrayElement(env, jhypos, i, jhypo);
		(*env)->DeleteLocalRef(env, jhypo);
	}
	return jhypos;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_Hypo_getType(JNIEnv* env, jobject self)
{
	PgfHypo* hypo = get_ref(env, self);

	jclass type_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/Type");
	jmethodID cid = (*env)->GetMethodID(env, type_class, "<init>", "(Ljava/lang/Object;J)V");
	jobject jtype = (*env)->NewObject(env, type_class, cid, self, p2l(hypo->type));

	return jtype;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_Generator_generateAll(JNIEnv* env, jclass clazz, jobject jpgf, jstring jstartCat)
{
	GuPool* pool = gu_new_pool();
	GuPool* out_pool = gu_new_pool();
    GuString startCat = j2gu_string(env, jstartCat, pool);
    GuExn* err = gu_exn(pool);

	PgfType* type = gu_new_flex(pool, PgfType, exprs, 0);
	type->hypos   = gu_empty_seq();
	type->cid     = startCat;
	type->n_exprs = 0;

	GuEnum* res =
		pgf_generate_all(get_ref(env, jpgf), type, err, pool, out_pool);
	if (res == NULL) {
		throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The generation failed");
		gu_pool_free(pool);
		return NULL;
	}

	jclass expiter_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/ExprIterator");
	jmethodID cid = (*env)->GetMethodID(env, expiter_class, "<init>", "(Lorg/grammaticalframework/pgf/PGF;JJJ)V");
	jobject jexpiter = (*env)->NewObject(env, expiter_class, cid, jpgf, p2l(pool), p2l(out_pool), p2l(res));

	return jexpiter;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_PGF_compute(JNIEnv* env, jobject self, jobject jexpr)
{
	GuPool* pool = gu_new_pool();
	GuPool* tmp_pool = gu_local_pool();
	GuExn* err = gu_exn(tmp_pool);

	PgfExpr res =
		pgf_compute(get_ref(env, self), gu_variant_from_ptr((void*) get_ref(env, jexpr)), err, tmp_pool, pool);
	if (!gu_ok(err)) {
		if (gu_exn_caught(err, PgfExn)) {
			GuString msg = (GuString) gu_exn_caught_data(err);
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
		} else {
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The expression cannot be computed");
		}
		gu_pool_free(tmp_pool);
		gu_pool_free(pool);
		return NULL;
	}

	gu_pool_free(tmp_pool);

	jclass pool_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/Pool");
	jmethodID pool_cid = (*env)->GetMethodID(env, pool_class, "<init>", "(J)V");
	jobject jpool = (*env)->NewObject(env, pool_class, pool_cid, p2l(pool));

	jclass expr_class  = (*env)->GetObjectcls(env, jexpr);
	jmethodID cid = (*env)->GetMethodID(env, expr_class, "<init>", "(Lorg/grammaticalframework/pgf/Pool;Ljava/lang/Object;J)V");
	jexpr = (*env)->NewObject(env, expr_class, cid, jpool, NULL, p2l(gu_variant_to_ptr(res)));

	return jexpr;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_PGF_inferExpr(JNIEnv* env, jobject self, jobject jexpr)
{
	GuPool* pool = gu_new_pool();
	GuPool* tmp_pool = gu_local_pool();
	GuExn* err = gu_exn(tmp_pool);

	PgfExpr expr =
		gu_variant_from_ptr((void*) get_ref(env, jexpr));
	PgfType* tp  =
		pgf_infer_expr(get_ref(env, self), &expr, err, pool);
	if (!gu_ok(err)) {
		if (gu_exn_caught(err, PgfExn)) {
			GuString msg = (GuString) gu_exn_caught_data(err);
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
		} else if (gu_exn_caught(err, PgfTypeError)) {
			GuString msg = (GuString) gu_exn_caught_data(err);
			throw_string_exception(env, "org/grammaticalframework/pgf/TypeError", msg);
 		} else {
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The type cannot be inferred");
		}
		gu_pool_free(tmp_pool);
		gu_pool_free(pool);
		return NULL;
	}

	gu_pool_free(tmp_pool);

	jclass pool_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/Pool");
	jmethodID pool_cid = (*env)->GetMethodID(env, pool_class, "<init>", "(J)V");
	jobject jpool = (*env)->NewObject(env, pool_class, pool_cid, p2l(pool));

	jclass expr_class  = (*env)->GetObjectcls(env, jexpr);
	jmethodID expr_cid = (*env)->GetMethodID(env, expr_class, "<init>", "(Lorg/grammaticalframework/pgf/Pool;Ljava/lang/Object;J)V");
	jexpr = (*env)->NewObject(env, expr_class, expr_cid, jpool, NULL, p2l(gu_variant_to_ptr(expr)));

	jclass type_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/Type");
	jmethodID type_cid = (*env)->GetMethodID(env, type_class, "<init>", "(Ljava/lang/Object;J)V");
	jobject jtype = (*env)->NewObject(env, type_class, type_cid, jpool, p2l(tp));
	
	jclass typed_expr_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/TypedExpr");
	jmethodID typed_expr_cid = (*env)->GetMethodID(env, typed_expr_class, "<init>", "(Lorg/grammaticalframework/pgf/Expr;Lorg/grammaticalframework/pgf/Type;)V");
	jobject jtyped_expr = (*env)->NewObject(env, typed_expr_class, typed_expr_cid, jexpr, jtype);

	return jtyped_expr;
}

JNIEXPORT jobject JNICALL
Java_org_grammaticalframework_pgf_PGF_checkExpr(JNIEnv* env, jobject self, jobject jexpr, jobject jtp)
{
	GuPool* pool = gu_new_pool();
	GuPool* tmp_pool = gu_local_pool();
	GuExn* err = gu_exn(tmp_pool);

	PgfExpr expr =
		gu_variant_from_ptr((void*) get_ref(env, jexpr));
	PgfType* tp  =
		get_ref(env, jtp);
	pgf_check_expr(get_ref(env, self), &expr, tp, err, pool);
	if (!gu_ok(err)) {
		if (gu_exn_caught(err, PgfExn)) {
			GuString msg = (GuString) gu_exn_caught_data(err);
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", msg);
		} else if (gu_exn_caught(err, PgfTypeError)) {
			GuString msg = (GuString) gu_exn_caught_data(err);
			throw_string_exception(env, "org/grammaticalframework/pgf/TypeError", msg);
 		} else {
			throw_string_exception(env, "org/grammaticalframework/pgf/PGFError", "The type cannot be inferred");
		}
		gu_pool_free(tmp_pool);
		gu_pool_free(pool);
		return NULL;
	}

	gu_pool_free(tmp_pool);

	jclass pool_class = (*env)->FindClass(env, "org/grammaticalframework/pgf/Pool");
	jmethodID pool_cid = (*env)->GetMethodID(env, pool_class, "<init>", "(J)V");
	jobject jpool = (*env)->NewObject(env, pool_class, pool_cid, p2l(pool));

	jclass expr_class  = (*env)->GetObjectcls(env, jexpr);
	jmethodID expr_cid = (*env)->GetMethodID(env, expr_class, "<init>", "(Lorg/grammaticalframework/pgf/Pool;Ljava/lang/Object;J)V");
	jexpr = (*env)->NewObject(env, expr_class, expr_cid, jpool, NULL, p2l(gu_variant_to_ptr(expr)));

	return jexpr;
}
*/