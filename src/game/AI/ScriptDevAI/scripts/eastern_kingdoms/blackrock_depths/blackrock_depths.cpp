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
SDName: Blackrock_Depths
SD%Complete: 95
SDComment: Quest support: 4134, 4201, 4322, 7604, 9015.
SDCategory: Blackrock Depths
EndScriptData */

/* ContentData
go_bar_beer_keg
go_shadowforge_brazier
go_relic_coffer_door
at_shadowforge_bridge
at_ring_of_law
npc_grimstone
npc_kharan_mighthammer
npc_phalanx
npc_mistress_nagmara
npc_rocknot
npc_marshal_windsor
npc_dughal_stormwing
npc_tobias_seecher
npc_hurley_blackbreath
boss_doomrel
boss_plugger_spazzring
go_bar_ale_mug
npc_ironhand_guardian
EndContentData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "blackrock_depths.h"
#include "AI/ScriptDevAI/base/escort_ai.h"

/*######
## go_bar_beer_keg
######*/
class go_bar_beer_keg : public GameObjectScript
{
public:
    go_bar_beer_keg() : GameObjectScript("go_bar_beer_keg") { }

    bool OnGameObjectUse(Player* /*pPlayer*/, GameObject* pGo) override
    {
        if (ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData())
        {
            if (pInstance->GetData(TYPE_HURLEY) == IN_PROGRESS || pInstance->GetData(TYPE_HURLEY) == DONE) // GOs despawning on use, this check should never be true but this is proper to have it there
                return false;
                // Every time we set the event to SPECIAL, the instance script increments the number of broken kegs, capping at 3
            pInstance->SetData(TYPE_HURLEY, SPECIAL);
        }
        return false;
    }



};

/*######
## go_shadowforge_brazier
######*/
class go_shadowforge_brazier : public GameObjectScript
{
public:
    go_shadowforge_brazier() : GameObjectScript("go_shadowforge_brazier") { }

    bool OnGameObjectUse(Player* /*pPlayer*/, GameObject* pGo) override
    {
        if (ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData())
        {
            if (pInstance->GetData(TYPE_LYCEUM) == IN_PROGRESS)
                pInstance->SetData(TYPE_LYCEUM, DONE);
            else
                pInstance->SetData(TYPE_LYCEUM, IN_PROGRESS);
        }
        return false;
    }



};

/*######
## go_relic_coffer_door
######*/
class go_relic_coffer_door : public GameObjectScript
{
public:
    go_relic_coffer_door() : GameObjectScript("go_relic_coffer_door") { }

    bool OnGameObjectUse(Player* /*pPlayer*/, GameObject* pGo) override
    {
        if (ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData())
        {
            // check if the event is already done
            if (pInstance->GetData(TYPE_VAULT) != DONE && pInstance->GetData(TYPE_VAULT) != IN_PROGRESS)
                pInstance->SetData(TYPE_VAULT, SPECIAL);
        }

        return false;
    }



};

/*######
## at_shadowforge_bridge
######*/

static const float aGuardSpawnPositions[2][4] =
{
    {642.3660f, -274.5155f, -43.10918f, 0.4712389f},                // First guard spawn position
    {740.1137f, -283.3448f, -42.75082f, 2.8623400f}                 // Second guard spawn position
};

enum
{
    SAY_GUARD_AGGRO                    = -1230043
};

