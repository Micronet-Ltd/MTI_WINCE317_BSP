/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998-2009 Texas Instruments Incorporated           |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**          
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**     
**+----------------------------------------------------------------------+**
***************************************************************************/
 
/** \file   bmtrace.c 
 *  \brief  The WinMobile bmtrace performance tracing implementation 
 *
 *  \see    bmtraceApp.c, bmtrace_api.h 
 */

#include <windows.h>
#include "bmtrace_api.h"


#define False                       0
#define True                        1
#define likely
#define unlikely
#define PAGE_SIZE                   UserKInfo[KINX_PAGESIZE]
#define MAXMSG                      256
#define MAX_EVENT_NAME              64
#define MAX_LUT                     64
#define BM_NUM_ENTRIES              1024
#define BM_PARAM_ENTRY_BIT          0x8000000
#define BM_MIN(a, b)                ((a) < (b) ? (a) : (b))
#define TLB_CACHE_SIZE              NUMBER_OF_TLBS*PAGE_SIZE
#define MIN_CAUSE_CACHE             (CACHE_SIZE/CACHE_LINE_SIZE)*PAGE_SIZE
#define TLB_FAIL_MEM_REQUIRED       2*BM_MIN(TLB_CACHE_SIZE, MIN_CAUSE_CACHE)
/* CPU depend parameters */         
#define CACHE_SIZE                  16384
#define CACHE_LINE_SIZE             32
#define NUMBER_OF_TLBS              128
#define NUM_OF_TRACE_ERROR          100
#define NUM_OF_LOOP_TEST            1000000
#define NUM_OF_FUNCTION_CALL_TEST   100

typedef struct {
      int event_id;             /* trace entry identification */
      unsigned long ts;         /* Timestamp                  */
      unsigned long param; 
} bm_entry_t;

typedef struct {
	  int               init_done;
	  int               enable;
      unsigned long     prev_param; /* Timestamp                  */
      CRITICAL_SECTION  critic;
} bm_control_t;

typedef struct {
      char event_name[MAX_EVENT_NAME];
} bm_event_record_in_lut_t;

typedef struct {
    int counter;
    bm_event_record_in_lut_t lut[MAX_LUT+1];
} bm_trace_lut_t;

typedef struct bm_test_st{
    unsigned long sum;
    struct bm_test_st* next;
} bm_test_t;

#define BM_ALLOC(seg_id, size, alignment) LocalAlloc(LMEM_ZEROINIT, size)
#define BM_ALLOC_IN_ISR(seg_id, size, alignment) LocalAlloc(LMEM_FIXED, size)
#define BM_FREE(seg_id, mem_addr, size) (LocalFree(mem_addr))
#define BM_FREE_IN_ISR(seg_id, mem_addr, size) (LocalFree(mem_addr))
#define BM_GET_TIMESTAMP(tmp) tmp = TIQueryPerformanceCounter();

#pragma data_seg(".bmshmem")

bm_trace_lut_t  bm_trace_lut                                 = {0, 0};
bm_entry_t      bm_entries[BM_NUM_ENTRIES]                   = {0};
bm_entry_t     *bm_entry_start                               = &bm_entries[0];
bm_entry_t     *bm_entry_pointer                             = &bm_entries[0];
bm_entry_t     *bm_entry_end                                 = &bm_entries[0] + BM_NUM_ENTRIES;
bm_control_t    bm_control                                   = {False, False, 0, 0};

#pragma data_seg()



/*--------------------------------------------------------------------------------------*/

void print_tic_time()
{

    LARGE_INTEGER Frequency ;

    QueryPerformanceFrequency(&Frequency );

    printf("\n=============> tic time is * %ld * ns <=============\n\n", (1000000000L + (Frequency.LowPart >> 1)) / Frequency.LowPart);

} /* print_tic_time */

/*--------------------------------------------------------------------------------------*/

