#include "WorldServer.h"
#include "Entity\Monster.h"
#include "FileTypes\ZON.h"
#include "FileTypes\ZMO.h"
#include "FileTypes\VFS.h"
#include "FileTypes\QSD.h"
#include "D:\Programmieren\GlobalLogger\GlobalLogger.h"
#include "D:\Programmieren\Better_GUI\GUILabel.h"
#include "D:\Programmieren\Better_GUI\GUIButton.h"

#pragma comment(lib, "DbgHelp.lib")

void startServer() {
	MYSQL mySQL;
	clock_t startTime = clock();
	WorldServer server(29200, &mySQL);
	GlobalLogger& logger = GlobalLogger::getLogger();
	server.loadEncryption();
	logger.info("Startup took %i seconds\n", (clock() - startTime) / 1000);
	if (config->getValueBool("DumpQuests")) {
		logger.info("Dumping Quests...\n");
		logger.debug("Output folder: %s", (::workingPath + "\\QUEST_DUMP\\").c_str());
		server.dumpQuest((::workingPath + "\\QUEST_DUMP\\").c_str(), config->getValueBool("UseQuestNames"));
		logger.info("Dumping Quests finished!\n");
	}
	if (config->getValueBool("DumpAI")) {
		logger.info("Dumping AI...\n");
		logger.debug("Output folder: %s", (::workingPath + "\\AI\\").c_str());
		server.dumpAISeparated((::workingPath + "\\AI\\"));
		logger.info("Dumping AI finished!\n");
	}
	if (config->getValueBool("DumpTelegates")) {
		logger.info("Dumping Telegates...\n");
		logger.debug("Output folder: %s", (::workingPath + "\\Telegates.log").c_str());
		server.dumpTelegates((::workingPath + "\\Telegates.log").c_str());
		logger.info("Dumping Telegates finished!\n");
	}
	if (!server.start()) {
		logger.fatal("Could not start the server!\n");
	}
}

int main() {
	startServer();
	return 0;
}