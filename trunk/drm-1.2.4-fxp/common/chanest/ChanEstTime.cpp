/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	Channel estimation in time direction, base class
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later 
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/

#include "ChanEstTime.h"


/* Implementation *************************************************************/
void CPilotModiClass::InitRot(CParameter& Parameter) 
{
	/* Inits for timing correction. We need FFT size and start carrier */
	/* Pre calculate the argument of the exp function */
  //	rArgMult = (_REAL) -2.0 * crPi / Parameter.iFFTSizeN;
        rArgMult = -2 * (205970/ Parameter.iFFTSizeN);
	
	/* Index of minimum useful carrier */
	iKminAbs = Parameter.iShiftedKmin;
}

_COMPLEX CPilotModiClass::Rotate(const _COMPLEX cI, const int iCN, 
								 const int iTiDi) const
{
  _COMPLEX_I cI_INT = _COMPLEX_I( ((_INT) real(cI)*1<<8), ((_INT) imag(cI)*1<<8)   );
         _REAL tmp1, tmp2;              //Added by DANIEL
         _INT  tmp1_fixed, tmp2_fixed;  //Added by DANIEL
   

	/* If "iTiDi" equals "0", rArg is also "0", we need no cos or sin
	   function */
	if (iTiDi != 0)
	{
		/* First calculate the argument */
		//const _REAL rArg = rArgMult * iTiDi * (iKminAbs + iCN);
                _REAL rArg = (rArgMult * iTiDi * (iKminAbs + iCN));
                rArg = rArg/(1<<16);

                tmp1 = cos(rArg);
                tmp2 = sin(rArg);	
                        tmp1_fixed = (tmp1*(1<<8));
	       		tmp2_fixed = (tmp2*(1<<8));
			//_COMPLEX     _tmp_org   = _COMPLEX(cos(rArg), sin(rArg)) * cI;
			_COMPLEX_I   _tmp_INT = _COMPLEX_I(tmp1_fixed, tmp2_fixed) * cI_INT;//this is what should be return
                
                tmp1 =(_REAL) real(_tmp_INT);
                tmp2 =(_REAL) imag(_tmp_INT);

		      tmp1 = tmp1/(1<<16);
		      tmp2 = tmp2/(1<<16);
 

		//  printf("DANIEL Complex INT    REAL %d IMAG %d  \n" , (real(_tmp_INT)),(imag(_tmp_INT)));
		//printf("DANIEL Complex INT    REAL %d IMAG %d  \n" , (real(_tmp_INT)),(imag(_tmp_INT)));
		//printf("DANIEL Complex DOUBLE REAL %f IMAG %f  \n" , (real(_tmp_org)* (1<<16)),(imag(_tmp_org)*(1<<16)) );


		/* * exp(2 * pi * TimeDiff / norm) */
		//		return _COMPLEX(cos(rArg), sin(rArg)) * cI;
		return _COMPLEX(tmp1, tmp2);
	}
	else
		return cI;
}
