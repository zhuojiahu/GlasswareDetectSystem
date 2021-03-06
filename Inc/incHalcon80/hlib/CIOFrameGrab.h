/*****************************************************************************
 * CIOFrameGrab.h
 ***************************************************************************** 
 *
 * Project:     HALCON/libhalcon
 * Description: image acquisition integration interface (includes)
 *
 * (c) 1996-2007 by MVTec Software GmbH
 *                  www.mvtec.com
 * 
 *****************************************************************************
 *
 * $Revision: 1.43 $
 * $Date: 2007/05/11 15:14:14 $
 *
 *****************************************************************************
 *
 * See also:  "Image Acquisition Integration Programmer's Manual
 *
 *****************************************************************************
 */


#ifndef CIOFRAMEGRAB_H
#define CIOFRAMEGRAB_H

#define FG_INTERFACE_VERSION 3
#define FG_NUM_RESERVED      20

#define FG_MAX_NUM  128  /* max. number of different frame grabber (classes) */
#define FG_MAX_INST 64   /* max. number of handles (instances) per class     */

#ifndef FG_PREFIX
#if defined(WIN32) && defined(H_PARALLEL)
#define FG_PREFIX "parhAcq"
#else
#define FG_PREFIX "hAcq"
#endif
#endif
 
#define FG_QUERY_PORT             0      /* Query: Available port values     */
#define FG_QUERY_CAMERA_TYPE      1      /* Query: Available camera types    */
#define FG_QUERY_GENERAL          2      /* Query: General information       */
#define FG_QUERY_DEFAULTS         3      /* Query: Default values            */
#define FG_QUERY_PARAMETERS       4      /* Query: Additional parameters     */
#define FG_QUERY_INFO_BOARDS      5      /* Query: Actual installed boards   */
#define FG_QUERY_REVISION         6      /* Query: Interface revision        */
#define FG_QUERY_INFO_PARAMETERS  7      /* Query: Info to avail. parameters */
#define FG_QUERY_FIELD            8      /* Query: Available field values    */
#define FG_QUERY_BITS_PER_CHANNEL 9      /* Query: Available bits per channel*/
#define FG_QUERY_COLOR_SPACE      10     /* Query: Available color spaces    */
#define FG_QUERY_EXT_TRIGGER      11     /* Query: Available ext. trigger    */
#define FG_QUERY_DEVICE           12     /* Query: Available device values   */
#define FG_QUERY_PARAMETERS_RO    13     /* Query: Add. read-only parameters */
#define FG_QUERY_PARAMETERS_WO    14     /* Query: Add. write-only parameters*/
#define FG_QUERY_IMAGE_HEIGHT     15     /* Query: Available image_height    */
#define FG_QUERY_IMAGE_WIDTH      16     /* Query: Available image_widht     */
#define FG_QUERY_START_ROW        17     /* Query: Available start_row values*/
#define FG_QUERY_START_COLUMN     18     /* Query: Available start_column    */
#define FG_QUERY_HORIZONTAL_RESOLUTION 19     /* Query: horizontal_resolution*/
#define FG_QUERY_VERTICAL_RESOLUTION   20     /* Query: vertical resolution  */
#define FG_QUERY_LINE_IN          21     /* Query: Available line_in values  */
#define FG_QUERY_PARAMETERS_HIDDEN 22    /* Query: Not for display in        */
                                         /*        generic GUI               */

#define FG_FIRST_FIELD            0      /* grab first field                 */
#define FG_SECOND_FIELD           1      /* grab second field                */
#define FG_NEXT_FIELD             3      /* grab next field                  */
#define FG_FULL_FRAME             2      /* grab frame / interlaced          */
#define FG_PROGRESSIVE_FRAME      4      /* grab frame / progressive         */
#define FG_FIELD_DEFAULT          5      /* default                          */

/* corresponding text encoding: */
#define FG_FIRST_FIELD_TXT               "first" 
#define FG_SECOND_FIELD_TXT              "second"
#define FG_NEXT_FIELD_TXT                "next"  
#define FG_FULL_FRAME_TXT                "interlaced" 
#define FG_PROGRESSIVE_FRAME_TXT         "progressive"
#define FG_FIELD_DEFAULT_TXT             "default"