// Two NPCs spawn when AT-1786 is triggeredclass at_shadowforge_bridge : public AreaTriggerScript
{
public:
    at_shadowforge_bridge() : AreaTriggerScript("at_shadowforge_bridge") { }

    bool OnTrigger(Player* pPlayer, AreaTriggerEntry const* /*pAt*/) override
    {
        if (ScriptedInstance* pInstance = (ScriptedInstance*)pPlayer->GetInstanceData())
        {
            if (pPlayer->isGameMaster() || !pPlayer->IsAlive() || pInstance->GetData(TYPE_BRIDGE) == DONE)
                return false;

            Creature* pPyromancer = pInstance->GetSingleCreatureFromStorage(NPC_LOREGRAIN);

            if (!pPyromancer)
                return false;

            if (Creature* pMasterGuard = pPyromancer->SummonCreature(NPC_ANVILRAGE_GUARDMAN, aGuardSpawnPositions[0][0], aGuardSpawnPositions[0][1], aGuardSpawnPositions[0][2], aGuardSpawnPositions[0][3], TEMPSPAWN_DEAD_DESPAWN, 0))
            {
                pMasterGuard->SetWalk(false);
                pMasterGuard->GetMotionMaster()->MoveWaypoint();
                DoDisplayText(pMasterGuard, SAY_GUARD_AGGRO, pPlayer);
                float fX, fY, fZ;
                pPlayer->GetContactPoint(pMasterGuard, fX, fY, fZ);
                pMasterGuard->GetMotionMaster()->MovePoint(1, fX, fY, fZ);

                if (Creature* pSlaveGuard = pPyromancer->SummonCreature(NPC_ANVILRAGE_GUARDMAN, aGuardSpawnPositions[1][0], aGuardSpawnPositions[1][1], aGuardSpawnPositions[1][2], aGuardSpawnPositions[1][3], TEMPSPAWN_DEAD_DESPAWN, 0))
                {
                    pSlaveGuard->GetMotionMaster()->MoveFollow(pMasterGuard, 2.0f, 0);
                }
            }
            pInstance->SetData(TYPE_BRIDGE, DONE);
        }
        return false;
    }



};

/*######
## npc_grimstone
######*/

/* Note about this event:
 * Quest-Event: This needs to be clearified - there is some suggestion, that Theldren&Adds also might come as first wave.
 */









/*######
## npc_marshal_windsor
######*/

enum
{
    // Windsor texts
    SAY_WINDSOR_AGGRO1          = -1230011,
    SAY_WINDSOR_AGGRO2          = -1230012,
    SAY_WINDSOR_AGGRO3          = -1230013,
    SAY_WINDSOR_START           = -1230014,
    SAY_WINDSOR_CELL_DUGHAL_1   = -1230015,
    SAY_WINDSOR_CELL_DUGHAL_3   = -1230016,
    SAY_WINDSOR_EQUIPMENT_1     = -1230017,
    SAY_WINDSOR_EQUIPMENT_2     = -1230018,
    SAY_WINDSOR_EQUIPMENT_3     = -1230019,
    SAY_WINDSOR_EQUIPMENT_4     = -1230020,
    SAY_WINDSOR_CELL_JAZ_1      = -1230021,
    SAY_WINDSOR_CELL_JAZ_2      = -1230022,
    SAY_WINDSOR_CELL_SHILL_1    = -1230023,
    SAY_WINDSOR_CELL_SHILL_2    = -1230024,
    SAY_WINDSOR_CELL_SHILL_3    = -1230025,
    SAY_WINDSOR_CELL_CREST_1    = -1230026,
    SAY_WINDSOR_CELL_CREST_2    = -1230027,
    SAY_WINDSOR_CELL_TOBIAS_1   = -1230028,
    SAY_WINDSOR_CELL_TOBIAS_2   = -1230029,
    SAY_WINDSOR_FREE_1          = -1230030,
    SAY_WINDSOR_FREE_2          = -1230031,

    // Additional gossips
    SAY_DUGHAL_FREE             = -1230010,
    GOSSIP_ID_DUGHAL            = -3230000,
    GOSSIP_TEXT_ID_DUGHAL       = 2846,

    SAY_TOBIAS_FREE_1           = -1230032,
    SAY_TOBIAS_FREE_2           = -1230033,
    GOSSIP_ID_TOBIAS            = -3230001,
    GOSSIP_TEXT_ID_TOBIAS       = 2847,

    NPC_REGINALD_WINDSOR        = 9682,

    QUEST_JAIL_BREAK            = 4322,

    SPELL_WINDSORS_FRENZY       = 15167,
};
class npc_marshal_windsor : public CreatureScript
{
public:
    npc_marshal_windsor() : CreatureScript("npc_marshal_windsor") { }

    bool OnQuestAccept(Player* pPlayer, Creature* pCreature, const Quest* pQuest) override
    {
        if (pQuest->GetQuestId() == QUEST_JAIL_BREAK)
        {
            pCreature->SetFactionTemporary(FACTION_ESCORT_A_NEUTRAL_ACTIVE, TEMPFACTION_RESTORE_RESPAWN);

            if (npc_marshal_windsor::npc_marshal_windsorAI* pEscortAI = dynamic_cast<npc_marshal_windsor::npc_marshal_windsorAI*>(pCreature->AI()))
                pEscortAI->Start(false, pPlayer, pQuest);

            return true;
        }

        return false;
    }



