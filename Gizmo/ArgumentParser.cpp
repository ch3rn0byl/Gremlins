#include "ArgumentParser.h"
#include <sstream>
#include <iterator>

ArgumentParser::ArgumentParser(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (flag_aliases.find(arg) != flag_aliases.end()) {
            std::string canonical_flag = flag_aliases.at(arg);

            if (canonical_flag == "--initial-run") {
				// This flag does not take any arguments.
				arguments[canonical_flag] = {};
				continue;
			}

            ++i;
            while (i < argc && flag_aliases.find(argv[i]) == flag_aliases.end()) {
                std::vector<std::string> parts = split(argv[i], ", ");
                arguments[canonical_flag].insert(arguments[canonical_flag].end(), parts.begin(), parts.end());
                ++i;
            }
            --i; // Step back so the outer loop processes the next flag correctly.
        }
        else {
            // This place can be used to handle invalid arguments, if needed.
            // For now, it just ignores unrecognized flags.
        }
    }
}

std::vector<std::string> ArgumentParser::split(const std::string& s, const std::string& delimiters) {
    std::vector<std::string> tokens;
    size_t lastPos = s.find_first_not_of(delimiters, 0);
    size_t pos = s.find_first_of(delimiters, lastPos);
    while (lastPos != std::string::npos) {
        tokens.push_back(s.substr(lastPos, pos - lastPos));
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
    return tokens;
}

bool ArgumentParser::isFlagSet(const std::string& flag) const {
    return arguments.find(flag) != arguments.end();
}

std::vector<std::string> ArgumentParser::getFlagArguments(const std::string& flag) const {
    auto iter = arguments.find(flag);
    if (iter != arguments.end()) {
        return iter->second;
    }
    return {};
}