#pragma once

#include "Runtime/Platform/Platform.h"

class FEngineLoop
{
public:
	FEngineLoop();

	virtual ~FEngineLoop();

	virtual int32 PreInit(int32 argc, WIDECHAR* argv);

	virtual int32 Init();

	virtual int32 PostInit();

	virtual void Exit();

	virtual void Tick();

	virtual void PreInitRHI();

	virtual void InitRHI();

	virtual void PostInitRHI();

	virtual void PreInitApp();

	virtual void InitApp();

	virtual void PostInitApp();

	virtual void PreExitApp();

	virtual void ExitApp();

protected:

	double		m_TotalTickTime;
	double		m_MaxTickTime;
	double		m_MinTickTime;
	
	uint64		m_MaxFrameCounter;
};

extern FEngineLoop GEngineLoop;