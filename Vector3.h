//
// Created by yona on 11/5/16.
//

#ifndef EXPLORACNC_VECTOR3_H
#define EXPLORACNC_VECTOR3_H

#include <stdint.h>
#include <Stream.h>

uint32_t sqrt32(uint32_t a_nInput);
void writeFixedTo(Stream& stream, int32_t fixed01);

class Vector3 {
public:
	int32_t x;
	int32_t y;
	int32_t z;

	Vector3(): Vector3(0, 0, 0) { }

	Vector3(int32_t x, int32_t y, int32_t z)
		: x(x), y(y), z(z) { }

	Vector3& operator=(const Vector3 & other) {
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}
	Vector3& operator+=(const Vector3 & other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	Vector3& operator-=(const Vector3 & other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	Vector3 operator+(const Vector3 & other) {
		return Vector3(
			x + other.x,
			y + other.y,
			z + other.z
		);
	}

	Vector3 operator-(const Vector3 & other) {
		return Vector3(
			x - other.x,
			y - other.y,
			z - other.z
		);
	}

	Vector3 operator*(int32_t i) {
		return Vector3(x * i, y * i, z * i);
	}

	Vector3 operator/(int32_t i) {
		return Vector3(x / i, y / i, z / i);
	}

	void writeXTo(Stream& stream) {
		stream.print(" X");
		stream.print(x/100); stream.print("."); stream.print(abs(x)%100);
	}

	void writeYTo(Stream& stream) {
		stream.print(" Y");
		stream.print(y/100); stream.print("."); stream.print(abs(y)%100);
	}

	void writeZTo(Stream& stream) {
		stream.print(" Z");
		stream.print(z/100); stream.print("."); stream.print(abs(z)%100);
	}

	void writeXYTo(Stream& stream) {
		writeXTo(stream);
		writeYTo(stream);
	}

	void writeXYZTo(Stream& stream) {
		writeXTo(stream);
		writeYTo(stream);
		writeZTo(stream);
	}

	bool operator<(const Vector3 & other) { return sizeSq() < other.sizeSq(); }
	bool operator>(const Vector3 & other) { return sizeSq() > other.sizeSq(); }
	bool operator>=(const Vector3 & other) { return sizeSq() >= other.sizeSq(); }
	bool operator<=(const Vector3 & other) { return sizeSq() <= other.sizeSq(); }
	bool operator==(const Vector3 & other) { return x == other.x || y == other.y || z == other.z; }
	bool operator!=(const Vector3 & other) { return x != other.x || y != other.y || z != other.z; }

	int32_t sizeSq() const;

	int32_t size() const {
		return sqrt32(sizeSq());
	}
};


#endif //EXPLORACNC_VECTOR3_H
