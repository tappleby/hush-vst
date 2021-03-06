/*
  WDL - besselfilter.h
  (c) Theo Niessink 2011
  <http://www.taletn.com/>

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.


  This file provides classes for a low-pass Bessel filter design using the
  matched Z-transform method.

  This Bessel filter implementation was originally extracted from from the
  source code of mkfilter, written by A.J. Fisher.
  <http://www-users.cs.york.ac.uk/~fisher/mkfilter>


  Example #1:

	// 8th order anti-alias filter
	#define WDL_BESSEL_FILTER_ORDER 8
	#include "besselfilter.h"

	int oversampling = 8;

	WDL_BesselFilterCoeffs bessel;
	WDL_BesselFilterStage filter;

	bessel.Calc(0.5 / (double)oversampling);
	filter.Reset();

	for (int i = 0; i < nFrames; ++i)
	{
		filter.Process(inputs[0][i], bessel.Coeffs());
		outputs[0][i] = filter.Output();
	}

  Example #2:

	#include "besselfilter.h"

	int order = 4;
	int oversampling = 8;

	// 2 cascaded filters
	WDL_BesselFilterStage filter[2];
	filter[0].Reset();
	filter[1].Reset();

	WDL_BesselFilterCoeffs coeffs;
	coeffs.Calc(0.5 / (double)oversampling, order);

	for (int i = 0; i < nFrames; ++i)
	{
		filter[0].Process(inputs[0][i], &coeffs);
		filter[1].Process(filter[0].Output(), &coeffs);
		outputs[0][i] = filter[1].Output();
	}

  Example #3:

	#include "besselfilter.h"

	int order = 8;
	int oversampling = 8;

	WDL_BesselFilter bessel;
	bessel.Calc(0.5 / (double)oversampling, order);
	bessel.Reset();

	for (int i = 0; i < nFrames; ++i)
	{
		bessel.Process(inputs[0][i]);
		outputs[0][i] = bessel.Output();
	}

*/


#ifndef _BESSELFILTER_H_
#define _BESSELFILTER_H_


#include <math.h>
#ifdef _MSC_VER
#pragma warning(disable:4996) // hypot
#endif

#include <string.h>
#include <assert.h>

#include "wdltypes.h"


// Defining WDL_BESSEL_FILTER_ORDER will make the Bessel filter order fixed,
// which increases efficiency.
#ifdef WDL_BESSEL_FILTER_ORDER
	#if !(WDL_BESSEL_FILTER_ORDER >= 1 && WDL_BESSEL_FILTER_ORDER <= 10)
		#error WDL_BESSEL_FILTER_ORDER should be in 1..10 range
	#endif
	#define WDL_BESSEL_FILTER_MAX WDL_BESSEL_FILTER_ORDER

// Defining WDL_BESSEL_FILTER_MAX limits the maximum Bessel filter order,
// which reduces buffer sizes.
#else
	#if defined(WDL_BESSEL_FILTER_MAX) && WDL_BESSEL_FILTER_MAX < 1
		#define WDL_BESSEL_FILTER_MAX 1
	#elif !defined(WDL_BESSEL_FILTER_MAX) || WDL_BESSEL_FILTER_MAX > 10
		#define WDL_BESSEL_FILTER_MAX 10
	#endif
#endif


class WDL_BesselFilterCoeffs
{
	friend class WDL_BesselFilterStage;

public:
	inline WDL_BesselFilterCoeffs() {}

#ifdef WDL_BESSEL_FILTER_ORDER
	// alpha = cornerFreq / (oversampling * sampleRate)
	inline WDL_BesselFilterCoeffs(const double alpha) { Calc(alpha); }

