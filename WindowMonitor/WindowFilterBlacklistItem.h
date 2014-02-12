#pragma once
class WindowFilterBlacklistItem
{
public:
	WindowFilterBlacklistItem();
	WindowFilterBlacklistItem(std::wstring const & title, std::wstring const & className);
	void SetTitlePattern(std::wstring const & titlePattern);
	void SetClassNamePattern(std::wstring const & classNamePattern);
	bool IsFiltered(std::wstring const & title, std::wstring const & className);

private:
	std::wregex titleRegex, classNameRegex;
	bool testTitle, testClassName;
};

