#include <math.h>
#include "data.h"
#include "writer.h"

PgfWriter::PgfWriter(PgfText **langs, FILE *out)
{
    this->out = out;
    this->langs = langs;
    this->abstract = 0;
}

void PgfWriter::write_uint8(uint8_t b)
{
    size_t n_items = fwrite(&b, sizeof(b), 1, out);
    if (ferror(out))
        throw pgf_error("an error occured while writing out the grammar");
    if (n_items != 1)
        throw pgf_error("couldn't write to the output file");
}

void PgfWriter::write_u16be(uint16_t u)
{
    uint8_t buf[2] = { (uint8_t) ((u>>8) & 0xFF)
                     , (uint8_t) (u & 0xFF)
                     };

    size_t n_items = fwrite(&buf, sizeof(buf), 1, out);
    if (ferror(out))
        throw pgf_error("an error occured while writing out the grammar");
    if (n_items != 1)
        throw pgf_error("couldn't write to the output file");
}

void PgfWriter::write_u64be(uint64_t u)
{
    uint8_t buf[8] = { (uint8_t) ((u>>56) & 0xFF)
                     , (uint8_t) ((u>>48) & 0xFF)
                     , (uint8_t) ((u>>40) & 0xFF)
                     , (uint8_t) ((u>>32) & 0xFF)
                     , (uint8_t) ((u>>24) & 0xFF)
                     , (uint8_t) ((u>>16) & 0xFF)
                     , (uint8_t) ((u>>8)  & 0xFF)
                     , (uint8_t) (u & 0xFF)
                     };

    size_t n_items = fwrite(&buf, sizeof(buf), 1, out);
    if (ferror(out))
        throw pgf_error("an error occured while writing out the grammar");
    if (n_items != 1)
        throw pgf_error("couldn't write to the output file");
}

void PgfWriter::write_double(double d)
{
   	int sign = signbit(d) > 0;
	unsigned rawexp;
	uint64_t mantissa;

	switch (::fpclassify(d)) {
	case FP_NAN:
		rawexp   = 0x7ff;
		mantissa = 1;
		break;
	case FP_INFINITE:
		rawexp   = 0x7ff;
		mantissa = 0;
		break;
	default: {
		int exp;
		mantissa = (uint64_t) scalbn(frexp(d, &exp), 53);
		mantissa &= ~ (1ULL << 52);
		exp -= 53;

		rawexp = exp + 1075;
    }
    }

    uint64_t u = (((uint64_t) sign) << 63) |
                 (((uint64_t) rawexp & 0x7ff) << 52) |
                 mantissa;

	write_u64be(u);
}

void PgfWriter::write_uint(uint64_t u)
{
    for (;;) {
		uint8_t b = u & 0x7F;
		u = u >> 7;
		if (u == 0) {
            size_t n_items = fwrite(&b, sizeof(b), 1, out);
            if (ferror(out))
                throw pgf_error("an error occured while writing out the grammar");
            if (n_items != 1)
                throw pgf_error("couldn't write to the output file");

			break;
		} else {
            b = b | 0x80;

            size_t n_items = fwrite(&b, sizeof(b), 1, out);
            if (ferror(out))
                throw pgf_error("an error occured while writing out the grammar");
            if (n_items != 1)
                throw pgf_error("couldn't write to the output file");
		}
	}
}

void PgfWriter::write_text(PgfText *text)
{
	write_len(text->size);
    size_t n_items = fwrite(&text->text, text->size, 1, out);
    if (ferror(out))
        throw pgf_error("an error occured while writing out the grammar");
    if (text->size != 0 && n_items != 1)
        throw pgf_error("couldn't write to the output file");
}

template<class V>
void PgfWriter::write_namespace(Namespace<V> nmsp, void (PgfWriter::*write_value)(ref<V>))
{
    write_len(namespace_size(nmsp));
    write_namespace_helper(nmsp, write_value);
}

template<class V>
void PgfWriter::write_namespace_helper(Namespace<V> nmsp, void (PgfWriter::*write_value)(ref<V>))
{
    if (nmsp == 0)
        return;

    write_namespace_helper(nmsp->left, write_value);
    (this->*write_value)(nmsp->value);
    write_namespace_helper(nmsp->right, write_value);
}

template<class V>
void PgfWriter::write_vector(ref<Vector<V>> vec, void (PgfWriter::*write_value)(ref<V> val))
{
    write_len(vec->len);
    for (size_t i = 0; i < vec->len; i++) {
        (this->*write_value)(vector_elem<V>(vec,i));
    }
}

