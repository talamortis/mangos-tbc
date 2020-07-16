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
SDName: instance_zulfarrak
SD%Complete: 80%
SDComment:
SDCategory: Zul'Farrak
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "zulfarrak.h"


class instance_zulfarrak : public InstanceMapScript
{
public:
    instance_zulfarrak() : InstanceMapScript("instance_zulfarrak") { }

    InstanceData* GetInstanceScript(Map* pMap) const override
    {
        return new instance_zulfarrakAI(pMap);
    }

    struct instance_zulfarrakAI : public ScriptedInstance
    {
        instance_zulfarrakAI(Map* pMap) : ScriptedInstance(pMap),
            m_uiPyramidEventTimer(0)
        {
            Initialize();
        }

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        GuidList m_lShallowGravesGuidList;
        GuidList m_lPyramidTrollsGuidList;

        uint32 m_uiPyramidEventTimer;

        void GetShallowGravesGuidList(GuidList& lList) const { lList = m_lShallowGravesGuidList; }

        const char* Save() const override { return m_strInstData.c_str(); }

        void Initialize()
        {
            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
        }

        void OnCreatureCreate(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
            case NPC_ANTUSUL:
            case NPC_SERGEANT_BLY:
                m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
                break;
            case NPC_SANDFURY_SLAVE:
            case NPC_SANDFURY_DRUDGE:
            case NPC_SANDFURY_CRETIN:
            case NPC_SANDFURY_ACOLYTE:
            case NPC_SANDFURY_ZEALOT:
                m_lPyramidTrollsGuidList.push_back(pCreature->GetObjectGuid());
                break;
            }
        }

        void OnObjectCreate(GameObject* pGo)
        {
            if (pGo->GetEntry() == GO_SHALLOW_GRAVE)
                m_lShallowGravesGuidList.push_back(pGo->GetObjectGuid());
            else if (pGo->GetEntry() == GO_END_DOOR)
            {
                if (GetData(TYPE_PYRAMID_EVENT) == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
            }
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            switch (uiType)
            {
            case TYPE_VELRATHA:
            case TYPE_GAHZRILLA:
            case TYPE_ANTUSUL:
            case TYPE_THEKA:
            case TYPE_ZUMRAH:
            case TYPE_CHIEF_SANDSCALP:
                m_auiEncounter[uiType] = uiData;
                break;
            case TYPE_NEKRUM:
                m_auiEncounter[uiType] = uiData;
                if (uiData == DONE && GetData(TYPE_SEZZZIZ) == DONE)
                    SetData(TYPE_PYRAMID_EVENT, DONE);
                break;
            case TYPE_SEZZZIZ:
                m_auiEncounter[uiType] = uiData;
                if (uiData == DONE && GetData(TYPE_NEKRUM) == DONE)
                    SetData(TYPE_PYRAMID_EVENT, DONE);
                break;
            case TYPE_PYRAMID_EVENT:
                m_auiEncounter[uiType] = uiData;
                if (uiData == IN_PROGRESS)
                    m_uiPyramidEventTimer = 20000;
                else if (uiData == DONE)
                    m_uiPyramidEventTimer = 0;
                break;
            default:
                return;
            }

            if (uiData == DONE)
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " " << m_auiEncounter[3]
                    << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " " << m_auiEncounter[6] << " " << m_auiEncounter[7]
                    << " " << m_auiEncounter[8];

                m_strInstData = saveStream.str();

                SaveToDB();
                OUT_SAVE_INST_DATA_COMPLETE;
            }
        }

        void Load(const char* chrIn)
        {
            if (!chrIn)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(chrIn);

            std::istringstream loadStream(chrIn);
            loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
                >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7]
                >> m_auiEncounter[8];

            for (uint32& i : m_auiEncounter)
            {
                if (i == IN_PROGRESS)
                    i = NOT_STARTED;
            }

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        uint32 GetData(uint32 uiType) const
        {
            if (uiType < MAX_ENCOUNTER)
                return m_auiEncounter[uiType];

            return 0;
        }

