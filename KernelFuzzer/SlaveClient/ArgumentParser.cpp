#include "ArgumentParser.h"

ArgumentParser::ArgumentParser()
{

}

std::string ArgumentParser::GetArgument(std::string szArguments, std::string szArgumentValue)
{
	std::string szToken;
	std::stringstream ssStringStream(szArguments);
	while (std::getline(ssStringStream, szToken, ' '))
	{
		if (szToken == szArgumentValue)
		{
			std::getline(ssStringStream, szToken, ' ');
			return szToken;
		}
	}
}