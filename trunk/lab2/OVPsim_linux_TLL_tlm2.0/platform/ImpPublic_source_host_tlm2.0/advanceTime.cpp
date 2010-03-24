/*****************************************************************************

  The following code is derived, directly or indirectly, from the SystemC
  source code Copyright (c) 1996-2006 by all Contributors.
  All Rights reserved.

  The contents of this file are subject to the restrictions and limitations
  set forth in the SystemC Open Source License Version 2.4 (the "License");
  You may not use this file except in compliance with such restrictions and
  limitations. You may obtain instructions on how to receive a copy of the
  License at http://www.systemc.org/. Software distributed by Contributors
  under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
  ANY KIND, either express or implied. See the License for the specific
  language governing rights and limitations under the License.

 *****************************************************************************/

/*****************************************************************************

   Acknowledgement: The tracing mechanism is based on the tracing
   mechanism developed at Infineon (formerly Siemens HL).  The
   contribution of Infineon in the development of this tracing
   technology is hereby acknowledged.

 *****************************************************************************/

#include "advanceTime.h"

void time_advance::cycle(bool this_is_a_delta_cycle)
{
    if (!this_is_a_delta_cycle && m_advanceFn) {
        m_advanceFn(sc_time_stamp().to_seconds(), m_user);
    }
}
