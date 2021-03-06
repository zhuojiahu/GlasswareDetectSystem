/*****************************************************************************
 * HCtype.h
 *****************************************************************************
 *
 * Project:      Halcon (libhalcon)
 * Description:  (Global) data structures defining the processing context 
 *               for HALCON operator instances and HALCON operator threads
 *
 * (c) 1996-2007 by MVTec Software GmbH
 *                  www.mvtec.com
 *
 *****************************************************************************
 *
 * $Revision: 1.79 $
 * $Date: 2007/02/27 09:15:03 $
 *
 */

#ifndef HCTYPE_H
#define HCTYPE_H

#if defined(__cplusplus)
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* remarks about the name convention in this file:                           */
/* INSTANCE  denotes a single "instance" of a HALCON operator (e.g. the call */
/*           of an operator or a HALCON thread that has been derived from    */
/*           an instance when parallelizing the instance). All structures    */
/*           that carry the name INSTANCE contain information that is only   */
/*           relevant for a single operator call/thread.                     */
/* SYS       stands for "system" and denotes any information that is         */
/*           relevant for more than one operator instance.                   */
/* SHARED    means that this information is shared between all threads of an */
/*           instance. Thus the denoted information is either read-only or   */
/*           can only be written under mutual exclusion.                     */
/* EXCLUSIVE indicates that this information is used exclusively by one      */
/*           instance.                                                       */
/*---------------------------------------------------------------------------*/

typedef Herror (*HCoreProcedure)(Hproc_handle);
typedef void* (*HExtAllocFunc)(size_t);
typedef void  (*HExtFreeFunc)(void*);

