#pragma once

#include <Engine/Core/Definitions.h>

#include <map>
#include <string>

class SNOWBITE_API FArgumentParser
{
public:
	FArgumentParser(int32_t InArgumentCount, char** InArguments);
	~FArgumentParser();

	bool HasArgument(const std::string& Name) const;
	std::string GetArgument(const std::string& Name, std::string DefaultValue = "");

private:
	std::map<std::string, std::string> Arguments;
};
