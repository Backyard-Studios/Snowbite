#include "pch.h"

#include <Engine/Core/ArgumentParser.h>
#include <iostream>

FArgumentParser::FArgumentParser(const int32_t InArgumentCount, char** InArguments)
{
	// Skip the first argument, which is the executable path
	for (int32_t Index = 1; Index < InArgumentCount; ++Index)
	{
		std::string Argument = InArguments[Index];
		if (Argument.starts_with("--"))
			Argument = Argument.erase(0, 2);
		else if (Argument.starts_with("-"))
			Argument = Argument.erase(0, 1);
		if (Argument.find("=") != std::string::npos)
		{
			std::string Name = Argument.substr(0, Argument.find("="));
			std::string Value = Argument.substr(Argument.find("=") + 1);
			Arguments.emplace(Name, Value);
		}
		else
		{
			Arguments.emplace(Argument, "true");
		}
	}
}

FArgumentParser::~FArgumentParser() = default;

bool FArgumentParser::HasArgument(const std::string& Name) const
{
	return Arguments.contains(Name);
}

std::string FArgumentParser::GetArgument(const std::string& Name, std::string DefaultValue)
{
	if (Arguments.contains(Name))
		return Arguments[Name];
	return DefaultValue;
}