    UnitAI* GetAI(Creature* pCreature)
    {
        return new npc_marshal_windsorAI(pCreature);
    }



    struct npc_marshal_windsorAI : public npc_escortAI
    {
        npc_marshal_windsorAI(Creature* m_creature) : npc_escortAI(m_creature)
        {
            m_pInstance = (ScriptedInstance*)m_creature->GetInstanceData();
            Reset();
        }

        ScriptedInstance* m_pInstance;

        uint8 m_uiEventPhase;

        void Reset() override
        {
            if (!HasEscortState(STATE_ESCORT_ESCORTING))
                m_uiEventPhase = 0;
        }

        void Aggro(Unit* pWho) override
        {
            switch (urand(0, 2))
            {
                case 0: DoScriptText(SAY_WINDSOR_AGGRO1, m_creature, pWho); break;
                case 1: DoScriptText(SAY_WINDSOR_AGGRO2, m_creature); break;
                case 2: DoScriptText(SAY_WINDSOR_AGGRO3, m_creature, pWho); break;
            }
        }

        void WaypointReached(uint32 uiPointId) override
        {
            switch (uiPointId)
            {
                case 1:
                    if (m_pInstance)
                        m_pInstance->SetData(TYPE_QUEST_JAIL_BREAK, IN_PROGRESS);

                    DoScriptText(SAY_WINDSOR_START, m_creature);
                    break;
                case 7:
                    if (Player* pPlayer = GetPlayerForEscort())
                        DoScriptText(SAY_WINDSOR_CELL_DUGHAL_1, m_creature, pPlayer);
                    if (m_pInstance)
                    {
                        if (Creature* pDughal = m_pInstance->GetSingleCreatureFromStorage(NPC_DUGHAL))
                        {
                            pDughal->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            m_creature->SetFacingToObject(pDughal);
                        }
                    }
                    ++m_uiEventPhase;
                    SetEscortPaused(true);
                    break;
                case 9:
                    if (Player* pPlayer = GetPlayerForEscort())
                        DoScriptText(SAY_WINDSOR_CELL_DUGHAL_3, m_creature, pPlayer);
                    break;
                case 14:
                    if (Player* pPlayer = GetPlayerForEscort())
                        DoScriptText(SAY_WINDSOR_EQUIPMENT_1, m_creature, pPlayer);
                    break;
                case 15:
                    // ToDo: fix this emote!
                    // m_creature->HandleEmoteCommand(EMOTE_ONESHOT_USESTANDING);
                    break;
                case 16:
                    if (m_pInstance)
                        m_pInstance->DoUseDoorOrButton(GO_JAIL_DOOR_SUPPLY);
                    break;
                case 18:
                    DoScriptText(SAY_WINDSOR_EQUIPMENT_2, m_creature);
                    break;
                case 19:
                    // ToDo: fix this emote!
                    // m_creature->HandleEmoteCommand(EMOTE_ONESHOT_USESTANDING);
                    break;
                case 20:
                    if (m_pInstance)
                        m_pInstance->DoUseDoorOrButton(GO_JAIL_SUPPLY_CRATE);
                    break;
                case 21:
                    m_creature->UpdateEntry(NPC_REGINALD_WINDSOR);
                    break;
                case 22:
                    if (Player* pPlayer = GetPlayerForEscort())
                    {
                        DoCastSpellIfCan(nullptr, SPELL_WINDSORS_FRENZY, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
                        DoScriptText(SAY_WINDSOR_EQUIPMENT_3, m_creature, pPlayer);
                        m_creature->SetFacingToObject(pPlayer);
                    }
                    break;
                case 23:
                    DoScriptText(SAY_WINDSOR_EQUIPMENT_4, m_creature);
                    if (Player* pPlayer = GetPlayerForEscort())
                        m_creature->SetFacingToObject(pPlayer);
                    break;
                case 30:
                    if (m_pInstance)
                    {
                        if (Creature* pJaz = m_pInstance->GetSingleCreatureFromStorage(NPC_JAZ))
                            m_creature->SetFacingToObject(pJaz);
                    }
                    DoScriptText(SAY_WINDSOR_CELL_JAZ_1, m_creature);
                    ++m_uiEventPhase;
                    SetEscortPaused(true);
                    break;
                case 32:
                    DoScriptText(SAY_WINDSOR_CELL_JAZ_2, m_creature);
                    break;
                case 35:
                    if (m_pInstance)
                    {
                        if (Creature* pShill = m_pInstance->GetSingleCreatureFromStorage(NPC_SHILL))
                            m_creature->SetFacingToObject(pShill);
                    }
                    DoScriptText(SAY_WINDSOR_CELL_SHILL_1, m_creature);
                    ++m_uiEventPhase;
                    SetEscortPaused(true);
                    break;
                case 37:
                    DoScriptText(SAY_WINDSOR_CELL_SHILL_2, m_creature);
                    break;
                case 38:
                    DoScriptText(SAY_WINDSOR_CELL_SHILL_3, m_creature);
                    break;
                case 45:
                    if (m_pInstance)
                    {
                        if (Creature* pCrest = m_pInstance->GetSingleCreatureFromStorage(NPC_CREST))
                            m_creature->SetFacingToObject(pCrest);
                    }
                    DoScriptText(SAY_WINDSOR_CELL_CREST_1, m_creature);
                    ++m_uiEventPhase;
                    SetEscortPaused(true);
                    break;
                case 47:
                    DoScriptText(SAY_WINDSOR_CELL_CREST_2, m_creature);
                    break;
                case 49:
                    DoScriptText(SAY_WINDSOR_CELL_TOBIAS_1, m_creature);
                    if (m_pInstance)
                    {
                        if (Creature* pTobias = m_pInstance->GetSingleCreatureFromStorage(NPC_TOBIAS))
                        {
                            pTobias->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            m_creature->SetFacingToObject(pTobias);
                        }
                    }
                    ++m_uiEventPhase;
                    SetEscortPaused(true);
                    break;
                case 51:
                    if (Player* pPlayer = GetPlayerForEscort())
                        DoScriptText(SAY_WINDSOR_CELL_TOBIAS_2, m_creature, pPlayer);
                    break;
                case 57:
                    DoScriptText(SAY_WINDSOR_FREE_1, m_creature);
                    if (Player* pPlayer = GetPlayerForEscort())
                        m_creature->SetFacingToObject(pPlayer);
                    break;
                case 58:
                    DoScriptText(SAY_WINDSOR_FREE_2, m_creature);
                    if (m_pInstance)
                        m_pInstance->SetData(TYPE_QUEST_JAIL_BREAK, DONE);

                    if (Player* pPlayer = GetPlayerForEscort())
                        pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_JAIL_BREAK, m_creature);
                    break;
            }
        }

