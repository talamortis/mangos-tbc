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
SDName: Zulfarrak
SD%Complete: 100
SDComment:
SDCategory: Zul'Farrak
EndScriptData */

/* ContentData
event_go_zulfarrak_gong
event_spell_unlocking
at_zulfarrak
EndContentData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "zulfarrak.h"

/*######
## event_go_zulfarrak_gong
######*/
class event_go_zulfarrak_gong : public ObjectScript
{
public:
    event_go_zulfarrak_gong() : ObjectScript("event_go_zulfarrak_gong") { }

    bool OnProcessEvent(uint32 /*uiEventId*/, Object* pSource, Object* /*pTarget*/, bool bIsStart) override
    {
        if (bIsStart && pSource->GetTypeId() == TYPEID_PLAYER)
        {
            if (instance_zulfarrak* pInstance = (instance_zulfarrak*)((Player*)pSource)->GetInstanceData())
            {
                if (pInstance->GetData(TYPE_GAHZRILLA) == NOT_STARTED || pInstance->GetData(TYPE_GAHZRILLA) == FAIL)
                {
                    pInstance->SetData(TYPE_GAHZRILLA, IN_PROGRESS);
                    return false;                               // Summon Gahz'rilla by Database Script
                }
                return true;
                // Prevent DB script summoning Gahz'rilla
            }
        }
        return false;
    }



};

/*######
## event_spell_unlocking
######*/
class event_spell_unlocking : public ObjectScript
{
public:
    event_spell_unlocking() : ObjectScript("event_spell_unlocking") { }

    bool OnProcessEvent(uint32 /*uiEventId*/, Object* pSource, Object* /*pTarget*/, bool bIsStart) override
    {
        if (bIsStart && pSource->GetTypeId() == TYPEID_PLAYER)
        {
            if (instance_zulfarrak* pInstance = (instance_zulfarrak*)((Player*)pSource)->GetInstanceData())
            {
                if (pInstance->GetData(TYPE_PYRAMID_EVENT) == NOT_STARTED)
                {
                    pInstance->SetData(TYPE_PYRAMID_EVENT, IN_PROGRESS);
                    return false;                               // Summon pyramid trolls by Database Script
                }
                return true;
            }
        }
        return false;
    }



};

/*######
## at_zulfarrak
######*/
class at_zulfarrak : public AreaTriggerScript
{
public:
    at_zulfarrak() : AreaTriggerScript("at_zulfarrak") { }

    bool OnTrigger(Player* pPlayer, AreaTriggerEntry const* pAt) override
    {
        if (pAt->id == AREATRIGGER_ANTUSUL)
        {
            if (pPlayer->isGameMaster() || pPlayer->IsDead())
                return false;

            instance_zulfarrak* pInstance = (instance_zulfarrak*)pPlayer->GetInstanceData();

            if (!pInstance)
                return false;

            if (pInstance->GetData(TYPE_ANTUSUL) == NOT_STARTED || pInstance->GetData(TYPE_ANTUSUL) == FAIL)
            {
                if (Creature* pAntuSul = pInstance->GetSingleCreatureFromStorage(NPC_ANTUSUL))
                {
                    if (pAntuSul->IsAlive())
                        pAntuSul->AI()->AttackStart(pPlayer);
                }
            }
        }

        return false;
    }



};

void AddSC_zulfarrak()
{
    new event_go_zulfarrak_gong();
    new event_spell_unlocking();
    new at_zulfarrak();

}
