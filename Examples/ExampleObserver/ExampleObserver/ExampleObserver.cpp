// ExampleObserver.cpp : Defines the entry point for the console application.
//
#include "sc2api/sc2_api.h"
#include "sc2utils/sc2_manage_process.h"

#include <iostream>
#include <windows.h>

#include "../../../CameraModule.h"


void loadReplayPaths(const std::string & name, std::vector<std::string> & v)
{
	if (name.find("2Replay") != std::string::npos)
	{
		v.push_back(name);
		return;
	}
	std::string pattern(name);
	pattern.append("\\*");
	WIN32_FIND_DATAA data;
	HANDLE hFind;
	if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (data.dwFileAttributes != 16lu)
			{
				const std::string fileName = name + "/" + data.cFileName;
				if (std::find(v.begin(), v.end(), fileName) == v.end())
				{
					v.push_back(fileName);
				}
			}
		}
		while (FindNextFileA(hFind, &data) != 0);
		FindClose(hFind);
	}
}


class Replay : public sc2::ReplayObserver
{
public:
	CameraModuleObserver m_cameraModule;

	Replay() : sc2::ReplayObserver(),m_cameraModule(this)
	{
	}

	void OnGameStart() final
	{
		m_cameraModule.onStart();
	}

	void OnUnitCreated(const sc2::Unit* unit) final
	{
		m_cameraModule.moveCameraUnitCreated(unit);
	}

	void OnStep() final
	{
		m_cameraModule.onFrame();
	}

	void OnGameEnd() final
	{
		std::cout << "Game end." << std::endl;
	}
};


int main(int argc, char* argv[]) {
	if (argc != 2)
	{
		std::cout << "Please provide the replay path as command line argument." << std::endl;
		return 1;
	}
	
	
	
	std::string replayPath = argv[argc - 1];
	std::vector<std::string> replayFiles;
	int replayIndex = 0;
	bool shouldStop = false;
	sc2::Coordinator coordinator;
	if (!coordinator.LoadSettings(argc - 1, argv)) {
		return 1;
	}
	Replay replay_observer;
	coordinator.AddReplayObserver(&replay_observer);
	while (true)
	{
		
		loadReplayPaths(replayPath, replayFiles);
		if (replayIndex == replayFiles.size())
		{
			std::cout << "There are no more replays at "<< replayPath <<"! I will wait for 30 seconds and try again.\n" << std::endl;
			Sleep(30000);
			continue;
		}
		const std::string replayFile = replayFiles[replayIndex];
		if (!coordinator.SetReplayPath(replayFile))
		{
			std::cout << "Could not read the replay: " << replayFile << std::endl;
			std::cout << "Please provide the replay path as command line argument." << std::endl;
			return 1;
		}
		if (!coordinator.HasReplays())
		{
			std::cout << "Could not read the replay: " << replayFile << std::endl;
			std::cout << "Please provide the replay path as command line argument." << std::endl;
			return 1;
		}

		while (coordinator.Update() && !coordinator.AllGamesEnded())
		{

		}
		++replayIndex;
	}
	coordinator.LeaveGame();
}
