// ########## 本文件为生成文件，不要手动修改！！！ ##########
// ########## UniqsProto uniqs@163.com ##########

#pragma once

#include "sm_env.h"
#include "Player.h"

namespace UniqsModel {
class DataCenter {
public:
	smd::shm_map<int64_t, UniqsModel::Player> players;
};
} // namespace UniqsModel