        void OnCreatureEnterCombat(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
            case NPC_VELRATHA: SetData(TYPE_VELRATHA, IN_PROGRESS); break;
            case NPC_GAHZRILLA: SetData(TYPE_GAHZRILLA, IN_PROGRESS); break;
            case NPC_ANTUSUL: SetData(TYPE_ANTUSUL, IN_PROGRESS); break;
            case NPC_THEKA: SetData(TYPE_THEKA, IN_PROGRESS); break;
            case NPC_ZUMRAH: SetData(TYPE_ZUMRAH, IN_PROGRESS); break;
            case NPC_NEKRUM: SetData(TYPE_NEKRUM, IN_PROGRESS); break;
            case NPC_SEZZZIZ: SetData(TYPE_SEZZZIZ, IN_PROGRESS); break;
            case NPC_CHIEF_SANDSCALP: SetData(TYPE_CHIEF_SANDSCALP, IN_PROGRESS); break;
            }
        }

        void OnCreatureEvade(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
            case NPC_VELRATHA: SetData(TYPE_VELRATHA, FAIL); break;
            case NPC_GAHZRILLA: SetData(TYPE_GAHZRILLA, FAIL); break;
            case NPC_ANTUSUL: SetData(TYPE_ANTUSUL, FAIL); break;
            case NPC_THEKA: SetData(TYPE_THEKA, FAIL); break;
            case NPC_ZUMRAH: SetData(TYPE_ZUMRAH, FAIL); break;
            case NPC_NEKRUM: SetData(TYPE_NEKRUM, FAIL); break;
            case NPC_SEZZZIZ: SetData(TYPE_SEZZZIZ, FAIL); break;
            case NPC_CHIEF_SANDSCALP: SetData(TYPE_CHIEF_SANDSCALP, FAIL); break;
            }
        }

        void OnCreatureDeath(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
            case NPC_VELRATHA: SetData(TYPE_VELRATHA, DONE); break;
            case NPC_GAHZRILLA: SetData(TYPE_GAHZRILLA, DONE); break;
            case NPC_ANTUSUL: SetData(TYPE_ANTUSUL, DONE); break;
            case NPC_THEKA: SetData(TYPE_THEKA, DONE); break;
            case NPC_ZUMRAH: SetData(TYPE_ZUMRAH, DONE); break;
            case NPC_NEKRUM: SetData(TYPE_NEKRUM, DONE); break;
            case NPC_SEZZZIZ: SetData(TYPE_SEZZZIZ, DONE); break;
            case NPC_CHIEF_SANDSCALP: SetData(TYPE_CHIEF_SANDSCALP, DONE); break;
            }
        }

        void Update(uint32 uiDiff)
        {
            if (m_uiPyramidEventTimer)
            {
                if (m_uiPyramidEventTimer <= uiDiff)
                {
                    if (m_lPyramidTrollsGuidList.empty())
                    {
                        m_uiPyramidEventTimer = urand(3000, 10000);
                        return;
                    }

                    GuidList::iterator iter = m_lPyramidTrollsGuidList.begin();
                    advance(iter, urand(0, m_lPyramidTrollsGuidList.size() - 1));

                    // Remove the selected troll
                    ObjectGuid selectedGuid = *iter;
                    m_lPyramidTrollsGuidList.erase(iter);

                    // Move the selected troll to the top of the pyramid. Note: the algorythm may be more complicated than this, but for the moment this will do.
                    if (Creature* pTroll = instance->GetCreature(selectedGuid))
                    {
                        // Pick another one if already in combat or already killed
                        if (pTroll->GetVictim() || !pTroll->IsAlive())
                        {
                            m_uiPyramidEventTimer = urand(0, 2) ? urand(3000, 10000) : 1000;
                            return;
                        }

                        float fX, fY, fZ;
                        if (Creature* pBly = GetSingleCreatureFromStorage(NPC_SERGEANT_BLY))
                        {
                            // ToDo: research if there is anything special if these guys die
                            if (!pBly->IsAlive())
                            {
                                m_uiPyramidEventTimer = 0;
                                return;
                            }

                            pBly->GetRandomPoint(pBly->GetPositionX(), pBly->GetPositionY(), pBly->GetPositionZ(), 4.0f, fX, fY, fZ);
                            pTroll->SetWalk(false);
                            pTroll->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                        }
                    }
                    m_uiPyramidEventTimer = urand(0, 2) ? urand(3000, 10000) : 1000;
                }
                else
                    m_uiPyramidEventTimer -= uiDiff;
            }
        }
    };


};

