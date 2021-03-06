/*****************************************************************************
 * HInterfaceC.h
 ***************************************************************************** 
 *
 * Project:      Halcon/libhalcon
 * Author:       M. Lueckenhaus
 * Description:  Prototypes for HALCON/C interface
 *
 * (c) 1996-2005 by MVTec Software GmbH
 *                  www.mvtec.com
 * 
 *****************************************************************************
 * $Source: /halcon/CVSROOT/include/HInterfaceC.h,v $
 * $Revision: 1.7 $
 * $Date: 2006/08/10 12:08:47 $
 * $Author: froeschlin $
 * $State: Exp $
 * 
 */

#ifndef HINTERFACE_C_H
#define HINTERFACE_C_H

/*===========================================================================*/
/*                             Prototypes                                    */
/*===========================================================================*/

extern LIntExport Herror 
HcPreCall(INT proc_index, Hproc_handle *proc_handle);

extern LIntExport Herror 
HcCall(Hproc_handle proc_handle);

extern LIntExport Herror 
HcPostCall(Hproc_handle proc_handle, Herror err);

extern LIntExport Herror 
HcStoreIO(Hproc_handle proc_handle, INT par, Hobject inObjVar);

extern LIntExport Herror 
HcStoreICL(Hproc_handle proc_handle, INT par, Hlong value);

extern LIntExport Herror 
HcStoreICD(Hproc_handle proc_handle, INT par, double value);

extern LIntExport Herror 
HcStoreICS(Hproc_handle proc_handle, INT par, const char *value);

extern LIntExport Herror 
HcStoreICT(Hproc_handle proc_handle, INT par, Htuple value);

extern LIntExport Herror 
HcStoreOO(Hproc_handle proc_handle, INT par, Hobject *outObjVar, Herror err);

extern LIntExport Herror 
HcStoreOCL(Hproc_handle proc_handle, INT par, Hlong *outCtrlVar, Herror err);

extern LIntExport Herror 
HcStoreOCD(Hproc_handle proc_handle, INT par, double *outCtrlVar, Herror err);

extern LIntExport Herror 
HcStoreOCS(Hproc_handle proc_handle, INT par, char *outCtrlVar, Herror err);

extern LIntExport Herror 
HcStoreOCT(Hproc_handle proc_handle, INT par, Htuple *outCtrlVar, Herror err);

#endif