void PgfWriter::write_literal(PgfLiteral literal)
{
    auto tag = ref<PgfLiteral>::get_tag(literal);
    write_tag(tag);

    switch (tag) {
	case PgfLiteralInt::tag: {
        auto lint = ref<PgfLiteralInt>::untagged(literal);
        write_len(lint->size);
        for (size_t i = 0; i < lint->size; i++) {
            write_uint(lint->val[i]);
        }
		break;
	}
	case PgfLiteralStr::tag: {
		auto lstr = ref<PgfLiteralStr>::untagged(literal);
        write_text(&lstr->val);
		break;
	}
	case PgfLiteralFlt::tag: {
		auto lflt = ref<PgfLiteralFlt>::untagged(literal);
        write_double(lflt->val);
		break;
	}
	default:
		throw pgf_error("Unknown literal tag");
	}
}

void PgfWriter::write_expr(PgfExpr expr)
{
    auto tag = ref<PgfExpr>::get_tag(expr);
    write_tag(tag);

    switch (tag) {
    case PgfExprAbs::tag: {
        auto eabs = ref<PgfExprAbs>::untagged(expr);
        write_tag(eabs->bind_type);
        write_name(&eabs->name);
        write_expr(eabs->body);
        break;
    }
    case PgfExprApp::tag: {
        auto eapp = ref<PgfExprApp>::untagged(expr);
        write_expr(eapp->fun);
        write_expr(eapp->arg);
        break;
    }
    case PgfExprLit::tag: {
        auto elit = ref<PgfExprLit>::untagged(expr);
        write_literal(elit->lit);
        break;
    }
    case PgfExprMeta::tag: {
        write_int(ref<PgfExprMeta>::untagged(expr)->id);
        break;
    }
	case PgfExprFun::tag: {
        write_name(&ref<PgfExprFun>::untagged(expr)->name);
        break;
    }
	case PgfExprVar::tag: {
        write_int(ref<PgfExprVar>::untagged(expr)->var);
        break;
	}
	case PgfExprTyped::tag: {
        auto etyped = ref<PgfExprTyped>::untagged(expr);
        write_expr(etyped->expr);
        write_type(etyped->type);
        break;
	}
	case PgfExprImplArg::tag: {
        write_expr(ref<PgfExprImplArg>::untagged(expr)->expr);
        break;
	}
	default:
		throw pgf_error("Unknown expression tag");
    }
}

void PgfWriter::write_hypo(ref<PgfHypo> hypo)
{
    write_tag(hypo->bind_type);
	write_name(hypo->cid);
	write_type(hypo->type);
}

void PgfWriter::write_type(ref<PgfDTyp> ty)
{
    write_vector<PgfHypo>(ty->hypos, &PgfWriter::write_hypo);
    write_name(&ty->name);
    write_vector<PgfExpr>(ty->exprs, &PgfWriter::write_expr);
}

void PgfWriter::write_flag(ref<PgfFlag> flag)
{
    write_name(&flag->name);
    write_literal(flag->value);
}

void PgfWriter::write_absfun(ref<PgfAbsFun> absfun)
{
    write_name(&absfun->name);
    write_type(absfun->type);
    write_int(absfun->arity);
    if (absfun->bytecode == 0)
        write_tag(0);
    else {
        write_tag(1);
        write_len(0);
    }
    write_double(expf(-absfun->prob));
}

void PgfWriter::write_abscat(ref<PgfAbsCat> abscat)
{
    write_name(&abscat->name);
    write_vector(abscat->context, &PgfWriter::write_hypo);	
	write_double(expf(-abscat->prob));
}

void PgfWriter::write_abstract(ref<PgfAbstr> abstract)
{
    this->abstract = abstract;

    write_name(abstract->name);
    write_namespace<PgfFlag>(abstract->aflags, &PgfWriter::write_flag);
    write_namespace<PgfAbsFun>(abstract->funs, &PgfWriter::write_absfun);
    write_namespace<PgfAbsCat>(abstract->cats, &PgfWriter::write_abscat);

    this->abstract = 0;
}

void PgfWriter::write_variable_range(ref<PgfVariableRange> var)
{
    write_int(var->var);
    write_int(var->range);
}

void PgfWriter::write_lparam(ref<PgfLParam> lparam)
{
    write_int(lparam->i0);
    write_len(lparam->n_terms);
    for (size_t i = 0; i < lparam->n_terms; i++) {
        write_int(lparam->terms[i].factor);
        write_int(lparam->terms[i].var);
    }
}

void PgfWriter::write_parg(ref<PgfPArg> parg)
{
    write_lparam(parg->param);
}

void PgfWriter::write_presult(ref<PgfPResult> pres)
{
    if (pres->vars != 0)
        write_vector(pres->vars, &PgfWriter::write_variable_range);
    else
        write_len(0);
    write_lparam(ref<PgfLParam>::from_ptr(&pres->param));
}

