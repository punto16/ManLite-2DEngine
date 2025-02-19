#include "App.h"

enum MainState
{
	CREATE = 1,
	AWAKE,
	START,
	LOOP,
	CLEAN,
	FAIL,
	EXIT
};

App* app = NULL;

int main(int argc, char* args[])
{
	//LOG(LogType::LOG_INFO, "Engine starting ...");

	MainState state = CREATE;
	int result = EXIT_FAILURE;

	while (state != EXIT)
	{
		switch (state)
		{
		case CREATE:
		{
			app = new App(argc, args);

			if (app != NULL) state = AWAKE;
			else state = FAIL;
			break;
		}
		case AWAKE:
		{
			//LOG(LogType::LOG_INFO, "AWAKE PHASE ===============================");
			if (app->Awake()) state = START;
			else
			{
				//LOG(LogType::LOG_ERROR, "Awake phase");
				state = FAIL;
			}
			break;
		}
		case START:
		{
			//LOG(LogType::LOG_INFO, "START PHASE ===============================");
			if (app->Start())
			{
				state = LOOP;
				//LOG(LogType::LOG_INFO, "UPDATE PHASE ===============================");
			}
			else
			{
				state = FAIL;
				//LOG(LogType::LOG_ERROR, "Start phase");
			}
			break;
		}
		case LOOP:
		{
			if (!app->Update()) state = CLEAN;
			break;
		}
		case CLEAN:
		{
			//LOG(LogType::LOG_INFO, "CLEANUP PHASE ===============================");
			if (app->CleanUp())
			{
				if (app != nullptr)
				{
					delete app;
					app = nullptr;
				}
				result = EXIT_SUCCESS;
				state = EXIT;
			}
			else
				state = FAIL;
			break;
		}
		case FAIL:
		{
			//LOG(LogType::LOG_ERROR, "Exiting with errors :(");
			result = EXIT_FAILURE;
			state = EXIT;
			break;
		}
		default:
			break;
		}
	}
	return result;
}