unsigned long TIQueryPerformanceCounter()
{
  LARGE_INTEGER performanceCount;

  QueryPerformanceCounter(&performanceCount);

  return performanceCount.LowPart;

} /* TIQueryPerformanceCounter */

/*--------------------------------------------------------------------------------------*/

unsigned long bm_act_trace_in()
{
    unsigned long ts;

    if (unlikely(bm_control.enable==False))
    {
        return 0;
    }
    BM_GET_TIMESTAMP(ts);
    if (ts <= bm_control.prev_param)
    {
        ts = bm_control.prev_param + 1; /* work around to avoid previous out_ts <= ts */
    }

    return ts;  

} /* bm_act_trace_in() */

/*--------------------------------------------------------------------------------------*/

int bm_act_register_event(char* module, char* context, char* group, unsigned char level, char* name, char* suffix, int is_param)
{
    int counter = 0;

    if (unlikely(bm_control.enable==False))
    {
        return 0;
    }
    EnterCriticalSection(&bm_control.critic);
    if (likely(bm_trace_lut.counter < MAX_LUT))
    {
        _snprintf(bm_trace_lut.lut[bm_trace_lut.counter].event_name,MAX_EVENT_NAME-1,"%s: %s: %s: %d: %s%s", module, context, group, level, name,suffix);
    }
    else
    {
        _snprintf(bm_trace_lut.lut[MAX_LUT].event_name,MAX_EVENT_NAME-1,"LUT Overflow!, please set MAX_LUT to %d ",bm_trace_lut.counter+1);
    }
    counter = bm_trace_lut.counter;
    if (unlikely(is_param))
    {
        counter |= BM_PARAM_ENTRY_BIT;
    }
    ++bm_trace_lut.counter;
    LeaveCriticalSection(&bm_control.critic);

    return counter;

} /* bm_act_register_event() */

/*--------------------------------------------------------------------------------------*/

void bm_act_trace_out(int event_id, unsigned long in_ts)
{
    unsigned long out_ts;

    if (unlikely(bm_control.enable == False))
    {
        return;
    }
    BM_GET_TIMESTAMP(out_ts);
    if (out_ts <= in_ts)
    {
        out_ts = in_ts + 1; /* avoid events length < 1 */
    }
    EnterCriticalSection(&bm_control.critic);
    bm_entry_pointer->event_id = event_id;
    bm_entry_pointer->ts       = in_ts;
    bm_entry_pointer->param    = bm_control.prev_param = out_ts;
    ++bm_entry_pointer;
    if (unlikely(bm_entry_pointer == bm_entry_end))
    {
        bm_control.enable = False;
        bm_control.prev_param = 0;
        printf("Bmtrace buffer is full. Stopping traces.\n");
    }
    LeaveCriticalSection(&bm_control.critic);

} /* bm_act_trace_out() */

/*--------------------------------------------------------------------------------------*/

int print_out_buffer(char *buf)
{
    int event_id;
    int len = 0;
    unsigned int i;
    unsigned int g_position;

    print_tic_time();
    g_position = (bm_entry_pointer - bm_entry_start);
	printf("Enter to print_out_buffer\n");
	len += printf("\n---------------- TRACE BUFFER, Entries Number = %d  --------------------\n\n", g_position);

    for (i = 0; i < g_position; i++)
	{
        event_id = bm_entries[i].event_id & (~BM_PARAM_ENTRY_BIT);
        if (bm_entries[i].event_id & BM_PARAM_ENTRY_BIT)
        {
            len += printf("TI-SoT: CLT_REC: UINT64: %s: %lu: %lu\n",
                           bm_trace_lut.lut[event_id].event_name,
                           bm_entries[i].ts, 
						   bm_entries[i].param);
        }
        else
        {
            len += printf("TI-SoT: CLT_REC: %s: <->: %lu: %lu\n",
                           bm_trace_lut.lut[event_id].event_name,
                           bm_entries[i].ts,
                           bm_entries[i].param);
        }
	}

	len += printf("\n------------------------ END OF TRACE BUFFER -------------------------\n\n");

    bm_entry_pointer = &bm_entries[0];
    bm_control.prev_param = 0;

	return(len);

} /* print_out_buffer() */

