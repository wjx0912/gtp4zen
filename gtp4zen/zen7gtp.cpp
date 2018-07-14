#include "stdafx.h"
#include <assert.h>
#include <Windows.h>
#include "zen7gtp.h"

#ifndef LIBZENAPI
#define LIBZENAPI	__cdecl
#endif

typedef void(LIBZENAPI *_ZenClearBoard)(void);
typedef int(LIBZENAPI *_ZenGetNextColor)(void);
typedef void(LIBZENAPI *_ZenGetTopMoveInfo)(int, int &, int &, int &, float &, char *, int);
typedef void(LIBZENAPI *_ZenInitialize)(char const *);
typedef bool(LIBZENAPI *_ZenIsInitialized)(void);
typedef bool(LIBZENAPI *_ZenIsThinking)(void);
typedef void(LIBZENAPI *_ZenPass)(int);
typedef bool(LIBZENAPI *_ZenPlay)(int, int, int);
typedef void(LIBZENAPI *_ZenSetBoardSize)(int);
typedef void(LIBZENAPI *_ZenSetKomi)(float);
typedef void(LIBZENAPI *_ZenSetMaxTime)(float);
typedef void(LIBZENAPI *_ZenSetNumberOfSimulations)(int);
typedef void(LIBZENAPI *_ZenSetNumberOfThreads)(int);
typedef void(LIBZENAPI *_ZenSetPnLevel)(int);
typedef void(LIBZENAPI *_ZenSetPnWeight)(float);
typedef void(LIBZENAPI *_ZenStartThinking)(int);
typedef void(LIBZENAPI *_ZenStopThinking)(void);
typedef bool(LIBZENAPI *_ZenUndo)(int);

struct _zen_dll_proxy {
	// 内部变量
	HMODULE			m_hModule;
	std::string		m_dllpath;
	int			m_boardsize;
	ptime			m_maintime;
	ptime			m_lefttime;
	lua_State		*m_L;
	int			m_curmovenum;
	std::string		winrate;

	// 导出函数
	//ZenAddStone ZenAddStone1;
	_ZenClearBoard ZenClearBoard;
	//void (*ZenFixedHandicap)(int);
	//void (*ZenGetBestMoveRate)(void);
	//void (*ZenGetBoardColor)(int, int);
	//void (*ZenGetHistorySize)(void);
	_ZenGetNextColor ZenGetNextColor;
	//void (*ZenGetNumBlackPrisoners)(void);
	//void (*ZenGetNumWhitePrisoners)(void);
	//void (*ZenGetPriorKnowledge)(int(*const)[19]);
	//void (*ZenGetTerritoryStatictics)(int(*const)[19]);
	_ZenGetTopMoveInfo ZenGetTopMoveInfo;
	_ZenInitialize ZenInitialize;
	_ZenIsInitialized ZenIsInitialized;
	//void (*ZenIsInitialized)(void);
	//void (*ZenIsLegal)(int, int, int);
	//void (*ZenIsSuicide)(int, int, int);
	_ZenIsThinking ZenIsThinking;
	//void (*ZenMakeShapeName)(int, int, int, char *, int);
	_ZenPass ZenPass;
	_ZenPlay ZenPlay;
	//void (*ZenReadGeneratedMove)(int &, int &, bool &, bool &);
	_ZenSetBoardSize ZenSetBoardSize;
	_ZenSetKomi ZenSetKomi;
	_ZenSetMaxTime ZenSetMaxTime;
	//void (*ZenSetNextColor)(int);
	_ZenSetNumberOfSimulations ZenSetNumberOfSimulations;
	_ZenSetNumberOfThreads ZenSetNumberOfThreads;
	_ZenSetPnLevel ZenSetPnLevel;
	_ZenSetPnWeight ZenSetPnWeight;
	_ZenStartThinking ZenStartThinking;
	_ZenStopThinking ZenStopThinking;
	_ZenUndo ZenUndo;
	//void (*ZenTimeLeft)(int, int, int);
	//void (*ZenTimeSettings)(int, int, int);
	//void (*ZenUndo)(int);
};

CZen7Gtp::CZen7Gtp() : m_proxy(NULL)
{
}

CZen7Gtp::~CZen7Gtp()
{
	unload();
}