void PgfWriter::write_symbol(PgfSymbol sym)
{
    auto tag = ref<PgfSymbol>::get_tag(sym);
    write_tag(tag);

    switch (tag) {
	case PgfSymbolCat::tag: {
        auto sym_cat = ref<PgfSymbolCat>::untagged(sym);
        write_int(sym_cat->d);
        write_lparam(ref<PgfLParam>::from_ptr(&sym_cat->r));
		break;
	}
	case PgfSymbolLit::tag: {
        auto sym_lit = ref<PgfSymbolLit>::untagged(sym);
        write_int(sym_lit->d);
        write_lparam(ref<PgfLParam>::from_ptr(&sym_lit->r));
		break;
	}
	case PgfSymbolVar::tag: {
        auto sym_var = ref<PgfSymbolVar>::untagged(sym);
        write_int(sym_var->d);
        write_int(sym_var->r);
		break;
	}
	case PgfSymbolKS::tag: {
        auto sym_ks = ref<PgfSymbolKS>::untagged(sym);
        write_text(&sym_ks->token);
		break;
	}
	case PgfSymbolKP::tag: {
        auto sym_kp = ref<PgfSymbolKP>::untagged(sym);
        write_len(sym_kp->alts.len);
        for (size_t i = 0; i < sym_kp->alts.len; i++) {
			PgfAlternative *alt = vector_elem(&sym_kp->alts, i);
            write_vector(ref<Vector<PgfSymbol>>::from_ptr(&alt->form->syms), &PgfWriter::write_symbol);
            write_vector(alt->prefixes, &PgfWriter::write_text);
        }
        write_vector(ref<Vector<PgfSymbol>>::from_ptr(&sym_kp->default_form->syms), &PgfWriter::write_symbol);
		break;
	}
	case PgfSymbolBIND::tag:
	case PgfSymbolSOFTBIND::tag:
	case PgfSymbolNE::tag:
	case PgfSymbolSOFTSPACE::tag:
	case PgfSymbolCAPIT::tag:
	case PgfSymbolALLCAPIT::tag:
		break;
	default:
		throw pgf_error("Unknown symbol tag");
	}
}

void PgfWriter::write_seq(ref<PgfSequence> seq)
{
	seq_ids.add(seq);
    write_vector(ref<Vector<PgfSymbol>>::from_ptr(&seq->syms), &PgfWriter::write_symbol);
}

void PgfWriter::write_phrasetable(PgfPhrasetable table)
{
    write_len(phrasetable_size(table));
    write_phrasetable_helper(table);
}

void PgfWriter::write_phrasetable_helper(PgfPhrasetable table)
{
    if (table == 0)
        return;

    write_phrasetable_helper(table->left);
    write_seq(table->value.seq);
    write_phrasetable_helper(table->right);
}

void PgfWriter::write_lincat(ref<PgfConcrLincat> lincat)
{
    write_name(&lincat->name);
    write_vector(lincat->fields, &PgfWriter::write_lincat_field);
    write_len(lincat->n_lindefs);
    write_vector(lincat->args, &PgfWriter::write_parg);
    write_vector(lincat->res, &PgfWriter::write_presult);
    write_vector(lincat->seqs, &PgfWriter::write_seq_id);
}

void PgfWriter::write_lincat_field(ref<ref<PgfText>> field)
{
    write_text(*field);
}

void PgfWriter::write_lin(ref<PgfConcrLin> lin)
{
    write_name(&lin->name);
    write_vector(lin->args, &PgfWriter::write_parg);
    write_vector(lin->res, &PgfWriter::write_presult);
    write_vector(lin->seqs, &PgfWriter::write_seq_id);
}

void PgfWriter::write_printname(ref<PgfConcrPrintname> printname)
{
    write_name(&printname->name);
    write_text(printname->printname);
}

void PgfWriter::write_concrete(ref<PgfConcr> concr)
{
    if (langs != NULL) {
        bool found = false;
        PgfText** p = langs;
        while (*p) {
            if (textcmp(*p, &concr->name) == 0) {
                found = true;
                break;
            }
            p++;
        }

        if (!found) {
            return;
        }
    }

	seq_ids.start(concr);

    write_name(&concr->name);
    write_namespace<PgfFlag>(concr->cflags, &PgfWriter::write_flag);
	write_phrasetable(concr->phrasetable);
    write_namespace<PgfConcrLincat>(concr->lincats, &PgfWriter::write_lincat);
	write_namespace<PgfConcrLin>(concr->lins, &PgfWriter::write_lin);
	write_namespace<PgfConcrPrintname>(concr->printnames, &PgfWriter::write_printname);

	seq_ids.end();
}

void PgfWriter::write_pgf(ref<PgfPGF> pgf)
{
    write_u16be(pgf->major_version);
    write_u16be(pgf->minor_version);

    write_namespace<PgfFlag>(pgf->gflags, &PgfWriter::write_flag);

    write_abstract(ref<PgfAbstr>::from_ptr(&pgf->abstract));

    if (langs == NULL)
        write_len(namespace_size(pgf->concretes));
    else {
        size_t len = 0;
        PgfText** p = langs;
        while (*p) {
            len++; p++;
        }
        write_len(len);
    }
    write_namespace_helper<PgfConcr>(pgf->concretes, &PgfWriter::write_concrete);
}
