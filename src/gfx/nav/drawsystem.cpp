#include <set>
#include "vs_path.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include "gfx/gauge.h"
#include "gfx/cockpit.h"
#include "universe.h"
#include "star_system.h"
#include "cmd/unit_generic.h"
#include "cmd/unit_factory.h"
#include "cmd/iterator.h"
#include "cmd/collection.h"
#include "gfx/hud.h"
#include "gfx/vdu.h"
#include "lin_time.h"//for fps
#include "config_xml.h"
#include "lin_time.h"
#include "cmd/images.h"
#include "cmd/script/mission.h"
#include "cmd/script/msgcenter.h"
#include "cmd/ai/flyjoystick.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/ai/aggressive.h"
#include "main_loop.h"
#include <assert.h>	// needed for assert() calls
#include "savegame.h"
#include "gfx/animation.h"
#include "gfx/mesh.h"
#include "universe_util.h"
#include "in_mouse.h"
#include "gui/glut_support.h"
#include "networking/netclient.h"
#include "cmd/unit_util.h"
#include "math.h"
#include "save_util.h"



#include "navscreen.h"
#include "gfx/masks.h"


//	**********************************



//	Main function for drawing a CURRENT system
//	works :
//		scans all items, records min + max coords of the system, for relevant items
//		rescans, and enlists the found items that it wants drawn
//			-	items with mouse over them will go into a mouselist.
//		draws the draw lists, with the mouse lists cycled 'n' times (according to kliks)
//	**********************************
void NavigationSystem::DrawSystem()
{
	UniverseUtil::PythonUnitIter bleh = UniverseUtil::getUnitList(); 
	if(!(*bleh))
		return;


	//what's my name
	//***************************
	TextPlane systemname;	//	will be used to display shits names
	string systemnamestring = "Current System : " + _Universe->activeStarSystem()->getName();
	int length = systemnamestring.size();
	float offset = (float(length)*0.005);
	systemname.col = GFXColor(1, 1, .7, 1);
	systemname.SetPos( (((screenskipby4[0]+screenskipby4[1])/2)-offset) , screenskipby4[3]);
	systemname.SetText(systemnamestring);
//	systemname.SetCharSize(1, 1);
	systemname.Draw();
	//***************************


	navdrawlist mainlist(0, screenoccupation, factioncolours);		//	lists of items to draw
	mainlist.unselectedalpha = unselectedalpha;
	navdrawlist mouselist(1, screenoccupation, factioncolours);	//	lists of items to draw that are in mouse range

	
	QVector pos;	//	item position
	QVector pos_flat;	//	item position flat on plane

	float zdistance = 0.0;
	float zscale = 0.0;

	Adjust3dTransformation();

	//	Set up first item to compare to + centres
	//	**********************************
	while( (*bleh) && (_Universe->AccessCockpit()->GetParent() != (*bleh)) &&(UnitUtil::isSun(*bleh) || !UnitUtil::isSignificant (*bleh))  )	//	no sun's in initial setup
	{
		++bleh;
	}

	if(!(*bleh))	//	nothing there that's significant, just do it all
		bleh = UniverseUtil::getUnitList();


	//GET THE POSITION
	//*************************
	pos = (*bleh)->Position();
	ReplaceAxes(pos);
	//*************************


	//Modify by old rotation amount
	//*************************
	pos = dxyz(pos, 0, ry, 0);
	pos = dxyz(pos, rx, 0, 0);
	//*************************

	
	float max_x = (float)pos.i;
	float min_x = (float)pos.i;
	float max_y = (float)pos.j;
	float min_y = (float)pos.j;
	float max_z = (float)pos.k;
	float min_z = (float)pos.k;

	float themaxvalue = fabs(pos.i);

	float center_nav_x = ((screenskipby4[0] + screenskipby4[1]) / 2);
	float center_nav_y = ((screenskipby4[2] + screenskipby4[3]) / 2);
	//	**********************************


	//Retrieve unit data min/max
	//**********************************
	while (*bleh)	//	this goes through one time to get the major components locations, and scales its output appropriately
	{
		if(UnitUtil::isSun(*bleh))
		{
			++bleh;
			continue;
		}
		string temp = (*bleh)->name; 



		pos = (*bleh)->Position();
		ReplaceAxes(pos);
		
		//Modify by old rotation amount
		//*************************
		pos = dxyz(pos, 0, ry, 0);
		pos = dxyz(pos, rx, 0, 0);
		//*************************
		//*************************
		

		if( (UnitUtil::isSignificant (*bleh)) || (_Universe->AccessCockpit()->GetParent() == (*bleh)) )
		{
			RecordMinAndMax(pos,min_x,max_x,min_y,max_y,min_z,max_z,themaxvalue);

		}

		++bleh;
	} 
	//**********************************



	//Find Centers
	//**********************************
	center_x = (min_x + max_x)/2;
	center_y = (min_y + max_y)/2;
	center_z = (min_z + max_z)/2;
	//**********************************


#define SQRT3 1.7320508
//	themaxvalue = sqrt(themaxvalue*themaxvalue + themaxvalue*themaxvalue + themaxvalue*themaxvalue);
	themaxvalue = SQRT3*themaxvalue;


	//Set Camera Distance
	//**********************************

	{
		float half_x=(0.5*(max_x - min_x));
		float half_y=(0.5*(max_y - min_y));
		float half_z=(0.5*(max_z - min_z));
	
		camera_z = sqrt( ( half_x * half_x ) + ( half_y * half_y ) + ( half_z * half_z ));
	
	}

	//**********************************

	DrawOriginOrientationTri(center_nav_x, center_nav_y);

/*
	string mystr ("max x "+XMLSupport::tostring (max_x)); 
	UniverseUtil::IOmessage (0,"game","all",mystr);


	string mystr2 ("min x "+XMLSupport::tostring (min_x)); 
	UniverseUtil::IOmessage (0,"game","all",mystr2);


	string mystr3 ("max y "+XMLSupport::tostring (max_y)); 
	UniverseUtil::IOmessage (0,"game","all",mystr3);


	string mystr4 ("min y "+XMLSupport::tostring (min_y)); 
	UniverseUtil::IOmessage (0,"game","all",mystr4);


	string mystrcx ("center x "+XMLSupport::tostring (center_x)); 
	UniverseUtil::IOmessage (0,"game","all",mystrcx);


	string mystrcy ("center y "+XMLSupport::tostring (center_y)); 
	UniverseUtil::IOmessage (0,"game","all",mystrcy);
*/



	Unit* ThePlayer = ( UniverseUtil::getPlayerX( UniverseUtil::getCurrentPlayer() ) );

	//	Enlist the items and attributes
	//	**********************************
	un_iter blah = UniverseUtil::getUnitList();  
	while (*blah)	//	this draws the points
	{

		//	Retrieve unit data
		//	**********************************
		string temp = (*blah)->name;
			


		pos = (*blah)->Position();
			
		float the_x, the_y, system_item_scale_temp;
		TranslateAndDisplay(pos, pos_flat, center_nav_x, center_nav_y, themaxvalue, zscale, zdistance, the_x, the_y,system_item_scale_temp);

		//IGNORE OFF SCREEN
		//**********************************
		if(	!TestIfInRange(screenskipby4[0], screenskipby4[1], screenskipby4[2], screenskipby4[3], the_x, the_y))
			{
				++blah;
				continue;
			}
		//**********************************


		//	Now starts the test that determines the type of things and inserts
		//	|
		//	|
		//	\/


		float insert_size = 0.0;
		int insert_type = navambiguous;

			


		if(	(*blah)->isUnit()==UNITPTR )	//	unit
		{
			if(UnitUtil::isPlayerStarship(*blah) > -1)	//	is a PLAYER SHIP
			{
				if (UnitUtil::isPlayerStarship (*blah)==UniverseUtil::getCurrentPlayer()) //	is THE PLAYER
				{
					insert_type = navcurrentplayer;
					insert_size = navcurrentplayersize;
				}
				else	//	is A PLAYER
				{
					insert_type = navplayer;
					insert_size = navplayersize;
				}
			}
			else	//	is a non player ship
			{
				if(UnitUtil::isSignificant (*blah))	//	capship or station
				{
					if((*blah)->GetComputerData().max_speed()==0)		//	is this item STATIONARY?
					{
						insert_type = navstation;
						insert_size = navstationsize;
					}
					else // it moves = capship
					{
						if(ThePlayer->InRange((*blah),false,true))	//	only insert if in range
						{
							insert_type = navcapship;
							insert_size = navcapshipsize;
						}
					}
				}
				else	//	fighter
				{
					if(ThePlayer->InRange((*blah),false,true))	//	only insert if in range
					{
						insert_type = navfighter;
						insert_size = navfightersize;
					}
				}
			}
		}


		else if((*blah)->isUnit()==PLANETPTR  )	//	is it a PLANET?
		{
			if(UnitUtil::isSun(*blah))	//	is this a SUN?
			{
				insert_type = navsun;
				insert_size = navsunsize;
			}				
				

			else if (!((*blah)->GetDestinations().empty()))	//	is a jump point (has destinations)
			{
				insert_type = navjump;
				insert_size = navjumpsize;
			}
			
			
			else	//	its a planet
			{
				insert_type = navplanet;
				insert_size = navplanetsize;
			}
		}
		
		
		else if((*blah)->isUnit()==MISSILEPTR)
		{
			//	a missile
			insert_type = navmissile;
			insert_size = navmissilesize;
		}


		else if((*blah)->isUnit()==ASTEROIDPTR)
		{
			//	a missile
			insert_type = navasteroid;
			insert_size = navasteroidsize;
		}


		else if((*blah)->isUnit()==NEBULAPTR)
		{
			//	a missile
			insert_type = navnebula;
			insert_size = navnebulasize;
		}
		

		else	//	undefined non unit
		{
			insert_type = navambiguous;
			insert_size = navambiguoussize;
		}
		
		if(system_item_scale_temp > (system_item_scale * 3))
		{
			system_item_scale_temp = (system_item_scale * 3);
		}

		insert_size*=system_item_scale_temp;

		if (_Universe->AccessCockpit()->GetParent()->Target()==(*blah))
		{
			// Get a color from the config
			static float col[4]={1, 0.3, 0.3, 0.8};
			static bool init = false;
			if (!init) {
				vs_config->getColor("nav", "targeted_unit", col, true);
				init=true;
			}

			DrawTargetCorners(the_x, the_y, insert_size, GFXColor(col[0],col[1],col[2],col[3]));
		}




		if (TestIfInRangeRad(the_x, the_y, insert_size, (-1+float(mousex)/(.5*g_game.x_resolution)), (1+float(-1*mousey)/(.5*g_game.y_resolution))) )
			mouselist.insert(insert_type, insert_size, the_x, the_y, (*blah));
		else
			mainlist.insert(insert_type, insert_size, the_x, the_y, (*blah));



		++blah;

	}

	//	**********************************	//	done enlisting items and attributes









	//	Adjust mouse list for 'n' kliks
	//	**********************************
	//	STANDARD	: (1 3 2) ~ [0] [2] [1]
	//	VS			: (1 2 3) ~ [0] [1] [2]	<-- use this
	if(mouselist.get_n_contents() > 0)	//	mouse is over a target when this is > 0
	{
		if(mouse_wentdown[2]== 1)	//	mouse button went down for mouse button 2(standard)
			rotations += 1;
	}


	if(rotations >= mouselist.get_n_contents())	//	dont rotate more than there is
		rotations = 0;


	int r = 0;
	while(r < rotations)	//	rotate whatver rotations, leaving n rotated items, tail on top
	{
		mouselist.rotate();
		r+=1;
	}
	//	**********************************






	//	Draw the damn shit
	//	**********************************
	mainlist.draw();	//	draw the items
	mainlist.wipe();	//	whipe the list
	//	**********************************
	
	
	
	
	//Check for selection query
	//give back the selected tail IF there is one
	//IF given back, undo the selection state
	//**********************************
	if(1||checkbit(buttonstates, 1))//button #2 is down, wanting a (selection)
	{
		if(mouselist.get_n_contents() > 0)//mouse is over a target when this is > 0
		{
			if(mouse_wentdown[0]== 1)//mouse button went down for mouse button 1
			{
				currentselection = mouselist.gettailunit();
				unsetbit(buttonstates, 1);
				// JUST FOR NOW, target == current selection. later it'll be used for other shit, that will then set target.
				if (currentselection.GetUnit()) {
					( UniverseUtil::getPlayerX( UniverseUtil::getCurrentPlayer() ) )->Target(currentselection.GetUnit());
					( UniverseUtil::getPlayerX( UniverseUtil::getCurrentPlayer() ) )->LockTarget(currentselection.GetUnit());
				}
			}
		}
	}
	//**********************************



	//	Clear the lists
	//	**********************************
	mouselist.draw();	//	draw mouse over'd items
	mouselist.wipe();	//	whipe mouse over'd list
	//	**********************************
}
//	**********************************




