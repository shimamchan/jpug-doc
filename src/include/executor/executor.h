/*-------------------------------------------------------------------------
 *
 * executor.h
 *	  support for the POSTGRES executor module
 *
 *
 * Portions Copyright (c) 1996-2002, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Id: executor.h,v 1.81 2002/12/05 15:50:36 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "executor/execdesc.h"


/* ----------------
 *		TupIsNull
 *
 *		This is used mainly to detect when there are no more
 *		tuples to process.
 * ----------------
 */
/* return: true if tuple in slot is NULL, slot is slot to test */
#define TupIsNull(slot) \
	((slot) == NULL || (slot)->val == NULL)

/*
 * prototypes from functions in execAmi.c
 */
extern void ExecReScan(PlanState *node, ExprContext *exprCtxt);
extern void ExecMarkPos(PlanState *node);
extern void ExecRestrPos(PlanState *node);
extern bool ExecSupportsMarkRestore(NodeTag plantype);

/*
 * prototypes from functions in execJunk.c
 */
extern JunkFilter *ExecInitJunkFilter(List *targetList, TupleDesc tupType,
				   TupleTableSlot *slot);
extern void ExecFreeJunkFilter(JunkFilter *junkfilter);
extern bool ExecGetJunkAttribute(JunkFilter *junkfilter, TupleTableSlot *slot,
					 char *attrName, Datum *value, bool *isNull);
extern HeapTuple ExecRemoveJunk(JunkFilter *junkfilter, TupleTableSlot *slot);


/*
 * prototypes from functions in execMain.c
 */
extern void ExecutorStart(QueryDesc *queryDesc);
extern TupleTableSlot *ExecutorRun(QueryDesc *queryDesc,
			ScanDirection direction, long count);
extern void ExecutorEnd(QueryDesc *queryDesc);
extern EState *CreateExecutorState(void);
extern void ExecCheckRTPerms(List *rangeTable, CmdType operation);
extern void ExecConstraints(const char *caller, ResultRelInfo *resultRelInfo,
				TupleTableSlot *slot, EState *estate);
extern TupleTableSlot *EvalPlanQual(EState *estate, Index rti,
			 ItemPointer tid);

/*
 * prototypes from functions in execProcnode.c
 */
extern PlanState *ExecInitNode(Plan *node, EState *estate);
extern TupleTableSlot *ExecProcNode(PlanState *node);
extern int	ExecCountSlotsNode(Plan *node);
extern void ExecEndNode(PlanState *node);
extern TupleDesc ExecGetTupType(PlanState *node);

/*
 * prototypes from functions in execQual.c
 */
extern Datum ExecEvalParam(Param *expression, ExprContext *econtext,
			  bool *isNull);
extern Datum GetAttributeByNum(TupleTableSlot *slot, AttrNumber attrno,
				  bool *isNull);
extern Datum GetAttributeByName(TupleTableSlot *slot, char *attname,
				   bool *isNull);
extern Datum ExecMakeFunctionResult(FunctionCachePtr fcache,
					   List *arguments,
					   ExprContext *econtext,
					   bool *isNull,
					   ExprDoneCond *isDone);
extern Tuplestorestate *ExecMakeTableFunctionResult(Node *funcexpr,
							ExprContext *econtext,
							TupleDesc expectedDesc,
							TupleDesc *returnDesc);
extern Datum ExecEvalExpr(Node *expression, ExprContext *econtext,
			 bool *isNull, ExprDoneCond *isDone);
extern Datum ExecEvalExprSwitchContext(Node *expression, ExprContext *econtext,
						  bool *isNull, ExprDoneCond *isDone);
extern Node *ExecInitExpr(Node *node, PlanState *parent);
extern bool ExecQual(List *qual, ExprContext *econtext, bool resultForNull);
extern int	ExecTargetListLength(List *targetlist);
extern int	ExecCleanTargetListLength(List *targetlist);
extern TupleTableSlot *ExecProject(ProjectionInfo *projInfo,
			ExprDoneCond *isDone);

/*
 * prototypes from functions in execScan.c
 */
typedef TupleTableSlot *(*ExecScanAccessMtd) (ScanState *node);

extern TupleTableSlot *ExecScan(ScanState *node, ExecScanAccessMtd accessMtd);

/*
 * prototypes from functions in execTuples.c
 */