enum
{
    SAY_INTRO = -1209000,
    SAY_AGGRO = -1209001,
    SAY_KILL = -1209002,
    SAY_SUMMON = -1209003,

    SPELL_SHADOW_BOLT = 12739,
    SPELL_SHADOW_BOLT_VOLLEY = 15245,
    SPELL_WARD_OF_ZUMRAH = 11086,
    SPELL_HEALING_WAVE = 12491,
    SPELL_SUMMON_ZOMBIES = 10247,            // spell should be triggered by missing trap 128972

    // NPC_WARD_OF_ZUMRAH       = 7785,
    // NPC_SKELETON_OF_ZUMRAH   = 7786,
    NPC_ZULFARRAK_ZOMBIE = 7286,             // spawned by the graves
    NPC_ZULFARRAK_DEAD_HERO = 7276,             // spawned by the graves

    FACTION_HOSTILE = 14,
};
class boss_zumrah : public CreatureScript
{
public:
    boss_zumrah() : CreatureScript("boss_zumrah") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new boss_zumrahAI(pCreature);
    }



    struct boss_zumrahAI : public ScriptedAI
    {
        boss_zumrahAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = (instance_zulfarrak::instance_zulfarrakAI*)pCreature->GetInstanceData();
            m_bHasTurnedHostile = false;
            Reset();
        }

        instance_zulfarrak::instance_zulfarrakAI* m_pInstance;

        uint32 m_uiShadowBoltTimer;
        uint32 m_uiShadowBoltVolleyTimer;
        uint32 m_uiWardOfZumrahTimer;
        uint32 m_uHealingWaveTimer;
        uint32 m_uiSpawnZombieTimer;

        bool m_bHasTurnedHostile;

        void Reset() override
        {
            m_uiShadowBoltTimer = 1000;
            m_uiShadowBoltVolleyTimer = urand(6000, 30000);
            m_uiWardOfZumrahTimer = urand(7000, 20000);
            m_uHealingWaveTimer = urand(10000, 15000);
            m_uiSpawnZombieTimer = 1000;

            m_attackDistance = 10.0f;
        }

        void Aggro(Unit* /*pWho*/) override
        {
            DoScriptText(SAY_AGGRO, m_creature);
        }

        void KilledUnit(Unit* /*pVictim*/) override
        {
            DoScriptText(SAY_KILL, m_creature);
        }

        void MoveInLineOfSight(Unit* pWho) override
        {
            if (!m_bHasTurnedHostile && pWho->GetTypeId() == TYPEID_PLAYER && m_creature->IsWithinDistInMap(pWho, 9.0f) && m_creature->IsWithinLOSInMap(pWho))
            {
                m_creature->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_TOGGLE_IMMUNE_TO_PLAYER);
                DoScriptText(SAY_INTRO, m_creature);
                m_bHasTurnedHostile = true;
                AttackStart(pWho);
            }

            ScriptedAI::MoveInLineOfSight(pWho);
        }

        void JustSummoned(Creature* pSummoned) override
        {
            if (pSummoned->GetEntry() == NPC_ZULFARRAK_ZOMBIE || pSummoned->GetEntry() == NPC_ZULFARRAK_DEAD_HERO)
                pSummoned->AI()->AttackStart(m_creature->GetVictim());
        }

        GameObject* SelectNearbyShallowGrave()
        {
            if (!m_pInstance)
                return nullptr;

            // Get the list of usable graves (not used already by players)
            GuidList lTempList;
            GameObjectList lGravesInRange;

            m_pInstance->GetShallowGravesGuidList(lTempList);
            for (GuidList::const_iterator itr = lTempList.begin(); itr != lTempList.end(); ++itr)
            {
                GameObject* pGo = m_creature->GetMap()->GetGameObject(*itr);
                // Go spawned and no looting in process
                if (pGo && pGo->IsSpawned() && pGo->GetLootState() == GO_READY)
                    lGravesInRange.push_back(pGo);
            }

            if (lGravesInRange.empty())
                return nullptr;

            // Sort the graves
            lGravesInRange.sort(ObjectDistanceOrder(m_creature));

            return *lGravesInRange.begin();
        }

        void UpdateAI(const uint32 uiDiff) override
        {
            if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
                return;

            if (m_uiSpawnZombieTimer)
            {
                if (m_uiSpawnZombieTimer <= uiDiff)
                {
                    // Use a nearby grave to spawn zombies
                    if (GameObject* pGrave = SelectNearbyShallowGrave())
                    {
                        m_creature->CastSpell(pGrave->GetPositionX(), pGrave->GetPositionY(), pGrave->GetPositionZ(), SPELL_SUMMON_ZOMBIES, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, pGrave->GetObjectGuid());
                        pGrave->SetLootState(GO_JUST_DEACTIVATED);

                        if (roll_chance_i(30))
                            DoScriptText(SAY_SUMMON, m_creature);

                        m_uiSpawnZombieTimer = 20000;
                    }
                    else                                        // No Grave usable any more
                        m_uiSpawnZombieTimer = 0;
                }
                else
                    m_uiSpawnZombieTimer -= uiDiff;
            }

            if (m_uiShadowBoltTimer < uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_SHADOW_BOLT) == CAST_OK)
                        m_uiShadowBoltTimer = urand(3500, 5000);
                }
            }
            else
                m_uiShadowBoltTimer -= uiDiff;

            if (m_uiShadowBoltVolleyTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SHADOW_BOLT_VOLLEY) == CAST_OK)
                    m_uiShadowBoltVolleyTimer = urand(10000, 18000);
            }
            else
                m_uiShadowBoltVolleyTimer -= uiDiff;

            if (m_uiWardOfZumrahTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_WARD_OF_ZUMRAH) == CAST_OK)
                    m_uiWardOfZumrahTimer = urand(15000, 32000);
            }
            else
                m_uiWardOfZumrahTimer -= uiDiff;

            if (m_uHealingWaveTimer < uiDiff)
            {
                if (Unit* pTarget = DoSelectLowestHpFriendly(40.0f))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_HEALING_WAVE) == CAST_OK)
                        m_uHealingWaveTimer = urand(15000, 23000);
                }
            }
            else
                m_uHealingWaveTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };



};

class event_go_zulfarrak_gong : public ObjectScript
{
public:
    event_go_zulfarrak_gong() : ObjectScript("event_go_zulfarrak_gong") { }

    bool OnProcessEvent(uint32 /*uiEventId*/, Object* pSource, Object* /*pTarget*/, bool bIsStart) override
    {
        if (bIsStart && pSource->GetTypeId() == TYPEID_PLAYER)
        {
            if (instance_zulfarrak::instance_zulfarrakAI* pInstance = (instance_zulfarrak::instance_zulfarrakAI*)((Player*)pSource)->GetInstanceData())
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
            if (instance_zulfarrak::instance_zulfarrakAI* pInstance = (instance_zulfarrak::instance_zulfarrakAI*)((Player*)pSource)->GetInstanceData())
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

            instance_zulfarrak::instance_zulfarrakAI* pInstance = (instance_zulfarrak::instance_zulfarrakAI*)pPlayer->GetInstanceData();

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

void AddSC_instance_zulfarrak()
{
    new instance_zulfarrak();
    new boss_zumrah();
    new event_go_zulfarrak_gong();
    new event_spell_unlocking();
    new at_zulfarrak();

}
