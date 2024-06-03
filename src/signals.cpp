//
// Created by Lourenco on 31/05/2024.
//

#include "../inc/main.hpp"

int turn_off = false;

void	ctrlc(int s)
{
	if (s == 2)
	{
		turn_off = true;
	}
}

void	sigHandler()
{
	signal(SIGINT, ctrlc);
	signal(SIGQUIT, SIG_IGN);
}