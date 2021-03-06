/*****************************************************************************
 * HAlloc.h
 ***************************************************************************** 
 *
 * Project:     Halcon/libhalcon
 * Description: Memory allocation
 *
 * (c) 1996-2006 by MVTec Software GmbH
 *                  www.mvtec.com
 * 
 *****************************************************************************
 * 
 * $Revision: 1.38 $
 * $Date: 2007/03/02 15:16:47 $
 *
 */

#ifndef HALLOC_H
#define HALLOC_H


#if defined(__cplusplus)
extern "C" {
#endif

/*===========================================================================*/
/*                                  Defines                                  */
/*===========================================================================*/
#define H_LOCAL_ALLOC  -111                    /* AllocLevel for local. mem. */
#define H_GLOBAL_ALLOC -112                    /* use global AllocLevel      */

/* factor for memory allocation with HNewSpace():                            
   HInitMem()          -> 2*H_NEW_SPACE_FAC*MAX_SAVE
   else (HXAlloc())    ->   H_NEW_SPACE_FAC*MAX_SAVE                         */
#define H_NEW_SPACE_FAC   1024
/* maximal size of elements (in alignment units) to be put into cache:       */
#ifndef MAX_SAVE
#  define MAX_SAVE        128
#endif

/*===========================================================================*/
/*                               Data Structures                             */
/*===========================================================================*/
/* table element for stack management:                                       */
typedef struct 
{ 
  VOIDP          data;      /* pointer to stored data of a stack element   */
  MACHINE_WORD   *next;     /* pointer to next free memory area            */
  VOIDP          last;      /* pointer to last allocated area              */
  INT4           size;      /* size of memory within this stack element    */
  INT4           free;      /* free memory within stack element            */
} MemTElt;

typedef struct MEM_TT_EL_SAVE
{ 
  VOIDP                 *data;      /* memory area to manage               */
  INT4                  size;       /* size of memory                      */
  struct MEM_TT_EL_SAVE *next;      /* next element in list                */
} MemTEltSave;
  

/* element of global memory list, needed if data should be collected to 
 * test and check allocated memory*/
typedef struct MEM_GLOBAL_ENTRY
{ 
  MACHINE_WORD            magic;    /* unlikely address: -1 */
  MACHINE_WORD            *block;   /* start address of allocated memory   */
  MACHINE_WORD            *data;    /* data start address                  */
  MACHINE_WORD            size;     /* size of data                        */
  HBOOL                   cached;   /* was memory cached or allocated by 
                                     * malloc?                             */
  char                    *file;    /* file of allocation                  */
  MACHINE_WORD            line;     /* line of code of allocation          */
  INT                     level;    /* level of allocation                 */
  struct MEM_GLOBAL_ENTRY *succ;    /* next entry in list                  */
  struct MEM_GLOBAL_ENTRY *pred;    /* previous entry in list              */
} GlbMemEntry; 



/* memory management handle (with instance/thread-specific content)        */
typedef struct MEM_HANDLE 
{
  /*------------------------------ management of stack memory ---------*/
  MemTElt        *MemTTable;    /* management list for stack memory    */
  INT            MemTTLength;   /* current length of management list   */
  INT            MemTStack;     /* current stack depth                 */
  INT4           MemTmpCount;   /* num. of allocated stack mem. blocks */
  HBOOL          MemTmpInit;    /* must we init. the stack management? */
  /*------------------------------ management of local memory ---------*/
  MACHINE_WORD   *MemLocalList; /* management list for local memory    */
  H_pthread_mutex_t MutexLocalList; /* mutex variable for MemLocalList */
  INT4           MemLocalCount; /* num. of allocated local mem. blocks */
  /*------------------------------ management of global memory --------*/
  VOIDP          *GlbMemCache;  /* holds a cache for global 
                                 * memory locally */
} MEMHANDLE;
typedef MEMHANDLE *MemHandle;

/*===========================================================================*/
/*                          Prototypes of Functions                          */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*                          Test Memory Allocation                           */
/*---------------------------------------------------------------------------*/
/* HXTestPtrRange - tests memory (address) allocated with HAlloc/HAllocLocal */
extern HLibExport Herror HXTestPtrRange(void *ptr, const char *file, 
                                        INT4_8 line,HBOOL do_print);

/* HXTestPtr - tests memory (address+size) allocated with HAlloc/HAllocLocal */
extern HLibExport Herror HXTestPtr(void *ptr, const char *file, INT4_8 line);

/* HTestMem - tests all the memory currently allocated by HAlloc/HAllocLocal */
extern HLibExport Herror HTestMem (void);

/*---------------------------------------------------------------------------*/
/*                 Auxiliary Functions for Memory Management                 */
/*---------------------------------------------------------------------------*/
extern HLibExport HBOOL  HTraceMemory; /* switch if memory statistics 
                             * should be collected (do not set this directly)*/

/* set HMemoryTrace Flag to TRUE (efficient) / FALSE (debug information 
 * available)
 * NOTE: this function is not thread save and has to be used exclusively     */
extern HLibExport Herror HSetTraceMemory(Hproc_handle proc_handle,
                                         HBOOL trace_memory);

/* HTmpStackSize - return stack size for temporary memory blocks             */
extern HLibExport Herror HTmpStackSize(Hproc_handle proc_handle, INT4 *size);

/* HSetAllocLevel - set 'level' of memory allocation                         */
extern HLibExport Herror HSetAllocLevel(INT4_8 level);

/* HGetAllocLevel - read 'level' of memory allocation                        */
extern HLibExport Herror HGetAllocLevel(INT4_8 *level);

/* HGetImageCacheSize - current size in bytes of all cached images           */
extern HLibExport Herror HGetImageCacheSize(INT4_8 *size);

/* HGetImageCacheCapacity - maximum size in bytes of all cached images       */
extern HLibExport Herror HGetImageCacheCapacity(INT4_8 *size);

/* HGetImageCacheCapacity - set maximum size in bytes of all cached images 
 *                      0 switches off image caching                         */
extern HLibExport Herror HSetImageCacheCapacity(INT4_8 size);

/* HMallocInfo - return the number of calls of clib malloc                   */
extern HLibExport Herror HMallocInfo(INT4_8 *mcalls);

/* HMemInfo - retrieve informationen zaboutc current memory allocation       */
extern HLibExport Herror HMemInfo(Hproc_handle proc_handle,
                                  INT4_8 *alloc_mem, INT4_8 *free_mem,
                                  INT4_8 *used_mem, INT4_8 *alloc_call,
                                  INT4_8 *free_call, double *hit_rate,
                                  double *small_rate, size_t *cache,
                                  size_t *stack);

/* HNumMemElements - get number of currently allocated memory blocks         */
extern HLibExport Herror HNumMemElements(INT4_8  *num);

/* HAllocInfo - get line of call/size of all blocks with level >= AllocLevel */
extern HLibExport Herror HAllocInfo(INT max, char *file[], INT4 line[],
                                    INT4 size[], INT4 level[], INT4_8 *num);

/* HPrepMem - prepares memory (allocates+frees memory) => more efficiency    */
extern HLibExport Herror HPrepMem (Hproc_handle proc_handle, size_t size,
                                   size_t *si_ze);

/* HFreeUnusedTmpMem - free any unused temporary memory                      */
extern HLibExport Herror HFreeUnusedTmpMem(Hproc_handle proc_handle);

/*---------------------------------------------------------------------------*/
/*                   Management of Free Usable Memory                        */
/*---------------------------------------------------------------------------*/
/* HInitMem - initialize global cache-memory and free memory management   */
extern HLibExport Herror HInitMem (Hproc_handle proc_handle, size_t size);

/* HCloseG??bMem - shut down management of cache memory and free memory       */
extern HLibExport Herror HCloseMem(void);

/* HMemInitHandle - initialize client-specific memory management handle      */
extern HLibExport Herror HMemInitHandle(Hproc_handle proc_handle);

/* HMemCloseHandle - give free client-specific memory management handle      */
extern HLibExport Herror HMemCloseHandle(Hproc_handle proc_handle);

/* HMemResetHandle - give free client-specific memory management handle      */
extern HLibExport Herror HMemResetHandle(Hproc_handle proc_handle);

/* HXAlloc - allocate free usable memory                                     */
extern HLibExport Herror HXAlloc (Hproc_handle proc_handle, size_t size,
                                  VOIDP void_p);

/* HXAllocMemCheck - allocate free usable memory and collect test info       */
extern HLibExport Herror HXAllocMemCheck (Hproc_handle proc_handle, 
                                          size_t size, const char *file, 
                                          INT4_8 line, INT4 level,
                                          VOIDP void_p);

/* HXFree - give free (free usable) memory (insert it into cache) */
extern HLibExport Herror HXFree(Hproc_handle proc_handle, VOIDP mem);

/* HXFree - give free (free usable) memory (insert it into cache) and test it*/
extern HLibExport Herror HXFreeMemCheck(Hproc_handle proc_handle, VOIDP mem,
                                        const char *file, INT4_8 line);

/* HXRealloc - reallocate memory and test it (memory still available?)       */
extern HLibExport Herror HXRealloc(Hproc_handle proc_handle, VOIDP ptr,
                                   size_t size, VOIDP void_p,
                                   const char *file, INT4_8 line);

/* HXAllocRLNum - allocate permanent memory for specific region 
 * (pass Hrlregion** to region parameter)                                    */
extern HLibExport Herror HXAllocRLNum(Hproc_handle proc_handle, 
                                      VOIDP region, size_t num,
                                      const char *file, INT4_8 line);

/* HXReallocRLNum - reallocate global memory for specific region         
 * (pass Hrlregion** to reg_new parameter but Hrlregion * to reg_old)        */
extern HLibExport Herror HXReallocRLNum(Hproc_handle proc_handle, 
                                        VOIDP reg_old,size_t num_new,
                                        VOIDP reg_new, 
                                        const char *file, INT4_8 line);

/* HXAllocRL - allocate permanent memory for new regions                    
 * (pass Hrlregion** to region parameter)                                    */
extern HLibExport Herror HXAllocRL(Hproc_handle proc_handle, 
                                   VOIDP region,
                                   const char *file, INT4_8 line);

/* HXFreeRL - give free (permanent) memory of a specific region              */
extern HLibExport Herror HXFreeRL(Hproc_handle proc_handle,VOIDP region, 
                                  const char *file, INT4_8 line);

/* HAllocImageCached - fetch image memory from cache                         */
extern HLibExport void* HAllocImageCached(size_t size);

/* HFreeImageCached - cache image memory                                     */
extern HLibExport void HFreeImageCached(VOIDP image);



/*---------------------------------------------------------------------------*/
/*                   Management of Local Free Memory                         */
/*---------------------------------------------------------------------------*/
/* HXAllocLocal - allocate local memory (AllocLevel=H_LOCAL_ALLOC)           */
extern HLibExport Herror HXAllocLocal (Hproc_handle proc_handle, size_t size,
                                       const char *file,
                                       INT4_8 line, VOIDP void_p);

/* HXFreeLocal - give free local memory (insert it in cache) and test it     */
extern HLibExport Herror HXFreeLocal(Hproc_handle proc_handle, 
                                     VOIDP mem, const char *file, 
                                     INT4_8 line);

/* HXReallocLocal - reallocate local memory (AllocLevel=H_LOCAL_ALLOC)       */
extern HLibExport Herror HXReallocLocal(Hproc_handle proc_handle, VOIDP ptr, 
                                        size_t size, VOIDP void_p, 
                                        const char *file, INT4_8 line);

/* HXAllocRLNumLocal - allocate local memory for specific region             
 * (pass Hrlregion** to region parameter)                                    */
extern HLibExport Herror HXAllocRLNumLocal(Hproc_handle proc_handle, 
                                           VOIDP region, size_t num, 
                                           const char *file, INT4_8 line);

/* HXReallocRLNumLocal - reallocate local memory for specific region         
 * (pass Hrlregion** to reg_new parameter but Hrlregion * to reg_old)        */
extern HLibExport Herror HXReallocRLNumLocal(Hproc_handle proc_handle, 
                                             VOIDP reg_old,size_t num_new,
                                             VOIDP reg_new, 
                                             const char *file, INT4_8 line);

/* HXAllocRLLocal -  allocate local memory for new regions                  
 * (pass Hrlregion** to region parameter)                                    */
extern HLibExport Herror HXAllocRLLocal(Hproc_handle proc_handle, 
                                        VOIDP region,
                                        const char *file, INT4_8 line);

/* HXFreeRLLocal - give free (local) memory of a specific region             */
extern HLibExport Herror HXFreeRLLocal(Hproc_handle proc_handle,
                                       VOIDP region, 
                                       const char *file, INT4_8 line);

/* HMemClrLocal - give free the whole local memory (alloc. by HXAllocLocal)  */
extern HLibExport Herror HMemClrLocal(Hproc_handle proc_handle);

/*---------------------------------------------------------------------------*/
/*                         Management of Stack Memory                        */
/*---------------------------------------------------------------------------*/
/* HMemResTTable - change size of table MemTTable for temporary memory       */
extern HLibExport Herror HFreeMemTTable(Hproc_handle proc_handle);

/* HXFreeAllTmp - give free all the allocated temporary memory               */
extern HLibExport Herror HXFreeAllTmp(Hproc_handle proc_handle,
                                      const char *file, INT4_8 line);

/* HMemClrTTable - free (logically) all the allocated stack memory(MemTTable)*/
extern HLibExport Herror HMemClrTTable(Hproc_handle proc_handle);

/* HXAllocTmp - allocate temporary memory within a stack                     */
extern HLibExport Herror HXAllocTmp(Hproc_handle proc_handle, VOIDP ptr,
                                    size_t size, const char *file,
                                    INT4_8 line);

/* HXAllocRLNumTmp - allocate temporary memory for specific region           
 * (pass Hrlregion** to region parameter)                                    */
extern HLibExport Herror HXAllocRLNumTmp(Hproc_handle proc_handle,
                                         VOIDP region, size_t num,
                                         const char *file, INT4_8 line);

/* HXAllocRLTmp - allocate temporary memory for new regions                  
 * (pass Hrlregion** to region parameter)                                    */
extern HLibExport Herror HXAllocRLTmp(Hproc_handle proc_handle, 
                                      VOIDP region,
                                      const char *file, INT4_8 line);

/* HXTestAllTmp - test all stacks for temporary memory of consistency        */
extern HLibExport Herror HXTestAllTmp(Hproc_handle proc_handle,
                                      const char *file, INT4_8 line);

/* HXTestTmp - test the specified temporary memory block of consistency      */
extern HLibExport Herror HXTestTmp(Hproc_handle proc_handle,
                                   VOIDP ptr, const char *file, INT4_8 line, 
                                   HBOOL do_printf);

/* HXFreeTmp - gives free the lastly allocated (!) stack memory              */
extern HLibExport Herror HXFreeTmp(Hproc_handle proc_handle, VOIDP ptr,
                                   const char *file, INT4_8 line);

/* HXFreeRLTmp - gives free temporary memory of a specific region            */
extern HLibExport Herror HXFreeRLTmp(Hproc_handle proc_handle,
                                     VOIDP region,
                                     const char *file, INT4_8 line);

/* HXFreeNTmp - gives free the 'num' lastly allocated stack memory blocks    */
extern HLibExport Herror HXFreeNTmp(Hproc_handle proc_handle, INT num,
                                    const char *file, INT4_8 line);

/* HXFreeUpToTmp - give free stack memory up to a specified memory block     */
extern HLibExport Herror HXFreeUpToTmp(Hproc_handle proc_handle, void *ptr, 
                                       const char *file, INT4_8 line);

/* HFreeUnusedStacks - gives free (physically) all currently unused stacks   */
extern HLibExport Herror HFreeUnusedStacks(Hproc_handle proc_handle);

/* HShrinkRL - gives free the unused memory of a specific region             
 * (pass Hrlregion** to region parameter)                                    */
extern HLibExport Herror HShrinkRL(Hproc_handle proc_handle, 
                                   VOIDP region);

/* HlibAlloc
 * HlibFree
 *  workarround for windows: under windows every DLL has its own memory heap
 *  -> the free method must be called in the same DLL as the malloc
 *---------------------------------------------------------------------------*/
#ifdef _WIN32
extern HLibExport void* Hlib_malloc(size_t size);
extern HLibExport void  Hlib_free(void* mem);
#else
#define Hlib_malloc malloc
#define Hlib_free   free
#endif

/* HUpdateMemInfo - updates the do_low_error_level memory check information  */
extern Herror HUpdateMemInfo(void);

#if defined(__cplusplus)
}
#endif


#endif