        void UpdateEscortAI(const uint32 /*uiDiff*/) override
        {
            // Handle escort resume events
            if (m_pInstance && m_pInstance->GetData(TYPE_QUEST_JAIL_BREAK) == SPECIAL)
            {
                switch (m_uiEventPhase)
                {
                    case 1:                     // Dughal
                    case 3:                     // Ograbisi
                    case 4:                     // Crest
                    case 5:                     // Shill
                    case 6:                     // Tobias
                        SetEscortPaused(false);
                        break;
                    case 2:                     // Jaz
                        ++m_uiEventPhase;
                        break;
                }

                m_pInstance->SetData(TYPE_QUEST_JAIL_BREAK, IN_PROGRESS);
            }

            if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };



};



/*######
## npc_dughal_stormwing
######*/
class npc_dughal_stormwing : public CreatureScript
{
public:
    npc_dughal_stormwing() : CreatureScript("npc_dughal_stormwing") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
        {
            // Set instance data in order to allow the quest to continue
            if (ScriptedInstance* pInstance = (ScriptedInstance*)pCreature->GetInstanceData())
                pInstance->SetData(TYPE_QUEST_JAIL_BREAK, SPECIAL);

            DoScriptText(SAY_DUGHAL_FREE, pCreature, pPlayer);
            pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

            pCreature->SetWalk(false);
            pCreature->GetMotionMaster()->MoveWaypoint();

            pPlayer->CLOSE_GOSSIP_MENU();
        }