/*---------------------------------------------------------------------------*/
/* H_SYS_INFO_READ_ONLY - data structure that contains `global' information  */
/* used by several HALCON modules and operators. The name `READ_ONLY'        */
/* indicates that the information is mainly written only once and further on */
/* only read. Thus it is implemented as one shared memory structure          */
/* without any synchronization mechanisms. That means that all HALCON        */
/* functions work on the same H_SYS_INFO_READ_ONLY instance.                 */
/*---------------------------------------------------------------------------*/
typedef enum {nullified=0,exclusive=(1<<0),shared=(1<<1)} GlobalMemCacheState;
typedef struct{
  /*-------------------------------- HALCON version -------------------*/
  char          *Version;         /* version number; format: "V.rr"    */
  char          *LastUpdate;      /* update date: "dd.mm.jjjj"         */
  /*-------------------------------- number of procedures (operators) -*/
  INT2          NumSysProc;       /* number of system operators        */
  INT2          NumUserProc;      /* number of user defined operators  */
  INT2          NumProc;          /* total number of operators         */
  /*-------------------------------- program language interface -------*/
  INT4          VersionFlag;      /* consistent versions of HALCON     */
  /* core and language interface?      */
  /*-------------------------------- HALCON debugging spy -------------*/
  /*                                 (alterable via 'set_spy')         */
  HBOOL         NewSpy;           /* protocol HALCON operator call?    */
  HBOOL         Spy;              /* protocol i/o of images/regions?   */
  HBOOL         SpyDB;            /* "dump" of HALCON database         */
  HBOOL         SpyPredecessor;   /* protocol accesses on HALCON db ?  */
  HBOOL         SpyError;         /* protocol occurrences of errors?   */
  HBOOL         SpyAllProc;       /* HALCON operator debugging         */
  /*-------------------------------- memory management ----------------*/
  INT4          AllocMem;         /* number of bytes to alloc at start */
  INT4          PrepMem;          /* bytes allocated at starttime      */
  INT4          CurrentAlloc;     /* Count the max. temp. mem. used by */
  INT4          CurrentMaxAlloc;  /* an operator. This includes stack  */
  INT           PreviousMaxAlloc; /* and heap.                         */
  HBOOL         alloctmp_single_block; /* use one block per tmp. mem.? */
  INT4          alloctmp_max_blocksize;/*max number of bytes -> tmp mem*/
  GlobalMemCacheState global_memory_cache;/* use global cache for smaller
                                           * memory blocks? */
  HExtAllocFunc extern_alloc_funct;/*ext funct for alloc of result imgs*/
  HExtFreeFunc  extern_free_funct;/* ext funct for free of result imgs */
  /*-------------------------------- programming environment ----------*/
  char          ProgLanguage[40]; /* !!!!! OBSOLETE !!!!!              */
  char          Language[MAX_STRING];/* language for i/o (GB, D,...)   */
  /*-------------------------------- window management ----------------*/
  char          DefaultFont[MAX_STRING]; /*default font for open_window*/
  /*-------------------------------- graphical context ----------------*/
  HBOOL         DefaultColors;    /* default HALCON graphic colors(Y/N)*/
  /*-------------------------------- common `knowledge base' ----------*/
  UINT1         *HSqrtTab;        /* table for Hfsqrt and Hisqrt       */
  /*-------------------------------- special HALCON objedts -----------*/
  Hkey          EMPTY;            /* empty region                      */
  /*-------------------------------- architecture compatibility -------*/
  /*                                 (hide different software features)*/
  HBOOL         SingleLUT;        /* only one LUT per screen (X11R3)   */
  INT4          MinInteger;       /* maximal/minimal integer           */
  INT4	        MaxInteger;       /* in current language               */
  /*-------------------------------- Tabellierter Arc-Tangens ---------*/
  HBYTE         *AtanTabLow;      /* arctan (+/- 1 deg); for ATANTAB   */
  HBYTE         *AtanTabMed;      /* arctan (+/- 1 degree)             */
  HBYTE         *AtanTabHigh;     /* arctan (+/- 1 degree)             */
  /*-------------------------------- processor information ------------*/
  HBOOL         mmx_available;    /* usage of MMX (Intel) enabled?     */
  HBOOL         sse_available;    /* usage of SSE (Intel) enabled?     */
  HBOOL         sse2_available;   /* usage of SSE2 (Intel) enabled?    */
  HBOOL         sse3_available;   /* usage of SSE3 (Intel) enabled?    */
  HBOOL         msb_first;        /* Byte order of the processor       */
  /*-------------------------------- operating system information -----*/
  char          os_name[64];      /* Name of the operating system      */
  char          os_version[64];   /* Version and revision of the OS    */
  /*-------------------------------- parallel or sequential processing */
  UINT1         parallel;         /* 0: HALCON works sequentially      */
                                  /* 1: HALCON works in parallel       */
                                  /* 2: HALCON works in parallel AND   */
                                  /*    uses automatic parallelization */
  UINT          cpu_num;          /* number of available processors    */
  HBOOL         enduring_threads; /* use permanent threads?            */
  /*-------------------------------- mutex variable for HCrImage  ---- */
  H_pthread_mutex_t mutex_new_f;  /* locking access to image lists     */
  /*-------------------------------- clock mode ---------------------- */
  INT           clock_mode;       /* elapsed_time or processor_time?   */
  /*-------------------------------- connection ---------------------- */
  UINT4         max_connection;   /* max. num of regions returned by
                                   * the operator connection           */
  /*-------------------------------- shape model creation -------------*/
  HBOOL         pregen_shape_models; /* Completely pregenerate shape   */
                                  /* models?                           */
  /*-------------------------------- ocr train file format ------------*/
  INT           ocr_trainf_version; /* write ocr train files with      */
                                    /* binary format?                  */
  /*-------------------------------- image resolution ---------------- */
  UINT4         image_dpi;        /* DPI value to store in image files */
  /*-------------------------------- shape model creation -------------*/
  HBOOL         border_shape_models; /* Match shape models that cross  */
                                  /* the border of the image?          */
  /*-------------------------------- processor cache information ------*/
  INT4          level1_data_cache;/* 1st level data cache size (kB)    */
  INT4          level1_inst_cache;/* 1st level instruction cache (kB)  */
  INT4          level2_cache;     /* 2nd level cache size (kB)         */
  INT4          level3_cache;     /* 3rd level cache size (kB)         */
  /*-------------------------------- filename encoding ----------------*/
  INT           locale_filename_encoding; /* use native or UTF8        */
                                  /* character encoding for filenames  */
} H_SYS_INFO_READ_ONLY;



