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


#ifndef _INTICP_H_
#define _INTICP_H_

#include "icmPeripheral.h"

using namespace sc_core;

class IntICP : public icmPeripheral
{

private:
    const char *getModel() {
        return icmGetVlnvString (NULL, "arm.ovpworld.org", "peripheral", "IntICP", "1.01", "pse");
    }



public:
    icmSlavePort      m_bport1;
	icmOutputNetPort  m_irq;
	icmOutputNetPort  m_fiq;
	icmInputNetPort   m_ir0;
	icmInputNetPort   m_ir1;
	icmInputNetPort   m_ir2;
	icmInputNetPort   m_ir3;
	icmInputNetPort   m_ir4;
	icmInputNetPort   m_ir5;
	icmInputNetPort   m_ir6;
	icmInputNetPort   m_ir7;
	icmInputNetPort   m_ir8;
	icmInputNetPort   m_ir9;
	
	icmInputNetPort   m_ir16;

	icmInputNetPort   m_ir23;
	icmInputNetPort   m_ir24;

    IntICP(sc_core::sc_module_name name, icmAttrListObject *initialAttrs = 0 )

    : icmPeripheral(name, getModel(), 0, initialAttrs) 
    , m_bport1(this,"bport1", 0x1000)
	, m_irq(this, "irq")
	, m_fiq(this, "fiq")
	, m_ir0(this, "ir0")
	, m_ir1(this, "ir1")
	, m_ir2(this, "ir2")
	, m_ir3(this, "ir3")
	, m_ir4(this, "ir4")
	, m_ir5(this, "ir5")
	, m_ir6(this, "ir6")
	, m_ir7(this, "ir7")
	, m_ir8(this, "ir8")
	, m_ir9(this, "ir9")
	, m_ir16(this, "ir16")
	, m_ir23(this,"ir23")
	, m_ir24(this,"ir24")
    {
    }
};
#endif
