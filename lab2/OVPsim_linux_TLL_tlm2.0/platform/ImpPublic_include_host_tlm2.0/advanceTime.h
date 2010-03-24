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

  Original Author - Abhijit Ghosh, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

   Acknowledgement: The tracing mechanism is based on the tracing
   mechanism developed at Infineon (formerly Siemens HL). Though this
   code is somewhat different, and significantly enhanced, the basics
   are identical to what was originally contributed by Infineon.  The
   contribution of Infineon in the development of this tracing
   technology is hereby acknowledged.

 *****************************************************************************/

/*
 * This the the mechanism for getting systemC kernel to tell us when time is advancing.
 * Thanks to Robert Guenzel  (robert.guenzel@greensocs.com) for advice on how to do this.
 */

#ifndef ADVANCE_TIME_H
#define ADVANCE_TIME_H

#include <cstdio>
#include "icm/icmCpuManager.h"
#include "sysc/tracing/sc_trace.h"

#include "sysc/kernel/sc_simcontext.h"

using namespace sc_core;

class time_advance : public sc_trace_file
{

protected:

    // These are all virtual functions in sc_trace_file. They need to be defined here.

     void trace(const bool& object, const std::string& name) {}
     virtual void trace( const sc_dt::sc_bit& object,  const std::string& name) {}
     void trace(const sc_dt::sc_logic& object, const std::string& name) {}
     void trace(const unsigned char& object, const std::string& name, int width) {}
     void trace(const unsigned short& object, const std::string& name, int width) {}
     void trace(const unsigned int& object, const std::string& name, int width) {}
     void trace(const unsigned long& object, const std::string& name, int width) {}
     void trace(const char& object, const std::string& name, int width) {}
     void trace(const short& object, const std::string& name, int width) {}
     void trace(const int& object, const std::string& name, int width) {}
     void trace(const long& object, const std::string& name, int width) {}
     void trace(const sc_dt::int64& object, const std::string& name, int width) {}
     void trace(const sc_dt::uint64& object, const std::string& name, int width) {}
     void trace(const float& object, const std::string& name) {}
     void trace(const double& object, const std::string& name) {}
     void trace (const sc_dt::sc_uint_base& object, const std::string& name) {}
     void trace (const sc_dt::sc_int_base& object, const std::string& name) {}
     void trace (const sc_dt::sc_unsigned& object, const std::string& name) {}
     void trace (const sc_dt::sc_signed& object, const std::string& name) {}
     void trace( const sc_dt::sc_fxval& object, const std::string& name ) {}
     void trace( const sc_dt::sc_fxval_fast& object, const std::string& name ) {}
     void trace( const sc_dt::sc_fxnum& object, const std::string& name ) {}
     void trace( const sc_dt::sc_fxnum_fast& object, const std::string& name )  {}
     virtual void trace(const sc_dt::sc_bv_base& object, const std::string& name) {}
     virtual void trace(const sc_dt::sc_lv_base& object, const std::string& name) {}
     void trace(const unsigned& object, const std::string& name, const char** enum_literals) {}

     void write_comment(const std::string& comment) {}
     void delta_cycles(bool flag) {  }

     // this is the only virtual function we actually need !!
     void cycle(bool delta_cycle);

private:
     typedef void (*timeFn)(long double time, void *user);

     timeFn  m_advanceFn;
     void   *m_user;

public:
    // constructor and destructor
    time_advance()  {}
    ~time_advance() {}

    void set_time_unit( int exponent10_seconds ) {}
    void set_time_unit( double v, sc_time_unit tu) {}

    // call a static function when time changes, with no dependence on its class.
    void setCallback(timeFn fn, void *user) { m_advanceFn = fn; m_user = user; }
};


#endif