/*---------------------------------------------------------------------------*/
/* H_OPERATOR_INFO - there is one instance of this structure for every       */
/* single HALCON operator. This structure contains all the static            */
/* characteristica of an operator. The several instances are built up once   */
/* when booting HALCON and further on are only readed.                       */
/*---------------------------------------------------------------------------*/
typedef struct {
        UINT1         NumInpObjPar;     /* number of input object parameter  */
	UINT1         NumOutpObjPar;    /* number of output object parameter */
	UINT1         NumInpCtrlPar;    /* number of input control parameter */
	UINT1         NumOutpCtrlPar;   /* number of output control parameter*/
	UINT4         ModuleMask;       /* bit pattern for module check      */
        HCoreProcedure Proc;            /* Pointer to c-procedure            */
	const char    *LogicalName;     /* name of operator in host language */
	const char    *PhysicalName;    /* name of C-procedure               */
	char          **ParameterName[4];/* names of the parameters          */
	const char    *ParameterType[2];/* I/i, F/f, S/s, N/n, d             */
                                        /* with: I/i    integer              */
                                        /*       F/f    float                */
                                        /*       S/s    string               */
                                        /*       N/n    ??                   */
                                        /*       d      ??                   */
        UINT2         ParameterCosts[MAX_INP_CTRL_PAR]; /* weight of single  */
                                        /* control parameter costs           */
                                        /* (for checking applicability. of   */
                                        /* operator parallelization)         */
        UINT2         ParameterPost[MAX_OUTP_CTRL_PAR]; /* info. about post  */
                                        /* processing of  outp. ctrl. par.s  */
                                        /* (info. about how to merge result  */
                                        /* values after parallel processing) */
        UINT1         RegionPost;       /* info. about post processing of    */
                                        /* region components of outp. obj.s  */
                                        /* after parallelization on domain l.*/
	INT4          HelpIndex;        /* index in help file                */
	INT4          InfoLength;       /* number of bytes in Halcon.hlp     */
	char          *Class[2];        /* chapter/sub chapter               */
	char          *Application[3];  /* operation type, application, or   */
                                        /* special application               */
	INT4          package_id;       /* index of the corresponding package*/
        UINT1         process_excl_local; /* processing context:             */
                                        /* if 0 -> no restriction            */
                                        /* if 1 -> process locally by        */
                                        /*         main thread/process       */
                                        /* if 2 -> process under mutual      */
                                        /*         exclusion, i.e. the same  */
                                        /*         operator may only run     */
                                        /*         once at a time            */
                                        /* if 3 -> process exclusively, i.e. */
                                        /*         no other operator may run */
                                        /*         while this operator works;*/
                                        /*         the latter also means     */
                                        /*         that the operator is      */
                                        /*         also processed locally    */
        UINT4_8       parallelization;  /* potentiality of parallelization   */
                                        /* (tuple par., channel par., etc.)  */
        UINT4         parallelMinCosts; /* minimal costs for which a         */
                                        /* parallelization gets lucrative    */
        UINT1         parallelRegionsplit; /* potentiality to split region   */
                                        /* (if 0: regions may be splitted    */
                                        /* disjunctively, else: use over-    */
                                        /* lapping dependent on the content  */
                                        /* of the parallelRegionsplit-th     */
                                        /* input control parameter)          */
} H_OPERATOR_INFO;




/*---------------------------------------------------------------------------*/
/* ATTENTION! Increase H_OPERATOR_INFO_VERSION whenever you change the       */
/*            H_OPERATOR_INFO structure! Furthermore check the function      */
/*            HMain whether it has to be extended to read the operator       */
/*            information (H_OPERATOR_INFO) of user extensions properly      */
/*            (this may also imply changes of the hcomp code)...             */
/*---------------------------------------------------------------------------*/
#define H_OPERATOR_INFO_VERSION 3.1



