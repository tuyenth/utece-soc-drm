/*
 *
 * Copyright (c) 2005-2009 Imperas Ltd. All Rights Reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS
 * OF IMPERAS LTD. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED
 * EXCEPT AS MAY BE PROVIDED FOR IN A WRITTEN AGREEMENT WITH IMPERAS LTD.
 *
 */

/* TLM2.0 Wrapper for LcdPL110 peripheral model.
 *
 * This model has the following features:
 *     registers are accessed through TLM target "m_bort"
 *     Interrupt output is via signal "m_intr"
 *     user defined attributes can be supplied to the constructor.
 */


#ifndef _LCDPL110_H_
#define _LCDPL110_H_

#include "icmPeripheral.h"

using namespace sc_core;

class LcdPL110 : public icmPeripheral
{

private:
    const char *getModel() {
        return icmGetVlnvString (NULL, "arm.ovpworld.org", "peripheral", "LcdPL110", "1.0", "pse");
    }

    const char *getSHL() {
        return icmGetVlnvString (NULL, "arm.ovpworld.org", "peripheral", "LcdPL110", "1.0", "model");
    }


public:
    icmSlavePort      m_bport1;
	icmSlavePort      m_memport1;


    LcdPL110(sc_core::sc_module_name name, icmAttrListObject *initialAttrs = 0 )

    : icmPeripheral(name, getModel(), getSHL(), initialAttrs)
    , m_bport1(this, "bport1", 0x1000)
    , m_memport1(this, "memory", 0)
    {
    }
};
#endif
