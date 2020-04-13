#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <math.h>

/**
 * \brief Initializes the prng
 **/
void prng_init(){
    srand((unsigned) time(NULL));
}

/**
 * \brief Returns a new random double
 **/
double prng_get_random_double(){
    return ((double) rand() / (RAND_MAX)) * (DBL_MAX);
}

/**
 * \brief Returns a new random double in range [0,1)
 **/
double prng_get_random_double_0_1(){
    return ((double) rand() / (RAND_MAX));
}

/**
 * \brief Returns a new random double from normal distribution
 **/
double prng_get_random_double_normal() {
    // box-muller method:
    const double u = prng_get_random_double_0_1();
    const double v = prng_get_random_double_0_1();
    const double lnu = log(u);
    const double twopiv = 2.0*M_PI*v;
    const double x = sqrt(-2.0*lnu) * cos(twopiv);
    const double y = sqrt(-2.0*lnu) * sin(twopiv); // wasted
    return x;
}

/**
 * \brief Returns a new random double in  range [lower_bound, upper_bound)
 * \param lower_bound Lower bound on the value of the random double
 * \param upper_bound Upper bound on the value of the random double
 **/
double prng_get_random_double_in_range(double lower_bound, double upper_bound){
    return ((double) rand() / (RAND_MAX)) * (upper_bound-lower_bound) + lower_bound;
}

/**
 * \brief Returns a new random integer
 **/
int prng_get_random_int(){
    return rand();
}

/**
 * \brief Returns a new random integer in range [lower_bound, upper_bound]
 * \param lower_bound Lower bound on the value of the random integer
 * \param upper_bound Upper bound on the value of the random integer
 **/
int prng_get_random_int_in_range(int lower_bound, int upper_bound){
    int r = rand();
    return (rand() % (upper_bound-lower_bound+1)) + lower_bound;
}