/*---------------------------------------------------------------------------*/
/* H_INSTANCE_INFO_EXCLUSIVE - data structure that contains information      */
/* relevant only for a specific operator instance (call of a HALCON          */
/* operator). This information is only valid for the processing time of the  */
/* associated instance/HALCON thread. All read/write accesses to the         */
/* information are done exclusively (thus different HALCON threads of the    */
/* same instance differ in the content of this structure).                   */
/*---------------------------------------------------------------------------*/
typedef struct {
  /*-------------------------------- image<->object information -------------*/
  INT2    obj_images;                   /* max. number of images per object  */
  INT2    def_obj_images;               /* default num. of images per object */
  /*-------------------------------- operator parameters --------------------*/
  /*______________________________________________input object parameters:___*/
  Hkey    *InpObjList[MAX_INP_OBJ_PAR+1]; /* for every input object par      *
					   * an array of object keys         */
  INT4    InpObjNum[MAX_INP_OBJ_PAR+1];  /* number of objects per input obj. *
					  * parameter; index: 1..inp_obj     */

  /*_____________________________________________input control parameters:___*/
  Hctuple *InpCtrlList;     /* for storing inp. ctrl. values;    *
			     * one Hctuple element per parameter */
  INT1    *freeICP;
  HBOOL   ctrl_inherited;   /* true, if all ctrl. values are     *
			     * inherited from a father instance  *
			     * (only important for correctly     *
			     * freeing the InpCtrlList)          *
			     *                                   */
  /*_____________________________________________output object parameters:___*/
  Hkey    *OutpObjList[MAX_OUTP_OBJ_PAR+1];/* for every output object par.   *
					    * an array of objects keys       */
  INT4    OutpObjNum[MAX_OUTP_OBJ_PAR+1];  /* number of objects per output   *
					    * object par.; index: 1..        */
  INT4    OutpObjLen[MAX_OUTP_OBJ_PAR+1];  /* length of OutpObjList per      *
					    * output object par.; index: 1.. */
  /*____________________________________________output control parameters:___*/
  Hctuple OutpCtrlList[MAX_OUTP_CTRL_PAR]; /* array of output ctrl values    *
					    * (one Hctuple per parameter)    */
  HBOOL   usedOCP[MAX_OUTP_CTRL_PAR];      /* used this parameter?           */
  INT1    freeOCP[MAX_OUTP_CTRL_PAR];      /* free OutpCtrlPar when clearing
                                            * OutpCtrlList?                  */

  /*-------------------------------- control parameter string memory --*/
  char    *StringMem;       /* string memory for HPGet(C)Par (ro)*/
  char    *LastStringMem;   /* string memory for HPGet(C)Par (ro)*/
  /*-------------------------------- memory management ----------------*/
  VOIDP   Mem_handle;       /* management lists for database     */
  /*-------------------------------- object database ------------------*/
  VOIDP   DB_handle;        /* management lists for database     */
  /*-------------------------------- parallelisation info -------------*/
  INT     parallel_method;  /* flag of parallelisation method:   */
                            /* only HAG_PAR_SPLIT_NONE,          */
                            /*      HAG_PAR_SPLIT_DOMAIN by now  */
} H_INSTANCE_INFO_EXCLUSIVE;

#define HAG_PAR_SPLIT_NONE    0
#define HAG_PAR_SPLIT_TUPLE   1<<1
#define HAG_PAR_SPLIT_CHANNEL 1<<2
#define HAG_PAR_SPLIT_DOMAIN  1<<3




