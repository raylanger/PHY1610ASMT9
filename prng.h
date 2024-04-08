#ifndef _MYPRNGH_
#define _MYPRNGH_

#include <random>
#include <sstream>

// Global pseudo random number generator based off of C++'s minstd_rand
class PRNG: public std::minstd_rand
{
  public:
    PRNG(result_type x = default_seed) : std::minstd_rand(x) {}    
    void discard(result_type z) {
        // Faster forward routine for this multiplicative linear
        // congruent generator, O(log z) instead of O(z)
        // compute az = multiplier^z mod modulus:
        result_type b = multiplier, az = 1, x;
        while (z > 0) {
            if (z % 2)
                az= (az*b) % modulus;
            b = (b*b) % modulus;
            z >>= 1;
        }
        // apply to current state:
        std::stringstream s;
        s << (*this); // get state as a string
        s >> x;       // convert back to unsigned long
        seed((x * az) % modulus); // set forwarded state
    }
};

double randomDouble(int ndiscard = 0);

#endif
