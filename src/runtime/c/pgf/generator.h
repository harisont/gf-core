#ifndef GENERATOR_H
#define GENERATOR_H

class PGF_INTERNAL_DECL PgfGenerator : public PgfUnmarshaller
{
protected:
    constexpr static prob_t VAR_PROB = 0.1;

    struct Scope {

        Scope *next;
        PgfType type;
        PgfMarshaller *m;
        PgfBindType bind_type;
        PgfText var;
    };

    ref<PgfPGF> pgf;
    size_t depth;
    PgfMarshaller *m;
    PgfInternalMarshaller i_m;
    PgfUnmarshaller *u;
    std::vector<ref<PgfConcr>> concrs;

    PgfGenerator(ref<PgfPGF> pgf,
                 size_t depth,
                 PgfMarshaller *m, PgfUnmarshaller *u)
    {
        this->pgf = pgf;
        this->depth = depth;
        this->m = m;
        this->u = u;
    }

    bool function_has_lins(PgfText *);

public:
    void addConcr(ref<PgfConcr> concr);

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
};

class PGF_INTERNAL_DECL PgfRandomGenerator : public PgfGenerator 
{
    uint64_t *seed;
    prob_t prob;

    Scope *scope;
    size_t scope_len;

    prob_t rand() {
        *seed = *seed * 1103515245 + 12345;
        return (prob_t)((*seed/65536) % 32768)/32768;
    }

public:
    PgfRandomGenerator(ref<PgfPGF> pgf, size_t depth, uint64_t *seed,
                       PgfMarshaller *m, PgfUnmarshaller *u);
    prob_t getProb() { return prob; }
    ~PgfRandomGenerator();

    PgfExpr descend(PgfExpr expr, size_t n_hypos, PgfTypeHypo *hypos);

    virtual PgfType dtyp(size_t n_hypos, PgfTypeHypo *hypos,
                         PgfText *cat,
                         size_t n_exprs, PgfExpr *exprs);
};

class PGF_INTERNAL_DECL PgfExhaustiveGenerator : public PgfGenerator, public PgfExprEnum
{
    struct Result;

    Result *top_res;
    size_t top_res_index;

    struct State {
        Result *res;
        prob_t prob;
        virtual bool process(PgfExhaustiveGenerator *gen) = 0;
        virtual void free_refs(PgfUnmarshaller *u);
        static void release(State *state, PgfUnmarshaller *u);
    };

    struct State0 : State {
        PgfProbspace space;
        virtual bool process(PgfExhaustiveGenerator *gen);
    };

    struct ExprInstance {
        PgfExpr expr;
        prob_t prob;
        size_t depth;

        ExprInstance(PgfExpr expr, prob_t prob, size_t depth) {
            this->expr = expr;
            this->prob = prob;
            this->depth = depth;
        }
    };

    struct State1 : State {
        ref<PgfDTyp> type;
        size_t n_args;
        PgfExpr expr;
        size_t depth;

        virtual bool process(PgfExhaustiveGenerator *gen);
        virtual void free_refs(PgfUnmarshaller *u);
        void combine(PgfExhaustiveGenerator *gen, 
                     Scope *scope, ExprInstance &p);
        void complete(PgfExhaustiveGenerator *gen);
    };

    typedef std::pair<ref<PgfText>,Scope*> Goal;

    class CompareGoal : public std::less<Goal> {
    public:
        bool operator() (const Goal &g1, const Goal &g2) const {
            int cmp = textcmp(g1.first,g2.first);
            if (cmp < 0)
                return true;
            else if (cmp > 0)
                return false;

            return (g1.second < g2.second);
        }
    };

    struct Result {
        size_t ref_count;
        Scope *scope;
        size_t scope_len;
        std::vector<State1*> states;
        std::vector<ExprInstance> exprs;

        Result() {
            this->ref_count = 0;
            this->scope     = NULL;
            this->scope_len = 0;
        }

        Result(Scope *scope, size_t scope_len) {
            this->ref_count = 0;
            this->scope     = scope;
            this->scope_len = scope_len;
        }
    };

    class CompareState : public std::less<State*> {
    public:
        bool operator() (const State* s1, const State* s2) const {
            return s1->prob > s2->prob;
        }
    };

    std::map<Goal, Result*, CompareGoal> results;
    std::priority_queue<State*, std::vector<State*>, CompareState> queue;
    std::vector<Scope*> scopes;

    void predict_literal(ref<PgfText> cat, Result *res);
    void push_left_states(PgfProbspace space, PgfText *cat, Result *res, prob_t outside_prob);

public:
    PgfExhaustiveGenerator(ref<PgfPGF> pgf, size_t depth,
                           PgfMarshaller *m, PgfUnmarshaller *u);
    virtual ~PgfExhaustiveGenerator();

    virtual PgfType dtyp(size_t n_hypos, PgfTypeHypo *hypos,
                         PgfText *cat,
                         size_t n_exprs, PgfExpr *exprs);

    virtual PgfExpr fetch(PgfDB *db, prob_t *prob);
};

#endif
