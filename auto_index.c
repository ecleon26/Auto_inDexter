@@ -0,0 +1,43 @@
#include "postgres.h"
#include "fmgr.h"
#include "utils/elog.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

static emit_log_hook_type prev_log_hook = NULL;

static void
mc_log_hook(ErrorData *edata)
{
    if (edata->elevel >= ERROR &&
        edata->sqlerrcode == ERRCODE_SYNTAX_ERROR)
    {
        edata->message = pstrdup("mc");
    }

    if (prev_log_hook)
        prev_log_hook(edata);
}

void
_PG_init(void)
{
    prev_log_hook = emit_log_hook;
    emit_log_hook = mc_log_hook;
}

void
_PG_fini(void)
{
    emit_log_hook = prev_log_hook;
}

PG_FUNCTION_INFO_V1(mc_dummy);

Datum
mc_dummy(PG_FUNCTION_ARGS)
{
    PG_RETURN_VOID();
}