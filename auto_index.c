#include "auto_index.h"


PG_MODULE_MAGIC;


planner_hook_type prev_planner_hook = NULL;
bool got_sigterm = false, inside_hook = false;


static void handle_sigterm(int signum)
{
   
    got_sigterm = true;
}

   
PGDLLEXPORT void
auto_index_worker_main(Datum main_arg)
{
    elog(LOG, "WORKER CHAL RHA H !!!!");
    BackgroundWorkerInitializeConnection("postgres", NULL, 0);

    
    pqsignal(SIGTERM, handle_sigterm);
    BackgroundWorkerUnblockSignals();

   
    while (!got_sigterm){
        elog(LOG, "WORKER WHILE LOOP ME H !!!!");
        int ret, i;
        CHECK_FOR_INTERRUPTS();

        ret = SPI_connect();
        if (ret != SPI_OK_CONNECT)
        {
           
            elog(WARNING, "AutoIndexWorker: SPI_connect failed");
            pg_usleep(5000000L); // Sleep 5s
            continue;
        }
        
        elog(LOG, "Worker aaya");
        const char *query = "SELECT tablename, colname FROM aidx_queries WHERE benefit * num_queries > cost;";

        // ret = SPI_exec(query, 0);
        // if (ret != SPI_OK_SELECT)
        // {
        //     elog(WARNING, "AutoIndexWorker: SPI_exec failed for SELECT");
        //     SPI_finish();
        //     pg_usleep(5000000L);
        //     continue;
        // }

        // for (i = 0; i < SPI_processed; i++)
        // {
        //     char *tablename = SPI_getvalue(SPI_tuptable->vals[i], SPI_tuptable->tupdesc, 1);
        //     char *colname = SPI_getvalue(SPI_tuptable->vals[i], SPI_tuptable->tupdesc, 2);

        //     if (tablename && colname)
        //     {
        //         StringInfoData create_index;
        //         initStringInfo(&create_index);

        //         appendStringInfo(&create_index,
        //             "CREATE INDEX IF NOT EXISTS idx_%s_%s ON %s (%s);",
        //             tablename, colname, tablename, colname);

        //         elog(LOG, "AutoIndexWorker: Creating index: %s", create_index.data);
        //         SPI_exec(create_index.data, 0);

        //         StringInfoData delete_entry;
        //         initStringInfo(&delete_entry);
        //         appendStringInfo(&delete_entry,
        //             "DELETE FROM aidx_queries WHERE tablename = '%s' AND colname = '%s';",
        //             tablename, colname);

        //         SPI_exec(delete_entry.data, 0);

        //         pfree(create_index.data);
        //         pfree(delete_entry.data);
        //     }
        // }

        SPI_finish();
        pg_usleep(10000000L); 
    }

    elog(LOG, "AutoIndexWorker: Server shutting down, exiting...");
    proc_exit(0);  t
}


static PlannedStmt *
auto_index_planner_hook(Query *parse, const char *query_string, int cursorOptions, ParamListInfo boundParams)
{
   
    elog(LOG, "AutoIndex: Planner hook triggered");

    PlannedStmt *stmt;

   
    if(parse->commandType == CMD_SELECT){
        elog(LOG, "AutoIndex: SELECT QUERY AAAAYA!!!");
        
    }
    
    if (prev_planner_hook)
    {
        elog(LOG, "AutoIndex: If me ghuse");
        stmt = prev_planner_hook(parse, query_string, cursorOptions, boundParams);
    }
    else
        
    {
        
        elog(LOG, "AutoIndex: else me ghuse");
        stmt = standard_planner(parse, query_string, cursorOptions, boundParams);
    }
    

    return stmt;
}

void
_PG_init(void)
{
   
    elog(LOG, "AutoIndex: Installing planner hook");

    if (planner_hook != auto_index_planner_hook)
    {
        prev_planner_hook = planner_hook;
        planner_hook = auto_index_planner_hook;
    

        BackgroundWorker worker;

        memset(&worker, 0, sizeof(worker));
    
        worker.bgw_flags = BGWORKER_SHMEM_ACCESS | BGWORKER_BACKEND_DATABASE_CONNECTION;
        worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
        worker.bgw_restart_time = BGW_NEVER_RESTART;
        snprintf(worker.bgw_name, BGW_MAXLEN, "AutoIndex Worker");
    
        
        sprintf(worker.bgw_library_name, "auto_index");
        sprintf(worker.bgw_function_name,"auto_index_worker_main");
    
        RegisterBackgroundWorker(&worker);
    }
}