#define FG_PARAM_HORIZONTAL_RESOLUTION   "horizontal_resolution"
#define FG_PARAM_VERTICAL_RESOLUTION     "vertical_resolution"
#define FG_PARAM_IMAGE_WIDTH             "image_width"
#define FG_PARAM_IMAGE_HEIGHT            "image_height"
#define FG_PARAM_START_ROW               "start_row"
#define FG_PARAM_START_COL               "start_column"
#define FG_PARAM_FIELD                   "field"
#define FG_PARAM_BITS_PER_CHANNEL        "bits_per_channel"
#define FG_PARAM_COLOR_SPACE             "color_space"
#define FG_PARAM_GAIN                    "gain"        // only for compatibilty
#define FG_PARAM_GENERIC                 "generic"
#define FG_PARAM_EXT_TRIGGER             "external_trigger"
#define FG_PARAM_CAMERA_TYPE             "camera_type"
#define FG_PARAM_DEVICE                  "device"
#define FG_PARAM_PORT                    "port"
#define FG_PARAM_LINE_IN                 "line_in"

#define FG_MAX_LUT_LENGTH          65536  /* max. length of a Lut */ 

#define FG_INIT_NAME               "FGInit"

#define FG_MAX_PARAM               1024

/* defines for encoding which method is used for allocating Hrlregions */
#define FG_RLALLOC_PERMANENT       1
#define FG_RLALLOC_TMP             2
#define FG_RLALLOC_LOCAL           4

