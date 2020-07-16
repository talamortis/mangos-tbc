/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Example_Misc
SD%Complete: 100
SDComment: Item, Areatrigger and other small code examples
SDCategory: Script Examples
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"

enum
{
    SAY_HI  = -1999925
};

extern void LoadDatabase();

class at_example : public AreaTriggerScript
{
public:
    at_example() : AreaTriggerScript("at_example") { }
    bool OnTrigger(Player* pPlayer, AreaTriggerEntry const* atEntry) override
    {
        DoScriptText(SAY_HI, pPlayer);
        return true;
    }
};

class example_item : public ItemScript
{
public:
    example_item() : ItemScript("example_item") { }
    bool OnItemUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets) override
    { 
         LoadDatabase();
         return true;
    }
};

class example_go_teleporter : public GameObjectScript
{
public:
    example_go_teleporter() : GameObjectScript("example_go_teleporter") { }
    bool OnGameObjectUse(Player* pPlayer, GameObject* pGo) override
    { 
        pPlayer->TeleportTo(0, 1807.07f, 336.105f, 70.3975f, 0.0f);
        return false;
    }
};

void AddSC_example_misc()
{
    new at_example();
    new example_item();
    new example_go_teleporter();
}
