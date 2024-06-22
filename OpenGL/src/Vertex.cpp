#include "Vertex.h"

Vertex::Vertex(float x_, float y_, float z_)
{
	x = x_;
	y = y_;
	z = z_;
}

auto Vertex::magnitude() const -> float
{
	return std::sqrtf(x * x + y * y + z * z);
}

auto Vertex::normalize() -> void
{
	float magnitude = this->magnitude();
	x /= magnitude;
	y /= magnitude;
	z /= magnitude;
}

bool Vertex::operator==(const Vertex& b) const
{
	if (&b == this)
	{
		return true;
	}
	//this->normalize();
	//b.normalize();
	if (compare_float(x, b.x) && compare_float(y, b.y) && compare_float(z, b.z))
	{
		return true;
	}
	if (compare_float(-x, b.x) && compare_float(-y, b.y) && compare_float(-z, b.z))
	{
		return true;
	}
	return false;
}

auto Distance(const struct Vertex& p, const struct Vertex& q) -> float
{
	float x_dist = p.x - q.x;
	float y_dist = p.y - q.y;
	float z_dist = p.z - q.z;
	return sqrt(x_dist * x_dist + y_dist * y_dist + z_dist * z_dist * 1.0);
}

Vertex& Vertex::operator-(const Vertex& b)
{
	float x_ = this->x - b.x;
	float y_ = this->y - b.y;
	float z_ = this->z - b.z;
	Vertex new_V(x_, y_, z_);
	return new_V;
}

Vertex& cross_Product(const Vertex& vect_1, const Vertex& vect_2)
{
	float x_ = vect_1.y * vect_2.z - vect_1.z * vect_2.y;
	float y_ = -vect_1.x * vect_2.z + vect_1.z * vect_2.x;
	float z_ = vect_1.x * vect_2.y + vect_1.y * vect_2.x;
	Vertex new_norm(x_, y_, z_);
	return new_norm;
}

int Vertex::get_sign(const Vertex& b) const
{
	if (compare_float(this->x, -b.x) && compare_float(this->y, -b.y) && compare_float(this->z, -b.z))
	{
		return -1;
	}
	return 1;
}

void Vertex::operator*(float val)
{
	this->x *= val;
	this->y *= val;
	this->z *= val;
}

/*
size_t Hash::operator()(const Vertex& vertex) const {
	// 1,1,1
	// -1,-1,-1
	// 0,1,1
	// 0,-1,-1
	// 0,1,0
	// 0,-1,0
	bool flipped = false;
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
		std::string temp = std::to_string(vertex.x * -1) + '|' + std::to_string(vertex.y * -1) + '|' + std::to_string(vertex.z * -1);
		return std::hash<std::string>()(temp);
	}
	std::string temp = std::to_string(vertex.x) + '|' + std::to_string(vertex.y) + '|' + std::to_string(vertex.z);
	return std::hash<std::string>()(temp);
}
*/