/*--------------------------------------------------------------------------------------*/

void bm_disable(void)
{
	bm_control.enable = False;
    printf("Bmtrace buffer disabled.\n");

} /* bm_disable() */

/*--------------------------------------------------------------------------------------*/

void bm_enable(void) 
{
    printf("Bmtrace buffer enabled.\n");
	bm_control.enable = True;

} /* bm_enable() */

/*--------------------------------------------------------------------------------------*/

void bm_init(void)
{
    if (bm_control.init_done == False)
    {
        memset(&bm_entries,   0, sizeof(bm_entries));
        memset(&bm_trace_lut, 0, sizeof(bm_trace_lut));
        bm_control.init_done = True;
        InitializeCriticalSection(&bm_control.critic);
    }

} /* bm_init() */

/*--------------------------------------------------------------------------------------*/

void bm_deinit(void)
{
    if (bm_control.init_done == True)
    {
        DeleteCriticalSection(&bm_control.critic);
        bm_control.init_done = False;
    }

} /* bm_deinit() */
/*--------------------------------------------------------------------------------------*/

BOOL __stdcall DllEntry(HINSTANCE  hInstance,
                        ULONG      Reason,
                        LPVOID     pReserved)
{
   switch (Reason)
   {
      case DLL_PROCESS_ATTACH:
          DisableThreadLibraryCalls((HMODULE) hInstance);
          bm_init();
          break;

       case DLL_PROCESS_DETACH:
           break;
   }
   return TRUE;

} /* DllEntry() */

/*--------------------------------------------------------------------------------------*/

/*======================================================================================*/
/*                           General Measurements functions                                 */
/*======================================================================================*/

/*--------------------------------------------------------------------------------------*/

static void bm_test_two_seq_samples(void)
{
    CL_TRACE_START_L0();
    CL_TRACE_END_L0();

} /* bm_test_two_seq_samples() */

/*--------------------------------------------------------------------------------------*/

static void bm_test_trace_error(void)
{
    int i;

    for (i=0; i<NUM_OF_TRACE_ERROR; ++i)
    {
        CL_TRACE_START_L0();
        CL_TRACE_END_L0();
    }

} /* bm_test_trace_error() */

/*--------------------------------------------------------------------------------------*/

static unsigned long bm_test_simple_loop(unsigned char with_printing, int loop_count, int nop)
{
    unsigned long num = 0x5a5; /* this will mask the optimizer to cut out the loop */
    int i;

    CL_TRACE_START_L0();
    for (i=0; i<loop_count; ++i) 
    {
        ++num;
        if (nop)
        {
            printf("%d\n",num);
        }
    }
    CL_TRACE_END_L0();

    if (with_printing!=False)
    {
        printf("bm_test_simple_loop result: %lu\n", num); /* disable the optimizer to optimize this test */
    }
    return num;
} /* bm_test_simple_loop() */

/*--------------------------------------------------------------------------------------*/

static void bm_test_pointer(unsigned char with_printing)
{
    int  counter = NUM_OF_LOOP_TEST;

    bm_test_t* node;
    bm_test_t node1, node2;

    node1.sum = 1;
    node2.sum = 1;

    node1.next = &node2;
    node2.next = &node1;

    node = &node1;
    {
        CL_TRACE_START_L0();
        while (--counter)
        {
            node->sum += node->next->sum;
            node = node->next;
        }
        CL_TRACE_END_L0();
    }
    if (with_printing!=False)
    {
        printf("bm_test_pointer result: %lu\n", node->sum); /* disable the optimizer to optimize this test */
    }
} /* bm_test_pointer() */

