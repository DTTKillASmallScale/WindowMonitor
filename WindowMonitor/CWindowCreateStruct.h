#pragma once

class CWindow;
class CWindowCreateStruct
{
public:
	void Create(CWindow & window);

protected:
	virtual void Configure(CREATESTRUCT & cs) = 0;
};