#ifndef AUTO_INDEX_H
#define AUTO_INDEX_H

#include "postgres.h"
#include "postmaster/interrupt.h"
#include "storage/lwlock.h"

#include "access/xact.h"
#include "pgstat.h"
#include "utils/acl.h"
#include "utils/snapmgr.h"

#include "fmgr.h"

#include "miscadmin.h"
#include "storage/ipc.h"
#include "storage/latch.h"
#include "storage/proc.h"
#include "postmaster/bgworker.h"

#include "executor/spi.h"
#include "commands/dbcommands.h"
#include "utils/builtins.h"
#include "utils/elog.h"
#include "utils/memutils.h"
#include "utils/lsyscache.h"
#include "tcop/utility.h"

#include "lib/stringinfo.h"
#include "optimizer/planner.h"


extern planner_hook_type prev_planner_hook;

static PlannedStmt *auto_index_planner_hook(Query *parse, const char *query_string, int cursorOptions, ParamListInfo boundParams);

PGDLLEXPORT void auto_index_worker_main(Datum main_arg);
void _PG_fini(void);
void start_auto_index_worker(void);

#endif