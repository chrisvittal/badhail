#include "hail.h"

#include <stdlib.h>
#include <string.h>

struct _hail_tvoid tvoid = { .ty={.tag=HAIL_TY_VOID} };
struct _hail_tbool tbool = { .ty={.tag=HAIL_TY_BOOL} };
struct _hail_tint32 tint32 = { .ty={.tag=HAIL_TY_INT32} };
struct _hail_tint64 tint64 = { .ty={.tag=HAIL_TY_INT64} };
struct _hail_tfloat32 tfloat32 = { .ty={.tag=HAIL_TY_FLOAT32} };
struct _hail_tfloat64 tfloat64 = { .ty={.tag=HAIL_TY_FLOAT64} };
struct _hail_tstr tstr = { .ty={.tag=HAIL_TY_STR} };
struct _hail_tcall tcall = { .ty={.tag=HAIL_TY_CALL} };

struct hail_type *hail_tvoid = &tvoid.ty;
struct hail_type *hail_tbool = &tbool.ty;
struct hail_type *hail_tint32 = &tint32.ty;
struct hail_type *hail_tint64 = &tint64.ty;
struct hail_type *hail_tfloat32 = &tfloat32.ty;
struct hail_type *hail_tfloat64 = &tfloat64.ty;
struct hail_type *hail_tstr = &tstr.ty;
struct hail_type *hail_tcall = &tcall.ty;
