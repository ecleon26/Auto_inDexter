#include "postgres.h"
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

#include "auto_index.h"

#ifdef PG_MODULE_MAGIC

#endif

void
auto_index_worker_main(Datum main_arg)
{
    BackgroundWorkerInitializeConnection("postgres", NULL, 0);

    while (true)
    {
        int ret, i;

        CHECK_FOR_INTERRUPTS();

        ret = SPI_connect();
        if (ret != SPI_OK_CONNECT)
        {
            elog(WARNING, "AutoIndexWorker: SPI_connect failed");
            pg_usleep(5000000L); 
            continue;
        }

        const char *query = "SELECT tablename, colname FROM aidx_queries WHERE benefit > cost";

        ret = SPI_exec(query, 0);
        if (ret != SPI_OK_SELECT)
        {
            elog(WARNING, "AutoIndexWorker: SPI_exec failed for SELECT");
            SPI_finish();
            pg_usleep(5000000L);
            continue;
        }

        for (i = 0; i < SPI_processed; i++)
        {
            char *tablename = SPI_getvalue(SPI_tuptable->vals[i], SPI_tuptable->tupdesc, 1);
            char *colname = SPI_getvalue(SPI_tuptable->vals[i], SPI_tuptable->tupdesc, 2);

            if (tablename && colname)
            {
                StringInfoData create_index;
                initStringInfo(&create_index);

                appendStringInfo(&create_index,
                    "CREATE INDEX IF NOT EXISTS idx_%s_%s ON %s (%s);",
                    tablename, colname, tablename, colname);

                elog(LOG, "AutoIndexWorker: Creating index: %s", create_index.data);
                SPI_exec(create_index.data, 0);

                StringInfoData delete_entry;
                initStringInfo(&delete_entry);
                appendStringInfo(&delete_entry,
                    "DELETE FROM aidx_queries WHERE tablename = '%s' AND colname = '%s';",
                    tablename, colname);

                SPI_exec(delete_entry.data, 0);

                pfree(create_index.data);
                pfree(delete_entry.data);
            }
        }

        SPI_finish();

        pg_usleep(10000000L); 
    }
}