        return true;
    }



    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {
        if (pPlayer->GetQuestStatus(QUEST_JAIL_BREAK) == QUEST_STATUS_INCOMPLETE)
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ID_DUGHAL, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_DUGHAL, pCreature->GetObjectGuid());
        return true;
    }



};


/*######
## npc_tobias_seecher
######*/
class npc_tobias_seecher : public CreatureScript
{
public:
    npc_tobias_seecher() : CreatureScript("npc_tobias_seecher") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
        {
            // Set instance data in order to allow the quest to continue
            if (ScriptedInstance* pInstance = (ScriptedInstance*)pCreature->GetInstanceData())
                pInstance->SetData(TYPE_QUEST_JAIL_BREAK, SPECIAL);

            DoScriptText(urand(0, 1) ? SAY_TOBIAS_FREE_1 : SAY_TOBIAS_FREE_2, pCreature);
            pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

            pCreature->SetWalk(false);
            pCreature->GetMotionMaster()->MoveWaypoint();

            pPlayer->CLOSE_GOSSIP_MENU();
        }

        return true;
    }



    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {
        if (pPlayer->GetQuestStatus(QUEST_JAIL_BREAK) == QUEST_STATUS_INCOMPLETE)
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ID_TOBIAS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_TOBIAS, pCreature->GetObjectGuid());
        return true;
    }



};


/*######
## npc_hurley_blackbreath
######*/

enum
{
    YELL_HURLEY_SPAWN      = -1230041,
    SAY_HURLEY_AGGRO       = -1230042,

    // SPELL_DRUNKEN_RAGE      = 14872,
    SPELL_FLAME_BREATH     = 9573,

    NPC_RIBBLY_SCREWSPIGOT = 9543,
    NPC_RIBBLY_CRONY       = 10043,
};
enum
{

    SPELL_DRUNKEN_RAGE = 14872,

};
class npc_hurley_blackbreath : public CreatureScript
{
public:
    npc_hurley_blackbreath() : CreatureScript("npc_hurley_blackbreath") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new npc_hurley_blackbreathAI(pCreature);
    }



    struct npc_hurley_blackbreathAI : public npc_escortAI
    {
        npc_hurley_blackbreathAI(Creature* pCreature) : npc_escortAI(pCreature)
        {
            m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
            Reset();
        }

        ScriptedInstance* m_pInstance;

        uint32 uiFlameBreathTimer;
        uint32 m_uiEventTimer;
        bool   bIsEnraged;

        void Reset() override
        {
            // If reset after an fight, we made him move to the keg room (end of the path)
            if (HasEscortState(STATE_ESCORT_ESCORTING) || HasEscortState(STATE_ESCORT_PAUSED))
            {
                SetCurrentWaypoint(5);
                SetEscortPaused(false);
            }
            else
                m_uiEventTimer  = 1000;

            bIsEnraged          = false;
        }

        // We want to prevent Hurley to go rampage on Ribbly and his friends.
        // Everybody loves Ribbly. Except his family. They want him dead.
        void AttackStart(Unit* pWho) override
        {
            if (pWho && (pWho->GetEntry() == NPC_RIBBLY_SCREWSPIGOT || pWho->GetEntry() == NPC_RIBBLY_CRONY))
                return;
            ScriptedAI::AttackStart(pWho);
        }

        void Aggro(Unit* /*pWho*/) override
        {
            uiFlameBreathTimer  = 7000;
            bIsEnraged  = false;
            DoScriptText(SAY_HURLEY_AGGRO, m_creature);
        }

        void WaypointReached(uint32 uiPointId) override
        {
            if (!m_pInstance)
                return;

            switch (uiPointId)
            {
                case 2:
                    DoScriptText(YELL_HURLEY_SPAWN, m_creature);
                    SetRun(true);
                    break;
                case 6:
                    {
                        SetEscortPaused(true);
                        // Make Hurley and his cronies able to attack players (and be attacked)
                        m_creature->SetImmuneToPlayer(false);
                        CreatureList lCroniesList;
                        GetCreatureListWithEntryInGrid(lCroniesList, m_creature, NPC_BLACKBREATH_CRONY, 30.0f);
                        for (auto& itr : lCroniesList)
                        {
                            if (itr->IsAlive())
                                itr->SetImmuneToPlayer(false);
                        }
                        break;
                    }
            }
        }

        void UpdateEscortAI(const uint32 uiDiff) override
        {
            if (!m_pInstance)
                return;

            // Combat check
            if (m_creature->SelectHostileTarget() && m_creature->GetVictim())
            {
                if (uiFlameBreathTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_FLAME_BREATH) == CAST_OK)
                        uiFlameBreathTimer = 10000;
                }
                else
                    uiFlameBreathTimer -= uiDiff;

                if (m_creature->GetHealthPercent() < 31.0f && !bIsEnraged)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_DRUNKEN_RAGE) == CAST_OK)
                        bIsEnraged = true;
                }

                DoMeleeAttackIfReady();
            }
            else
            {
                if (m_uiEventTimer)
                {
                    if (m_uiEventTimer <= uiDiff)
                    {
                        Start(false);
                        SetEscortPaused(false);
                        m_uiEventTimer = 0;
                    }
                    else
                        m_uiEventTimer -= uiDiff;
                }
            }
        }
    };



};


