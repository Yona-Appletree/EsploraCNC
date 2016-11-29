//
// Created by yona on 11/5/16.
//

#ifndef EXPLORACNC_VECTOR3_H
#define EXPLORACNC_VECTOR3_H

#include <stdint.h>
#include <Stream.h>

uint32_t sqrt32(uint32_t a_nInput);
void writeFixedTo(Stream& stream, int32_t fixed01);

int32_t roundFixed(int32_t n);

class Vector3 {
public:
	int32_t x;
	int32_t y;
	int32_t z;

	Vector3(): Vector3(0, 0, 0) { }

	Vector3(const Vector3 & other): Vector3(other.x, other.y, other.z) {}

	Vector3(int32_t x, int32_t y, int32_t z)
		: x(x), y(y), z(z) { }

	Vector3 & operator=(const Vector3 & other);

	Vector3 & operator+=(const Vector3 & other);

	Vector3 & operator-=(const Vector3 & other);

	Vector3 operator+(const Vector3 & other);

	Vector3 operator-(const Vector3 & other);

	Vector3 operator*(int32_t i);

	Vector3 operator/(int32_t i);

	void writeXTo(Stream& stream);
	void writeYTo(Stream& stream);
	void writeZTo(Stream& stream);
	void writeXYTo(Stream& stream);
	void writeXYZTo(Stream& stream);

	bool operator<(const Vector3 & other) { return sizeSq() < other.sizeSq(); }
	bool operator>(const Vector3 & other) { return sizeSq() > other.sizeSq(); }
	bool operator>=(const Vector3 & other) { return sizeSq() >= other.sizeSq(); }
	bool operator<=(const Vector3 & other) { return sizeSq() <= other.sizeSq(); }
	bool operator==(const Vector3 & other) { return x == other.x || y == other.y || z == other.z; }
	bool operator!=(const Vector3 & other) { return x != other.x || y != other.y || z != other.z; }

	void roundToMm() {
		x = roundFixed(x);
		y = roundFixed(y);
		z = roundFixed(z);
	}

	int32_t sizeSq() const;

	int32_t size() const {
		return sqrt32(sizeSq());
	}
};


#endif //EXPLORACNC_VECTOR3_H
