// GameNetWrapper.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "NetWork/NetEventLoop.h"
#include "NetWork/TcpListener.h"
#include <iostream>


int _tmain(int argc, _TCHAR* argv[])
{
	g_loop.init();

	for (int j = 0;j != 10;++j)
	{
		g_loop.push_event([j](){
			std::cout << "hello:"<<j << std::endl;
		});
	}

	TcpListener lis(g_loop);

	int i;
	std::cin >> i;

	g_loop.close();
	return 0;
}

