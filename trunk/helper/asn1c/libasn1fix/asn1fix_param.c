#include "asn1fix_internal.h"

typedef struct resolver_arg {
	asn1p_expr_t	  *(*resolver)(asn1p_expr_t *, void *arg);
	arg_t		  *arg;
	asn1p_expr_t	  *original_expr;
	asn1p_paramlist_t *lhs_params;
	asn1p_expr_t	  *rhs_pspecs;
} resolver_arg_t;

static asn1p_expr_t *resolve_expr(asn1p_expr_t *, void *resolver_arg);
static int compare_specializations(arg_t *, asn1p_expr_t *a, asn1p_expr_t *b);
static asn1p_expr_t *find_target_specialization_byref(resolver_arg_t *rarg, asn1p_ref_t *ref);
static asn1p_expr_t *find_target_specialization_bystr(resolver_arg_t *rarg, char *str);

asn1p_expr_t *
asn1f_parameterization_fork(arg_t *arg, asn1p_expr_t *expr, asn1p_expr_t *rhs_pspecs) {
	resolver_arg_t rarg;	/* resolver argument */
	asn1p_expr_t *exc;	/* expr clone */
	asn1p_expr_t *rpc;	/* rhs_pspecs clone */
	void *p;
	struct asn1p_pspec_s *pspec;
	int npspecs;

	assert(rhs_pspecs);
	assert(expr->lhs_params);
	assert(expr->parent_expr == 0);

	DEBUG("Forking parameterization at %d for %s (%d alr)",
		rhs_pspecs->_lineno, expr->Identifier,
		expr->specializations.pspecs_count);

	/*
	 * Find if this exact specialization has been used already.
	 */
	for(npspecs = 0;
		npspecs < expr->specializations.pspecs_count;
			npspecs++) {
		if(compare_specializations(arg, rhs_pspecs,
			expr->specializations.pspec[npspecs].rhs_pspecs) == 0) {
			DEBUG("Reused parameterization for %s",
				expr->Identifier);
			return expr->specializations.pspec[npspecs].my_clone;
		}
	}

	rarg.resolver = resolve_expr;
	rarg.arg = arg;
	rarg.original_expr = expr;
	rarg.lhs_params = expr->lhs_params;
	rarg.rhs_pspecs = rhs_pspecs;
	exc = asn1p_expr_clone_with_resolver(expr, resolve_expr, &rarg);
	rpc = asn1p_expr_clone(rhs_pspecs, 0);
	assert(exc && rpc);

	/*
	 * Create a new specialization.
	 */
	npspecs = expr->specializations.pspecs_count;
	p = realloc(expr->specializations.pspec,
			(npspecs + 1) * sizeof(expr->specializations.pspec[0]));
	assert(p);
	expr->specializations.pspec = p;
	pspec = &expr->specializations.pspec[npspecs];
	memset(pspec, 0, sizeof *pspec);

	pspec->rhs_pspecs = rpc;
	pspec->my_clone = exc;
	exc->spec_index = npspecs;

	DEBUG("Forked new parameterization for %s", expr->Identifier);

	/* Commit */
	expr->specializations.pspecs_count = npspecs + 1;
	return exc;
}

static int
compare_specializations(arg_t *arg, asn1p_expr_t *a, asn1p_expr_t *b) {
	asn1p_expr_t *ac = TQ_FIRST(&a->members);
	asn1p_expr_t *bc = TQ_FIRST(&b->members);

	for(;ac && bc; ac = TQ_NEXT(ac, next), bc = TQ_NEXT(bc, next)) {
	  retry:
		if(ac == bc) continue;
		if(ac->meta_type != bc->meta_type) break;
		if(ac->expr_type != bc->expr_type) break;

		if(!ac->reference && !bc->reference)
			continue;

		if(ac->reference) {
			ac = asn1f_lookup_symbol(arg,
				ac->module, ac->rhs_pspecs, ac->reference);
			if(!ac) break;
		}
		if(bc->reference) {
			bc = asn1f_lookup_symbol(arg,
				bc->module, bc->rhs_pspecs, bc->reference);
			if(!bc) break;
		}
	  goto retry;
	}

	if(ac || bc)
		/* Specializations do not match: different size option sets */
		return -1;

	return 0;
}

static asn1p_expr_t *
resolve_expr(asn1p_expr_t *expr_to_resolve, void *resolver_arg) {
	resolver_arg_t *rarg = resolver_arg;
	arg_t *arg = rarg->arg;
	asn1p_expr_t *expr;
	asn1p_expr_t *nex;

	DEBUG("Resolving %s (meta %d)",
		expr_to_resolve->Identifier, expr_to_resolve->meta_type);

	if(expr_to_resolve->meta_type == AMT_TYPEREF) {
		expr = find_target_specialization_byref(rarg,
				expr_to_resolve->reference);
		if(!expr) return NULL;
	} else if(expr_to_resolve->meta_type == AMT_VALUE) {
		assert(expr_to_resolve->value);
		expr = find_target_specialization_bystr(rarg,
				expr_to_resolve->Identifier);
		if(!expr) return NULL;
	} else {
		errno = ESRCH;
		return NULL;
	}

	DEBUG("Found target %s (%d/%x)",
		expr->Identifier, expr->meta_type, expr->expr_type);
	if(expr->meta_type == AMT_TYPE
	|| expr->meta_type == AMT_VALUE
	|| expr->meta_type == AMT_VALUESET) {
		DEBUG("Target is a simple type %s",
			ASN_EXPR_TYPE2STR(expr->expr_type));
		nex = asn1p_expr_clone(expr, 0);
		free(nex->Identifier);
		nex->Identifier = expr_to_resolve->Identifier
			? strdup(expr_to_resolve->Identifier) : 0;
		return nex;
	} else {
		FATAL("Feature not implemented for %s (%d/%x), "
			"please contact the asn1c author",
			rarg->original_expr->Identifier,
			expr->meta_type, expr->expr_type);
		errno = EPERM;
		return NULL;
	}

	return NULL;
}

static asn1p_expr_t *
find_target_specialization_byref(resolver_arg_t *rarg, asn1p_ref_t *ref) {
	char *refstr;

	if(!ref || ref->comp_count != 1) {
		errno = ESRCH;
		return NULL;
	}

	refstr = ref->components[0].name;	/* T */

	return find_target_specialization_bystr(rarg, refstr);
}

static asn1p_expr_t *
find_target_specialization_bystr(resolver_arg_t *rarg, char *refstr) {
	arg_t *arg = rarg->arg;
	asn1p_expr_t *target;
	int i;

	target = TQ_FIRST(&rarg->rhs_pspecs->members);
	for(i = 0; i < rarg->lhs_params->params_count;
			i++, target = TQ_NEXT(target, next)) {
		struct asn1p_param_s *param = &rarg->lhs_params->params[i];
		if(!target) break;

		if(strcmp(param->argument, refstr))
			continue;

		return target;
	}
	if(i != rarg->lhs_params->params_count) {
		FATAL("Parameterization of %s failed: "
			"parameters number mismatch",
				rarg->original_expr->Identifier);
		errno = EPERM;
		return NULL;
	}

	errno = ESRCH;
	return NULL;
}
