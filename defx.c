#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#define max(a,b) ((a) > (b) ? a : b)
#define min(a,b) ((a) < (b) ? a : b)

#define MOD48MASK ((1ULL << 48) - 1)
#define MOD56MASK ((1ULL << 56) - 1)
static __inline__ uint64_t is_square(__uint128_t p)
{
    if ((int64_t)(0xC840C04048404040ULL << (p & 63))>=0) return 0;
    uint64_t m48 = (uint64_t)(p >> 96) + ((uint64_t)(p >> 48) & MOD48MASK) + ((uint64_t)p & MOD48MASK);
    m48 = (m48 & MOD48MASK) + (m48 >> 48);
    m48 = (m48 & MOD48MASK) + (m48 >> 48); //important repetition
    uint64_t res, res1;
    // mod 63 & 65, try to cue the compiler to get out-of-order instructions to use two ALUs
    res = (m48 * 0x4104104104105ULL) & MOD56MASK;
    res1 = (m48 * 0x3F03F03F03F04ULL) & MOD56MASK;
    res = (res << 6) - res;
    res1 += (res1 << 6);
    res >>= 56;
    res1 >>= 56;
    if ((int64_t)(0xC940A2480C124020ULL << res) >= 0) return 0;
    if ((int64_t)(0xC862806619805184ULL << res1) > 0) return 0;
    // mod 17
    res = (m48 * 0xF0F0F0F0F0F10ULL) & MOD56MASK;
    res += (res << 4);
    res >>= 56;
    if ((int64_t)(0xE8C5800000000000ULL << res) >= 0) return 0;
    uint64_t c = rintl(sqrtl(p));
    return (p == (__uint128_t)c*(__uint128_t)c) ? c : 0;
}

static __inline__ uint64_t string_to_u64(const char * s) {
  uint64_t i;
  char c ;
  int scanned = sscanf(s, "%" SCNu64 "%c", &i, &c);
  if (scanned == 1) return i;
  if (scanned > 1) {
    // TBD about extra data found
    return i;
    }
  // TBD failed to scan;
  return 0;
}

void u128_to_string(const __uint128_t n, char * s)
{
    uint64_t d4, d3, d2, d1, d0, q;
	const int size = 40; // floor(log10(2^128-1))
    char u[40];
    char * t = u;

	// n = d3*2^96 + d2*2^64 + d1*2^32 + d0
	// n = d3*79228162514264337593543950336 + d2*18446744073709551616 + d1*4294967296 + d0

	// n = d3*79_228162514_264337593_543950336 + d2*18_446744073_709551616 + d1*4_294967296 + d0

	// n = d3*79*10^27 + d3*228162514*10^18 + d3*264337593*10^9 + d3*543950336
	//                 + d2*       18*10^18 + d2*446744073*10^9 + d2*709551616
	//                                      + d1*        4*10^9 + d1*294967296
	//                                                          + d0*000000001

	// define constants

	const uint32_t k3 = 79;
	const uint32_t k2 = 228162514;
	const uint32_t k1 = 264337593;
	const uint32_t k0 = 543950336;

	const uint32_t l2 = 18;
	const uint32_t l1 = 446744073;
	const uint32_t l0 = 709551616;

	const uint32_t m1 = 4;
	const uint32_t m0 = 294967296;

	const uint32_t dec_unit = 1000000000;

    d0 = (uint32_t)n;
    d1 = (uint32_t)(n >> 32);
    d2 = (uint32_t)(n >> 64);
    d3 = n >> 96;

    d0 = (k0 * d3) + (l0 * d2) + (m0 * d1) + d0;
    q  = d0 / dec_unit;
    d0 = d0 % dec_unit;

    d1 = q + (k1 * d3) + (l1 * d2) + (m1 * d1);
    q  = d1 / dec_unit;
    d1 = d1 % dec_unit;

    d2 = q + (k2 * d3) + (l2 * d2);
    q  = d2 / dec_unit;
    d2 = d2 % dec_unit;

    d3 = q + (k3 * d3);
    q  = d3 / dec_unit;
    d3 = d3 % dec_unit;

    d4 = q;

    bzero(t, size); // zero the buffer
	sprintf(t,"%u%.9u%.9u%.9u%.9u",(uint32_t)d4,(uint32_t)d3,(uint32_t)d2,(uint32_t)d1,(uint32_t)d0);

	// trim leading zeros
	while (*t && *t == '0') t++;
	if ( *t == 0x0 ) t--; // in case number = 0

    strcpy(s, t);
}

int main(int argc, char** argv)
{
    uint32_t d, e, f, r=0;
    uint64_t g2=1, d2, e2, f2, sgd, sge, x;
    __uint128_t x2;
    char x2s[40];

    if (argc == 2) {
        g2 = string_to_u64(argv[1]);
        g2 += 1 - g2&1;
    }

    fprintf(stdout, "d\te\tf\tg2\tx2 (bits)\n");
    while (1) {
        for (d = rintl(sqrt(6*sqrt(g2)-5)); ; d++) {
            d2 = (uint64_t)d*(uint64_t)d;
            if (d2 > 2*g2/3) break;
            sgd = 2*g2-d2;
            sge = sgd > g2 ? sgd - g2 : 0;
            for (e = max(d + 1, ceil(sqrt(sge))); ; e++) {
                e2 = (uint64_t)e*(uint64_t)e;
                if (e2 >= sgd/2) break;
                sge = sgd - e2;
                f = is_square(sge);
                if (!f) continue;
                f2 = (uint64_t)f*(uint64_t)f;
                x2 = ((__uint128_t)g2-(__uint128_t)d2)*((__uint128_t)g2-(__uint128_t)e2)*((__uint128_t)g2-(__uint128_t)f2)*(__uint128_t)g2;
                x = is_square(x2);
                if (g2%1000==1) {
                    u128_to_string(x2, x2s);
                    fprintf(stdout, "\r%" PRIu32 "\t%" PRIu32 "\t%" PRIu32 "\t%" PRIu64 "\t%s (%i)", d, e, f, g2, x2s, (int)log2l(x2));
                }
                if (!x) continue;
                u128_to_string(x2, x2s);
                fprintf(stdout, "\r%" PRIu32 "\t%" PRIu32 "\t%" PRIu32 "\t%" PRIu64 "\t%s (%i)", d, e, f, g2, x2s, (int)log2l(x2));
                fprintf(stdout, "\n");
                r++;
                if (r >= 100) return(0);
           }
        }
        g2 += 2;
    }
}
