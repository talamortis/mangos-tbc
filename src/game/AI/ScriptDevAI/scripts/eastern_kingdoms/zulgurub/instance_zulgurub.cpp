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
SDName: Instance_ZulGurub
SD%Complete: 80
SDComment: Missing reset function after killing a boss for Ohgan, Thekal.
SDCategory: Zul'Gurub
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "zulgurub.h"

class instance_zulgurub : public InstanceMapScript
{
public:
    instance_zulgurub() : InstanceMapScript("instance_zulgurub") { }

    InstanceData* GetInstanceScript(Map* pMap) const override
    {
        return new instance_zulgurubAI(pMap);
    }

    struct instance_zulgurubAI : public ScriptedInstance
    {
    
        instance_zulgurubAI(Map* pMap) : ScriptedInstance(pMap),
            m_bHasIntroYelled(false),
            m_bHasAltarYelled(false)
        {
            Initialize();
        }


        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        GuidList m_lRightPantherTriggerGUIDList;
        GuidList m_lLeftPantherTriggerGUIDList;
        GuidList m_lSpiderEggGUIDList;
        GuidList m_lProwlerGUIDList;

        bool m_bHasIntroYelled;
        bool m_bHasAltarYelled;

        const char* Save() const override { return m_strInstData.c_str(); }

        void Initialize()
        {
            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
        }

        void DoYellAtTriggerIfCan(uint32 uiTriggerId)
        {
            if (uiTriggerId == AREATRIGGER_ENTER && !m_bHasIntroYelled)
            {
                DoOrSimulateScriptTextForThisInstance(SAY_HAKKAR_PROTECT, NPC_HAKKAR);
                m_bHasIntroYelled = true;
            }
            else if (uiTriggerId == AREATRIGGER_ALTAR && !m_bHasAltarYelled)
            {
                DoOrSimulateScriptTextForThisInstance(SAY_MINION_DESTROY, NPC_HAKKAR);
                m_bHasAltarYelled = true;
            }
        }

        void OnCreatureCreate(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
            case NPC_LORKHAN:
            case NPC_ZATH:
            case NPC_THEKAL:
            case NPC_JINDO:
            case NPC_HAKKAR:
            case NPC_BLOODLORD_MANDOKIR:
            case NPC_MARLI:
                m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
                break;
            case NPC_PANTHER_TRIGGER:
                if (pCreature->GetPositionY() < -1626)
                    m_lLeftPantherTriggerGUIDList.push_back(pCreature->GetObjectGuid());
                else
                    m_lRightPantherTriggerGUIDList.push_back(pCreature->GetObjectGuid());
                break;
            }
        }

