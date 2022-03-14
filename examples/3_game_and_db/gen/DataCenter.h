// ########## 本文件为生成文件，不要手动修改！！！ ##########
// ########## UniqsProto uniqs@163.com ##########

#pragma once

#include "smd.h"
#include "Player.h"

namespace UniqsModel {
struct StData {
	smd::shm_map<int64_t, UniqsModel::Player> players;
};
} // namespace UniqsModel

// 数据中心
class DataCenter : public smd::Env<UniqsModel::StData> {
public:
};
