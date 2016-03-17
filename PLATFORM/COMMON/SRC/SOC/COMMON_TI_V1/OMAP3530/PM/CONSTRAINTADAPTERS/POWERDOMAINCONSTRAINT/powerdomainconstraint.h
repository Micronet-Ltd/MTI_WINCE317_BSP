// All rights reserved ADENEO EMBEDDED 2010
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  PowerDomainConstraint.h
//

#ifndef __POWERDOMAINCONSTRAINT_H__
#define __POWERDOMAINCONSTRAINT_H__

#include "omap3530_clocks.h"
#include "omap3530_prcm.h"
//------------------------------------------------------------------------------
//
//  Class:  PowerDomainConstraint
//
class PowerDomainConstraint
{    
protected:
    //-------------------------------------------------------------------------
    // member variables
    //-------------------------------------------------------------------------
    DWORD                       m_rgPowerDomainConstraints[PwrDeviceMaximum];
    DWORD                       m_powerDomainCeiling;
    DWORD                       m_powerDomainFloor;
    DWORD                       m_powerDomainForce;
    DWORD                       m_powerDomainCurrent;
    IndexList<DWORD>            m_IndexList;
        
public:
    //-------------------------------------------------------------------------
    // constructor/destructor
    //-------------------------------------------------------------------------

    PowerDomainConstraint() : m_IndexList(),
                              m_powerDomainCeiling(D0), 
                              m_powerDomainFloor(D4), 
                              m_powerDomainForce((DWORD)-1),
                              m_powerDomainCurrent((DWORD)PwrDeviceUnspecified)
    {
        memset(&m_rgPowerDomainConstraints, 0, sizeof(m_rgPowerDomainConstraints));
    }

protected:
    //-------------------------------------------------------------------------
    // protected routines
    //-------------------------------------------------------------------------

    virtual
    BOOL
    UpdatePowerDomainConstraint(
        DWORD idContext
        );

    virtual
    DWORD
    GetPowerDomain() const = 0;
    
public:
    //-------------------------------------------------------------------------
    // inline public routines
    //-------------------------------------------------------------------------

    void
    Lock()
    {
    }
    
    void
    Unlock()
    {
    }
    
    DWORD        
    GetPowerDomainConstraint()
    {
        return m_powerDomainCurrent;
    }

    HANDLE
    CreatePowerDomainConstraint()
    {
        BOOL b;
        DWORD id;
        DWORD *pDataNode;
        HANDLE rc = NULL;

        Lock();
        b = m_IndexList.NewIndex(&pDataNode, &id);
        Unlock();

        // get new index
        if (b == FALSE)
            {
            goto cleanUp;
            }

        // initialize values
        rc = (HANDLE)(id + 1);
        *pDataNode = (UINT32)PwrDeviceUnspecified;

    cleanUp:    
        return rc;
    }

    BOOL
    ClosePowerDomainContraint(
        HANDLE hConstraintContext
        )
    {
        BOOL rc = FALSE;
        DWORD *pDataNode;
        DWORD id = (DWORD)hConstraintContext - 1;

        // validate
        if (id > m_IndexList.MaxIndex()) goto cleanUp;

        // get data node
        pDataNode = m_IndexList.GetIndex(id);
        if (pDataNode == NULL) goto cleanUp;

        Lock();

        // free any forced constraint
        if (m_powerDomainForce == id)
            {
            m_powerDomainForce = (DWORD)-1;
            }

        // update operating mode
        if (*pDataNode != PwrDeviceUnspecified)
            {
            m_rgPowerDomainConstraints[*pDataNode] -= 1;
            UpdatePowerDomainConstraint(id);
            }

        m_IndexList.DeleteIndex(id);
        
        Unlock();
        
        rc = TRUE;
    cleanUp:    
        return rc;
    } 

     
public:
    //-------------------------------------------------------------------------
    // public routines
    //-------------------------------------------------------------------------

    BOOL        
    InitializePowerDomain(
        _TCHAR const *szContext
        );
    
    virtual
    BOOL
    FlushDomainPowerState() = 0;

    BOOL
    UpdatePowerDomainConstraint(
        HANDLE hConstraintContext, 
        DWORD msg, 
        DWORD newState, 
        UINT  size,
        DWORD idContext
        );
};


//------------------------------------------------------------------------------
//
//  Class:  MpuPowerDomainConstraint
//
class MpuPowerDomainConstraint : public PowerDomainConstraint
{
protected:
    //-------------------------------------------------------------------------
    // protected routines
    //-------------------------------------------------------------------------

    virtual
    BOOL
    UpdatePowerDomainConstraint(
        DWORD idContext
        );

    virtual
    DWORD
    GetPowerDomain() const          {return POWERDOMAIN_MPU;}
    
public:
    //-------------------------------------------------------------------------
    // public routines
    //-------------------------------------------------------------------------
    
    virtual
    BOOL
    FlushDomainPowerState();
};


//------------------------------------------------------------------------------
//
//  Class:  IvaPowerDomainConstraint
//
class IvaPowerDomainConstraint : public PowerDomainConstraint
{
protected:
    //-------------------------------------------------------------------------
    // protected routines
    //-------------------------------------------------------------------------

