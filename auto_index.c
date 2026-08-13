#include "postgres.h"
#include "fmgr.h"
#include "optimizer/planner.h"
#include "executor/spi.h"
#include "utils/elog.h"

#include "auto_index.h"

#ifdef PG_MODULE_MAGIC

#endif
planner_hook_type prev_planner_hook = NULL;

static PlannedStmt *
auto_index_planner_hook(Query *parse, const char *query_string, int cursorOptions, ParamListInfo boundParams)
{

    PlannedStmt *stmt;
    const char *sql;

    if (prev_planner_hook && prev_planner_hook != auto_index_planner_hook)
        stmt = prev_planner_hook(parse, query_string, cursorOptions, boundParams);
    else
        stmt = standard_planner(parse, query_string, cursorOptions, boundParams);

    sql = "INSERT INTO aidx_queries (tablename, colname, cost, benefit) "
          "VALUES ('dummy_table', 'col1', 50.0, 10.0) "
          "ON CONFLICT DO NOTHING;";

    if (SPI_connect() == SPI_OK_CONNECT)
    {
        SPI_exec(sql, 0);
        SPI_finish();
    }
    else
    {
        elog(WARNING, "AutoIndex: SPI_connect failed");
    }

    return stmt;
}
void
_PG_init(void)
{
    elog(LOG, "AutoIndex: installing planner hook");
    if (planner_hook != auto_index_planner_hook)
    {
        prev_planner_hook = planner_hook;
        planner_hook = auto_index_planner_hook;
    }
}

void
_PG_fini(void)
{
   
    if (planner_hook == auto_index_planner_hook)
        planner_hook = prev_planner_hook;
}