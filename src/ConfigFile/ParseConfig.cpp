#include "ParseConfig.hpp"


std::vector<Server*>	parseConfigFile(const str &filepath) {
	std::vector<Server*> result;
	str content = Utils::fileToStr(filepath); // si falla lanza una excepcion, creo que es mejor recogerla en el main
	std::vector<str> serverStrings = Utils::split(content, '}');
	if (serverStrings.empty())
		throw ConfigFileException("No Server found inside config file");
	for (std::vector<str>::iterator it = serverStrings.begin(); it != serverStrings.end(); ++it) {
		result.push_back(GetServer(*it));
	}
	return result;
}