#if defined(__cplusplus)
extern "C" {
#endif

/*****************************************************************************
 *
 * struct FGInstance
 *
 *****************************************************************************/
 
typedef struct _FGInstance {
  struct _FGClass *fgclass;       /* a pointer to the corresponding class */
  /* ---------------------- regular parameters -------------------------- */
  INT    horizontal_resolution,   /* desired resolution delivered         */
         vertical_resolution;     /* by the board                         */
  INT    image_width,image_height;/* desired image size                   */
  INT    start_row,start_col;     /* upper left corner                    */
  INT    field;                   /* even- or odd-field, full image mode  */
  INT    bits_per_channel;        /* color depth per pixel & channel      */
  char   color_space[MAX_STRING]; /* "gray", "rgb", "yuv", ...            */
  float  gain;                    /* for backwards compatibility only!    */
  HBOOL  external_trigger;        /* trigger mode                         */
  char   camera_type[MAX_STRING]; /* used camera type (fg-specific!)      */
  char   device[MAX_STRING];      /* device name                          */
  INT    port;                    /* port to use                          */
  INT    line_in;                 /* multiplexer input line               */
  Hcpar  *generic;                /* generic parameter                    */
  INT    num_generic;
  /* --------------------- miscellaneous parameters --------------------- */
  INT4_8 mode;                    /* current operating mode               */
  INT    width_max,height_max;    /* maximum image size                   */
  INT    num_channels;            /* number of image channels             */
  HBOOL  async_grab;              /* TRUE <=> async grabbing engaged      */
  Himage *image;                  /* image objects to grab into (aux.)    */
  void   *gen_pointer;            /* generic pointer (auxiliary)          */
  /* ------------------- external memory allocation --------------------- */
  HBOOL  halcon_malloc;           /* TRUE <-> standard memory allocation  */
                                  /* by HNewImage                         */
  HImageFreeProc clear_proc;      /* pointer to specific clear function   */
                                  /* (if halcon_malloc==FALSE)            */
} FGInstance;


/*****************************************************************************
 *
 * struct FGClass
 *
 *****************************************************************************/
 
typedef struct _FGClass {
  /* -------------------------- internal -------------------------------- */
  char  name[MAX_STRING];        /* frame grabber name (interface module) */
  void  *lib_handle;             /* handle of interface library           */
  INT   interface_version;       /* current HALCON frame grabber          */
                                 /* interface version                     */
  /* ------------------- properties / management ------------------------ */
  HBOOL  available;              /* supported for the current platform    */
  INT    instances_num;          /* current number instances (INTERNAL!)  */
  INT    instances_max;          /* maximum number of instances           */
  FGInstance *instance[FG_MAX_INST]; /* list of instances (INTERNAL!)     */
  /* ----------------- interface-specific functions --------------------- */
  FGInstance** (*OpenRequest)(Hproc_handle proc_id,FGInstance *fginst);
  Herror (*Open)             (Hproc_handle proc_id,FGInstance *fginst);
  Herror (*Close)            (Hproc_handle proc_id,FGInstance *fginst);
  Herror (*Grab)             (Hproc_handle proc_id,FGInstance *fginst,
                              Himage *image,INT *num_image);
  Herror (*GrabStartAsync)   (Hproc_handle proc_id,FGInstance *fginst,
			      double maxDelay);
  Herror (*GrabAsync)        (Hproc_handle proc_id,FGInstance *fginst,
			      double maxDelay,Himage *image,
                              INT *num_image);
  Herror (*GrabData)         (Hproc_handle proc_id,FGInstance *fginst,
                              Himage **image,INT **num_channel,INT *num_image,
                              Hrlregion ***region,INT *num_region,
                              Hcont ***cont,INT *num_cont,
                              Hcpar **data,INT *num_data);
  Herror (*GrabDataAsync)    (Hproc_handle proc_id,FGInstance *fginst,
                              double maxDelay,Himage **image,
                              INT **num_channel,INT *num_image,
                              Hrlregion ***region,INT *num_region,
                              Hcont ***cont,INT  *num_cont,
                              Hcpar **data,INT *num_data);
  Herror (*Info)             (Hproc_handle proc_id,INT queryType, 
                              char **info,Hcpar **values,INT *numValues);
  Herror (*SetParam)         (Hproc_handle proc_id, FGInstance *fginst, 
                              char *param,Hcpar *value,INT num);
  Herror (*GetParam)         (Hproc_handle proc_id, FGInstance *fginst, 
                              char *param,Hcpar *value,INT *num);
  Herror (*SetLut)           (Hproc_handle proc_id,FGInstance *fginst,
                              INT4_8 *red,INT4_8 *green,INT4_8 *blue,INT num);
  Herror (*GetLut)           (Hproc_handle proc_id,FGInstance *fginst,
                              INT4_8 *red,INT4_8 *green,INT4_8 *blue,INT *num);
  /* -------------------------- default values -------------------------- */
  INT    horizontal_resolution,   /* desired resolution delivered         */
         vertical_resolution;     /* by the board                         */
  INT    image_width,image_height;/* desired image size                   */
  INT    start_row,start_col;     /* upper left corner                    */
  INT    field;                   /* even- or odd-field, full image mode  */
  INT    bits_per_channel;        /* color depth per pixel & channel      */
  char   color_space[MAX_STRING]; /* "gray", "rgb", "yuv", ...            */
  float  gain;                    /* for backwards compatibility only!    */
  HBOOL  external_trigger;        /* trigger mode                         */
  char   camera_type[MAX_STRING]; /* used camera type (fg-specific!)      */
  char   device[MAX_STRING];      /* device name                          */
  INT    port;                    /* port to use                          */
  INT    line_in;                 /* multiplexer input line               */
  Hcpar  *generic;                /* generic parameter                    */
  INT    num_generic;
  /* ------------------------- miscellaneous ---------------------------- */
  void   *reserved[FG_NUM_RESERVED];
} FGClass;


typedef Herror (*FG_INIT_FUNC) (Hproc_handle ,FGClass *);

/* function for integrating frame grabbers into the system using a linked 
   interface (instead of a dynamic object accessed online */
extern HLibExport Herror HAddFgClass(Hproc_handle proc_id, char *name, 
		                     FG_INIT_FUNC init_func);

/* function for writing the frame grabber specific default values
   into an Hcpar array */
extern HLibExport Herror HFgGetDefaults(Hproc_handle proc_id, FGClass *fgClass,
                                        Hcpar **values, INT *numValues);

/* helper function in HALCON library for displaying low-level error messages */
extern HLibExport Herror IOPrintErrorMessage(char *err);

#if defined(__cplusplus)
}
#endif


#endif

