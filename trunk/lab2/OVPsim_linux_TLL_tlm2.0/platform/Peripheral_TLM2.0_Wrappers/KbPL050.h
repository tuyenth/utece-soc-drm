/*
 *
 * Copyright (c) 2005-2009 Imperas Ltd. All Rights Reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS
 * OF IMPERAS LTD. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED
 * EXCEPT AS MAY BE PROVIDED FOR IN A WRITTEN AGREEMENT WITH IMPERAS LTD.
 *
 */

/* TLM2.0 Wrapper for KbPL050 peripheral model.
 *
 * This model has the following features:
 *     registers are accessed through TLM target "m_bort"
 *     Interrupt output is via signal "m_intr"
 *     user defined attributes can be supplied to the constructor.
 */


#ifndef _KBPL050_H_
#define _KBPL050_H_

#include "icmPeripheral.h"

using namespace sc_core;

class KbPL050 : public icmPeripheral
{

private:
    const char *getModel() {
        return icmGetVlnvString (NULL, "arm.ovpworld.org", "peripheral", "KbPL050", "1.0", "pse");
    }

    const char *getSHL() {
        return icmGetVlnvString (NULL, "arm.ovpworld.org", "peripheral", "KbPL050", "1.0", "model");
    }


public:
    icmSlavePort      m_bport1;
	icmOutputNetPort  m_irq;


    KbPL050(sc_core::sc_module_name name, icmAttrListObject *initialAttrs = 0 )

    : icmPeripheral(name, getModel(), getSHL(), initialAttrs)
    , m_bport1(this, "bport1", 0x1000)
	, m_irq(this, "irq")
    {
    }
};
#endif
