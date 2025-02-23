#pragma once

#ifdef CrabEngine_EXPORTS
#define CrabEngineAPI __declspec(dllexport)
#else
#define CrabEngineAPI __declspec(dllimport)
#endif