extern TupleTable ExecCreateTupleTable(int initialSize);
extern void ExecDropTupleTable(TupleTable table, bool shouldFree);
extern TupleTableSlot *ExecAllocTableSlot(TupleTable table);
extern TupleTableSlot *MakeTupleTableSlot(void);
extern TupleTableSlot *ExecStoreTuple(HeapTuple tuple,
			   TupleTableSlot *slot,
			   Buffer buffer,
			   bool shouldFree);
extern TupleTableSlot *ExecClearTuple(TupleTableSlot *slot);
extern void ExecSetSlotDescriptor(TupleTableSlot *slot,
					  TupleDesc tupdesc, bool shouldFree);
extern void ExecSetSlotDescriptorIsNew(TupleTableSlot *slot, bool isNew);
extern void ExecInitResultTupleSlot(EState *estate, PlanState *planstate);
extern void ExecInitScanTupleSlot(EState *estate, ScanState *scanstate);
extern TupleTableSlot *ExecInitExtraTupleSlot(EState *estate);
extern TupleTableSlot *ExecInitNullTupleSlot(EState *estate,
					  TupleDesc tupType);
extern TupleDesc ExecTypeFromTL(List *targetList, bool hasoid);
extern void SetChangedParamList(PlanState *node, List *newchg);

typedef struct TupOutputState
{
	/* use "struct" here to allow forward reference */
	struct AttInMetadata *metadata;
	DestReceiver *destfunc;
} TupOutputState;

extern TupOutputState *begin_tup_output_tupdesc(CommandDest dest, TupleDesc tupdesc);
extern void do_tup_output(TupOutputState *tstate, char **values);
extern void do_text_output_multiline(TupOutputState *tstate, char *text);
extern void end_tup_output(TupOutputState *tstate);

/*
 * Write a single line of text given as a C string.
 *
 * Should only be used with a single-TEXT-attribute tupdesc.
 */
#define do_text_output_oneline(tstate, text_to_emit) \
	do { \
		char *values_[1]; \
		values_[0] = (text_to_emit); \
		do_tup_output(tstate, values_); \
	} while (0)


/*
 * prototypes from functions in execUtils.c
 */
extern void ResetTupleCount(void);
extern void ExecAssignExprContext(EState *estate, PlanState *planstate);
extern void ExecAssignResultType(PlanState *planstate,
					 TupleDesc tupDesc, bool shouldFree);
extern void ExecAssignResultTypeFromOuterPlan(PlanState *planstate);
extern void ExecAssignResultTypeFromTL(PlanState *planstate);
extern TupleDesc ExecGetResultType(PlanState *planstate);
extern void ExecAssignProjectionInfo(PlanState *planstate);
extern void ExecFreeProjectionInfo(PlanState *planstate);
extern void ExecFreeExprContext(PlanState *planstate);
extern TupleDesc ExecGetScanType(ScanState *scanstate);
extern void ExecAssignScanType(ScanState *scanstate,
				   TupleDesc tupDesc, bool shouldFree);
extern void ExecAssignScanTypeFromOuterPlan(ScanState *scanstate);

extern ExprContext *MakeExprContext(TupleTableSlot *slot,
				MemoryContext queryContext);
extern void FreeExprContext(ExprContext *econtext);

#define ResetExprContext(econtext) \
	MemoryContextReset((econtext)->ecxt_per_tuple_memory)

extern ExprContext *MakePerTupleExprContext(EState *estate);

/* Get an EState's per-output-tuple exprcontext, making it if first use */
#define GetPerTupleExprContext(estate) \
	((estate)->es_per_tuple_exprcontext ? \
	 (estate)->es_per_tuple_exprcontext : \
	 MakePerTupleExprContext(estate))

#define GetPerTupleMemoryContext(estate) \
	(GetPerTupleExprContext(estate)->ecxt_per_tuple_memory)

/* Reset an EState's per-output-tuple exprcontext, if one's been created */
#define ResetPerTupleExprContext(estate) \
	do { \
		if ((estate)->es_per_tuple_exprcontext) \
			ResetExprContext((estate)->es_per_tuple_exprcontext); \
	} while (0)

extern void ExecOpenIndices(ResultRelInfo *resultRelInfo);
extern void ExecCloseIndices(ResultRelInfo *resultRelInfo);
extern void ExecInsertIndexTuples(TupleTableSlot *slot, ItemPointer tupleid,
					  EState *estate, bool is_vacuum);

extern void RegisterExprContextCallback(ExprContext *econtext,
							ExprContextCallbackFunction function,
							Datum arg);
extern void UnregisterExprContextCallback(ExprContext *econtext,
							  ExprContextCallbackFunction function,
							  Datum arg);

#endif   /* EXECUTOR_H  */
