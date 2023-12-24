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

struct FUInt2
{
	uint32_t X = 0;
	uint32_t Y = 0;

	FUInt2(const uint32_t X, const uint32_t Y)
		: X(X), Y(Y)
	{
	}
};

struct FUInt3
{
	uint32_t X = 0;
	uint32_t Y = 0;
	uint32_t Z = 0;

	FUInt3(const uint32_t X, const uint32_t Y, const uint32_t Z)
		: X(X), Y(Y), Z(Z)
	{
	}
};
