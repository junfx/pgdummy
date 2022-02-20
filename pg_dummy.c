#include "postgres.h"
#include "fmgr.h"

#include "miscadmin.h" //process_shared_preload_libraries_in_progress
#include "portability/instr_time.h"

/*
$ grep -n "typedef.*hook" -r .
./doc/src/sgml/custom-scan.sgml:44:typedef void (*set_rel_pathlist_hook_type) (PlannerInfo *root,
./doc/src/sgml/custom-scan.sgml:96:typedef void (*set_join_pathlist_hook_type) (PlannerInfo *root,

./src/include/catalog/objectaccess.h:125:typedef void (*object_access_hook_type) (ObjectAccessType access,
./src/include/commands/user.h:23:typedef void (*check_password_hook_type) (const char *username, const char *shadow_pass, PasswordType password_type, Datum validuntil_time, bool validuntil_null);
./src/include/commands/explain.h:65:typedef void (*ExplainOneQuery_hook_type) (Query *query,
./src/include/commands/explain.h:75:typedef const char *(*explain_get_index_name_hook_type) (Oid indexId);
./src/include/executor/executor.h:65:typedef void (*ExecutorStart_hook_type) (QueryDesc *queryDesc, int eflags);
./src/include/executor/executor.h:69:typedef void (*ExecutorRun_hook_type) (QueryDesc *queryDesc,
./src/include/executor/executor.h:76:typedef void (*ExecutorFinish_hook_type) (QueryDesc *queryDesc);
./src/include/executor/executor.h:80:typedef void (*ExecutorEnd_hook_type) (QueryDesc *queryDesc);
./src/include/executor/executor.h:84:typedef bool (*ExecutorCheckPerms_hook_type) (List *, bool);
./src/include/libpq/auth.h:26:typedef void (*ClientAuthentication_hook_type) (Port *, int);
./src/include/libpq/libpq-be.h:293:typedef void (*openssl_tls_init_hook_typ) (SSL_CTX *context, bool isServerStart);
./src/include/optimizer/paths.h:29:typedef void (*set_rel_pathlist_hook_type) (PlannerInfo *root,
./src/include/optimizer/paths.h:36:typedef void (*set_join_pathlist_hook_type) (PlannerInfo *root,
./src/include/optimizer/paths.h:45:typedef RelOptInfo *(*join_search_hook_type) (PlannerInfo *root,
./src/include/optimizer/plancat.h:21:typedef void (*get_relation_info_hook_type) (PlannerInfo *root,
./src/include/optimizer/planner.h:26:typedef PlannedStmt *(*planner_hook_type) (Query *parse,
./src/include/optimizer/planner.h:33:typedef void (*create_upper_paths_hook_type) (PlannerInfo *root,
./src/include/parser/analyze.h:20:typedef void (*post_parse_analyze_hook_type) (ParseState *pstate,
./src/include/rewrite/rowsecurity.h:37:typedef List *(*row_security_policy_hook_type) (CmdType cmdtype,
./src/include/storage/ipc.h:22:typedef void (*shmem_startup_hook_type) (void);
./src/include/tcop/utility.h:71:typedef void (*ProcessUtility_hook_type) (PlannedStmt *pstmt,
./src/include/utils/lsyscache.h:62:typedef int32 (*get_attavgwidth_hook_type) (Oid relid, AttrNumber attnum);
./src/include/utils/selfuncs.h:120:typedef bool (*get_relation_stats_hook_type) (PlannerInfo *root,
./src/include/utils/selfuncs.h:125:typedef bool (*get_index_stats_hook_type) (PlannerInfo *root,
./src/include/utils/elog.h:397:typedef void (*emit_log_hook_type) (ErrorData *edata);
./src/include/fmgr.h:764:typedef bool (*needs_fmgr_hook_type) (Oid fn_oid);
./src/include/fmgr.h:766:typedef void (*fmgr_hook_type) (FmgrHookEventType event,
*/