    virtual
    BOOL
    UpdatePowerDomainConstraint(
        DWORD idContext
        );

    virtual
    DWORD
    GetPowerDomain() const          {return POWERDOMAIN_IVA2;}
    
public:
    //-------------------------------------------------------------------------
    // public routines
    //-------------------------------------------------------------------------
    
    virtual
    BOOL
    FlushDomainPowerState();    
};


//------------------------------------------------------------------------------
//
//  Class:  CorePowerDomainConstraint
//
class CorePowerDomainConstraint : public PowerDomainConstraint
{
protected:
    //-------------------------------------------------------------------------
    // protected routines
    //-------------------------------------------------------------------------

    virtual
    BOOL
    UpdatePowerDomainConstraint(
        DWORD idContext
        );

    virtual
    DWORD
    GetPowerDomain() const          {return POWERDOMAIN_CORE;}

public:
    //-------------------------------------------------------------------------
    // public routines
    //-------------------------------------------------------------------------
    
    virtual
    BOOL
    FlushDomainPowerState();    
};


//------------------------------------------------------------------------------
//
//  Class:  PeripheralPowerDomainConstraint
//
class PeripheralPowerDomainConstraint : public PowerDomainConstraint
{
protected:
    //-------------------------------------------------------------------------
    // protected routines
    //-------------------------------------------------------------------------

    virtual
    BOOL
    UpdatePowerDomainConstraint(
        DWORD idContext
        );

    virtual
    DWORD
    GetPowerDomain() const          {return POWERDOMAIN_PERIPHERAL;}
    
public:
    //-------------------------------------------------------------------------
    // public routines
    //-------------------------------------------------------------------------
    
    virtual
    BOOL
    FlushDomainPowerState();    
};


//------------------------------------------------------------------------------
//
//  Class:  DssPowerDomainConstraint
//
class DssPowerDomainConstraint : public PowerDomainConstraint
{
protected:
    //-------------------------------------------------------------------------
    // protected routines
    //-------------------------------------------------------------------------

    virtual
    BOOL
    UpdatePowerDomainConstraint(
        DWORD idContext
        );

    virtual
    DWORD
    GetPowerDomain() const          {return POWERDOMAIN_DSS;}
    
public:
    //-------------------------------------------------------------------------
    // public routines
    //-------------------------------------------------------------------------
    
    virtual
    BOOL
    FlushDomainPowerState();    
};


//------------------------------------------------------------------------------
//
//  Class:  SgxPowerDomainConstraint
//
class SgxPowerDomainConstraint : public PowerDomainConstraint
{
protected:
    //-------------------------------------------------------------------------
    // protected routines
    //-------------------------------------------------------------------------

    virtual
    BOOL
    UpdatePowerDomainConstraint(
        DWORD idContext
        );

    virtual
    DWORD
    GetPowerDomain() const          {return POWERDOMAIN_SGX;}
    
public:
    //-------------------------------------------------------------------------
    // public routines
    //-------------------------------------------------------------------------
    
    virtual
    BOOL
    FlushDomainPowerState();    
};


//------------------------------------------------------------------------------
//
//  Class:  CameraPowerDomainConstraint
//
class CameraPowerDomainConstraint : public PowerDomainConstraint
{
protected:
    //-------------------------------------------------------------------------
    // protected routines
    //-------------------------------------------------------------------------

    virtual
    BOOL
    UpdatePowerDomainConstraint(
        DWORD idContext
        );

    virtual
    DWORD
    GetPowerDomain() const          {return POWERDOMAIN_CAMERA;}
    
public:
    //-------------------------------------------------------------------------
    // public routines
    //-------------------------------------------------------------------------
    
    virtual
    BOOL
    FlushDomainPowerState();    
};


//------------------------------------------------------------------------------
//
//  Class:  UsbHostPowerDomainConstraint
//
class UsbHostPowerDomainConstraint : public PowerDomainConstraint
{
protected:
    //-------------------------------------------------------------------------
    // protected routines
    //-------------------------------------------------------------------------

    virtual
    BOOL
    UpdatePowerDomainConstraint(
        DWORD idContext
        );

    virtual
    DWORD
    GetPowerDomain() const          {return POWERDOMAIN_USBHOST;}
    
public:
    //-------------------------------------------------------------------------
    // public routines
    //-------------------------------------------------------------------------
    
    virtual
    BOOL
    FlushDomainPowerState();    
};

//------------------------------------------------------------------------------
//
//  Class:  NeonPowerDomainConstraint
//
class NeonPowerDomainConstraint : public PowerDomainConstraint
{
protected:
    //-------------------------------------------------------------------------
    // protected routines
    //-------------------------------------------------------------------------

    virtual
    BOOL
    UpdatePowerDomainConstraint(
        DWORD idContext
        );

    virtual
    DWORD
    GetPowerDomain() const          {return POWERDOMAIN_NEON;}
    
public:
    //-------------------------------------------------------------------------
    // public routines
    //-------------------------------------------------------------------------
    
    virtual
    BOOL
    FlushDomainPowerState();    
};

//------------------------------------------------------------------------------
#endif //__POWERDOMAINCONSTRAINT_H__