// cur_move_num：当前手数，time_left：剩余时间
int CZen7Gtp::lua_genmove_calctime(int cur_move_num, int time_left)
{
	// 执行初始化函数，注意平衡堆栈，lua_gettop(m_L)可以获取当前栈使用
	int ret = lua_getglobal(((_zen_dll_proxy*)m_proxy)->m_L, "genmove_calctime");
	int result = -1;
	if (ret > 0) {
		lua_pushinteger(((_zen_dll_proxy*)m_proxy)->m_L, cur_move_num);
		lua_pushinteger(((_zen_dll_proxy*)m_proxy)->m_L, time_left);
		lua_pcall(((_zen_dll_proxy*)m_proxy)->m_L, 2, 1, 0);
		result = (int)luaL_checkinteger(((_zen_dll_proxy*)m_proxy)->m_L, 1);
	} else {
		lua_pop(((_zen_dll_proxy*)m_proxy)->m_L, 1);
	}
	lua_settop(((_zen_dll_proxy*)m_proxy)->m_L, 0);	
	return result;
}

// 
float CZen7Gtp::lua_komi_get()
{
	// 执行初始化函数，注意平衡堆栈，lua_gettop(m_L)可以获取当前栈使用
	int ret = lua_getglobal(((_zen_dll_proxy*)m_proxy)->m_L, "komi");
	float result = -1;
	if (ret > 0) {
		lua_pcall(((_zen_dll_proxy*)m_proxy)->m_L, 0, 1, 0);
		result = (float)luaL_checknumber(((_zen_dll_proxy*)m_proxy)->m_L, 1);
	} else {
		lua_pop(((_zen_dll_proxy*)m_proxy)->m_L, 1);
	}
	lua_settop(((_zen_dll_proxy*)m_proxy)->m_L, 0);
	return result;
}