/*
$ grep -n "typedef.*Hook" -r .
./src/bin/psql/variables.h:31:typedef bool (*VariableAssignHook) (const char *newval);
./src/bin/psql/variables.h:54:typedef char *(*VariableSubstituteHook) (char *newval);
./src/include/nodes/params.h:100:typedef ParamExternData *(*ParamFetchHook) (ParamListInfo params,
./src/include/nodes/params.h:104:typedef void (*ParamCompileHook) (ParamListInfo params, struct Param *param,
./src/include/nodes/params.h:108:typedef void (*ParserSetupHook) (struct ParseState *pstate, void *arg);
./src/include/parser/parse_node.h:87:typedef Node *(*PreParseColumnRefHook) (ParseState *pstate, ColumnRef *cref);
./src/include/parser/parse_node.h:88:typedef Node *(*PostParseColumnRefHook) (ParseState *pstate, ColumnRef *cref, Node *var);
./src/include/parser/parse_node.h:89:typedef Node *(*ParseParamRefHook) (ParseState *pstate, ParamRef *pref);
./src/include/parser/parse_node.h:90:typedef Node *(*CoerceParamHook) (ParseState *pstate, Param *param,
./src/include/utils/guc.h:174:typedef bool (*GucBoolCheckHook) (bool *newval, void **extra, GucSource source);
./src/include/utils/guc.h:175:typedef bool (*GucIntCheckHook) (int *newval, void **extra, GucSource source);
./src/include/utils/guc.h:176:typedef bool (*GucRealCheckHook) (double *newval, void **extra, GucSource source);
./src/include/utils/guc.h:177:typedef bool (*GucStringCheckHook) (char **newval, void **extra, GucSource source);
./src/include/utils/guc.h:178:typedef bool (*GucEnumCheckHook) (int *newval, void **extra, GucSource source);
./src/include/utils/guc.h:180:typedef void (*GucBoolAssignHook) (bool newval, void *extra);
./src/include/utils/guc.h:181:typedef void (*GucIntAssignHook) (int newval, void *extra);
./src/include/utils/guc.h:182:typedef void (*GucRealAssignHook) (double newval, void *extra);
./src/include/utils/guc.h:183:typedef void (*GucStringAssignHook) (const char *newval, void *extra);
./src/include/utils/guc.h:184:typedef void (*GucEnumAssignHook) (int newval, void *extra);
./src/include/utils/guc.h:186:typedef const char *(*GucShowHook) (void);
./src/interfaces/libpq/libpq-fe.h:623:typedef int (*PQsslKeyPassHook_OpenSSL_type) (char *buf, int size, PGconn *conn);
*/



#include "commands/explain.h"
static ExplainOneQuery_hook_type prev_ExplainOneQuery_hook = NULL;
//explain_get_index_name_hook_type

#include "optimizer/planner.h"
static planner_hook_type prev_planner_hook = NULL;
//create_upper_paths_hook_type

#include "executor/executor.h"
static ExecutorStart_hook_type prev_ExecutorStart_hook = NULL;
static ExecutorRun_hook_type prev_ExecutorRun_hook = NULL;
//static ExecutorFinish_hook_type prev_ExecutorRun_hook = NULL;
//static ExecutorEnd_hook_type prev_ProcessUtility_hook = NULL;
//static ExecutorCheckPerms_hook_type PreviousExecutorCheckPermsHook = NULL;

#include "tcop/utility.h"
static ProcessUtility_hook_type prev_ProcessUtility_hook = NULL;

extern void _PG_init(void);
static void PgdummyExecutorStart(QueryDesc *queryDesc, int eflags);
static void PgdummyExecutorRun(QueryDesc *queryDesc, ScanDirection direction,
							   uint64 count, bool once);
static void PgdummyExplain(Query *query, int cursorOptions, IntoClause *into, ExplainState *es, 
						   const char *queryString, ParamListInfo params, QueryEnvironment *queryEnv);
static PlannedStmt * PgdummyPlanner(Query *parse, const char *query_string, int cursorOptions,
								    ParamListInfo boundParams);
static void PgdummyProcessUtility(PlannedStmt *pstmt, const char *queryString,
								  ProcessUtilityContext context, ParamListInfo params,
								  QueryEnvironment *queryEnv,
								  DestReceiver *dest, QueryCompletion *qc);