/*---------------------------------------------------------------------------*/
/* H_INSTANCE_INFO_SHARED - data structure that contains information         */
/* relevant only for a specific operator instance (call of a HALCON          */
/* operator). This information is only valid for the processing time of the  */
/* associated instance. Several HALCON threads derived from the same         */
/* operator instance all work on the same instance of the                    */
/* H_INSTANCE_INFO_SHARED structure.                                         */
/* Nevertheless not all elements of this structure need synchronization, as  */
/* there are several elements (marked with 'ro') that are written only once  */
/* when starting an operator instance and are further used read-only. Thus   */
/* these elements need no mutex variable whereas all other shared elements   */
/* that are used with read/write-access have got an associated mutex         */
/* variable for synchronizing write accesses.                                */
/*---------------------------------------------------------------------------*/
typedef struct {
        /*-------------------------------- programming language interface ---*/
        HBOOL         CompactObj;       /* Processing of object tuples;  (ro)*/
	
        /*-------------------------------- sharing image data (HCrImage) ----*/
        HBOOL         NewFilter;        /* multiple regions per image        */
        Hkey          *nf_inp_key;      /* key of first input image object   */
        Hkey          *nf_rem_inp_keys; /* keys of remaining input object 
                                         * parameters                        */
        INT2          *nf_index;        /* filter index                      */
        INT2          *nf_img_type;     /* image types                       */
        Hkey          *nf_out;          /* key of output image object        */
        INT1          **nf_image1;      /* pointer to image data             */
        INT1          **nf_image2;      /* second pointer to image data      */
        INT4          nf_num;           /* number of stored images           */
        INT4          nf_num_alloc;     /* number of allocated key pointers  */
        H_pthread_mutex_t mutex_NewFil; /* mutex var. for locking access to  */
                                        /* all above elements; Newfilter etc.*/
                                        
        /*--- prog. lang. ff (appended to preserve binary compatibility) --- */
        char          *lang_int_id;     /* originating language interface id */
                                        
}H_INSTANCE_INFO_SHARED;

/*---------------------------------------------------------------------------*/
/* H_SYS_INFO_EXCLUSIVE - data structure containing (system-wide)            */
/* information that is relevant for the processing of HALCON operators.      */
/* This information is valid beyond the duration of the processing of one    */
/* single operator. To nevertheless allow exclusive access to the            */
/* information elements, every operator instance (or HALCON thread) gets its */
/* own copy of the H_SYS_INFO_EXCLUSIVE structure. The changed values in     */
/* this copy are then written back to a central (global) version of the      */
/* H_SYS_INFO_EXCLUSIVE structure ("reference instance"), as soon as the     */
/* processing of the associated operator instance (or HALCON thread) has     */
/* been completed. When writing back values, mutex variables are used for    */
/* mutual exclusion of write accesses to the central H_SYS_INFO_EXCLUSIVE    */
/* structure. Moreover, whenever writing back a value, a special update      */
/* mechanism is used to control the update under consideration of the        */
/* new value AND the old value (in the central structure). The following     */
/* keywords are used to describe the update mechanism:                       */
/* maxi_valid => write back the maximum of the new and the old value         */
/* mini_valid => write back the minimum of the new and the old value         */
/* write_back => write back the new value without considering the old value  */
/*               (this overwrites the old value and may only lead to         */
/*                sensible results when working sequentially)                */
/*---------------------------------------------------------------------------*/
typedef struct {
	/*-------------------------------- image format/size ----------------*/
	INT4          imsize;           /* image format: max(Height/Width)   */
                                        /* (maxi_valid)                      */
	INT4          Width;            /* max. width of all images          */
                                        /* (maxi_valid)                      */
	INT4          Height;           /* max. height of all images         */
                                        /* (maxi_valid)                      */
	/*-------------------------------- region management ----------------*/
	INT4          CurrentRL;        /* current (max.) number of runs     */
                                        /* (maxi_valid)                      */
        /*-------------------------------- processing context ---------------*/
        /*                                 ('set_system' or 'set_check')     */
	HBOOL         ClipRegion;       /* clipping at image (width/height)  */
                                        /* (write_back)                      */
  HBOOL         StoreEmptyRegion; /* segmentation results              */
                                        /* (write_back)                      */
	HBOOL         InitNewImage;     /* set to zero before filtering      */
                                        /* (write_back)                      */
	UINT1         Neighbour;        /* Neighbourhood definition 4/8      */
                                        /* (write_back)                      */
        /*----------------------------- operator-specific information -------*/
	HBOOL         DoIntZoom;        /* int/double coordinates in         */
	                                /* set_part, image_transform         */
                                        /* (write_back)                      */
        /*----------------------------- operator-specific information -------*/
        H_pthread_mutex_t mutex;        /* mutex variable to write back 
                                         * structure to reference instance   */
}H_SYS_INFO_EXCLUSIVE;


