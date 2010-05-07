/*
 *
 * Copyright (c) 2005-2009 Imperas Ltd. All Rights Reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS
 * OF IMPERAS LTD. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED
 * EXCEPT AS MAY BE PROVIDED FOR IN A WRITTEN AGREEMENT WITH IMPERAS LTD.
 *
 */

/* TLM2.0 Wrapper for ICPcontrol peripheral model.
 *
 * This model has the following features:
 *     registers are accessed through TLM target "m_bort"
 *     Interrupt output is via signal "m_intr"
 *     user defined attributes can be supplied to the constructor.
 */


#ifndef _ICPCONTROL_H_
#define _ICPCONTROL_H_

#include "icmPeripheral.h"

using namespace sc_core;

class ICPcontrol : public icmPeripheral
{

private:
    const char *getModel() {
        return icmGetVlnvString (NULL, "arm.ovpworld.org", "peripheral", "IcpControl", "1.0", "pse");
    }


public:
    icmSlavePort      m_bport1;

    ICPcontrol(sc_core::sc_module_name name, icmAttrListObject *initialAttrs = 0 )

    : icmPeripheral(name, getModel(), 0, initialAttrs) 
    , m_bport1(this, "bport1", 0x10)
    {
    }
};
#endif