PG_MODULE_MAGIC;
PG_FUNCTION_INFO_V1(pgdumy_helloworld);

void
_PG_init(void)
{
	if (!process_shared_preload_libraries_in_progress) {
		ereport(ERROR, (errmsg("pgdumy can only be loaded via shared_preload_libraries"),
						errhint("Add pqmasq to shared_preload_libraries configuration variable in postgresql.conf.")));
	}

	prev_planner_hook = planner_hook;
	planner_hook = PgdummyPlanner;

	prev_ExecutorStart_hook = ExecutorStart_hook;
	ExecutorStart_hook = PgdummyExecutorStart;

	prev_ExecutorRun_hook = ExecutorRun_hook;
	ExecutorRun_hook = PgdummyExecutorRun;

	prev_ProcessUtility_hook = ProcessUtility_hook;
	ProcessUtility_hook = PgdummyProcessUtility;

	prev_ExplainOneQuery_hook = ExplainOneQuery_hook;
	ExplainOneQuery_hook = PgdummyExplain;
}

static PlannedStmt * PgdummyPlanner(Query *parse, const char *query_string, int cursorOptions,
								   ParamListInfo boundParams)
{

	PlannedStmt *result;

	if (prev_planner_hook)
		result = (*prev_planner_hook) (parse, query_string, cursorOptions, boundParams);
	else
		result = standard_planner(parse, query_string, cursorOptions, boundParams);
	return result;
}

static void PgdummyExecutorStart(QueryDesc *queryDesc, int eflags)
{
	if (prev_ExecutorStart_hook)
		(*prev_ExecutorStart_hook) (queryDesc, eflags);
	else
		standard_ExecutorStart(queryDesc, eflags);
}

static void PgdummyExecutorRun(QueryDesc *queryDesc, ScanDirection direction,
							   uint64 count, bool execute_once)
{
	if (prev_ExecutorRun_hook)
		(*prev_ExecutorRun_hook) (queryDesc, direction, count, execute_once);
	else
		standard_ExecutorRun(queryDesc, direction, count, execute_once);
}

static void PgdummyProcessUtility(PlannedStmt *pstmt, const char *queryString,
								 ProcessUtilityContext context, ParamListInfo params,
								 QueryEnvironment *queryEnv,
								 DestReceiver *dest, QueryCompletion *qc)
{
	if (prev_ProcessUtility_hook)
		(*prev_ProcessUtility_hook) (pstmt, queryString,
								context, params, queryEnv,
								dest, qc);
	else
		standard_ProcessUtility(pstmt, queryString,
								context, params, queryEnv,
								dest, qc);
}

static void PgdummyExplain(Query *query, int cursorOptions, IntoClause *into, ExplainState *es, 
						   const char *queryString, ParamListInfo params, QueryEnvironment *queryEnv)
{
	if (prev_ExplainOneQuery_hook)
		(*prev_ExplainOneQuery_hook) (query, cursorOptions, into, es,
									  queryString, params, queryEnv);
	else
	{
		/*
		 * 'standard_ExplainOneQuery', copied from explain.c:ExplainOneQuery
		 */
		PlannedStmt *plan;
		instr_time	planstart,
					planduration;
		BufferUsage bufusage_start,
					bufusage;

		if (es->buffers)
			bufusage_start = pgBufferUsage;
		INSTR_TIME_SET_CURRENT(planstart);

		/* plan the query */
		plan = pg_plan_query(query, queryString, cursorOptions, params);

		INSTR_TIME_SET_CURRENT(planduration);
		INSTR_TIME_SUBTRACT(planduration, planstart);

		/* calc differences of buffer counters. */
		if (es->buffers)
		{
			memset(&bufusage, 0, sizeof(BufferUsage));
			BufferUsageAccumDiff(&bufusage, &pgBufferUsage, &bufusage_start);
		}

		/* run it (if needed) and produce output */
		ExplainOnePlan(plan, into, es, queryString, params, queryEnv,
					   &planduration, (es->buffers ? &bufusage : NULL));
	}
}
