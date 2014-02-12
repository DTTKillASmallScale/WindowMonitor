#include "stdafx.h"
#include "resource.h"
#include "WindowFilterBlacklistItem.h"
#include "WindowHelper.h"

WindowFilterBlacklistItem::WindowFilterBlacklistItem() :
	testTitle(false),
	testClassName(false)
{
}

WindowFilterBlacklistItem::WindowFilterBlacklistItem(std::wstring const & title, std::wstring const & className)
{
	SetTitlePattern(title);
	SetClassNamePattern(className);
}

void WindowFilterBlacklistItem::SetTitlePattern(std::wstring const & titlePattern)
{
	try
	{
		titleRegex.assign(titlePattern);
		testTitle = (titlePattern.length() > 0);
	}
	catch (std::regex_error e)
	{
		WindowHelper::DisplayExceptionMessage(IDS_BLACKLIST_ERROR_TITLE, IDS_BLACKLIST_ERROR_FILE, e);
		testTitle = false;
	}
}

void WindowFilterBlacklistItem::SetClassNamePattern(std::wstring const & classNamePattern)
{
	try
	{
		classNameRegex.assign(classNamePattern);
		testClassName = (classNamePattern.length() > 0);
	}
	catch (std::regex_error e)
	{
		WindowHelper::DisplayExceptionMessage(IDS_BLACKLIST_ERROR_TITLE, IDS_BLACKLIST_ERROR_FILE, e);
		testClassName = false;
	}
}

bool WindowFilterBlacklistItem::IsFiltered(std::wstring const & title, std::wstring const & className)
{
	if (testTitle && testClassName) return std::regex_match(title, titleRegex) && std::regex_match(className, classNameRegex);
	else if (testTitle) return std::regex_match(title, titleRegex);
	else if (testClassName) return std::regex_match(className, classNameRegex);
	return false;
}