/*---------------------------------------------------------------------------*/
/* H_SYS_INFO_SHARED - data structure containing (system-wide) information   */
/* that is relevant for the processing of HALCON operators. This information */
/* is valid beyond the duration of the processing of one single operator.    */
/* In contrast to the H_SYS_INFO_EXCLUSIVE structure, where it was possible  */
/* to use a local instance-specific copy for the duration of a single        */
/* operator processing, the H_SYS_INFO_SHARED structure contains all those   */
/* (system-wide) information where changes must be propagated immediately    */
/* to all other operator instances. Thus there is only one global instance   */
/* of the H_SYS_INFO_SHARED structure in the memory where all read/write     */
/* accesses work on. Therefore there exists one mutex variable for every     */
/* data element that guarantees that all write accesses to the associated    */
/* element are only allowed under mutual exclusion (whereas read accesses    */
/* can be done at any time without synchronization).                         */
/*---------------------------------------------------------------------------*/
typedef struct {
        /*-------------------------------- memory management ----------------*/
        /* INT2          AllocID;        * procedure class that alloc. mem   */
        /* INT4          MinCollectSize; * not jet used                      */
        /*-------------------------------- HALCON image database ------------*/
        HBOOL         DBInit;           /* reset_obj_db has been executed?   */
        H_pthread_mutex_t mutex_DBIni;  /* mutex variable for DBInit         */
        /*-------------------------------- error checks (set_check...) ------*/
        INT4          check;            /* error checking                    */
        H_pthread_mutex_t mutex_che;    /* mutex variable for check          */
        /*-------------------------------- control error handling -----------*/
        Herror        NoObjResult;      /* return value for no inp. objects  */
        H_pthread_mutex_t mutex_NoObjRes; /* mutex variable for NoObjResult  */
        Herror        EmptyObjResult;   /* return value for empty objects    */
        H_pthread_mutex_t mutex_EmpObjRes; /* mutex var. for EmptyObjResult  */
        /*-------------------------------- window management ----------------*/
        char          WindowName[MAX_STRING]; /* "default": as usual,        */
                                        /*else: window name for next window  */
        H_pthread_mutex_t mutex_WinNam; /* mutex variable for WindowName     */
        char          IconName[MAX_STRING]; /* "default": as usual,          */
                                        /* else: icon name for next window   */
        H_pthread_mutex_t mutex_IcoNam; /* mutex variable for IconName       */
        char          WinBackColor[MAX_STRING]; /* background color of window*/
        H_pthread_mutex_t mutex_WinBacCol; /* mutex variable for WinBackColor*/
        char          WinBorderColor[MAX_STRING]; /* frame color of window   */
        H_pthread_mutex_t mutex_WinBorCol; /* mutex var. for WinBorderColor  */
        INT4          XPackage;         /* optimize Transfer to X-Server     */
        H_pthread_mutex_t mutex_XPac;   /* mutex variable for XPackage      ?*/
        HBOOL         BackingStore;     /* of windows (only X-Windows)       */
        H_pthread_mutex_t mutex_BacSto; /* mutex variable for BackingStore   */
        INT2          GraphicBorderWidth;  /* border width for open_window   */
        H_pthread_mutex_t mutex_GraBorWid; /* m. var. for GraphicBorderWidth */
        /*-------------------------------- graphical context ----------------*/
        INT2          NumGrey[9];       /*number of grey values in Lut/planes*/
        H_pthread_mutex_t mutex_NumGre; /* mutex variable for NumGrey        */
        INT2          NumGraphicColors[9]; /* number of HALCON graphic       */
                                        /* colors in Lut                     */
        H_pthread_mutex_t mutex_NumGraCol; /* mutex var. for NumGraphicColors*/
        INT2          NumGreyPercentage;/* this percentage of the number of  */
                                        /* grey values must be available     */
        H_pthread_mutex_t mutex_NumGrePer; /* m. var. for NumGreyPercentage  */
        INT2          NumGraphicColorsPercentage; /* this percentage of the  */
                                        /* number of graphic colors must     */
                                        /* be available                      */
        H_pthread_mutex_t mutex_NumGraColPer; /* mutex variable for Num....  */
        char          UserDefinedColors[MAX_GRAPHIC_COLOR]
                                       [MAX_COLOR_NAME_LENGTH]; /* array of  */
                                        /*   names for HALCON graphic colors */
        H_pthread_mutex_t mutex_UseDefCol;  /* mutex variable for UserDef... */
        char          DrawMode[MAX_STRING]; /* e.g. for draw_rectangle1      */
        H_pthread_mutex_t mutex_DraMod; /* mutex variable for DrawMode       */
        HBOOL         IgnoreColormap;   /* ignore LUT while reading an image */
        H_pthread_mutex_t mutex_IgnCol; /* mutex variable for IgnoreColormap */
        HBOOL         UpdateLUT;        /* update of look-up-table           */
        H_pthread_mutex_t mutex_UpdLUT; /* mutex variable for UpdateLUT      */
        /*-------------------------------- visualization --------------------*/
        INT           Int2Bits;         /* for displaying int2 images        */
        H_pthread_mutex_t mutex_Int2Bit; /* mutex variable for Int2Bits      */
        /*-------------------------------- processing context ---------------*/
        /*                                 ('set_system' or 'set_check')     */
        HBOOL         FlushGraphic;     /*automatic flush after each operator*/
        H_pthread_mutex_t mutex_FluGra; /* mutex variable for FlushGraphic   */
        HBOOL         FlushFile;        /*automatic flush after each operator*/
        H_pthread_mutex_t mutex_FuFil;  /* mutex variable for FlushFile      */
        HBOOL         TestRegImaInters; /* test if region inside the image   */
        H_pthread_mutex_t mutex_TesRegImaInt; /* m. var. for TestRegImaInters*/
        /*-------------------------------- processing non-quadratic pixel ---*/
        float         PixelRatio;       /* width / height                    */
        H_pthread_mutex_t mutex_PixRat; /* mutex variable for PixelRatio     */
        INT4          WidthMult;        /* (width * WidthMult) * 2**20       */
        H_pthread_mutex_t mutex_WidMul; /* mutex variable for WidthMult      */
        INT4          HeightMult;       /* (height * HeightMult) * 2**20     */
        H_pthread_mutex_t mutex_HeiMul; /* mutex variable for HeightMult     */
        HBOOL         ZoomPixel;        /* PixelRatio != 1.0                 */
        H_pthread_mutex_t mutex_ZooPix; /* mutex variable for ZoomPixel      */
        float         PixelWidth;       /*                                   */
        H_pthread_mutex_t mutex_PixWid; /* mutex variable for PixelWidth     */
        float         PixelHeight;      /*                                   */
        H_pthread_mutex_t mutex_PixHei; /* mutex variable for PixelHeight    */
        /*-------------------------------- ressources: directory names ------*/
        /*                                 (alterable via 'set_system')      */
        char          ImageDir[MAX_IMAGE_DIR]; /* second dir. for images     */
        H_pthread_mutex_t mutex_ImaDir; /* mutex variable for ImageDir       */
        char          HelpDir[MAX_STRING];  /* help directory                */
        H_pthread_mutex_t mutex_HelDir; /* mutex variable for HelpDir        */
        char          ReferenceDir[MAX_STRING];     /* !!!!! OBSOLETE !!!!!  */
        H_pthread_mutex_t mutex_RefDir;             /* !!!!! OBSOLETE !!!!!  */
        char          HTMLDir[MAX_STRING];          /* !!!!! OBSOLETE !!!!!  */
        H_pthread_mutex_t mutex_HTMLDir;            /* !!!!! OBSOLETE !!!!!  */
        char          LicenseDir[MAX_STRING]; /* directory for license files */
        H_pthread_mutex_t mutex_LicDir;       /* mutex var. for LicenseDir   */
        char          LUTDir[MAX_STRING];     /* directory for LUT's         */
        H_pthread_mutex_t mutex_LUTDir;       /* mutex variable for LUTDir   */
        char          FilterDir[MAX_STRING];  /* directory for Filter kernels*/
        H_pthread_mutex_t mutex_FilDir;       /* mutex variable for FilterDir*/
        char          OCRDir[MAX_STRING];     /* directory for OCR-fonts     */
        H_pthread_mutex_t mutex_OCRDir;       /* mutex variable for OCRDir   */
        char          CalibDir[MAX_STRING];   /* directory for calib. files  */
        H_pthread_mutex_t mutex_CalibDir;     /* mutex variable for CalibDir */
        char          LS_R[MAX_STRING]; /* name of file with image file list */
        H_pthread_mutex_t mutex_LS_R;   /* mutex variable for LS_R           */
        /*-------------------------------- compatibilty variables -----------*/
        /*                                 (architecture properties)         */
        HBOOL         OnlyLines;        /* for bug in X11R3 (IOx11DispRL)    */
        H_pthread_mutex_t mutex_OnlLin; /* mutex variable for OnlyLines      */
        /*----------------------------- HALCON modules ----------------------*/
        UINT4         UsedModules;      /* number of loaded packages         */
        H_pthread_mutex_t mutex_UseMod; /* mutex variable for UsedModules    */
        /*----------------------------- HALCON flexlm -----------------------*/
        H_pthread_mutex_t mutex_flexlm; /* mutex var. for (externally)       */
                                        /* locking calls of flexlm functions */
        /*----------------------------- HALCON boot process -----------------*/
        H_pthread_mutex_t mutex_boot;   /* mutex var. for locking boot proc. */
        /*----------------------------- HALCON Measure Tool -----------------*/
        H_pthread_mutex_t mutex_measure; /* mutex var. for measure tool      */
        /*----------------------------- HALCON socket operators -------------*/
        H_pthread_mutex_t mutex_socket; /* mutex var. for booting sockets    */
        /*----------------------------- for threadsafe strtok usage ---------*/
        H_pthread_mutex_t mutex_strtok; /* mutex var. for using strtok       */
        /*----------------------------- for threadsafe localtime usage ------*/
        H_pthread_mutex_t mutex_ltime;  /* mutex var. for using localtime    */
        /*----------------------------- threadsafe HALCON graphics operators */
        H_pthread_mutex_t mutex_graphics;  /* mutex var. for graphics operat.*/
        /*----------------------------- threadsafe FFT functionality ------- */
        H_pthread_mutex_t mutex_fft;    /* mutex var. for FFT operators      */
        /*----------------------------- threadsafe Lapack library access --- */
        H_pthread_mutex_t mutex_lapack; /* mutex var. for lapack functions   */
}H_SYS_INFO_SHARED;

/*---------------------------------------------------------------------------*/
/* HALCON_PROC_HANDLE - data structure that encapsulates all the context     */
/* information of a HALCON operator instance (or any derived HALCON thread). */
/*---------------------------------------------------------------------------*/
typedef struct HALCON_PROCEDURE_HANDLE
{
  struct HALCON_PROCEDURE_HANDLE *generator; /* Operator, that generated a */
                                        /* H.-thread or NULL at root level */
  H_INSTANCE_INFO_EXCLUSIVE *info_excl; /* instance data (thread-specific) */
  H_INSTANCE_INFO_SHARED  *info_shared; /* instance data (thread-inherited)*/
  H_INSTANCE_INFO_SHARED  *info_shared_cache; /* structure cache when 
                                               * thread-inherited          */
  INT4                      proc_index; /* Index of HALCON operator        */
  H_SYS_INFO_EXCLUSIVE  *sys_info_excl; /* system-wide data (exclusive)    */
  H_SYS_INFO_SHARED   *sys_info_shared; /* system-wide date (shared)       */
  HBOOL                    initialized; /* is data structure initialized?  */
}H_PROC_HANDLE;
typedef H_PROC_HANDLE *Hproc_handle_;


#if defined(__cplusplus)
}
#endif

#endif
