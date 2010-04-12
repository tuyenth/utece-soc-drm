//http://search.safaribooksonline.com/0596007612/cplusplusckbk-CHP-8-SECT-14#snippet
//http://www.codeguru.com/cpp/cpp/algorithms/math/article.php/c12097/

//define &operator
//enable casting as double for fabs(), may just modify Metric.h
#ifndef _FXP_H
#define _FXP_H

#ifdef FXP_MATH
#include <stdlib.h>
#include <math.h>
#endif

#define FXP_PREC    16 

#define FXP_ONE 65536
#define FXP_MONE -FXP_ONE

//NOTE: Valid only for FXP_PREC = 16
#define FX16_PI 0x3243F
#define FX16_2PI 0x64873
#define FX16_PIO2 0x1921F
#define FX16_PIO4 0xC90F
#define FX16_E 0x2B7E1		//2.71828182846
#define FX16_LN10 0x24D76	//2.30258519299
#define FX16_ONE 0x10000

#define FX_PI FXP(FX16_PI,0)
#define FX_2PI FXP(FX16_2PI,0)
#define FX_PIO2 FXP(FX16_PIO2,0)
#define FX_PIO4 FXP(FX16_PIO4,0)
#define FX_E FXP(FX16_E,0)
#define FX_LN10 FXP(FX16_LN10,0)
#define FX_ONE FXP(FX16_ONE,0)


class FXP
{
	//FXP(FXP x) : value(x.value) {};
	//friend const FXP operator+(const FXP lhs, const FXP rhs);
	//friend const FXP operator-(const FXP lhs, const FXP rhs);
	//friend const FXP operator*(const FXP lhs, const FXP rhs);
	//friend const FXP operator/(const FXP lhs, const FXP rhs);

	//unary fixed+fixed,short,int,float,double

protected:
	//Internal use only: for math operations where shifting has been handled
	FXP(int x, int shft) : value(x) {};
	FXP(unsigned x, int shft) : value(x) {};
        //FXP(int x) : value(x) {};
	FXP(int x, int shft, bool rShft);

private:
	int value;

public:

	/* Casts */
	operator float(void) const { return ((float)value)/(1<<FXP_PREC); }
	operator double(void) const { return ((double)value)/(1<<FXP_PREC); }
	operator int(void) const { return (int)(value>>FXP_PREC); }
	operator short(void) const { return (short)(value>>FXP_PREC); }
	operator char(void) const { return (char)(value>>FXP_PREC); }
	operator unsigned int(void) const { return (unsigned int)(value>>FXP_PREC); }
	operator unsigned short(void) const { return (unsigned short)(value>>FXP_PREC); }
	operator unsigned char(void) const { return (unsigned char)(value>>FXP_PREC); }

	//repeat for double?, short, int,
	//cast to unsigned?

	FXP(void) : value(0) {};
	FXP(short x) : value(x<<FXP_PREC) {};
	FXP(int x) : value(x<<FXP_PREC) {};
	FXP(unsigned int x) : value(x<<FXP_PREC) {};
	FXP(float x) : value((int)(x*(1<<FXP_PREC))) {};
	FXP(double x) : value((int)(x*(1<<FXP_PREC))) {};
	FXP(const FXP& val) : value(val.value) {};
	FXP(const FXP* val) : value(val->value) {};
        FXP(long long x, int precision) : value(x>>(precision-FXP_PREC)) {};
	~FXP(void) {};

	void directCopy(int src) {this->value = src;};
	/* Increment/Decrement */
	FXP& operator++(void) {
		value += FXP_ONE;
		return(*this);
	}

	FXP& operator--(void) {
		value -= FXP_ONE;
		return(*this);
	}

	/* Assignment operators */
	FXP& operator=(const short rhs) {
		value = rhs<<FXP_PREC;
		return(*this);
	}

	FXP& operator=(const int rhs) {
		value = rhs<<FXP_PREC;
		return(*this);
	}

	FXP& operator=(const float rhs) {
		value = (int)(rhs*(1<<FXP_PREC));
		return(*this);
	}

	FXP& operator=(const double rhs) {
		value = (int)(rhs*(1<<FXP_PREC));
		return(*this);
	}

	FXP& operator=(const char rhs) {
		value = rhs<<FXP_PREC;
		return(*this);
	}

	FXP& operator=(const FXP rhs) {
		value = rhs.value;
		return(*this);
	}

	FXP& operator=(const unsigned short rhs) {
		value = rhs<<FXP_PREC;
		return(*this);
	}

