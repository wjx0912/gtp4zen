#ifndef __ZEN_GTP_H__
#define __ZEN_GTP_H__
#include <string>

class CGtp
{
public:
	virtual bool load(std::string zen_dll_path, std::string lua_engine_path) = 0;
	virtual bool unload() = 0;

public:
	virtual std::string list_commands() = 0;
	virtual std::string name() = 0;
	virtual std::string version() = 0;
	virtual std::string protocol_version() = 0;
	virtual std::string clear_board() = 0;
	virtual std::string boardsize(int size) = 0;
	virtual std::string komi(float k) = 0;
	virtual std::string free_handicap(const std::vector<std::string> &posarray) = 0;
	virtual std::string winrate() = 0;
	virtual std::string undo() = 0;
	virtual std::string play(std::string color, std::string position) = 0;
	virtual std::string genmove(std::string color) = 0;
	virtual std::string time_settings(int main_time, int byo_yomi_time, int byo_yomi_stones) = 0;
	virtual std::string time_left(std::string color, int time, int stones) = 0;
};

#endif

