#pragma once
#include <string>
#include <vector>
#include <unordered_map>

class ArgumentParser
{
public:
    ArgumentParser(int argc, char* argv[]);

	bool isFlagSet(const std::string& flag) const;
	std::vector<std::string> getFlagArguments(const std::string& flag) const;
    
private:
    std::unordered_map<std::string, std::vector<std::string>> arguments;

    // A set of all valid flags to check against.
    const std::unordered_map<std::string, std::string> flag_aliases = {
        {"-h", "--hook"},
        {"-r", "--restore"},
        {"-d", "--deny"},
        {"-i", "--initial-run"},
        {"-a", "--analyze"},
        // Adding the longer flags for reverse lookup:
        {"--hook", "--hook"},
        {"--restore", "--restore"},
        {"--deny", "--deny"},
        {"--initial-run", "--initial-run"},
        {"--analyze", "--analyze"},
    };

    // Helper function: Split a string by multiple delimiters.
    std::vector<std::string> split(const std::string& s, const std::string& delimiters);
};
