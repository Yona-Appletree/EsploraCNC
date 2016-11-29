//
// Created by yona on 11/5/16.
//

#include "Vector3.h"



/**
 * \brief    Fast Square root algorithm
 *
 * Fractional parts of the answer are discarded. That is:
 *      - SquareRoot(3) --> 1
 *      - SquareRoot(4) --> 2
 *      - SquareRoot(5) --> 2
 *      - SquareRoot(8) --> 2
 *      - SquareRoot(9) --> 3
 *
 * \param[in] a_nInput - unsigned integer for which to find the square root
 *
 * \return Integer square root of the input value.
 */
uint32_t sqrt32(uint32_t a_nInput)
{
	uint32_t op  = a_nInput;
	uint32_t res = 0;
	uint32_t one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type

	// "one" starts at the highest power of four <= than the argument.
	while (one > op)
	{
		one >>= 2;
	}

	while (one != 0)
	{
		if (op >= res + one)
		{
			op = op - (res + one);
			res = res +  2 * one;
		}
		res >>= 1;
		one >>= 2;
	}
	return res;
}

int32_t Vector3::sizeSq() const {
	return x * x + y * y + z * z;
}

void writeFixedTo(Stream& stream, int32_t fixed01) {
	stream.print(fixed01/100);
	stream.print(".");
	stream.print(abs(fixed01)%100);
}

Vector3 &Vector3::operator=(const Vector3 & other) {
	x = other.x;
	y = other.y;
	z = other.z;
	return *this;
}

Vector3 &Vector3::operator+=(const Vector3 & other) {
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

Vector3 &Vector3::operator-=(const Vector3 & other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

Vector3 Vector3::operator+(const Vector3 & other) {
	return Vector3(
		x + other.x,
		y + other.y,
		z + other.z
	);
}

Vector3 Vector3::operator-(const Vector3 & other) {
	return Vector3(
		x - other.x,
		y - other.y,
		z - other.z
	);
}

Vector3 Vector3::operator*(int32_t i) {
	return Vector3(x * i, y * i, z * i);
}

Vector3 Vector3::operator/(int32_t i) {
	return Vector3(x / i, y / i, z / i);
}

void Vector3::writeXTo(Stream& stream) {
	stream.print(" X");
	stream.print(x/100); stream.print("."); stream.print(abs(x)%100);
}

void Vector3::writeYTo(Stream& stream) {
	stream.print(" Y");
	stream.print(y/100); stream.print("."); stream.print(abs(y)%100);
}

void Vector3::writeZTo(Stream& stream) {
	stream.print(" Z");
	stream.print(z/100); stream.print("."); stream.print(abs(z)%100);
}

void Vector3::writeXYTo(Stream& stream) {
	writeXTo(stream);
	writeYTo(stream);
}

void Vector3::writeXYZTo(Stream& stream) {
	writeXTo(stream);
	writeYTo(stream);
	writeZTo(stream);
}

int32_t roundFixed(int32_t n) {
	int8_t m = n % 100;

	return (abs(m) < 50)
	       ? (n - m)
	       : (n < 0)
	         ? (n - 100 - m)
	         : (n + 100 - m);
}