	void Calc(double alpha)
	{
		const int order = WDL_BESSEL_FILTER_ORDER;

#else
	inline WDL_BesselFilterCoeffs(const double alpha, const int order) { Calc(alpha, order); }

	void Calc(double alpha, const int order)
	{
		assert(order >= 1 && order <= WDL_BESSEL_FILTER_MAX);
		mOrder = order;
#endif

		assert(alpha >= 1e-37 && alpha < 0.5);
		alpha *= 6.283185307179586476; // 2.*M_PI

		// compute S-plane poles for prototype LP filter
		// transform prototype into appropriate filter type (lp)
		// given S-plane poles & zeros, compute Z-plane poles & zeros, by matched z-transform
		complex zplane[WDL_BESSEL_FILTER_MAX];
		int p = (order*order) / 4;
		int n = 0;
		if (order & 1) zplane[n++] = exp(multiply(alpha, mPoles[p++]));
		for (int i = 0; i < order / 2; ++i)
		{
			zplane[n++] = exp(multiply(alpha, mPoles[p]));
			zplane[n++] = exp(multiply(alpha, conjugate(mPoles[p++])));
		}

		// compute product of poles or zeros as a polynomial of z
		complex coeffs[WDL_BESSEL_FILTER_MAX + 1];
		coeffs[0] = 1.;
		for (int i = 1; i <= order; ++i) coeffs[i] = 0.;

		for (int i = 0; i < order; ++i)
		{
			// multiply factor (z-w) into coeffs
			complex w = minus(zplane[i]);
			for (int i = order; i >= 1; --i) coeffs[i] = add(multiply(w, coeffs[i]), coeffs[i - 1]);
			coeffs[0] = multiply(w, coeffs[0]);
		}
		#ifndef NDEBUG
		// check computed coeffs of z^k are all real
		for (int n = 0; n <= order; ++n)
		{
			// mkfilter: coeff of z^n is not real; poles/zeros are not complex conjugates
			assert(fabs(coeffs[n].im) <= 1e-10);
		}
		#endif

		// given Z-plane poles [& zeros], compute [top &] bot polynomials in Z, and then recurrence relation
		complex gain = 0.;
		for (int i = order; i >= 0; --i) gain = add(gain, coeffs[i]);
		gain = inverse(gain);
		mCoeffs[0] = 1./hypot(gain.im, gain.re);
		for (int i = 1, j = order - 1; i <= order; ++i, --j) mCoeffs[i] = -(coeffs[j].re / coeffs[order].re);
	}

	inline int Order() const
	{
		#ifdef WDL_BESSEL_FILTER_ORDER
			return WDL_BESSEL_FILTER_ORDER;
		#else
	 		return mOrder;
		#endif
	}

	inline const double* Coeffs() const { return mCoeffs; }
	inline double Gain() const { return mCoeffs[0]; }

protected:
	double mCoeffs[WDL_BESSEL_FILTER_MAX + 1];

	#ifndef WDL_BESSEL_FILTER_ORDER
		int mOrder;
	#endif

	// Minimalistic complex number implementation

	struct complex
	{
		double re, im;
		complex() {}
		complex(const double r, const double j = 0.): re(r), im(j) {}
	};

	// z = z1 + z2
	inline complex add(const complex z1, const complex z2) const
	{
		return complex(z1.re + z2.re, z1.im + z2.im);
	}

	// z = r * z
	inline complex multiply(const double r, const complex z) const
	{
		return complex(r * z.re, r * z.im);
	}

	// z = z1 * z2
	inline complex multiply(const complex z1, const complex z2) const
	{
		return complex(z1.re * z2.re - z1.im * z2.im, z1.re * z2.im + z1.im * z2.re);
	}

	// z = -z
	inline complex minus(const complex z) const
	{
		return complex(-z.re, -z.im);
	}

	// z = conjugate(z)
	inline complex conjugate(const complex z) const
	{
		return complex(z.re, -z.im);
	}
	
	// z = 1/z
	inline complex inverse(const complex z) const
	{
		const double r = z.re*z.re + z.im*z.im;
		return complex(z.re / r, -z.im / r);
	}

	// z = exp(z)
	inline complex exp(const complex z) const
	{
		const double r = ::exp(z.re);
		return complex(r * cos(z.im), r * sin(z.im));
	}

	// Precalculated Bessel poles
	static const complex mPoles[10 * 3];
} WDL_FIXALIGN;

// table produced by mkfilter's bessel -- N.B. only one member of each C.Conj. pair is listed
const WDL_BesselFilterCoeffs::complex WDL_BesselFilterCoeffs::mPoles[10 * 3] =
{
	complex(-9.9999999999999978e-001, 0.0000000000000000e+000), complex(-1.1016013305921748e+000, 6.3600982475704204e-001),
	complex(-1.3226757999104561e+000, 0.0000000000000000e+000), complex(-1.0474091610089453e+000, 9.9926443628064676e-001),
	complex(-1.3700678305514509e+000, 4.1024971749375211e-001), complex(-9.9520876435027783e-001, 1.2571057394546732e+000),
	complex(-1.5023162714474929e+000, 0.0000000000000000e+000), complex(-1.3808773258604621e+000, 7.1790958762678447e-001),
	complex(-9.5767654856269568e-001, 1.4711243207304141e+000), complex(-1.5714904036160653e+000, 3.2089637422258727e-001),
	complex(-1.3818580975965893e+000, 9.7147189071159734e-001), complex(-9.3065652294686840e-001, 1.6618632689426145e+000),
	complex(-1.6843681792730136e+000, 0.0000000000000000e+000), complex(-1.6120387662260254e+000, 5.8924450693124097e-001),
	complex(-1.3789032167953983e+000, 1.1915667778006367e+000), complex(-9.0986778062340656e-001, 1.8364513530362705e+000),
	complex(-1.7574084004017807e+000, 2.7286757510254800e-001), complex(-1.6369394181268191e+000, 8.2279562513962257e-001),
	complex(-1.3738412176373478e+000, 1.3883565758775174e+000), complex(-8.9286971884713073e-001, 1.9983258436412903e+000),
	complex(-1.8566005012059337e+000, 0.0000000000000000e+000), complex(-1.8071705349890272e+000, 5.1238373057326947e-001),
	complex(-1.6523964845794235e+000, 1.0313895669915880e+000), complex(-1.3675883097976385e+000, 1.5677337122426156e+000),
	complex(-8.7839927616393221e-001, 2.1498005243204834e+000), complex(-1.9276196913244821e+000, 2.4162347094517739e-001),
	complex(-1.8421962445659017e+000, 7.2725759775348364e-001), complex(-1.6618102413500957e+000, 1.2211002185915161e+000),
	complex(-1.3606922783857325e+000, 1.7335057426584299e+000), complex(-8.6575690170894881e-001, 2.2926048309837426e+000)
};


#ifdef WDL_BESSEL_FILTER_ORDER
	#define WDL_BESSEL_FILTER_OUTPUT(n) (coeffs[WDL_BESSEL_FILTER_ORDER - n + 1] * mOutput[WDL_BESSEL_FILTER_ORDER - n + 1])
#endif

class WDL_BesselFilterStage
{
public:
	inline WDL_BesselFilterStage() {}
	inline WDL_BesselFilterStage(const double value) { Reset(value); }

