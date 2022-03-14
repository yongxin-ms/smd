#include <iostream>
#include <limits.h>
#include "main_game.h"
#include "DataCenter.h"

using std::cin;
using std::cout;
using std::endl;

bool working = false;

std::string getTime() {
	time_t curr_time = time(NULL);
	struct tm* stm = localtime(&curr_time);

	char sztmp[32];
	sprintf(sztmp, "%04d-%02d-%2d-%2d-%2d-%02d", 1900 + stm->tm_year, 1 + stm->tm_mon, stm->tm_mday, stm->tm_hour,
			stm->tm_min, stm->tm_sec);
	return sztmp;
}

int main_game(DataCenter* data_center) {
	working = true;

	auto obj = &data_center->GetEntry().players;
	std::string input;
	while (working) {
		cin >> input;
		std::transform(input.begin(), input.end(), input.begin(), [](char c) { return std::tolower(c); });
		if (input == "x" || input == "e" || input == "exit" || input == "q" || input == "quit") {
			working = false;
			break;
		}

		std::string op;
		int64_t playerId;
		
		auto parseInput = [&](const std::string& op){
			playerId = 0;
			auto pos = input.find(op);
			if (pos != std::string::npos){
				std::string strPlayerId = input.substr(pos + op.size(), input.size());
				playerId = std::atoll(strPlayerId.c_str());
			}
		};

		do {
			parseInput("login.");
			if (playerId > 0) {
				UniqsModel::Player player;
				player.playerid = playerId;
				player.level = 1;
				player.playername = "I am player [" + std::to_string(playerId) + "]";
				player.lastlogintime = getTime();
				obj->insert(std::make_pair(playerId, player));
				break;
			}

			parseInput("logout.");
			if (playerId > 0) {
				auto it = obj->find(playerId);
				if (it == obj->end()) {
					cout << "player not found." << endl;
					break;
				}
				auto& player = it->second;
				player.lastlogouttime = getTime();
				break;
			}

			parseInput("levelup.");
			if (playerId > 0) {
				auto it = obj->find(playerId);
				if (it == obj->end()) {
					cout << "player not found." << endl;
					break;
				}
				auto& player = it->second;
				++player.level;
				break;
			}

			parseInput("additem.");
			if (playerId > 0) {
				auto it = obj->find(playerId);
				if (it == obj->end()) {
					cout << "player not found." << endl;
					break;
				}
				auto& player = it->second;
				UniqsModel::Item item;
				item.itemid = 200 - player.items.size();
				item.param1 = 1234;
				cout << "added item " << item.itemid << endl;
				player.items.insert(std::make_pair(item.itemid, item));

				break;
			}

			parseInput("delitem.");
			if (playerId > 0) {
				auto it = obj->find(playerId);
				if (it == obj->end()) {
					cout << "player not found." << endl;
					break;
				}
				auto& player = it->second;
				if (player.items.empty()) {
					cout << "no item to delete" << endl;
				}
				int64_t itemId = INT_MAX;
				for (auto it_item : player.items) {
					itemId = std::min(it_item.first, itemId);
				}
				player.items.erase(player.items.find(itemId));
				cout << "del item " << itemId << endl;
			}
		} while (false);
	}

	return 0;
}