/*--------------------------------------------------------------------------------------*/

static int bm_test_fibonacci(int counter, unsigned long a, unsigned long b)
{
    unsigned long sum;

    if (--counter==0)
        return a+b;

    sum = a+b;

    return sum+bm_test_fibonacci(counter, b, sum);

} /* bm_test_fibonacci() */

/*--------------------------------------------------------------------------------------*/

static void bm_test_function_call(unsigned char with_printing)
{
    unsigned long result;

    CL_TRACE_START_L0();
    result = bm_test_fibonacci(NUM_OF_FUNCTION_CALL_TEST, 1, 1);
    CL_TRACE_END_L0();
    if (with_printing!=False)
    {
        printf("bm_test_function_call result: %lu\n", result); /* disable the optimizer to optimize this test */
    }
} /* bm_test_function_call() */

/*--------------------------------------------------------------------------------------*/

#define CACHE_MISS_MEM_REQUIRED (2*CACHE_SIZE)
static void bm_mis_calibration(unsigned char with_printing)
{
    int counter = NUM_OF_LOOP_TEST;
    int line_counter = 0;
    CL_TRACE_START_L0();
    while (--counter)
    {
        line_counter += (CACHE_LINE_SIZE/sizeof(int)); /* move to the next line */
        line_counter = line_counter%(CACHE_MISS_MEM_REQUIRED/sizeof(int)); /* make sure you stay in the memory chunk */
    }
    CL_TRACE_END_L0();
    if (with_printing!=False)
    {
        printf("bm_mis_calibration result: %d\n", line_counter); /* disable the optimizer to optimize this test */
    }

} /* bm_mis_calibration() */

/*--------------------------------------------------------------------------------------*/

static void bm_test_cache_miss(unsigned char with_printing)
{
    int* p_buffer;
    int counter = NUM_OF_LOOP_TEST;
    int line_counter = 0;
    int num = 0;
    int i;

    /* we used eight pages to generate cache miss (8 is factor of 2 good for optimizations) */
    /* the */
    /* one extra page for aligment */
    p_buffer = BM_ALLOC(0, CACHE_MISS_MEM_REQUIRED, 32);
    for (i=0; i<CACHE_MISS_MEM_REQUIRED/sizeof(int); ++i) /* init and make sure all TLBs' are set */
        p_buffer[i] = 1;

    {
        CL_TRACE_START_L0();
        while (--counter)
        {
            num += p_buffer[line_counter];
            line_counter += (CACHE_LINE_SIZE/sizeof(int)); /* move to the next line */
            line_counter = line_counter%(CACHE_MISS_MEM_REQUIRED/sizeof(int)); /* make sure you stay in the memory chunk */
        }
        CL_TRACE_END_L0();
    }
    if (with_printing!=False)
    {
        printf("bm_test_cache_miss result: %d\n", num); /* disable the optimizer to optimize this test */
    }
    BM_FREE(0, p_buffer, CACHE_MISS_MEM_REQUIRED);

} /* bm_test_cache_miss() */

/*--------------------------------------------------------------------------------------*/

static void bm_test_tlb_miss(unsigned char with_printing)
{
    int* p_buffer;
    int counter = NUM_OF_LOOP_TEST;
    int line_counter = 0;
    int num = 0;
    unsigned int i;

    /* we used eight pages to generate cache miss (8 is factor of 2 good for optimizations) */
    /* the */
    /* one extra page for aligment */
    p_buffer = BM_ALLOC(0, TLB_FAIL_MEM_REQUIRED, 32);
    for (i=0; i<TLB_FAIL_MEM_REQUIRED/sizeof(int); ++i)
        p_buffer[i] = 1;
    {
        CL_TRACE_START_L0();
        while (--counter)
        {
            num += p_buffer[line_counter];
            line_counter += (PAGE_SIZE/sizeof(int)); /* move to the next line */
            line_counter = line_counter%(TLB_FAIL_MEM_REQUIRED/sizeof(int)); /* make sure you stay in the memory chunk */
        }
        CL_TRACE_END_L0();
    }
    if (with_printing!=False)
    {
        printf("bm_test_tlb_miss result: %d\n", num); /* disable the optimizer to optimize this test */
    }
    BM_FREE(0, p_buffer, TLB_FAIL_MEM_REQUIRED);

} /* bm_test_tlb_miss() */

