#include <cstddef>
#include <iostream>

#include "vec_ops.hh"

/* Calculates scalar product of two vectors of doubles */
double vec_dot(std::vector<double> a, std::vector<double> b)
{
	size_t s = a.size();
	if (s - b.size()) {
		/*
		 * Inner product undefined for vectors of different dimensions. Should
		 * never even be called with such arguments, so failing silently is
		 * okay
		 */
		return 0.0;
	}

	double r = 0;

	for (size_t i = 0; i < s; ++i) 
		r += a[i] * b[i];
	return r;

}

/* Performs vector addition of two vectors of doubles */

std::vector<double> vec_add(std::vector<double> a, std::vector<double> b)
{
	size_t s = a.size();
	if (s - b.size()) {
		/*
		 * Addition undefined for vectors of different dimensions. Should
		 * never even be called with such arguments, so failing silently is
		 * okay
		 */
		return std::vector<double>(a);
	}

	std::vector<double> r (s);

	for (size_t i = 0; i < s; ++i) 
		r[i] = a[i] + b[i];
	return r;

}

/* It is convenient to have a separate subtraction function. Returns a - b */
std::vector<double> vec_sub(std::vector<double> a, std::vector<double> b)
{
	return vec_add(a, vec_mult(b, -1.0));
}

/* Multiplies the vector a by the scalar b */
std::vector<double> vec_mult(std::vector<double> a, double b)
{
	std::vector<double> r (a);
	for (size_t i = 0; i < r.size(); ++i)
		r[i] *= b;
	return r;

}

void vec_print(std::vector<double> a)
{
	std::cout << "[";
	for (size_t i = 0; i < a.size(); ++i) {
		if (i)
			std::cout << ", ";
		std::cout << a[i];
	}
	std::cout << "]";
	/* No trailing newline so that it can be printed inline */
	return;
}

/* Calculates vector product of two vectors of length 3 */
std::vector<double> vec_cross(std::vector<double> a, std::vector<double> b)
{
	size_t sa = a.size();
	if ((sa - 3) || (sa - b.size())) {
		return std::vector<double>(sa, 0);
	}
	std::vector<double> r(3, 0);
	for (size_t i = 0; i < 3; ++i) 
		r[i] = a[(i + 1) % 3] * b[(i + 2) % 3] - b[(i + 1) % 3] * a[(i + 2) % 3];
	return r;

}


std::vector<double> vec_recip2(std::vector<double> a, std::vector<double> b)
{
	if ((a.size() - b.size()) || (a.size() - 2))
		return std::vector<double>(a.size(), 0);

	std::vector<double> r({-1.0*b[1], b[0]});
	return vec_mult(r, 1.0/vec_dot(r, a));
}