	FXP& operator=(const unsigned int rhs) {
		value = rhs<<FXP_PREC;
		return(*this);
	}

	FXP& operator=(const unsigned char rhs) {
		value = rhs<<FXP_PREC;
		return(*this);
	}

	/* Comparison operators */
	bool operator==(const short rhs) const
	{
		return (value == rhs<<FXP_PREC);
	}

	bool operator==(const int rhs) const
	{
		return (value == rhs<<FXP_PREC);
	}

	//bool operator==(const float rhs)
	//{
	//	return (value == (int)::floor(rhs*(1<<FXP_PREC)));
	//}

	//bool operator==(const double rhs)
	//{
	//	return (value == floor(rhs*(1<<FXP_PREC)));
	//}

	bool operator==(const FXP rhs) const
	{
		return (value == rhs.value);
	}

	bool operator!=(const short rhs) const
	{
		return (value != rhs<<FXP_PREC);
	}

	bool operator!=(const int rhs) const
	{
		return (value != rhs<<FXP_PREC);
	}

	//bool operator!=(const float rhs)
	//{
	//	return (value != floor(rhs*(1<<FXP_PREC)));
	//}

	//bool operator!=(const double rhs)
	//{
	//	return (value != floor(rhs*(1<<FXP_PREC)));
	//}

	bool operator!=(const FXP rhs) const
	{
		return (value != rhs.value);
	}


	bool operator<(const short rhs) const
	{
		return (value < rhs<<FXP_PREC);
	}

	bool operator<(const int rhs) const
	{
		return (value < rhs<<FXP_PREC);
	}

	//bool operator<(const float rhs)
	//{
	//	return (value < floor(rhs*(1<<FXP_PREC)));
	//}

	//bool operator<(const double rhs)
	//{
	//	return (value < floor(rhs*(1<<FXP_PREC)));
	//}

	bool operator<(const FXP rhs) const
	{
		return (value < rhs.value);
	}

	bool operator>(const short rhs) const
	{
		return (value > rhs<<FXP_PREC);
	}

	bool operator>(const int rhs) const
	{
		return (value > rhs<<FXP_PREC);
	}

	//bool operator>(const float rhs)
	//{
	//	return (value > floor(rhs*(1<<FXP_PREC)));
	//}

	//bool operator>(const double rhs)
	//{
	//	return (value > floor(rhs*(1<<FXP_PREC)));
	//}

	bool operator>(const FXP rhs) const
	{
		return (value > rhs.value);
	}

	bool operator<=(const short rhs) const
	{
		return (value <= rhs<<FXP_PREC);
	}

	bool operator<=(const int rhs) const
	{
		return (value <= rhs<<FXP_PREC);
	}

	//bool operator<=(const float rhs)
	//{
	//	return (value <= floor(rhs*(1<<FXP_PREC)));
	//}

	//bool operator<=(const double rhs)
	//{
	//	return (value <= floor(rhs*(1<<FXP_PREC)));
	//}

	bool operator<=(const FXP rhs) const
	{
		return (value <= rhs.value);
	}

	bool operator>=(const short rhs) const
	{
		return (value >= rhs<<FXP_PREC);
	}

	bool operator>=(const int rhs) const
	{
		return (value >= rhs<<FXP_PREC);
	}

	//bool operator>=(const float rhs)
	//{
	//	return (value >= floor(rhs*(1<<FXP_PREC)));
	//}

	//bool operator>=(const double rhs)
	//{
	//	return (value >= floor(rhs*(1<<FXP_PREC)));
	//}

	bool operator>=(const FXP rhs) const
	{
		return (value >= rhs.value);
	}


	/* reversed comparison operators */
	/* finish the list */
	//bool operator<(int b, FXP a)
	//{
	//	retunr a >= b;
	//}

	//copy paste for !=, <, >, <=, >=
	//compare to char?
	//compare to unsigned?

	FXP floor(void)
	{
		return (FXP)(value>>FXP_PREC);
	}
	FXP ceil(void)
	{
	//Yes, we're assuming the decimal is NOT zero.
		return (FXP)(value>>FXP_PREC+1);
	}



	/* Math Functions */
/*	FXP sqrt(void);
	FXP pow(FXP exp);
	FXP log10(void);
	FXP log(void);
	FXP exp(void);
	FXP cos(void);
	FXP sin(void);
	FXP tan(void);
*/
	//FXP operator%(FXP rhs);  short, int, float, double
	//FXP operator*=, /=, -=, +=
	//

