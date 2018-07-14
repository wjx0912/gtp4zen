#ifndef __ZEN6_GTP_H__
#define __ZEN6_GTP_H__
#include <string>
#include "gtp.h"

class CZen6Gtp : public CGtp
{
public:
	CZen6Gtp();
	~CZen6Gtp();

public:
	bool load(std::string zen_dll_path, std::string lua_engine_path);
	bool unload();

public:
	std::string list_commands();
	std::string name();
	std::string version();
	std::string protocol_version();
	std::string clear_board();
	std::string boardsize(int size);
	std::string komi(float k);
	std::string free_handicap(const std::vector<std::string> &posarray);
	std::string winrate();
	std::string undo();
	std::string play(std::string color, std::string position);
	std::string genmove(std::string color);
	std::string time_settings(int main_time, int byo_yomi_time, int byo_yomi_stones);
	std::string time_left(std::string color, int time, int stones);

private:
	std::string __genmove(std::string _color, int _maxtime, int _strength);
	std::string __find_best_move(bool debug, int &x, int &y, int &simulation, float &W);
	int lua_genmove_calctime(int cur_move_num, int time_left);
	float lua_komi_get();

private:
	void	*m_proxy;
};

#endif

