
#include "Utils.hpp"

namespace Utils {

	std::vector<str> split(const str &input, char delimiter) {
		std::vector<str> tokens;
		str token;
		for (str::size_type i = 0; i < input.length(); ++i) { //Usamos std::string::size_type porque lenght() y las demas devuelven este tipo, si usamos int el compilador da warnings.
			if (input[i] == delimiter) {
				tokens.push_back(token); //Cuandoo encuentra delimiter, pushea y limpia.
				token.clear();
			} else {
				token += input[i]; //No es delimiter, mete la letra en el str actual.
			}
		}
		tokens.push_back(token); //Aqui no ha encontrado mas delimiters mete el resto.
		return tokens;
	}

	std::vector<str> split(const str &input, const str &delimiter) {
		std::vector<str> tokens;
		str::size_type start = 0;
		str::size_type pos;

		while ((pos = input.find(delimiter, start)) != str::npos) {
			tokens.push_back(input.substr(start, pos - start));
			start = pos + delimiter.length();
		}
		tokens.push_back(input.substr(start));
		return tokens;
	}
}
