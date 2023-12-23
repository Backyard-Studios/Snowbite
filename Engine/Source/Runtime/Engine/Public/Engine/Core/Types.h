#pragma once

struct FFloat2
{
	float X = 0.0f;
	float Y = 0.0f;

	FFloat2(const float X, const float Y)
		: X(X), Y(Y)
	{
	}
};

struct FFloat3
{
	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;

	FFloat3(const float X, const float Y, const float Z)
		: X(X), Y(Y), Z(Z)
	{
	}
};
