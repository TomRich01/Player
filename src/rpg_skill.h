#ifndef __skill__
#define __skill__
#include <string>
#include <vector>
#include "rpg_sound.h"

namespace RPG {
class Skill {
    
    public:
        Skill();
        
		int id;
		std::string name;
		int type;
		int sp_cost;
		std::string description;
		
		// type == Normal
		int scope;
		int animation_id;
		std::string using_message1;
		std::string using_message2;
		int failure_message;
		int pdef_f;
		int mdef_f;
		int variance;
		int power;
		int hit;
		bool affect_hp;
		bool affect_sp;
		bool affect_attack;
		bool affect_defense;
		bool affect_spirit;
		bool affect_agility;
		bool absorb_damage;
		bool ignore_defense;
		int condition_size;
		std::vector<char> condition_effects;
		int attribute_size;
		std::vector<char> attribute_effects;
		bool affect_attr_defence;

		// type == Teleport/Escape
		RPG::Sound* sound_effect;
		
		// type == Switch
		int switch_id;
		bool occasion_field;
		bool occasion_battle;
};

}

#endif
