#include "WorldServer.h"
#include "Entity\Monster.h"
#include "FileTypes\ZON.h"
#include "FileTypes\ZMO.h"
#include "FileTypes\VFS.h"
#include "FileTypes\QSD.h"

#pragma comment(lib, "DbgHelp.lib")

int main() {
	/*ConfigA conf; conf.init("D:\\Games\\iRose Online Server\\config.conf");
	VFS vfs(conf.getValueString("GameFolder"));

	ZMO rakkieAttack(&vfs, "3DDATA\\MOTION\\NPC\\RACCON\\RACCON_ATTACK.ZMO");
	ZMO rakkieShieldAttack(&vfs, "3DDATA\\MOTION\\NPC\\RACCON\\RACCON_SHIELD_ATTACK.ZMO");
	*/
	MYSQL mySQL;
	WorldServer server(29200, &mySQL);
	server.dumpQuest((::workingPath + "\\QUEST_DUMP\\").c_str(), false );
	/*server.loadEncryption();
	
	if (config->getValueBool("DumpAI")) {
		std::cout << "Dumping AI...\r";
		server.dumpAISeparated((::workingPath + "\\AI\\"));
		std::cout << "Dumping AI finished!\n";
	}
	if (config->getValueBool("DumpTelegates")) {
		std::cout << "Dumping Telegates...\r";
		server.dumpTelegates((::workingPath + "\\Telegates.log").c_str());
		std::cout << "Dumping Telegates finished!\n";
	}
	server.start();
	*/
	return 0x00;
}