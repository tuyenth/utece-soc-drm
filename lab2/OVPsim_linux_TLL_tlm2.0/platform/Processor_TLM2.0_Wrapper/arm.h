/*
 *
 * Copyright (c) 2005-2009 Imperas Ltd. All Rights Reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS
 * OF IMPERAS LTD. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED
 * EXCEPT AS MAY BE PROVIDED FOR IN A WRITTEN AGREEMENT WITH IMPERAS LTD.
 *
 */

/* TLM2.0 Wrapper for OVP ARM processor model.
 *
 * This model has the following features:
 *     Configures the OVP model to the ARM7 variant
 *     Supports as many interrupts as the OVP model
 *     newlib semihosting is enabled.
 *     Interrupt inputs are "irq" and "fiq"
 *     A single TLM initiator "m_socket" for code and data transactions
 *         which uses a 32-bit address space.
 */

#ifndef __TLM_ARM_H__
#define __TLM_ARM_H__

#include "icmCpu.h"

using namespace sc_core;

class arm : public icmCpu
{
    SC_HAS_PROCESS( arm );

private:
    icmAttrListObject *defaultAttrs() {
        icmAttrListObject *p = new icmAttrListObject();
        // add configuration options here
		p->addAttr("showHiddenRegs", "0");
        p->addAttr("compatibility", "ISA");
        p->addAttr("variant", "ARM926EJ-S");
        p->addAttr("endian", "little");
        p->addAttr("mips", 200);
		p->addAttr("override_debugMask",0);
        return p;
    }

    const char *getModel() {
        return icmGetVlnvString (NULL, "arm.ovpworld.org", "processor", "arm", "1.0", "model");
    }

   const char *getSHL() {
        return icmGetVlnvString (NULL, "arm.ovpworld.org", "semihosting", "armNewlib", "1.0", "model");
    }

public:
    icmCpuInterrupt *fiq, *irq;

    arm(
        sc_core::sc_module_name name,
        const unsigned int ID,
        icmNewProcAttrs attrs = ICM_ATTR_DEFAULT
    )

    : icmCpu(name, ID, "arm", getModel(), "modelAttrs", getSHL(), defaultAttrs(), attrs )
    {
        connectInterrupt(&fiq, "fiq");
        connectInterrupt(&irq, "irq");
    }
};
#endif
