/*
 * Copyright (C) 2016 necropotame (necropotame@gmail.com)
 * 
 * This file is part of TeeUniverse.
 * 
 * TeeUniverse is free software: you can redistribute it and/or  modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * TeeUniverse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with TeeUniverse.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <shared/system/debug.h>
#include <shared/system/string.h>
#include <shared/components/assetsmanager.h>
#include <shared/components/cli.h>
#include <editor/kernel.h>

#include <cstdlib>
#include <SDL.h>

int main(int argc, char* argv[])
{
	//Init SDL
	{
		if(SDL_Init(0) < 0)
		{
			dbg_msg("SDL", "unable to init SDL base: %s", SDL_GetError());
			return 0;
		}

		atexit(SDL_Quit);
	}
	
	CEditorKernel* pKernel = new CEditorKernel();
	if(!pKernel->Init(argc, (const char**) argv))
	{
		dbg_msg("Kernel", "unable to initialize client kernel");
		exit(EXIT_FAILURE);
	}
	
	pKernel->AssetsManager()->EnableAssetsHistory();
	
	pKernel->CLI()->ExecuteFile("config/settings_editor.cfg");
	
	bool Running = true;
	do
	{
		Running = Running && pKernel->PreUpdate();
		Running = Running && pKernel->PostUpdate();
	}
	while(Running);
	
	pKernel->CLI()->Execute("save_config config/settings_editor.cfg");
	
	pKernel->Shutdown();
	delete pKernel;

	exit(EXIT_SUCCESS);
}
