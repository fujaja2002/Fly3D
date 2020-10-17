#pragma once

class Noncopyable
{
public:

	Noncopyable()
	{

	}

private:

	Noncopyable(const Noncopyable& object);

	Noncopyable& operator=(const Noncopyable& object);
};
