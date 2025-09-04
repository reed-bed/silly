#ifndef VEC_OPS_H_I
#define VEC_OPS_H_I

/*
 * Implemented using vector<double> rather than double[3] because vector<double>
 * is C++-specific
 */
#include <vector>
/* Only need operations for vectors of doubles */
double vec_dot(std::vector<double> a, std::vector<double> b);
std::vector<double> vec_add(std::vector<double> a, std::vector<double> b);
/* It is convenient to have a separate subtraction function. Returns a - b */
std::vector<double> vec_sub(std::vector<double> a, std::vector<double> b);
std::vector<double> vec_mult(std::vector<double> a, double b);
std::vector<double> vec_cross(std::vector<double> a, std::vector<double> b);
/* returns the reciprocal vector of a for the pair of 2d vectors (a, b) */
std::vector<double> vec_recip2(std::vector<double> a, std::vector<double> b);

/* For debugging purposes */
void vec_print(std::vector<double> a);
#endif
