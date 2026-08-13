#ifndef AUTO_INDEX_H
#define AUTO_INDEX_H

#include "postgres.h"


#ifdef BUILDING_AUTO_INDEX_MAIN
#include "optimizer/planner.h"
#include "nodes/plannodes.h"
#include "tcop/tcopprot.h"

extern planner_hook_type prev_planner_hook;
PlannedStmt* auto_index_planner(Query *parse, const char *query_string, int cursorOptions, ParamListInfo boundParams);
#endif

void auto_index_worker_main(Datum main_arg);

#endif