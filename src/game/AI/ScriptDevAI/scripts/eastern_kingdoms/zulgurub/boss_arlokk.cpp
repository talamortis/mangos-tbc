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
SDName: Boss_Arlokk
SD%Complete: 80
SDComment: Vanish spell is replaced by workaround; Timers
SDCategory: Zul'Gurub
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "zulgurub.h"
enum
{

    SPELL_SNEAK = 22766,
};class npc_zulian_prowler : public CreatureScript
{
public:
    npc_zulian_prowler() : CreatureScript("npc_zulian_prowler") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new npc_zulian_prowlerAI(pCreature);
    }



    struct npc_zulian_prowlerAI : public ScriptedAI
    {
        npc_zulian_prowlerAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_bMoveToAid = m_creature->IsTemporarySummon();
            Reset();
        }

        GuidList m_lProwlerGUIDList;

        bool m_bMoveToAid;

        void Reset() override
        {
            DoCastSpellIfCan(m_creature, SPELL_SNEAK);

            // Do only once, and only for those summoned by Arlokk
            if (m_bMoveToAid)
            {
                // Add to GUID list to despawn later
                m_lProwlerGUIDList.push_back(m_creature->GetObjectGuid());

                // Count the number of prowlers alive
                uint32 count = 0;
                for (GuidList::const_iterator itr = m_lProwlerGUIDList.begin(); itr != m_lProwlerGUIDList.end(); ++itr)
                {
                    if (Unit* pProwler = m_creature->GetMap()->GetUnit(*itr))
                        if (pProwler->IsAlive())
                            count++;
                }

                // Check if more than 40 are alive, if so, despawn
                if (count > 40)
                {
                    m_creature->ForcedDespawn();
                    return;
                }

                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MovePoint(1, aArlokkWallShieldPos[0], aArlokkWallShieldPos[1], aArlokkWallShieldPos[2]);

                m_bMoveToAid = false;
            }
        }
    };



};


class go_gong_of_bethekk : public GameObjectScript
{
public:
    go_gong_of_bethekk() : GameObjectScript("go_gong_of_bethekk") { }

    bool OnGameObjectUse(Player* /*pPlayer*/, GameObject* pGo) override
    {
        if (ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData())
        {
            if (pInstance->GetData(TYPE_ARLOKK) == DONE || pInstance->GetData(TYPE_ARLOKK) == IN_PROGRESS)
                return true;

            pInstance->SetData(TYPE_ARLOKK, IN_PROGRESS);
        }

        return false;
    }



};

void AddSC_boss_arlokk()
{
    
    new npc_zulian_prowler();
    new go_gong_of_bethekk();

}
