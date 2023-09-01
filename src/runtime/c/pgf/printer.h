#ifndef PRINTER_H
#define PRINTER_H

class PGF_INTERNAL_DECL PgfPrinter : public PgfUnmarshaller {
    // List of free variables in order reverse to the order of binding
    PgfPrintContext *ctxt;

    // Each lambda abstraction is a separate object, but when we print
    // them we want to colapse them into one abstraction with
    // several variables. For that reason, if we are in the process
    // of printing nested lambda abstractions, printing_lambdas is true
    // and btype is the binding type for the last variable.
    bool printing_lambdas;
    PgfBindType btype;

    // This method should be called before printing any other form of
    // expression but a lambda. In this way the printing of a chain
    // of lambda expressions is finished.
    void flush_lambdas();

    // The current operator priority
    int prio;

    // The generated text
    PgfText *res;

    // The marshaller for pattern matching
    PgfMarshaller *m;

    void bindings(PgfPrintContext *context, size_t n_vars);

public:
    PgfPrinter(PgfPrintContext *context, int priority,
               PgfMarshaller *marshaller);

    ~PgfPrinter() { free(res); }

    // Push a new variable in the printing context. If the name
    // collides with an existing variable, the variable is renamed
    // by adding a number.
    void push_variable(PgfText *name);

    // Pop the last variable name from the context.
    void pop_variable();

    void puts(PgfText *s);
    void puts(const char *s);

    // buf_size is the expected buffer size. If larger is needed,
    // it will be allocated automatically.
#if defined(_MSC_VER)
#if _MSC_VER >= 1400
# include <sal.h>
# if _MSC_VER > 1400
    void nprintf(size_t buf_size, _Printf_format_string_ const char *format, ...);
# else
    void nprintf(size_t buf_size, __format_string const char *format, ...);
# endif /* FORMAT_STRING */
#else
    void nprintf(size_t buf_size, const char *format, ...);
#endif /* _MSC_VER */
#else
    void nprintf(size_t buf_size, const char *format, ...) __attribute__ ((format (printf, 3, 4)));
#endif

    PgfText *get_text();

    void dump() {
        PgfText *text = get_text();
        fprintf(stderr, "%.*s", (int) text->size, text->text);
        free(text);
    };

    void hypo(PgfTypeHypo *hypo, int prio);

    void parg(ref<PgfDTyp> ty, ref<PgfPArg> parg);
    void lvar(size_t var);
    void lparam(ref<PgfLParam> lparam);
    void lvar_ranges(ref<Vector<PgfVariableRange>> vars);
    void seq_id(PgfPhrasetableIds *seq_ids, ref<PgfSequence> seq);
    void symbol(PgfSymbol sym);
    void sequence(ref<PgfSequence> seq);

    virtual PgfExpr eabs(PgfBindType btype, PgfText *name, PgfExpr body);
    virtual PgfExpr eapp(PgfExpr fun, PgfExpr arg);
    virtual PgfExpr elit(PgfLiteral lit);
    virtual PgfExpr emeta(PgfMetaId meta);
    virtual PgfExpr efun(PgfText *name);
    virtual PgfExpr evar(int index);
    virtual PgfExpr etyped(PgfExpr expr, PgfType typ);
    virtual PgfExpr eimplarg(PgfExpr expr);
    virtual PgfLiteral lint(size_t size, uintmax_t *v);
    virtual PgfLiteral lflt(double v);
    virtual PgfLiteral lstr(PgfText *v);
    virtual PgfType dtyp(size_t n_hypos, PgfTypeHypo *hypos,
                         PgfText *cat,
                         size_t n_exprs, PgfExpr *exprs);
    virtual void free_ref(object x);

    void bindings(size_t n_vars) {
        bindings(ctxt,n_vars);
    }
};

#endif