/*--------------------------------------------------------------------------------------*/

static void bm_measure_all(unsigned char with_printing)
{
    unsigned long count;

    if (with_printing!=False)
        printf("Running: bm_test_two_seq_samples\n");
    bm_test_two_seq_samples();
    bm_test_two_seq_samples();
    bm_test_two_seq_samples();
/*
    if (with_printing!=False)
    {
        printf("Running: bm_test_trace_error\n");
        printf("         number of loops is %d\n", NUM_OF_TRACE_ERROR);
    }
    bm_test_trace_error(); 
*/
    if (with_printing!=False)
    {
        printf("Running: bm_test_simple_loop\n");
        printf("         number of loops is %d\n", NUM_OF_LOOP_TEST);
    }
    count = bm_test_simple_loop(with_printing, NUM_OF_LOOP_TEST, 0);

    if (with_printing!=False)
    {
        printf("Running: bm_test_pointer\n");
        printf("         number of redirections is %d\n", count);
    }
    bm_test_pointer(with_printing);

    if (with_printing!=False)
    {
        printf("Running: bm_test_function_call\n");
        printf("         via fibonacci, recursive depth is %d\n", NUM_OF_FUNCTION_CALL_TEST);
    }
    bm_test_function_call(with_printing);

    if (with_printing!=False)
    {
        printf("Running: bm_mis_calibration\n");
        printf("         number of reads %d\n", NUM_OF_LOOP_TEST);
    }
    bm_mis_calibration(with_printing);

    if (with_printing!=False)
    {
        printf("Running: bm_test_cache_miss\n");
        printf("         assume: d-cache size is %d and d-cache line is %d\n", CACHE_SIZE, CACHE_LINE_SIZE);
        printf("         number of reads %d\n", NUM_OF_LOOP_TEST);
    }
    bm_test_cache_miss(with_printing);

    if (with_printing!=False)
    {
        printf("Running: bm_test_tlb_miss\n");
        printf("         assume: %d TLBs in the CPU. Page size is %ld\n", NUMBER_OF_TLBS, PAGE_SIZE);
        printf("         number of reads %d\n", NUM_OF_LOOP_TEST);
    }
    bm_test_tlb_miss(with_printing);

} /* bm_measure_all() */

/*--------------------------------------------------------------------------------------*/

static void bm_act_print_lut(void)
{
    int i;

    for (i=0; i<bm_trace_lut.counter; ++bm_trace_lut.counter)
        printf("lut[%d].event_name=%s", i, bm_trace_lut.lut[i].event_name);

} /* bm_act_print_lut() */

/*--------------------------------------------------------------------------------------*/
#define ALL_TEST_COUNT 20
void bm_test_all(void)
{
  int j;

  CeSetThreadQuantum(GetCurrentThread(), 0);
  print_tic_time();
  printf("\nTrace enable\n");
  bm_enable();
  bm_measure_all(0); /* make sure that functions in cache, no printing */
  printf("\nReseting logs\n");
  bm_entry_pointer = bm_entry_start;
  printf("\nRunning measurements %d times, please wait... \n", ALL_TEST_COUNT);
  for (j = 0; j < ALL_TEST_COUNT; j++)
  {
      bm_measure_all(0); 
  }
  printf("\nTrace disable\n");
  bm_disable();
  printf("\nDone, please print the trace log.\n");
} /* bm_test_all() */

/*--------------------------------------------------------------------------------------*/