	//int operator=(const FXP rhs) {
	//	return rhs.value;
	//}

	//int operator=(const FXP rhs) {
	//	int x = (rhs.value>>FXP_PREC);
	//	return x;
	//}

	//float operator=(const FXP rhs) {
	//	float x = rhs/(1<<FXP_PREC);
	//	return x;
	//}

	/* Pointer Operators ? */
	bool equals(const FXP b)
	{
		return (value == b.value);
	}

	FXP add(const FXP b) const
	{
		FXP a;
		a.value = value+b.value;
		return a;
	}

	FXP subtract(const FXP b) const
	{
		FXP a;
		a.value = value-b.value;
		return a;
	}

	FXP multiply(const FXP b) const
	{
		FXP a;
		a.value = (int)(((long long)value*b.value)>>FXP_PREC);
		return a;
	}

#ifdef FXP_MATH
	FXP divide(const FXP b) const
	{
		if(b==0) return FXP(0xFFFFFFFF, 0);
		FXP a;
		if(((unsigned)abs(value)) <= ((unsigned)0x7FFFFFFF>>FXP_PREC)) { //"small" case
			a.value = ((value<<FXP_PREC)/b.value);
		} else { //"large" case
			a.value = (int)(((long long)value<<FXP_PREC)/b.value);
		} return a;
	}
#endif
	FXP operator+(const FXP b) const
	{
		return add(b);
	}
	FXP operator+(const float b) const
	{
		FXP _b = b;
		return add(_b);
	}
	FXP operator+(const double b) const
	{
		FXP _b = b;
		return add(_b);
	}
	FXP operator+(const int b) const
	{
		FXP _b = b;
		return add(_b);
	}
	FXP operator+(const short b) const
	{
		FXP _b = b;
		return add(_b);
	}

	FXP operator-(const FXP b) const
	{
		return subtract(b);
	}
	FXP operator-(const float b) const
	{
		FXP _b = b;
		return subtract(_b);
	}
	FXP operator-(const double b) const
	{
		FXP _b = b;
		return subtract(_b);
	}
	FXP operator-(const int b) const
	{
		FXP _b = b;
		return subtract(_b);
	}
	FXP operator-(const short b) const
	{
		FXP _b = b;
		return subtract(_b);
	}
	FXP operator-(const unsigned b) const
	{
		FXP _b = b;
		return subtract(_b);
	}

	FXP operator*(const FXP b) const
	{
		return multiply(b);
	}
	FXP operator*(const float b) const
	{
		FXP _b = b;
		return multiply(_b);
	}
	FXP operator*(const double b) const
	{
		FXP _b = b;
		return multiply(_b);
	}
	FXP operator*(const int b) const
	{
		FXP _b = b;
		return multiply(_b);
	}
	FXP operator*(const short b) const
	{
		FXP _b = b;
		return multiply(_b);
	}
        FXP operator*(const unsigned b) const
        {
                FXP _b = b;
                return multiply(_b);
        }
#ifdef FXP_MATH
	FXP operator/(const FXP b) const
	{
		return divide(b);
	}
	FXP operator/(const float b) const
	{
		FXP _b = b;
		return divide(_b);
	}
	FXP operator/(const double b) const
	{
		FXP _b = b;
		return divide(_b);
	}
	FXP operator/(const int b) const
	{
		FXP _b = b;
		return divide(_b);
	}
	FXP operator/(const short b) const
	{
		FXP _b = b;
		return divide(_b);
	}
#endif
	//Shouldn't be *= by float/double
	FXP operator*=(const FXP rhs)
	{
		value = (int)(((long long)value*rhs.value)>>FXP_PREC);
		return *this;
	}
	FXP operator *=(const int rhs)
	{
		value = value*rhs;
		return *this;
	}
	FXP operator *=(const short rhs)
	{
		value = value*rhs;
		return *this;
	}

	FXP operator /=(const FXP rhs)
	{
		value = (int)(((long long)(value<<FXP_PREC))/rhs.value);
		return *this;
	}
	FXP operator /=(const int rhs)
	{
		value = value/rhs;
		return *this;
	}
	FXP operator /=(const short rhs)
	{
		value = value/rhs;
		return *this;
	}

