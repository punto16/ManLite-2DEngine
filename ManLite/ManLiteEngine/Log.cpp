#include "Log.h"

#include "EngineCore.h"
#include "Defs.h"

#include <windows.h>
#include <stdio.h>
#include <vector>
#include <tchar.h>

void Log(const char file[], int line, LogType type, const char* format, ...)
{
	static char tmpString1[BUFFER_SIZE];
	static char tmpString2[BUFFER_SIZE];
	static va_list ap;

	va_start(ap, format);
	vsprintf_s(tmpString1, BUFFER_SIZE, format, ap);
	va_end(ap);

	sprintf_s(tmpString2, BUFFER_SIZE, "\n%s(%d) : %s", file, line, tmpString1);
	OutputDebugStringA(tmpString2);

	if (engine != nullptr)
	{
		sprintf_s(tmpString2, BUFFER_SIZE, "%s", tmpString1);
		engine->AddLog(type, tmpString2);
	}
}