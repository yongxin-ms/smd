#include "main_db.h"
#include <iostream>
#include <limits.h>
#include "DataCenter.h"

using std::cin;
using std::cout;
using std::endl;

std::string getTime() {
	time_t curr_time = time(NULL);
	struct tm* stm = localtime(&curr_time);

	char sztmp[32];
	sprintf(sztmp, "%04d-%02d-%2d-%2d-%2d-%02d", 1900 + stm->tm_year, 1 + stm->tm_mon, stm->tm_mday, stm->tm_hour,
			stm->tm_min, stm->tm_sec);
	return sztmp;
}

int main_db(UniqsModel::DataCenter& data_center) {
	auto obj = &data_center.players;
	std::string input;
	while (true) {
		cin >> input;
		std::transform(input.begin(), input.end(), input.begin(), [](char c) { return std::tolower(c); });
		if (input == "x" || input == "e" || input == "exit" || input == "q" || input == "quit") {
			break;
		}

		int64_t playerId;

		auto parseInput = [&](const std::string& op) {
			playerId = 0;
			auto pos = input.find(op);
			if (pos != std::string::npos) {
				std::string strPlayerId = input.substr(pos + op.size(), input.size());
				playerId = std::atoll(strPlayerId.c_str());
			}
		};

		do {
			parseInput("print.");
			if (playerId > 0) {
				auto it = obj->find(playerId);
				if (it == obj->end()) {
					cout << "player not found." << endl;
					break;
				}
				auto& player = it->second;
				cout << "playerid:" << player.playerid << "\tplayername：" << player.playername.ToString()
					 << "\tlevel:" << player.level << "\t";
				cout << endl;
				cout << "lastlogintime:" << player.lastlogintime.ToString()
					 << "\tlastlogouttime:" << player.lastlogouttime.ToString() << "\t";
				cout << endl;
				cout << "items:=============================" << endl;
				for (auto it_item : player.items) {
					cout << "itemid:" << it_item.second.itemid << "\t";
					cout << "param1:" << it_item.second.param1 << " | ";
				}
				cout << endl;
				cout << " ******************************************************************************** " << endl;
				cout << endl;

				break;
			}
		} while (false);
	}

	return 0;
}
