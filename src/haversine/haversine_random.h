
#include <math.h>

#include "libs/pcg/pcg.c"

#define CountLeadingZeroes64(Value)	__builtin_clzll(Value)

u64 
RandomU64(pcg64_random_t *RNG)
{
    u64 Result = pcg64_random_r(RNG);
    return Result;
}

//~ Random 64 bit float

// From: https://mumble.net/~campbell/tmp/random_real.c
/*
 *    Copyright (c) 2014, Taylor R Campbell
*
*    Verbatim copying and distribution of this entire article are
*    permitted worldwide, without royalty, in any medium, provided
*    this notice, and the copyright notice, are preserved.
*
*/

/*
 * random_real: Generate a stream of bits uniformly at random and
 * interpret it as the fractional part of the binary expansion of a
 * number in [0, 1], 0.00001010011111010100...; then round it.
 */
f64
RandomF64(pcg64_random_t *RNG)
{
	s32 Exponent = -64;
	u64 Significand;
	umm Shift;
    
	/*
	 * Read zeros into the exponent until we hit a one; the rest
	 * will go into the significand.
	 */
	while((Significand = RandomU64(RNG)) == 0) 
    {
		Exponent -= 64;
		/*
		 * If the exponent falls below -1074 = emin + 1 - p,
		 * the exponent of the smallest subnormal, we are
		 * guaranteed the result will be rounded to zero.  This
		 * case is so unlikely it will happen in realistic
		 * terms only if RandomU64 is broken.
		 */
		if ((Exponent < -1074))
			return 0;
	}
    
	/*
	 * There is a 1 somewhere in significand, not necessarily in
	 * the most significant position.  If there are leading zeros,
	 * shift them into the exponent and refill the less-significant
	 * bits of the significand.  Can't predict one way or another
	 * whether there are leading zeros: there's a fifty-fifty
	 * chance, if RandomU64() is uniformly distributed.
	 */
	Shift = CountLeadingZeroes64(Significand);
	if (Shift != 0) {
		Exponent -= Shift;
		Significand <<= Shift;
		Significand |= (RandomU64(RNG) >> (64 - Shift));
	}
    
	/*
	 * Set the sticky bit, since there is almost surely another 1
	 * in the bit stream.  Otherwise, we might round what looks
	 * like a tie to even when, almost surely, were we to look
	 * further in the bit stream, there would be a 1 breaking the
	 * tie.
	 */
	Significand |= 1;
    
	/*
	 * Finally, convert to f64 (rounding) and scale by
	 * 2^exponent.
	 */
	return ldexp((f64)Significand, Exponent);
}

f64
RandomUnilateral(pcg64_random_t *RNG)
{
    return RandomF64(RNG);
}

f64
RandomBilateral(pcg64_random_t *RNG)
{
    f64 Result = 2.0*RandomUnilateral(RNG) - 1.0;
    return Result;
}

f64
RandomBetween(pcg64_random_t *RNG, f64 Min, f64 Max)
{
    f64 Range = Max - Min;
    f64 Result = Min + RandomUnilateral(RNG)*Range;
    return Result;
}