bool CZen7Gtp::load(std::string zen_dll_path, std::string lua_engine_path)
{
#if 0
	boost::filesystem::path boost_path(zen_dll_path);
	std::string boost_path_str = boost_path.filename().string();
	boost::to_lower(boost_path_str);
	if (!boost::ends_with(boost_path_str, "zen.dll"))
		fprintf(stderr, "WARNING: filename should be zen.dll!\n", zen_dll_path.c_str());
	else
		logprintf(L"dll文件名检测正常");
#endif

	// 正常流程
	unload();

	m_proxy = new _zen_dll_proxy();
	((_zen_dll_proxy*)m_proxy)->m_L = luaL_newstate();
	luaL_openlibs(((_zen_dll_proxy*)m_proxy)->m_L);
	luaL_dostring(((_zen_dll_proxy*)m_proxy)->m_L, "package.path = package.path .. ';./?.lua'");
	luaL_dofile(((_zen_dll_proxy*)m_proxy)->m_L, lua_engine_path.c_str());

	((_zen_dll_proxy*)m_proxy)->m_dllpath = zen_dll_path;
	//((_zen_dll_proxy*)m_proxy)->m_hModule = ::LoadLibraryA(zen_dll_path.c_str());
	((_zen_dll_proxy*)m_proxy)->m_hModule = ::LoadLibrary(CAtoW(zen_dll_path.c_str()));
	//((_zen_dll_proxy*)m_proxy)->m_hModule = ::LoadLibraryEx(CAtoW(zen_dll_path.c_str())
	//	, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (NULL == ((_zen_dll_proxy*)m_proxy)->m_hModule) {
		fprintf(stderr, "load %s failed(errno: %d), zen.dll not exist?\n"
			, zen_dll_path.c_str(), GetLastError());
		logprintf(L"dll加载失败，zen.dll不存在？");
		return false;
	}
	logprintf(L"dll加载正常");

	HMODULE	 hModule = ((_zen_dll_proxy*)m_proxy)->m_hModule;
	((_zen_dll_proxy*)m_proxy)->ZenClearBoard = (_ZenClearBoard)GetProcAddress(hModule, "?ZenClearBoard@@YAXXZ");
	assert(((_zen_dll_proxy*)m_proxy)->ZenClearBoard);
	((_zen_dll_proxy*)m_proxy)->ZenGetNextColor = (_ZenGetNextColor)GetProcAddress(hModule, "?ZenGetNextColor@@YAHXZ");
	assert(((_zen_dll_proxy*)m_proxy)->ZenGetNextColor);
	((_zen_dll_proxy*)m_proxy)->ZenGetTopMoveInfo = (_ZenGetTopMoveInfo)GetProcAddress(hModule, "?ZenGetTopMoveInfo@@YAXHAAH00AAMPADH@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenGetTopMoveInfo);
	((_zen_dll_proxy*)m_proxy)->ZenInitialize = (_ZenInitialize)GetProcAddress(hModule, "?ZenInitialize@@YAXPBD@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenInitialize);
	((_zen_dll_proxy*)m_proxy)->ZenIsInitialized = (_ZenIsInitialized)GetProcAddress(hModule, "?ZenIsInitialized@@YA_NXZ");
	assert(((_zen_dll_proxy*)m_proxy)->ZenIsInitialized);
	((_zen_dll_proxy*)m_proxy)->ZenIsThinking = (_ZenIsThinking)GetProcAddress(hModule, "?ZenIsThinking@@YA_NXZ");
	assert(((_zen_dll_proxy*)m_proxy)->ZenIsThinking);
	((_zen_dll_proxy*)m_proxy)->ZenPass = (_ZenPass)GetProcAddress(hModule, "?ZenPass@@YAXH@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenPass);
	((_zen_dll_proxy*)m_proxy)->ZenPlay = (_ZenPlay)GetProcAddress(hModule, "?ZenPlay@@YA_NHHH@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenPlay);
	((_zen_dll_proxy*)m_proxy)->ZenSetBoardSize = (_ZenSetBoardSize)GetProcAddress(hModule, "?ZenSetBoardSize@@YAXH@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenSetBoardSize);
	((_zen_dll_proxy*)m_proxy)->ZenSetKomi = (_ZenSetKomi)GetProcAddress(hModule, "?ZenSetKomi@@YAXM@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenSetKomi);
	((_zen_dll_proxy*)m_proxy)->ZenSetMaxTime = (_ZenSetMaxTime)GetProcAddress(hModule, "?ZenSetMaxTime@@YAXM@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenSetMaxTime);
	((_zen_dll_proxy*)m_proxy)->ZenSetNumberOfSimulations = (_ZenSetNumberOfSimulations)GetProcAddress(hModule, "?ZenSetNumberOfSimulations@@YAXH@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenSetNumberOfSimulations);
	((_zen_dll_proxy*)m_proxy)->ZenSetNumberOfThreads = (_ZenSetNumberOfThreads)GetProcAddress(hModule, "?ZenSetNumberOfThreads@@YAXH@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenSetNumberOfThreads);
	((_zen_dll_proxy*)m_proxy)->ZenSetPnLevel = (_ZenSetPnLevel)GetProcAddress(hModule, "?ZenSetPnLevel@@YAXH@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenSetPnLevel);
	((_zen_dll_proxy*)m_proxy)->ZenSetPnWeight = (_ZenSetPnWeight)GetProcAddress(hModule, "?ZenSetPnWeight@@YAXM@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenSetPnWeight);
	((_zen_dll_proxy*)m_proxy)->ZenStartThinking = (_ZenStartThinking)GetProcAddress(hModule, "?ZenStartThinking@@YAXH@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenStartThinking);
	((_zen_dll_proxy*)m_proxy)->ZenStopThinking = (_ZenStopThinking)GetProcAddress(hModule, "?ZenStopThinking@@YAXXZ");
	assert(((_zen_dll_proxy*)m_proxy)->ZenStopThinking);
	((_zen_dll_proxy*)m_proxy)->ZenUndo = (_ZenUndo)GetProcAddress(hModule, "?ZenUndo@@YA_NH@Z");
	assert(((_zen_dll_proxy*)m_proxy)->ZenUndo);

	if (!((_zen_dll_proxy*)m_proxy)->ZenSetPnLevel || !((_zen_dll_proxy*)m_proxy)->ZenSetPnWeight) {
		fprintf(stderr, "zen.dll not copy from zen7 version?\n");
		return false;
	}
	logprintf(L"dll函数定位正常");

	// 初始化
	((_zen_dll_proxy*)m_proxy)->m_boardsize = 19;
	((_zen_dll_proxy*)m_proxy)->m_curmovenum = 0;
	((_zen_dll_proxy*)m_proxy)->m_maintime = microsec_clock::universal_time() + days(365);
	((_zen_dll_proxy*)m_proxy)->m_lefttime = microsec_clock::universal_time() + days(365);
	((_zen_dll_proxy*)m_proxy)->ZenInitialize("");
	((_zen_dll_proxy*)m_proxy)->ZenSetBoardSize(((_zen_dll_proxy*)m_proxy)->m_boardsize);
	((_zen_dll_proxy*)m_proxy)->ZenSetKomi(6.5);
	((_zen_dll_proxy*)m_proxy)->ZenSetNumberOfThreads(g_threads);
	((_zen_dll_proxy*)m_proxy)->ZenSetNumberOfSimulations(99999999);
	((_zen_dll_proxy*)m_proxy)->ZenSetMaxTime(g_maxtime * (float)1.0);
	((_zen_dll_proxy*)m_proxy)->ZenClearBoard();
	bool is_init = ((_zen_dll_proxy*)m_proxy)->ZenIsInitialized();
	fprintf(stderr, "zen.dll(zen7) initialize %s.\n", is_init ? "success" : "failed");
	logprintf(L"zen.dll(zen7) initialize %s.", is_init ? L"success" : L"failed");
	fflush(stderr);
	if (!g_debug)
		fclose(stderr);
	return is_init;
}

bool CZen7Gtp::unload()
{
	if (m_proxy) {
		//
		lua_close(((_zen_dll_proxy*)m_proxy)->m_L);
		((_zen_dll_proxy*)m_proxy)->m_L = NULL;

		//
		::FreeLibrary(((_zen_dll_proxy*)m_proxy)->m_hModule);
		delete (_zen_dll_proxy *)m_proxy;
		m_proxy = NULL;
	}
	return true;
}

std::string CZen7Gtp::list_commands()
{
	return  "= list_commands\n"		\
		"name\n"			\
		"version\n"			\
		"protocol_version\n"		\
		"quit\n"			\
		"clear_board\n"			\
		"boardsize\n"			\
		"komi\n"			\
		"set_free_handicap\n"		\
		"place_free_handicap\n"		\
		"winrate\n"			\
		"undo\n"			\
		"play\n"			\
		"genmove\n"			\
		"time_settings\n"		\
		"time_left\n"			\
		"";
}

std::string CZen7Gtp::name()
{
	return "= Gtp4Zen(zen7)\n";
}

std::string CZen7Gtp::version()
{
	return string("= ") + GTP4ZEN_VERSION + string("\n");
}

std::string CZen7Gtp::protocol_version()
{
	return "= 2\n";
}

std::string CZen7Gtp::clear_board()
{
	logprintf(L"clear_board()");
	((_zen_dll_proxy*)m_proxy)->ZenClearBoard();
	((_zen_dll_proxy*)m_proxy)->m_curmovenum = 0;
	((_zen_dll_proxy*)m_proxy)->m_maintime = microsec_clock::universal_time() + days(365);
	((_zen_dll_proxy*)m_proxy)->m_lefttime = microsec_clock::universal_time() + days(365);
	return "= \n";
}

std::string CZen7Gtp::boardsize(int size)
{
	logprintf(L"boardsize(%d)", size);
	if (9 == size || 11 == size || 13 == size || 19 == size) {
		((_zen_dll_proxy*)m_proxy)->m_boardsize = size;
		((_zen_dll_proxy*)m_proxy)->ZenSetBoardSize(size);
		return "= \n";
	} else {
		return "? unknown boardsize\n";
	}
}

std::string CZen7Gtp::komi(float k)
{
	logprintf(L"komi(%.2f)", k);
	if (lua_komi_get() >= 0) {
		logprintf(L"\tlua set komi: %f", lua_komi_get());
		((_zen_dll_proxy*)m_proxy)->ZenSetKomi(lua_komi_get());
		return "= \n";
	}

	if (k >= 0 && k <= 300) {
		((_zen_dll_proxy*)m_proxy)->ZenSetKomi(k);
		return "= \n";
	} else {
		return "? komi range from 0 to 300, mostly 6.5 or 7.5\n";
	}
}

std::string CZen7Gtp::free_handicap(const std::vector<std::string> &posarray)
{
	logprintf(L"set_free_handicap()/place_free_handicap()");
	for (auto &item : posarray) {
		play("b", item.c_str());
	}
	return "= \n";
}

std::string CZen7Gtp::winrate()
{
	return "= " + ((_zen_dll_proxy*)m_proxy)->winrate + "\n";
}

// play b q16
// play w q3
std::string CZen7Gtp::play(std::string _color, std::string position)
{
	logprintf(L"play(%s,%s)"
		, (LPCTSTR)CAtoW(_color.c_str())
		, (LPCTSTR)CAtoW(position.c_str())
		);
	boost::to_lower(_color);
	if ("w" != _color && "b" != _color)
		return "? error parameter\n";
	int color = ("w" == _color ? GTP4ZEN_COLOR_WHITE : GTP4ZEN_COLOR_BLACK);
	((_zen_dll_proxy*)m_proxy)->m_curmovenum++;

	boost::to_upper(position);
	if ("PASS" == position) {
		((_zen_dll_proxy*)m_proxy)->ZenPass(color);
		return "= \n";
	}
	if ("RESIGN" == position) {
		return "= \n";
	}

	if (position.length() < 2 || position.length() > 3)
		return "? error parameter\n";

	// 转换坐标
	int pos_x = __ansi2num(position[0]);
	if (pos_x < 0)
		return "? error parameter\n";
	int pos_y = atoi(position.c_str() + 1);
	if (pos_y < 1 || pos_y >((_zen_dll_proxy*)m_proxy)->m_boardsize)
		return "? error parameter\n";
	pos_y = ((_zen_dll_proxy*)m_proxy)->m_boardsize - pos_y;

	// 落子
	//logprintf(L"ZenPlay(%d, %d, %d)", pos_x, pos_y, color);
	((_zen_dll_proxy*)m_proxy)->winrate = "";
	bool result = ((_zen_dll_proxy*)m_proxy)->ZenPlay(pos_x, pos_y, color);
	if (result)
		return "= \n";
	else
		return "? error parameter\n";
}

// X和Y是坐标, simulation模拟步数（zen计算量）
std::string CZen7Gtp::__find_best_move(bool debug, int &x, int &y, int &simulation, float &W)
{
	// 寻找最优
	int N = -1;
	char S[256];	// 全部手数（含当前手）
#if 1
	for (int i = 4; i >= 0; i--) {	// zen一般给出5个选点
		((_zen_dll_proxy*)m_proxy)->ZenGetTopMoveInfo(i, x, y, simulation, W, S, 99);
		//XTRACE("%d:  %d-%d, P:%d, W:%f,  %s\n", i, x, y, simulation, W, S);
		if (debug) {
			logprintf(L"ZenGetTopMoveInfo(): %d, %d-%d, P:%d, W:%f, S:%s"
				, i, x, y, simulation, W, (LPCTSTR)CAtoW(S));
		}
		if (g_debug && x >= 0 && y >= 0) {
			char debug_str[1024] = "";

			sprintf_s(debug_str
				, 1024
				, "%-10s Weight: %05.2f,    %s\n"
				, __num2ansi(x, y, ((_zen_dll_proxy*)m_proxy)->m_boardsize).c_str()
				, W * 100
				, S
				);
			fprintf(stderr, debug_str);
			fflush(stderr);
		}
		if (0 == i) {
			if (strstr(S, "pass"))
				return "pass";

			std::string result;
			if (x >= 0 && y >= 0 && simulation >= 0) {
				result = __num2ansi(x, y, ((_zen_dll_proxy*)m_proxy)->m_boardsize);
			} else {
				result = "resign";
				result = "pass";
			}
			return result;
		}
	}
#else
	//((_zen_dll_proxy*)m_proxy)->ZenGetTopMoveInfo(0, x, y, simulation, W, S, 99);
	//std::string result;
	//if (x >= 0 && y >= 0 && simulation >= 0) {
	//	result = __num2ansi(x, y, ((_zen_dll_proxy*)m_proxy)->m_boardsize);
	//} else {
	//	result = "pass";
	//	result = "resign";
	//}
	//return result;
#endif
	return "";
}

// 指定最大时间（毫秒），最多步数
std::string CZen7Gtp::__genmove(std::string _color, int _maxtime, int _strength)
{
	logprintf(L"__genmove(%s, %dms, %dstep)", (LPCTSTR)CAtoW(_color.c_str()), _maxtime, _strength);
	boost::to_lower(_color);
	if ("w" != _color && "b" != _color)
		return "? error parameter\n";
	int color = ("w" == _color ? GTP4ZEN_COLOR_WHITE : GTP4ZEN_COLOR_BLACK);

	// 落子
	((_zen_dll_proxy*)m_proxy)->ZenStartThinking(color);
	ptime time_start, time_now;
	millisec_posix_time_system_config::time_duration_type time_elapse;
	time_start = microsec_clock::universal_time();
	int x = -1, y = -1, simulation = -1;
	float W = 0;
	std::wstring reason;
	while (true) {
		if (!((_zen_dll_proxy*)m_proxy)->ZenIsThinking()) {
			reason = L"zen stop thinking";
			break;		// 在小于指定的时间内算好了
		}

		time_now = microsec_clock::universal_time();
		time_elapse = time_now - time_start;
		if (time_elapse.total_milliseconds() >= _maxtime) {
			reason = L"thinking time out";
			break;
		}

		Sleep(g_think_interval);
		__find_best_move(true, x, y, simulation, W);
		logprintf(L"......");
		if (g_debug) {
			fwprintf(stderr, L"......\n");
			fflush(stderr);
		}
		if (simulation >= _strength) {
			reason = L"thinking strength out";
			break;
		}
	}
	((_zen_dll_proxy*)m_proxy)->ZenStopThinking();
	std::string result = __find_best_move(true, x, y, simulation, W);

#if 1
	// log
	time_now = microsec_clock::universal_time();
	time_elapse = time_now - time_start;
	logprintf(L"%s: %s(%d,%d), max_time: %dms, strength: %d, time_elapse: %dms"
		, reason.c_str()
		, (LPCTSTR)CAtoW(result.c_str())
		, x
		, y
		, _maxtime
		, _strength
		, time_elapse.total_milliseconds()
		);
#endif
	play(_color, result);
	char winrate_str[32];
	sprintf_s(winrate_str, 32, "%.2f", W * 100);
	((_zen_dll_proxy*)m_proxy)->winrate = winrate_str;
	return "= " + result + "\n";
}

// genmove b
// genmove()单位是秒，__genmove()单位是毫秒
std::string CZen7Gtp::genmove(std::string _color)
{
	ptime time_now = microsec_clock::universal_time();
	millisec_posix_time_system_config::time_duration_type time_left
		= ((_zen_dll_proxy*)m_proxy)->m_lefttime - time_now;
	__int64 left_time_millseconds = time_left.total_milliseconds();
	logprintf(L"------------------------>>>");
	logprintf(L"genmove(%s)", (LPCTSTR)CAtoW(_color.c_str()));

	std::string result;
	int calctime = lua_genmove_calctime(((_zen_dll_proxy*)m_proxy)->m_curmovenum, (int)left_time_millseconds);
	if (calctime > 0) {
		// lua脚本中返回了时间
		logprintf(L"\tlua script return calctime: %dms", calctime);
		result = __genmove(_color, calctime, g_strength);
	} else {
		if (left_time_millseconds >= 180 * 1000) {
			// 时间大于180秒，随便用
			result = __genmove(_color, g_maxtime * 1000, g_strength);
		} else if (left_time_millseconds >= 90 * 1000) {
			// 小于180秒，大于90秒：每手最多2秒钟
			result = __genmove(_color, 2000, g_strength);
		} else {
			// 小于90秒，每手
			result = __genmove(_color, 100, g_strength);
		}
	}
	logprintf(L"------------------------<<<");
	//logprintf(L"%s(%s) leave<<<--------"
	//	, (LPCTSTR)CAtoW(__FUNCTION__), (LPCTSTR)CAtoW(_color.c_str()));
	return result;
}

std::string CZen7Gtp::undo()
{
	logprintf(L"undo()");
	((_zen_dll_proxy*)m_proxy)->ZenUndo(1);
	return "= \n";
}

// main_time是主要时间，byo_yomi_time是读秒时间，byo_yomi_stones是读秒次数
std::string CZen7Gtp::time_settings(int main_time, int byo_yomi_time, int byo_yomi_stones)
{
	logprintf(L"time_settings(%d, %d, %d)", main_time, byo_yomi_time, byo_yomi_stones);
	((_zen_dll_proxy*)m_proxy)->m_maintime = microsec_clock::universal_time();
	((_zen_dll_proxy*)m_proxy)->m_maintime += seconds(main_time);
	return "= \n";
}

// 如果在主要时间内，stones是0；进入读秒后stones是读秒次数
std::string CZen7Gtp::time_left(std::string color, int time, int stones)
{
	logprintf(L"time_left(%s, %d, %d)", (LPCTSTR)CAtoW(color.c_str()), time, stones);
	((_zen_dll_proxy*)m_proxy)->m_lefttime = microsec_clock::universal_time();
	((_zen_dll_proxy*)m_proxy)->m_lefttime += seconds(time);
	return "= \n";
}

