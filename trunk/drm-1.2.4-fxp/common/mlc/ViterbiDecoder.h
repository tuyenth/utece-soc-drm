/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *
 * 02/04/08
 * Modified by John Jones
 * Converted all floating point operations to integer.	
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

#if !defined(VITERBI_DECODER_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
#define VITERBI_DECODER_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_

#include "../GlobalDefinitions.h"
#include "../tables/TableMLC.h"
#include "ConvEncoder.h"
#include "ChannelCode.h"


/* Definitions ****************************************************************/

/* Removed all SIMD Instructions - JLJ */
/* Removed all MAX_LOG_MAP Instructions - JLJ */

/* Data type for Viterbi metric */
# define _VITMETRTYPE				int 	// Used to be float
# define _DECISIONTYPE				_BINARY

/* We initialize each new block of data all branches-metrics with the following
   value exept of the zero-state. This can be done since we actually KNOW that
   the zero state MUST be the transmitted one. The initialization vaule should
   be fairly high. But we have to be careful choosing this parameter. We
   should not take the largest value possible of the data type of the metric
   variable since in the Viterbi-routine we add something to this value and
   in that case we would force an overrun! */
# define MC_METRIC_INIT_VALUE		((_VITMETRTYPE) 10000)


/* In case of MAP decoder, all metrics must be stored for the entire input
   vector since we need them for the forward and backward direction */
/*# define METRICSET(a)		vecrMetricSet */


/* Classes ********************************************************************/
class CViterbiDecoder : public CChannelCode
{
public:
	CViterbiDecoder();
	virtual ~CViterbiDecoder() {}

	_FREAL	Decode(CVector<CFDistance>& vecNewDistance,
				   CVector<_DECISION>& vecOutputBits);
	void	Init(CParameter::ECodScheme eNewCodingScheme,
				 CParameter::EChanType eNewChannelType, int iN1, int iN2,
			     int iNewNumOutBitsPartA, int iNewNumOutBitsPartB,
			     int iPunctPatPartA, int iPunctPatPartB, int iLevel);

protected:
	/* Two trellis data vectors are needed for current and old state */
	int			vecTrelMetric1[MC_NUM_STATES];  // Used to be float
	int			vecTrelMetric2[MC_NUM_STATES];	// Used to be float

	int			vecrMetricSet[MC_NUM_OUTPUT_COMBINATIONS];  
				//Used to be _REAL[16]

	CVector<int>		veciTablePuncPat;

	int			iNumOutBits;
	int			iNumOutBitsWithMemory;

	CMatrix<_DECISIONTYPE>	matdecDecisions;
};


#endif // !defined(VITERBI_DECODER_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
