#pragma once
#include "locale_strings.h"
#include "Singleton.h"
#include <vector>
#include <string>

/**
 * \brief Manages localized strings application-wide
 */
class LocaleStrings : public Nimbus::Singleton<LocaleStrings>{
	friend class Singleton;
protected:

	using STRINGS_MAP = std::vector<std::vector<std::string>>;

	/**
	 * Main localized string collection, contains every string for every locale
	 */
	STRINGS_MAP _appStrings;

	/**
	 * Locales with available strings
	 */
	std::vector<std::string> _availableLocales;

	/**
	 * Currently selected locale
	 */
	int _currentLocale;

	LocaleStrings();

public:

	void initLocales();

	/**
	 * \param id Target string identifier (from generated LOCALE_STRINGS enum)
	 * \return The string for the current locale
	 */
	std::string getString(const LOCALE_STRINGS id);

	static std::string GetString(const LOCALE_STRINGS id);

	/**
	 * \param idx Target locale index
	 * \return The name of the locale
	 */
	std::string getLocale(int idx);

	/**
	 * \return Currently active locale index
	 */
	int getCurrentLocaleIdx() const;

	/**
	 * \return Currently active locale name
	 */
	std::string getCurrentLocaleStr();

	/**
	 * \return Collection of avaibale locale names
	 */
	const std::vector<std::string>& getLocales();

	/**
	 * \brief Changes active locale
	 * 
	 * \param idx New target locale index
	 */
	void setLocale(int idx);


};

/**
 * \brief Splits a CSV string line
 * 
 * \param s CSV string
 * \param delimiter CSV delimiter
 * \return Vector with separated strings
 */
std::vector<std::string> splitstring(std::string s, const std::string& delimiter);
