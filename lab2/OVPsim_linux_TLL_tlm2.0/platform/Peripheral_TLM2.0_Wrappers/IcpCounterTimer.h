/*
 *
 * Copyright (c) 2005-2009 Imperas Ltd. All Rights Reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS
 * OF IMPERAS LTD. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED
 * EXCEPT AS MAY BE PROVIDED FOR IN A WRITTEN AGREEMENT WITH IMPERAS LTD.
 *
 */

/* TLM2.0 Wrapper for IntICP peripheral model.
 *
 * This model has the following features:
 *     registers are accessed through TLM target "m_bort"
 *     Interrupt output is via signal "m_intr"
 *     user defined attributes can be supplied to the constructor.
 */


#ifndef _ICPPIT_H_
#define _ICPPIT_H_

#include "icmPeripheral.h"

using namespace sc_core;

class ICPPIT : public icmPeripheral
{

private:
    const char *getModel() {
        return icmGetVlnvString (NULL, "arm.ovpworld.org", "peripheral", "IcpCounterTimer", "1.0", "pse");
    }
	

public:
    icmSlavePort      m_bport1;
	icmOutputNetPort  m_irq0;
	icmOutputNetPort  m_irq1;
	icmOutputNetPort  m_irq2;

    ICPPIT(sc_core::sc_module_name name, icmAttrListObject *initialAttrs = 0 )

    : icmPeripheral(name, getModel(), 0, initialAttrs) 
    , m_bport1(this, "bport1", 0x1000)
	, m_irq0(this, "irq0")
	, m_irq1(this, "irq1")
	, m_irq2(this, "irq2")
    {
    }
};
#endif