/*######
## boss_doomrel
######*/

enum
{
    SAY_DOOMREL_START_EVENT     = -1230003,
    GOSSIP_ITEM_CHALLENGE       = -3230002,
    GOSSIP_TEXT_ID_CHALLENGE    = 2601,
};
class boss_doomrel : public CreatureScript
{
public:
    boss_doomrel() : CreatureScript("boss_doomrel") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        switch (uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                pPlayer->CLOSE_GOSSIP_MENU();
                DoScriptText(SAY_DOOMREL_START_EVENT, pCreature);
                // start event
                if (ScriptedInstance* pInstance = (ScriptedInstance*)pCreature->GetInstanceData())
                    pInstance->SetData(TYPE_TOMB_OF_SEVEN, IN_PROGRESS);

                break;
        }
        return true;
    }



    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {
        if (ScriptedInstance* pInstance = (ScriptedInstance*)pCreature->GetInstanceData())
        {
            if (pInstance->GetData(TYPE_TOMB_OF_SEVEN) == NOT_STARTED || pInstance->GetData(TYPE_TOMB_OF_SEVEN) == FAIL)
                pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_CHALLENGE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        }

        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_CHALLENGE, pCreature->GetObjectGuid());
        return true;
    }



};


/*######
## npc_ironhand_guardian
######*/

enum
{
    SPELL_GOUT_OF_FLAME     = 15529,
    SPELL_STONED_VISUAL     = 15533,
};
class npc_ironhand_guardian : public CreatureScript
{
public:
    npc_ironhand_guardian() : CreatureScript("npc_ironhand_guardian") { }

    UnitAI* GetAI(Creature* creature)
    {
        return new npc_ironhand_guardianAI(creature);
    }



    struct npc_ironhand_guardianAI : public ScriptedAI
    {
        npc_ironhand_guardianAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = (ScriptedInstance*)creature->GetInstanceData();
            Reset();
        }

        ScriptedInstance* instance;

        uint32 m_goutOfFlameTimer;
        uint8 m_phase;

        void Reset() override
        {
            m_goutOfFlameTimer = urand(4, 30) * IN_MILLISECONDS;
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!instance)
                return;

            if (instance->GetData(TYPE_IRON_HALL) == NOT_STARTED)
            {
                m_phase = 0;
                return;
            }

            switch (m_phase)
            {
                case 0:
                    m_creature->RemoveAurasDueToSpell(SPELL_STONED);
                    if (DoCastSpellIfCan(m_creature, SPELL_STONED_VISUAL) == CAST_OK)
                        m_phase = 1;
                    break;
                case 1:
                    if (m_goutOfFlameTimer < diff)
                    {
                        if (DoCastSpellIfCan(m_creature, SPELL_GOUT_OF_FLAME) == CAST_OK)
                            m_goutOfFlameTimer = urand(20, 40) * IN_MILLISECONDS;
                    }
                    else
                        m_goutOfFlameTimer -= diff;
                    break;
            }
        }
    };



};


void AddSC_blackrock_depths()
{
    new go_bar_beer_keg();
    new go_shadowforge_brazier();
    new go_relic_coffer_door();
    new at_shadowforge_bridge();


    new npc_marshal_windsor();
    new npc_tobias_seecher();
    new npc_dughal_stormwing();
    new npc_hurley_blackbreath();
    new boss_doomrel();

    new npc_ironhand_guardian();

}
