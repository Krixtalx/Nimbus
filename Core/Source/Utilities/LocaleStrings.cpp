#include "CorePch.h"
#include "LocaleStrings.h"


LocaleStrings::LocaleStrings() : _currentLocale(0) {

	std::ifstream langsFile("Assets/Lang/Nimbus_langs.csv");
	if (!langsFile.is_open()) {
		throw std::runtime_error("[LocaleStrings] Languages file not found.");
	}

	std::string fileline;
	std::getline(langsFile, fileline);

	auto fileSep = splitstring(fileline, "|");

	// Init locales and strings container
	_availableLocales = std::vector<std::string>(++fileSep.begin(), fileSep.end());
	for (int i = 0; i < _availableLocales.size(); i++) {
		_appStrings.emplace_back();
	}

	// Populate strings container with all locales
	while (std::getline(langsFile, fileline)) {
		//while(langsFile >> fileline){
		fileSep = splitstring(fileline, "|");
		for (int i = 1; i < fileSep.size(); i++) {
			_appStrings[i - 1].push_back(fileSep[i]);
		}
	}

	langsFile.close();
}


std::string LocaleStrings::getString(const LOCALE_STRINGS id) {
	return _appStrings[_currentLocale][id];
}

std::string LocaleStrings::GetString(const LOCALE_STRINGS id) {
	return getInstance()->getString(id);
}

std::string LocaleStrings::getLocale(const int idx) {
	return _availableLocales[idx];
}

int LocaleStrings::getCurrentLocaleIdx() const {
	return _currentLocale;
}

std::string LocaleStrings::getCurrentLocaleStr() {
	return _availableLocales[_currentLocale];
}

const std::vector<std::string>& LocaleStrings::getLocales() {
	return _availableLocales;
}

void LocaleStrings::setLocale(const int idx) {
	_currentLocale = idx;
}

std::vector<std::string> splitstring(std::string s, const std::string& delimiter) {
	size_t pos_start = 0, pos_end;
	const size_t delim_len = delimiter.length();
	std::vector<std::string> res;

	static const std::regex nlre("\\\\n", std::regex_constants::syntax_option_type::ECMAScript);
	s = std::regex_replace(s, nlre, "\n");

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		std::string token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}
