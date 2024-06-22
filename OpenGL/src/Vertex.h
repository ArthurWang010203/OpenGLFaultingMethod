#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <string>

typedef struct Vertex
{
	float x;
	float y;
	float z;
	Vertex() = default;
	Vertex(float x_, float y_, float z_);
	bool operator==(const Vertex& b) const;
	void operator*(float val);
	int get_sign(const Vertex& b) const;
	Vertex& operator-(const Vertex& b);
	//auto operator<<(const Vertex& lhs) -> std::ostream&;
	auto magnitude() const -> float;
	auto normalize() -> void;
} Vertex;
/*
struct Hash {
	size_t operator()(const Vertex& vertex) const;
};
*/

static bool compare_float(float x, float y, float epsilon = 0.01f) {
	if (fabs(x - y) < epsilon)
		return true; //they are same
	return false; //they are not same
}

Vertex& cross_Product(const Vertex& vect_1, const Vertex& vect_2);

class my_string_hash
{
public:
	size_t operator()(const Vertex& vertex) const // <-- don't forget const
	{
		bool flipped = false;
		float flip_x = 0.0;
		float flip_y = 0.0;
		float flip_z = 0.0;
		if (compare_float(vertex.x, 0.0))
		{
			if (compare_float(vertex.y, 0.0))
			{
				if (!compare_float(vertex.z, 0.0))
				{
					if (vertex.z < 0)
					{
						flipped = true;
					}
				}
			}
			else
			{
				if (vertex.y < 0)
				{
					flipped = true;
				}
			}
		}
		else
		{
			if (vertex.x < 0)
			{
				flipped = true;
			}
		}
		if (flipped)
		{
			if (!compare_float(vertex.x, 0.0))
			{
				flip_x = vertex.x * -1;
			}
			if (!compare_float(vertex.y, 0.0))
			{
				flip_y = vertex.y * -1;
			}
			if (!compare_float(vertex.z, 0.0))
			{
				flip_z = vertex.z * -1;
			}
			std::string temp = std::to_string(float(flip_x)) + '|' + std::to_string(float(flip_y)) + '|' + std::to_string(float(flip_z));
			return std::hash<std::string>()(temp);
		}
		if (!compare_float(vertex.x, 0.0))
		{
			flip_x = vertex.x;
		}
		if (!compare_float(vertex.y, 0.0))
		{
			flip_y = vertex.y;
		}
		if (!compare_float(vertex.z, 0.0))
		{
			flip_z = vertex.z;
		}
		std::string temp = std::to_string(float(flip_x)) + '|' + std::to_string(float(flip_y)) + '|' + std::to_string(float(flip_z));
		return std::hash<std::string>()(temp);
	}
};


auto Distance(const struct Vertex& p, const struct Vertex& q) -> float;