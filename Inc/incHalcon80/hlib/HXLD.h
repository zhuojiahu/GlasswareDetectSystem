/*****************************************************************************
 * HXLD.h
 ***************************************************************************** 
 *
 * Project:     Halcon/libhalcon
 * Description: Extended Line Description
 *
 * (c) 1996-2007 by MVTec Software GmbH
 *                  www.mvtec.com
 * 
 *****************************************************************************
 * 
 * $Revision: 1.1 $
 * $Date: 2007/01/18 14:08:28 $
 *
 */


#ifndef HXLD_H
#define HXLD_H


#if defined(__cplusplus)
extern "C" {
#endif


extern HLibExport Herror HAllocXLDCont(Hproc_handle proc_id, Hcont **cont,
                                       size_t num_points);
extern HLibExport Herror HAllocXLDContRC(Hproc_handle proc_id, Hcont *cont,
                                         size_t num_points);
extern HLibExport Herror HCopyXLDCont(Hproc_handle proc_id, Hcont *cont_in,
                                      HBOOL preserve_attribs, 
                                      HBOOL preserve_global_attribs, 
                                      Hcont **cont_out);
extern HLibExport Herror HCopyXLDContPart(Hproc_handle proc_id, Hcont *cont_in,
                                          INT4_8  min_index, INT4_8 max_index,
                                          HBOOL preserve_attribs, 
                                          HBOOL preserve_global_attribs,
                                          Hcont **cont_out);
extern HLibExport Herror HFreeXLDCont(Hproc_handle proc_id, Hcont *cont);
extern HLibExport Herror HLookupXLDContAttrib(Hcont *cont, char *name, 
                                              INT4 *indx);
extern HLibExport Herror HAddXLDContAttrib(Hproc_handle proc_id, 
                                           Hcont *cont, char *name,
                                           INT4 *indx);
extern HLibExport Herror HLookupXLDContGlobalAttrib(Hcont *cont, char *name,
                                                    INT4 *indx);
extern HLibExport Herror HAddXLDContGlobalAttrib(Hproc_handle proc_id,
                                                 Hcont *cont, char *name,
                                                 INT4 *indx);
extern HLibExport Herror HXLDFreeContour(Hproc_handle proc_id, Hcont *cont);

extern HLibExport Herror HXLDFreePolygon(Hproc_handle proc_id, Hpoly *poly);

extern HLibExport Herror HXLDFreeParallel(Hproc_handle proc_id, 
					  Hpara_poly *para);
extern HLibExport Herror HXLDFreeModParallel(Hproc_handle proc_id, 
					     Hmod_para *mod_para);
extern HLibExport Herror HXLDFreeExtParallel(Hproc_handle proc_id, 
					     Hext_para *ext_para);


#if defined(__cplusplus)
}
#endif


#endif