        void OnObjectCreate(GameObject* pGo)
        {
            switch (pGo->GetEntry())
            {
            case GO_GONG_OF_BETHEKK:
            case GO_FORCEFIELD:
                break;
            case GO_SPIDER_EGG:
                m_lSpiderEggGUIDList.push_back(pGo->GetObjectGuid());
                return;
            }

            m_goEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            switch (uiType)
            {
            case TYPE_JEKLIK:
            case TYPE_VENOXIS:
            case TYPE_THEKAL:
                m_auiEncounter[uiType] = uiData;
                if (uiData == DONE)
                    DoLowerHakkarHitPoints();
                break;
            case TYPE_MARLI:
                m_auiEncounter[uiType] = uiData;
                if (uiData == DONE)
                    DoLowerHakkarHitPoints();
                if (uiData == FAIL)
                {
                    for (GuidList::const_iterator itr = m_lSpiderEggGUIDList.begin(); itr != m_lSpiderEggGUIDList.end(); ++itr)
                    {
                        if (GameObject* pEgg = instance->GetGameObject(*itr))
                        {
                            // Note: this type of Gameobject needs to be respawned manually
                            pEgg->SetRespawnTime(2 * DAY);
                            pEgg->Respawn();
                        }
                    }
                }
                break;
            case TYPE_ARLOKK:
                m_auiEncounter[uiType] = uiData;
                if (uiData == IN_PROGRESS)
                    DoUseDoorOrButton(GO_FORCEFIELD);
                else if (GameObject* pForcefield = GetSingleGameObjectFromStorage(GO_FORCEFIELD))
                    pForcefield->ResetDoorOrButton();
                if (uiData == DONE)
                    DoLowerHakkarHitPoints();
                if (uiData == FAIL)
                {
                    // Note: this gameobject should change flags - currently it despawns which isn't correct
                    if (GameObject* pGong = GetSingleGameObjectFromStorage(GO_GONG_OF_BETHEKK))
                    {
                        pGong->SetRespawnTime(2 * DAY);
                        pGong->Respawn();
                    }
                }
                break;
            case TYPE_OHGAN:
                // Note: SPECIAL instance data is set via ACID!
                if (uiData == SPECIAL)
                {
                    if (Creature* pMandokir = GetSingleCreatureFromStorage(NPC_BLOODLORD_MANDOKIR))
                    {
                        pMandokir->SetWalk(false);
                        pMandokir->GetMotionMaster()->MovePoint(1, aMandokirDownstairsPos[0], aMandokirDownstairsPos[1], aMandokirDownstairsPos[2]);
                    }
                }
                m_auiEncounter[uiType] = uiData;
                break;
            case TYPE_LORKHAN:
            case TYPE_ZATH:
                m_auiEncounter[uiType] = uiData;
                break;
            }

            if (uiData == DONE)
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                    << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                    << m_auiEncounter[6] << " " << m_auiEncounter[7];

                m_strInstData = saveStream.str();

                SaveToDB();
                OUT_SAVE_INST_DATA_COMPLETE;
            }
        }

        // Each time High Priest dies lower Hakkar's HP
        void DoLowerHakkarHitPoints()
        {
            if (Creature* pHakkar = GetSingleCreatureFromStorage(NPC_HAKKAR))
            {
                if (pHakkar->IsAlive() && pHakkar->GetMaxHealth() > HP_LOSS_PER_PRIEST)
                {
                    pHakkar->SetMaxHealth(pHakkar->GetMaxHealth() - HP_LOSS_PER_PRIEST);
                    pHakkar->SetHealth(pHakkar->GetHealth() - HP_LOSS_PER_PRIEST);
                }
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
                >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7];

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

        Creature* SelectRandomPantherTrigger(bool bIsLeft)
        {
            GuidList* plTempList = bIsLeft ? &m_lLeftPantherTriggerGUIDList : &m_lRightPantherTriggerGUIDList;
            std::vector<Creature*> vTriggers;
            vTriggers.reserve(plTempList->size());

            for (GuidList::const_iterator itr = plTempList->begin(); itr != plTempList->end(); ++itr)
            {
                if (Creature* pTemp = instance->GetCreature(*itr))
                    vTriggers.push_back(pTemp);
            }

            if (vTriggers.empty())
                return nullptr;

            return vTriggers[urand(0, vTriggers.size() - 1)];
        }

    };


};
class at_zulgurub : public AreaTriggerScript
{
public:
    at_zulgurub() : AreaTriggerScript("at_zulgurub") { }

    bool OnTrigger(Player* pPlayer, AreaTriggerEntry const* pAt) override
    {
        if (pAt->id == AREATRIGGER_ENTER || pAt->id == AREATRIGGER_ALTAR)
        {
            if (pPlayer->isGameMaster() || pPlayer->IsDead())
                return false;

            if (instance_zulgurub::instance_zulgurubAI* pInstance = (instance_zulgurub::instance_zulgurubAI*)pPlayer->GetInstanceData())
                pInstance->DoYellAtTriggerIfCan(pAt->id);
        }

        return false;
    }



};


/* ContentData
boss_arlokk
npc_zulian_prowler
go_gong_of_bethekk
EndContentData */


