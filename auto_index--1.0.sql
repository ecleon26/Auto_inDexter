// Updated auto_index.c with debug logs
#include "postgres.h"
#include "fmgr.h"
#include "optimizer/planner.h"
#include "executor/spi.h"
#include "utils/elog.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
#include "catalog/pg_class.h"
#include "math.h"

#include "nodes/nodeFuncs.h"
#include "parser/parsetree.h"

#include "auto_index.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

planner_hook_type prev_planner_hook = NULL;

static char *
extract_column_name(Node *qual, Index varno)
{
    if (!qual || !IsA(qual, OpExpr))
        return NULL;

    OpExpr *opexpr = (OpExpr *) qual;
    if (list_length(opexpr->args) < 2)
        return NULL;

    Node *left = linitial(opexpr->args);

    if (IsA(left, Var))
    {
        Var *var = (Var *) left;
        if (var->varno == varno)
        {
            return get_attname(var->varno, var->varattno, false);
        }
    }

    return NULL;
}

static PlannedStmt *
auto_index_planner_hook(Query *parse, const char *query_string, int cursorOptions, ParamListInfo boundParams)
{
    elog(LOG, "AutoIndex: planner hook triggered");

    PlannedStmt *stmt;
 

    if (prev_planner_hook && prev_planner_hook != auto_index_planner_hook)
        stmt = prev_planner_hook(parse, query_string, cursorOptions, boundParams);
    else
        stmt = standard_planner(parse, query_string, cursorOptions, boundParams);

    
    Plan *plan = stmt->planTree;

    if (IsA(plan, SeqScan))
    {
       
        Scan *scan = (Scan *) plan;
        Index scanrelid = scan->scanrelid;
        RangeTblEntry *rte = rt_fetch(scanrelid, parse->rtable);
)
        Relation rel = relation_open(rte->relid, AccessShareLock);
        const char *relname = RelationGetRelationName(rel);

        double num_rows = rel->rd_rel->reltuples;
        double cost = (num_rows > 0) ? num_rows * log2(num_rows) : 1000;
        double benefit = plan->total_cost;

        relation_close(rel, AccessShareLock);

        char *colname = extract_column_name(parse->jointree->quals, scanrelid);
        if (!colname)
            colname = "unknown_col";

        char sql[1024];
        snprintf(sql, sizeof(sql),
            "INSERT INTO aidx_queries (tablename, colname, cost, benefit, num_queries) "
            "VALUES ('%s', '%s', %f, %f, 1) "
            "ON CONFLICT (tablename, colname) DO UPDATE "
            "SET benefit = aidx_queries.benefit + %f, "
            "cost = %f, "
            "num_queries = aidx_queries.num_queries + 1;",
            relname, colname, cost, benefit, benefit, cost);

        elog(LOG, "AutoIndex generated SQL: %s", sql);

        if (SPI_connect() == SPI_OK_CONNECT)
        {
            int ret = SPI_exec(sql, 0);
            if (ret != SPI_OK_INSERT && ret != SPI_OK_UPDATE)
            {
                elog(WARNING, "AutoIndex: SPI_exec failed with code %d", ret);
            }
            SPI_finish();
        }
        else
        {
            elog(WARNING, "AutoIndex: SPI_connect failed");
        }
    }

    return stmt;
}

void
_PG_init(void)
{
@@ -55,10 +120,9 @@ _PG_init(void)
    }
}

void
_PG_fini(void)
{
    if (planner_hook == auto_index_planner_hook)
        planner_hook = prev_planner_hook;
}