	inline void Reset() { memset(mOutput, 0, sizeof(mOutput)); }

	inline void Reset(const double value)
	{
		for (int i = 0; i < WDL_BESSEL_FILTER_MAX; ++i) mOutput[i] = value;
	}

#ifdef WDL_BESSEL_FILTER_ORDER

	inline void Process(const double input, const double* const coeffs)
	{
		#if WDL_BESSEL_FILTER_ORDER >= 10
			mOutput[10] = mOutput[9];
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 9
			mOutput[9] = mOutput[8];
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 8
			mOutput[8] = mOutput[7];
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 7
			mOutput[7] = mOutput[6];
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 6
			mOutput[6] = mOutput[5];
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 5
			mOutput[5] = mOutput[4];
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 4
			mOutput[4] = mOutput[3];
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 3
			mOutput[3] = mOutput[2];
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 2
			mOutput[2] = mOutput[1];
		#endif
		mOutput[1] = mOutput[0];

		mOutput[0] = coeffs[0] * input
		#if WDL_BESSEL_FILTER_ORDER >= 10
			+ WDL_BESSEL_FILTER_OUTPUT(10)
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 9
			+ WDL_BESSEL_FILTER_OUTPUT(9)
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 8
			+ WDL_BESSEL_FILTER_OUTPUT(8)
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 7
			+ WDL_BESSEL_FILTER_OUTPUT(7)
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 6
			+ WDL_BESSEL_FILTER_OUTPUT(6)
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 5
			+ WDL_BESSEL_FILTER_OUTPUT(5)
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 4
			+ WDL_BESSEL_FILTER_OUTPUT(4)
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 3
			+ WDL_BESSEL_FILTER_OUTPUT(3)
		#endif
		#if WDL_BESSEL_FILTER_ORDER >= 2
			+ WDL_BESSEL_FILTER_OUTPUT(2)
		#endif
		+ WDL_BESSEL_FILTER_OUTPUT(1);
	}

	inline void Process(const double input, const WDL_BesselFilterCoeffs* const bessel) { Process(input, bessel->mCoeffs); }

#else // #elif !defined(WDL_BESSEL_FILTER_ORDER)

	inline void Process(const double input, const double* const coeffs, const int order)
	{
		double output = coeffs[0] * input;
		for (int i = order; i > 0; --i)
		{
			mOutput[i] = mOutput[i - 1];;
			output += coeffs[i] * mOutput[i];
		}
		mOutput[0] = output;
	}

	inline void Process(const double input, const WDL_BesselFilterCoeffs* const bessel) { Process(input, bessel->mCoeffs, bessel->mOrder); }

#endif

	inline double Output() const { return mOutput[0]; }

protected:
	double mOutput[WDL_BESSEL_FILTER_MAX + 1];
} WDL_FIXALIGN;


class WDL_BesselFilter: public WDL_BesselFilterCoeffs, public WDL_BesselFilterStage
{
public:
	inline WDL_BesselFilter() {}

	#ifdef WDL_BESSEL_FILTER_ORDER
		inline WDL_BesselFilter(const double alpha) { Calc(alpha); }
		inline void Process(const double input) { WDL_BesselFilterStage::Process(input, mCoeffs); }
	#else
		inline WDL_BesselFilter(const double alpha, const int order) { Calc(alpha, order); }
		inline void Process(const double input) { WDL_BesselFilterStage::Process(input, mCoeffs, mOrder); }
	#endif
} WDL_FIXALIGN;


#endif // _BESSELFILTER_H_