enum
{
    SAY_AGGRO = -1309011,
    SAY_FEAST_PANTHER = -1309012,
    SAY_DEATH = -1309013,

    SPELL_SHADOW_WORD_PAIN = 23952,
    SPELL_GOUGE = 24698,
    SPELL_MARK_ARLOKK = 24210,
    SPELL_RAVAGE = 24213,
    SPELL_TRASH = 3391,
    SPELL_WHIRLWIND = 24236,
    SPELL_PANTHER_TRANSFORM = 24190,
    SPELL_SUMMON_ZULIAN_PROWLERS = 24247,

    SPELL_SNEAK = 22766,
};class boss_arlokk : public CreatureScript
{
public:
    boss_arlokk() : CreatureScript("boss_arlokk") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new boss_arlokkAI(pCreature);
    }



    struct boss_arlokkAI : public ScriptedAI
    {
        boss_arlokkAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = (instance_zulgurub::instance_zulgurubAI*)pCreature->GetInstanceData();
            Reset();
        }

        instance_zulgurub::instance_zulgurubAI* m_pInstance;

        uint32 m_uiShadowWordPainTimer;
        uint32 m_uiGougeTimer;
        uint32 m_uiMarkTimer;
        uint32 m_uiRavageTimer;
        uint32 m_uiTrashTimer;
        uint32 m_uiWhirlwindTimer;
        uint32 m_uiVanishTimer;
        uint32 m_uiVisibleTimer;
        uint32 m_uiTransformTimer;
        uint32 m_uiSummonTimer;
        Creature* m_pTrigger1;
        Creature* m_pTrigger2;

        GuidList m_lProwlerGUIDList;

        bool m_bIsPhaseTwo;

        void Reset() override
        {
            m_uiShadowWordPainTimer = 8000;
            m_uiGougeTimer = 14000;
            m_uiMarkTimer = 5000;
            m_uiRavageTimer = 12000;
            m_uiTrashTimer = 20000;
            m_uiWhirlwindTimer = 15000;
            m_uiTransformTimer = 30000;
            m_uiVanishTimer = 5000;
            m_uiVisibleTimer = 0;

            m_bIsPhaseTwo = false;

            // Restore visibility
            if (m_creature->GetVisibility() != VISIBILITY_ON)
                m_creature->SetVisibility(VISIBILITY_ON);
        }

        void Aggro(Unit* /*pWho*/) override
        {
            DoScriptText(SAY_AGGRO, m_creature);

            m_pTrigger1 = m_pInstance->SelectRandomPantherTrigger(true);
            if (m_pTrigger1)
                m_pTrigger1->CastSpell(m_pTrigger1, SPELL_SUMMON_ZULIAN_PROWLERS, TRIGGERED_NONE);

            m_pTrigger2 = m_pInstance->SelectRandomPantherTrigger(false);
            if (m_pTrigger2)
                m_pTrigger2->CastSpell(m_pTrigger2, SPELL_SUMMON_ZULIAN_PROWLERS, TRIGGERED_NONE);
        }

        void JustReachedHome() override
        {
            if (m_pInstance)
                m_pInstance->SetData(TYPE_ARLOKK, FAIL);

            // we should be summoned, so despawn
            m_creature->ForcedDespawn();

            DoStopZulianProwlers();
        }

        void JustDied(Unit* /*pKiller*/) override
        {
            DoScriptText(SAY_DEATH, m_creature);
            // Restore visibility in case of killed by dots
            if (m_creature->GetVisibility() != VISIBILITY_ON)
                m_creature->SetVisibility(VISIBILITY_ON);

            DoStopZulianProwlers();

            if (m_pInstance)
                m_pInstance->SetData(TYPE_ARLOKK, DONE);
        }

        // Wrapper to despawn the zulian panthers on evade / death
        void DoStopZulianProwlers()
        {
            if (m_pInstance)
            {
                // Stop summoning Zulian prowlers
                if (m_pTrigger1)
                    m_pTrigger1->RemoveAurasDueToSpell(SPELL_SUMMON_ZULIAN_PROWLERS);
                if (m_pTrigger2)
                    m_pTrigger2->RemoveAurasDueToSpell(SPELL_SUMMON_ZULIAN_PROWLERS);
            }
        }

        void UpdateAI(const uint32 uiDiff) override
        {
            if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
                return;

            if (m_uiVisibleTimer)
            {
                if (m_uiVisibleTimer <= uiDiff)
                {
                    // Restore visibility
                    m_creature->SetVisibility(VISIBILITY_ON);

                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                        AttackStart(pTarget);

                    m_uiVisibleTimer = 0;
                }
                else
                    m_uiVisibleTimer -= uiDiff;

                // Do nothing while vanished
                return;
            }

            // Troll phase
            if (!m_bIsPhaseTwo)
            {
                if (m_uiShadowWordPainTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    {
                        if (DoCastSpellIfCan(pTarget, SPELL_SHADOW_WORD_PAIN) == CAST_OK)
                            m_uiShadowWordPainTimer = 15000;
                    }
                }
                else
                    m_uiShadowWordPainTimer -= uiDiff;

                if (m_uiMarkTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MARK_ARLOKK, SELECT_FLAG_PLAYER))
                    {
                        if (DoCastSpellIfCan(pTarget, SPELL_MARK_ARLOKK) == CAST_OK)
                        {
                            DoScriptText(SAY_FEAST_PANTHER, m_creature, pTarget);
                            m_uiMarkTimer = 30000;
                        }
                    }
                }
                else
                    m_uiMarkTimer -= uiDiff;

                if (m_uiGougeTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_GOUGE) == CAST_OK)
                    {
                        if (m_creature->getThreatManager().getThreat(m_creature->GetVictim()))
                            m_creature->getThreatManager().modifyThreatPercent(m_creature->GetVictim(), -80);

                        m_uiGougeTimer = urand(17000, 27000);
                    }
                }
                else
                    m_uiGougeTimer -= uiDiff;

                // Transform to Panther
                if (m_uiTransformTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_PANTHER_TRANSFORM) == CAST_OK)
                    {
                        m_uiTransformTimer = 80000;
                        m_bIsPhaseTwo = true;
                    }
                }
                else
                    m_uiTransformTimer -= uiDiff;
            }
            // Panther phase
            else
            {
                if (m_uiRavageTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_RAVAGE) == CAST_OK)
                        m_uiRavageTimer = urand(10000, 15000);
                }
                else
                    m_uiRavageTimer -= uiDiff;

                if (m_uiTrashTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_TRASH) == CAST_OK)
                        m_uiTrashTimer = urand(13000, 15000);
                }
                else
                    m_uiTrashTimer -= uiDiff;

                if (m_uiWhirlwindTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_WHIRLWIND) == CAST_OK)
                        m_uiWhirlwindTimer = 15000;
                }
                else
                    m_uiWhirlwindTimer -= uiDiff;

                if (m_uiVanishTimer < uiDiff)
                {
                    // Note: this is a workaround because we do not know the real vanish spell
                    m_creature->SetVisibility(VISIBILITY_OFF);
                    DoResetThreat();

                    m_uiVanishTimer = 85000;
                    m_uiVisibleTimer = 45000;
                }
                else
                    m_uiVanishTimer -= uiDiff;

                // Transform back
                if (m_uiTransformTimer < uiDiff)
                {
                    m_creature->RemoveAurasDueToSpell(SPELL_PANTHER_TRANSFORM);
                    m_uiTransformTimer = 30000;
                    m_bIsPhaseTwo = false;
                }
                else
                    m_uiTransformTimer -= uiDiff;
            }

            DoMeleeAttackIfReady();
        }
    };



};


void AddSC_instance_zulgurub()
{
    new instance_zulgurub();
    new at_zulgurub();
    new boss_arlokk();

}