	FXP operator +=(const FXP rhs)
	{
		value = value + rhs.value;
		return *this;
	}
	FXP operator +=(const int rhs)
	{
		value = value+(rhs<<FXP_PREC);
		return *this;
	}
	FXP operator +=(const short rhs)
	{
		value = value+(rhs<<FXP_PREC);
		return *this;
	}
	FXP operator +=(const float rhs)
	{
		value = value+(int)(rhs*(1<<FXP_PREC));
		return *this;
	}
	FXP operator +=(const double rhs)
	{
		value = value+(int)(rhs*(1<<FXP_PREC));
		return *this;
	}

	FXP operator -=(const FXP rhs)
	{
		value = value - rhs.value;
		return *this;
	}
	FXP operator -=(const int rhs)
	{
		value = value-(rhs<<FXP_PREC);
		return *this;
	}
	FXP operator -=(const short rhs)
	{
		value = value-(rhs<<FXP_PREC);
		return *this;
	}
	FXP operator -=(const float rhs)
	{
		value = value-(int)(rhs*(1<<FXP_PREC));
		return *this;
	}
	FXP operator -=(const double rhs)
	{
		value = value-(int)(rhs*(1<<FXP_PREC));
		return *this;
	}

	int GetValue() { return value; };
	float GetFValue() { return ((float)value/(1<<FXP_PREC)); };
	int GetIValue() { return (value>>FXP_PREC); };


};
/* Negative, not part of FXP class */
inline FXP operator-(const FXP a)
{
	return FXP(0)-a;
}

inline FXP operator*(const int a, const FXP b)
{
	return b*FXP(a);
}


inline FXP floor(FXP a)
{
	return a.floor();
}

inline FXP ceil(FXP a)
{
	return a.ceil();
}

#ifdef FXP_MATH
/* These will be redefined later */
inline FXP cos(const FXP a)
{
	return FXP(cos((float)a));
}
inline FXP sin(const FXP a)
{
	return FXP(sin((float)a));
}
inline FXP tan(const FXP a)
{
	return FXP(tan((float)a));
}

inline FXP cosh(const FXP a)
{
	return FXP(cosh((float)a));
}

inline FXP sinh(const FXP a)
{
        return FXP(sinh((float)a));
}

inline FXP tanh(const FXP a)
{
        return FXP(tanh((float)a));
}

inline FXP atan2(const FXP a, const FXP b)
{
        return FXP(atan2((float)a, (float)b));
}
inline FXP abs(const FXP a)
{
        return FXP(fabs((double)a));
}

inline FXP fabs(const FXP a)
{
        return FXP(fabsf((float)a));
}

inline FXP sqrt(const FXP a)
{
        return FXP(sqrt((float)a));
}

inline FXP exp(const FXP a)
{
        return FXP(exp((float)a));
}

inline FXP log(const FXP a)
{
        return FXP(log((float)a));
}

inline FXP pow(const int base, const FXP exp)
{
	return FXP(pow((double)base, (double)exp));
}
inline FXP pow(const FXP base, const FXP exp)
{
	return FXP(pow((float)base,(int)exp));
}
inline double pow(const int base, const double exp)
{
	return pow((double) base, (double) exp);
}




inline FXP log10(const FXP a)
{
	return FXP(log10((float)a));
}

#endif
inline bool operator<(const int a, const FXP b)
{
	return b>=a;
}
inline bool operator>(const int a, const FXP b)
{
	return b<=a;
}

inline bool operator<=(const int a, const FXP b)
{
	return b>a;
}
inline bool operator>=(const int a, const FXP b)
{
	return b<a;
}

inline FXP operator+(const int a, const FXP b)
{
	return (FXP)a + b;
}
inline FXP operator-(const int a, const FXP b)
{
	return (FXP)a - b;
}


//FXP& FXP::FXP(const int x, const int shft, const bool rShift)
//{
//
//}

#ifdef OLDWAY
const FXP operator+(const FXP lhs, const FXP rhs)
{
	FXP tmp(lhs.value + rhs.value, 0);
	return tmp;
};

const FXP operator-(const FXP lhs, const FXP rhs)
{
	FXP tmp(lhs.value - rhs.value, 0);
	return tmp;
};

const FXP operator*(const FXP lhs, const FXP rhs)
{
	FXP tmp((lhs.value>>(FXP_PREC/2))*(rhs.value>>(FXP_PREC/2)),0);
	return tmp;
}

//Yields only integer result
const FXP operator/(const FXP lhs, const FXP rhs)
{
	FXP tmp( ((int)(lhs.value/rhs.value))<<FXP_PREC, 0);
	return tmp;
}
#endif


#endif

