#ifndef HAIL_H
#define HAIL_H
#include <stdbool.h>

enum _hail_type_tag {
	HAIL_TY_VOID,
	HAIL_TY_BOOL,
	HAIL_TY_INT32,
	HAIL_TY_INT64,
	HAIL_TY_FLOAT32,
	HAIL_TY_FLOAT64,
	HAIL_TY_STR,
	HAIL_TY_CALL,
	HAIL_TY_ARRAY,
	HAIL_TY_TUPLE,
};

struct hail_type {
	enum _hail_type_tag tag;
};

#define basic_type(name) struct name { struct hail_type ty; }
basic_type(_hail_tvoid);
basic_type(_hail_tbool);
basic_type(_hail_tint32);
basic_type(_hail_tint64);
basic_type(_hail_tfloat32);
basic_type(_hail_tfloat64);
basic_type(_hail_tstr);
basic_type(_hail_tcall);
#undef basic_type

extern struct hail_type *hail_tvoid;
extern struct hail_type *hail_tbool;
extern struct hail_type *hail_tint32;
extern struct hail_type *hail_tint64;
extern struct hail_type *hail_tfloat32;
extern struct hail_type *hail_tfloat64;
extern struct hail_type *hail_tstr;
extern struct hail_type *hail_tcall;

struct hail_tarray {
	struct hail_type ty;
	struct hail_type *element_ty;
};

struct hail_ttuple {
	struct hail_type ty;
	int len;
	struct hail_type *types[];
};

#endif /* HAIL_H */
