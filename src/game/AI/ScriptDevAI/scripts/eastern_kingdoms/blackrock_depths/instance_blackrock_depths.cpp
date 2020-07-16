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
SDName: Instance_Blackrock_Depths
SD%Complete: 80
SDComment:
SDCategory: Blackrock Depths
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "blackrock_depths.h"
#include "AI/ScriptDevAI/base/escort_ai.h"class instance_blackrock_depths : public InstanceMapScript
{
public:
    instance_blackrock_depths() : InstanceMapScript("instance_blackrock_depths") { }

    InstanceData* GetInstanceScript(Map* pMap) const override
    {
        return new instance_blackrock_depthsAI(pMap);
    }
    struct instance_blackrock_depthsAI : public ScriptedInstance
    {
        instance_blackrock_depthsAI(Map* pMap) : ScriptedInstance(pMap),
            m_bIsBarDoorOpen(false),
            m_uiBarAleCount(0),
            m_uiPatronEmoteTimer(2000),
            m_uiBrokenKegs(0),
            m_uiPatrolTimer(0),
            m_uiStolenAles(0),
            m_uiDagranTimer(0),
            m_uiCofferDoorsOpened(0),
            m_uiDwarfRound(0),

            m_uiDwarfFightTimer(0),
            m_fArenaCenterX(0.0f),
            m_fArenaCenterY(0.0f),
            m_fArenaCenterZ(0.0f)
        {
            Initialize();
        }

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        // Grim Guzzler bar events
        bool  m_bIsBarDoorOpen;
        uint32 m_uiBarAleCount;
        uint32 m_uiPatronEmoteTimer;
        uint8 m_uiBrokenKegs;
        uint32 m_uiPatrolTimer;
        uint8 m_uiStolenAles;
        uint32 m_uiDagranTimer;

        uint8 m_uiCofferDoorsOpened;

        uint8 m_uiDwarfRound;
        uint32 m_uiDwarfFightTimer;

        float m_fArenaCenterX, m_fArenaCenterY, m_fArenaCenterZ;

        GuidSet m_sVaultNpcGuids;
        GuidSet m_sArenaCrowdNpcGuids;
        GuidSet m_sBarPatronNpcGuids;
        GuidSet m_sBarPatrolGuids;

        // Arena Event
        void SetArenaCenterCoords(float fX, float fY, float fZ) { m_fArenaCenterX = fX; m_fArenaCenterY = fY; m_fArenaCenterZ = fZ; }
        void GetArenaCenterCoords(float& fX, float& fY, float& fZ) const
        {
            fX = m_fArenaCenterX; fY = m_fArenaCenterY; fZ = m_fArenaCenterZ;
        }
        void GetArenaCrowdGuid(GuidSet& sCrowdSet) const { sCrowdSet = m_sArenaCrowdNpcGuids; }

        // Bar events
        void SetBarDoorIsOpen() { m_bIsBarDoorOpen = true; }
        void GetBarDoorIsOpen(bool& bIsOpen) const { bIsOpen = m_bIsBarDoorOpen; }
        const char* Save() const override { return m_strInstData.c_str(); }

        void Initialize()
        {
            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
        }

        void OnCreatureCreate(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
            case NPC_PRINCESS:
                // replace the princess if required
                if (CanReplacePrincess())
                    pCreature->UpdateEntry(NPC_PRIESTESS);
                // no break;
            case NPC_EMPEROR:
            case NPC_MAGMUS:
            case NPC_PHALANX:
            case NPC_PLUGGER_SPAZZRING:
            case NPC_HATEREL:
            case NPC_ANGERREL:
            case NPC_VILEREL:
            case NPC_GLOOMREL:
            case NPC_SEETHREL:
            case NPC_DOOMREL:
            case NPC_DOPEREL:
            case NPC_SHILL:
            case NPC_CREST:
            case NPC_JAZ:
            case NPC_TOBIAS:
            case NPC_DUGHAL:
            case NPC_LOREGRAIN:
            case NPC_RIBBLY_SCREWSPIGGOT:
                m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
                break;
            case NPC_WARBRINGER_CONST:
                // Golems not in the Relict Vault?
                if (std::abs(pCreature->GetPositionZ() - aVaultPositions[2]) > 1.0f || !pCreature->IsWithinDist2d(aVaultPositions[0], aVaultPositions[1], 20.0f))
                    break;
                // Golems in Relict Vault need to have a stoned aura, set manually to prevent reapply when reached home
                pCreature->CastSpell(pCreature, SPELL_STONED, TRIGGERED_OLD_TRIGGERED);
                // Store the Relict Vault Golems into m_sVaultNpcGuids
            case NPC_WATCHER_DOOMGRIP:
                m_sVaultNpcGuids.insert(pCreature->GetObjectGuid());
                break;
                // Arena crowd
            case NPC_ARENA_SPECTATOR:
            case NPC_SHADOWFORGE_PEASANT:
            case NPC_SHADOWFORGE_CITIZEN:
            case NPC_SHADOWFORGE_SENATOR:
            case NPC_ANVILRAGE_SOLDIER:
            case NPC_ANVILRAGE_MEDIC:
            case NPC_ANVILRAGE_OFFICER:
                if (pCreature->GetPositionZ() < aArenaCrowdVolume.m_fCenterZ || pCreature->GetPositionZ() > aArenaCrowdVolume.m_fCenterZ + aArenaCrowdVolume.m_uiHeight ||
                    !pCreature->IsWithinDist2d(aArenaCrowdVolume.m_fCenterX, aArenaCrowdVolume.m_fCenterY, aArenaCrowdVolume.m_uiRadius))
                    break;
                m_sArenaCrowdNpcGuids.insert(pCreature->GetObjectGuid());
                if (m_auiEncounter[0] == DONE)
                    pCreature->SetFactionTemporary(FACTION_ARENA_NEUTRAL, TEMPFACTION_RESTORE_RESPAWN);
                break;
                // Grim Guzzler bar crowd
            case NPC_GRIM_PATRON:
            case NPC_GUZZLING_PATRON:
            case NPC_HAMMERED_PATRON:
                m_sBarPatronNpcGuids.insert(pCreature->GetObjectGuid());
                if (m_auiEncounter[11] == DONE)
                {
                    pCreature->SetFactionTemporary(FACTION_DARK_IRON, TEMPFACTION_RESTORE_RESPAWN);
                    pCreature->SetStandState(UNIT_STAND_STATE_STAND);
                }
                break;
            case NPC_PRIVATE_ROCKNOT:
            case NPC_MISTRESS_NAGMARA:
                if (m_auiEncounter[11] == DONE)
                    pCreature->ForcedDespawn();
                else
                    m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
                break;
            }
        }

        void OnObjectCreate(GameObject* pGo)
        {
            switch (pGo->GetEntry())
            {
            case GO_ARENA_1:
            case GO_ARENA_2:
            case GO_ARENA_3:
            case GO_ARENA_4:
            case GO_SHADOW_LOCK:
            case GO_SHADOW_MECHANISM:
            case GO_SHADOW_GIANT_DOOR:
            case GO_SHADOW_DUMMY:
            case GO_BAR_KEG_SHOT:
            case GO_BAR_KEG_TRAP:
            case GO_TOMB_ENTER:
            case GO_TOMB_EXIT:
            case GO_LYCEUM:
            case GO_GOLEM_ROOM_N:
            case GO_GOLEM_ROOM_S:
            case GO_THRONE_ROOM:
            case GO_SPECTRAL_CHALICE:
            case GO_CHEST_SEVEN:
            case GO_ARENA_SPOILS:
            case GO_SECRET_DOOR:
            case GO_SECRET_SAFE:
            case GO_JAIL_DOOR_SUPPLY:
            case GO_JAIL_SUPPLY_CRATE:
            case GO_DWARFRUNE_A01:
            case GO_DWARFRUNE_B01:
            case GO_DWARFRUNE_C01:
            case GO_DWARFRUNE_D01:
            case GO_DWARFRUNE_E01:
            case GO_DWARFRUNE_F01:
            case GO_DWARFRUNE_G01:
                break;
            case GO_BAR_DOOR:
                if (GetData(TYPE_ROCKNOT) == DONE)
                {
                    // Rocknot event done: set the Grim Guzzler door animation to "broken"
                    // tell the instance script it is open to prevent some of the other events
                    pGo->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
                    SetBarDoorIsOpen();
                }
                else if (m_auiEncounter[10] == DONE || m_auiEncounter[11] == DONE)
                {
                    // bar or Plugger event done: open the Grim Guzzler door
                    // tell the instance script it is open to prevent some of the other events
                    DoUseDoorOrButton(GO_BAR_DOOR);
                    SetBarDoorIsOpen();
                }
                break;

            default:
                return;
            }
            m_goEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            switch (uiType)
            {
            case TYPE_RING_OF_LAW:
                // If finished the arena event after theldren fight
                if (uiData == DONE && m_auiEncounter[0] == SPECIAL)
                    DoRespawnGameObject(GO_ARENA_SPOILS, HOUR);
                else if (uiData == DONE)
                {
                    for (auto m_sArenaCrowdNpcGuid : m_sArenaCrowdNpcGuids)
                    {
                        if (Creature* pSpectator = instance->GetCreature(m_sArenaCrowdNpcGuid))
                            pSpectator->SetFactionTemporary(FACTION_ARENA_NEUTRAL, TEMPFACTION_RESTORE_RESPAWN);
                    }
                }
                m_auiEncounter[0] = uiData;
                break;
            case TYPE_VAULT:
                if (uiData == SPECIAL)
                {
                    ++m_uiCofferDoorsOpened;

                    if (m_uiCofferDoorsOpened == MAX_RELIC_DOORS)
                    {
                        SetData(TYPE_VAULT, IN_PROGRESS);

                        Creature* pConstruct = nullptr;

                        // Activate vault constructs
                        for (auto m_sVaultNpcGuid : m_sVaultNpcGuids)
                        {
                            pConstruct = instance->GetCreature(m_sVaultNpcGuid);
                            if (pConstruct)
                                pConstruct->RemoveAurasDueToSpell(SPELL_STONED);
                        }

                        if (!pConstruct)
                            return;

                        // Summon doomgrip
                        pConstruct->SummonCreature(NPC_WATCHER_DOOMGRIP, aVaultPositions[0], aVaultPositions[1], aVaultPositions[2], aVaultPositions[3], TEMPSPAWN_DEAD_DESPAWN, 0);
                    }
                    // No need to store in this case
                    return;
                }
                if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_SECRET_DOOR);
                    DoToggleGameObjectFlags(GO_SECRET_SAFE, GO_FLAG_NO_INTERACT, false);
                }
                m_auiEncounter[1] = uiData;
                break;
            case TYPE_ROCKNOT:
                if (uiData == SPECIAL)
                    ++m_uiBarAleCount;
                else
                {
                    if (uiData == DONE)
                    {
                        HandleBarPatrons(PATRON_PISSED);
                        SetBarDoorIsOpen();
                    }
                    m_auiEncounter[2] = uiData;
                }
                break;
            case TYPE_TOMB_OF_SEVEN:
                // Don't set the same data twice
                if (uiData == m_auiEncounter[3])
                    break;
                // Combat door
                DoUseDoorOrButton(GO_TOMB_ENTER);
                // Start the event
                if (uiData == IN_PROGRESS)
                    DoCallNextDwarf();
                if (uiData == FAIL)
                {
                    // Reset dwarfes
                    for (unsigned int aTombDwarfe : aTombDwarfes)
                    {
                        if (Creature* pDwarf = GetSingleCreatureFromStorage(aTombDwarfe))
                        {
                            if (!pDwarf->IsAlive())
                                pDwarf->Respawn();
                        }
                    }

                    m_uiDwarfRound = 0;
                    m_uiDwarfFightTimer = 0;
                }
                if (uiData == DONE)
                {
                    DoRespawnGameObject(GO_CHEST_SEVEN, HOUR);
                    DoUseDoorOrButton(GO_TOMB_EXIT);
                }
                m_auiEncounter[3] = uiData;
                break;
            case TYPE_LYCEUM:
                if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_GOLEM_ROOM_N);
                    DoUseDoorOrButton(GO_GOLEM_ROOM_S);
                    if (Creature* magmus = GetSingleCreatureFromStorage(NPC_MAGMUS))
                        DoScriptText(YELL_MAGMUS_INTRO, magmus);
                }
                m_auiEncounter[4] = uiData;
                break;
            case TYPE_IRON_HALL:
                switch (uiData)
                {
                case IN_PROGRESS:
                    DoUseDoorOrButton(GO_GOLEM_ROOM_N);
                    DoUseDoorOrButton(GO_GOLEM_ROOM_S);
                    break;
                case FAIL:
                    DoUseDoorOrButton(GO_GOLEM_ROOM_N);
                    DoUseDoorOrButton(GO_GOLEM_ROOM_S);
                    break;
                case DONE:
                    DoUseDoorOrButton(GO_GOLEM_ROOM_N);
                    DoUseDoorOrButton(GO_GOLEM_ROOM_S);
                    DoUseDoorOrButton(GO_THRONE_ROOM);
                    break;
                }
                m_auiEncounter[5] = uiData;
                break;
            case TYPE_QUEST_JAIL_BREAK:
                m_auiEncounter[6] = uiData;
                return;
            case TYPE_FLAMELASH:
                for (int i = 0; i < MAX_DWARF_RUNES; ++i)
                    DoUseDoorOrButton(GO_DWARFRUNE_A01 + i);
                return;
            case TYPE_HURLEY:
                if (uiData == SPECIAL)
                {
                    ++m_uiBrokenKegs;

                    if (m_uiBrokenKegs == 3)
                    {
                        if (Creature* pPlugger = GetSingleCreatureFromStorage(NPC_PLUGGER_SPAZZRING))
                        {
                            // Summon Hurley Blackbreath
                            Creature* pHurley = pPlugger->SummonCreature(NPC_HURLEY_BLACKBREATH, aHurleyPositions[0], aHurleyPositions[1], aHurleyPositions[2], aHurleyPositions[3], TEMPSPAWN_DEAD_DESPAWN, 0);

                            if (!pHurley)
                                return;

                            // Summon cronies around Hurley
                            for (uint8 i = 0; i < MAX_CRONIES; ++i)
                            {
                                float fX, fY, fZ;
                                pPlugger->GetRandomPoint(aHurleyPositions[0], aHurleyPositions[1], aHurleyPositions[2], 2.0f, fX, fY, fZ);
                                if (Creature* pSummoned = pPlugger->SummonCreature(NPC_BLACKBREATH_CRONY, fX, fY, fZ, aHurleyPositions[3], TEMPSPAWN_DEAD_DESPAWN, 0))
                                {
                                    pSummoned->SetWalk(false);
                                    // The cronies should not engage anyone until their boss does so
                                    // the linking is done by DB
                                    pSummoned->SetImmuneToNPC(true);
                                    pSummoned->SetImmuneToPlayer(true);
                                    // The movement toward the kegs is handled by Hurley EscortAI
                                    // and we want the cronies to follow him there
                                    pSummoned->GetMotionMaster()->MoveFollow(pHurley, 1.0f, 0);
                                }
                            }
                            SetData(TYPE_HURLEY, IN_PROGRESS);
                        }
                    }
                }
                else
                    m_auiEncounter[8] = uiData;
                break;
            case TYPE_BRIDGE:
                m_auiEncounter[9] = uiData;
                return;
            case TYPE_BAR:
                if (uiData == IN_PROGRESS)
                    HandleBarPatrol(0);
                m_auiEncounter[10] = uiData;
                break;
            case TYPE_PLUGGER:
                if (uiData == SPECIAL)
                {
                    if (GetSingleCreatureFromStorage(NPC_PLUGGER_SPAZZRING))
                    {
                        ++m_uiStolenAles;
                        if (m_uiStolenAles == 3)
                            uiData = IN_PROGRESS;
                    }
                }
                m_auiEncounter[11] = uiData;
                break;
            case TYPE_NAGMARA:
                m_auiEncounter[12] = uiData;
                break;
            }

            if (uiData == DONE)
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                    << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                    << m_auiEncounter[6] << " " << m_auiEncounter[7] << " " << m_auiEncounter[8] << " "
                    << m_auiEncounter[9] << " " << m_auiEncounter[10] << " " << m_auiEncounter[11] << " "
                    << m_auiEncounter[12];

                m_strInstData = saveStream.str();

                SaveToDB();
                OUT_SAVE_INST_DATA_COMPLETE;
            }
        }

        uint32 GetData(uint32 uiType) const
        {
            switch (uiType)
            {
            case TYPE_RING_OF_LAW:
                return m_auiEncounter[0];
            case TYPE_VAULT:
                return m_auiEncounter[1];
            case TYPE_ROCKNOT:
            {
                if (m_auiEncounter[2] == IN_PROGRESS && m_uiBarAleCount == 3)
                    return SPECIAL;
                return m_auiEncounter[2];
            }
            case TYPE_TOMB_OF_SEVEN:
                return m_auiEncounter[3];
            case TYPE_LYCEUM:
                return m_auiEncounter[4];
            case TYPE_IRON_HALL:
                return m_auiEncounter[5];
            case TYPE_QUEST_JAIL_BREAK:
                return m_auiEncounter[6];
            case TYPE_FLAMELASH:
                return m_auiEncounter[7];
            case TYPE_HURLEY:
                return m_auiEncounter[8];
            case TYPE_BRIDGE:
                return m_auiEncounter[9];
            case TYPE_BAR:
                return m_auiEncounter[10];
            case TYPE_PLUGGER:
                return m_auiEncounter[11];
            case TYPE_NAGMARA:
                return m_auiEncounter[12];
            default:
                return 0;
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
                >> m_auiEncounter[8] >> m_auiEncounter[9] >> m_auiEncounter[10] >> m_auiEncounter[11]
                >> m_auiEncounter[12];

            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                if (m_auiEncounter[i] == IN_PROGRESS && i != TYPE_IRON_HALL) // specific check for Iron Hall event: once started, it never stops, the Ironhall Guardians switch to flamethrower mode and never stop even after event completion, i.e. the event remains started if Magmus resets
                    m_auiEncounter[i] = NOT_STARTED;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        void OnCreatureEnterCombat(Creature* pCreature)
        {
            if (pCreature->GetEntry() == NPC_MAGMUS)
                SetData(TYPE_IRON_HALL, IN_PROGRESS);
        }

        void OnCreatureEvade(Creature* pCreature)
        {
            if (GetData(TYPE_RING_OF_LAW) == IN_PROGRESS || GetData(TYPE_RING_OF_LAW) == SPECIAL)
            {
                for (unsigned int aArenaNPC : aArenaNPCs)
                {
                    if (pCreature->GetEntry() == aArenaNPC)
                    {
                        SetData(TYPE_RING_OF_LAW, FAIL);
                        return;
                    }
                }
            }

            switch (pCreature->GetEntry())
            {
                // Handle Tomb of the Seven reset in case of wipe
            case NPC_HATEREL:
            case NPC_ANGERREL:
            case NPC_VILEREL:
            case NPC_GLOOMREL:
            case NPC_SEETHREL:
            case NPC_DOPEREL:
            case NPC_DOOMREL:
                SetData(TYPE_TOMB_OF_SEVEN, FAIL);
                break;
            case NPC_MAGMUS:
                SetData(TYPE_IRON_HALL, FAIL);
                break;
            }
        }

        void OnCreatureDeath(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
            case NPC_WARBRINGER_CONST:
            case NPC_WATCHER_DOOMGRIP:
                if (GetData(TYPE_VAULT) == IN_PROGRESS)
                {
                    m_sVaultNpcGuids.erase(pCreature->GetObjectGuid());

                    // If all event npcs dead then set event to done
                    if (m_sVaultNpcGuids.empty())
                        SetData(TYPE_VAULT, DONE);
                }
                break;
            case NPC_OGRABISI:
            case NPC_SHILL:
            case NPC_CREST:
            case NPC_JAZ:
                if (GetData(TYPE_QUEST_JAIL_BREAK) == IN_PROGRESS)
                    SetData(TYPE_QUEST_JAIL_BREAK, SPECIAL);
                break;
                // Handle Tomb of the Seven dwarf death event
            case NPC_HATEREL:
            case NPC_ANGERREL:
            case NPC_VILEREL:
            case NPC_GLOOMREL:
            case NPC_SEETHREL:
            case NPC_DOPEREL:
                // Only handle the event when event is in progress
                if (GetData(TYPE_TOMB_OF_SEVEN) != IN_PROGRESS)
                    return;
                // Call the next dwarf only if it's the last one which joined the fight
                if (pCreature->GetEntry() == aTombDwarfes[m_uiDwarfRound - 1])
                    DoCallNextDwarf();
                break;
            case NPC_DOOMREL:
                SetData(TYPE_TOMB_OF_SEVEN, DONE);
                break;
            case NPC_MAGMUS:
                SetData(TYPE_IRON_HALL, DONE);
                break;
            case NPC_HURLEY_BLACKBREATH:
                SetData(TYPE_HURLEY, DONE);
                break;
            case NPC_RIBBLY_SCREWSPIGGOT:
                // Do nothing if the patrol was already spawned or is about to:
                // Plugger has made the bar hostile
            {
                // Do nothing if the patrol was already spawned or is about to:
                // Plugger has made the bar hostile
                if (GetData(TYPE_BAR) == IN_PROGRESS || GetData(TYPE_PLUGGER) == IN_PROGRESS || GetData(TYPE_BAR) == DONE || GetData(TYPE_PLUGGER) == DONE)
                    return;
                SetData(TYPE_BAR, IN_PROGRESS);
            }
            break;
            case NPC_SHADOWFORGE_SENATOR:
                // Emperor Dagran Thaurissan performs a random yell upon the death
                // of Shadowforge Senators in the Throne Room
                if (Creature* pDagran = GetSingleCreatureFromStorage(NPC_EMPEROR))
                {
                    if (!pDagran->IsAlive())
                        return;

                    if (m_uiDagranTimer > 0)
                        return;

                    switch (urand(0, 3))
                    {
                    case 0: DoScriptText(YELL_SENATOR_1, pDagran); break;
                    case 1: DoScriptText(YELL_SENATOR_2, pDagran); break;
                    case 2: DoScriptText(YELL_SENATOR_3, pDagran); break;
                    case 3: DoScriptText(YELL_SENATOR_4, pDagran); break;
                    }
                    m_uiDagranTimer = 30000;    // set a timer of 30 sec to avoid Emperor Thaurissan to spam yells in case many senators are killed in a short amount of time
                }
                break;
            }
        }

        void DoCallNextDwarf()
        {
            if (Creature* pDwarf = GetSingleCreatureFromStorage(aTombDwarfes[m_uiDwarfRound]))
            {
                if (Player* pPlayer = GetPlayerInMap())
                {
                    pDwarf->SetFactionTemporary(FACTION_DWARF_HOSTILE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_RESTORE_REACH_HOME);
                    pDwarf->AI()->AttackStart(pPlayer);
                }
            }
            m_uiDwarfFightTimer = 30000;
            ++m_uiDwarfRound;
        }

        // function that replaces the princess if requirements are met
        bool CanReplacePrincess() const
        {
            Map::PlayerList const& players = instance->GetPlayers();
            if (players.isEmpty())
                return false;

            for (const auto& player : players)
            {
                if (Player* pPlayer = player.getSource())
                {
                    // if at least one player didn't complete the quest, return false
                    if ((pPlayer->GetTeam() == ALLIANCE && !pPlayer->GetQuestRewardStatus(QUEST_FATE_KINGDOM))
                        || (pPlayer->GetTeam() == HORDE && !pPlayer->GetQuestRewardStatus(QUEST_ROYAL_RESCUE)))
                        return false;
                }
            }

            return true;
        }

        void HandleBarPatrons(uint8 uiEventType)
        {
            switch (uiEventType)
            {
                // case for periodical handle of random emotes
            case PATRON_EMOTE:
                if (GetData(TYPE_PLUGGER) == DONE)
                    return;

                for (auto m_sBarPatronNpcGuid : m_sBarPatronNpcGuids)
                {
                    // About 5% of patrons do emote at a given time
                    // So avoid executing follow up code for the 95% others
                    if (urand(0, 100) < 4)
                    {
                        // Only three emotes are seen in data: laugh, cheer and exclamation
                        // the last one appearing the least and the first one appearing the most
                        // emotes are stored in a table and frequency is handled there
                        if (Creature* pPatron = instance->GetCreature(m_sBarPatronNpcGuid))
                            pPatron->HandleEmote(aPatronsEmotes[urand(0, 5)]);
                    }
                }
                return;
                // case for Rocknot event when breaking the barrel
            case PATRON_PISSED:
                // Three texts are said, one less often than the two others
                // Only by patrons near the broken barrel react to Rocknot's rampage
                if (GameObject* pGo = GetSingleGameObjectFromStorage(GO_BAR_KEG_SHOT))
                {
                    for (auto m_sBarPatronNpcGuid : m_sBarPatronNpcGuids)
                    {
                        if (Creature* pPatron = instance->GetCreature(m_sBarPatronNpcGuid))
                        {
                            if (pPatron->GetPositionZ() > pGo->GetPositionZ() - 1 && pPatron->IsWithinDist2d(pGo->GetPositionX(), pGo->GetPositionY(), 18.0f))
                            {
                                switch (urand(0, 4))
                                {
                                case 0: DoScriptText(SAY_PISSED_PATRON_3, pPatron); break;
                                case 1:  // case is double to give this text twice the chance of the previous one do be displayed
                                case 2: DoScriptText(SAY_PISSED_PATRON_2, pPatron); break;
                                    // covers the two remaining cases
                                default: DoScriptText(SAY_PISSED_PATRON_1, pPatron); break;
                                }
                            }
                        }
                    }
                }
                return;
                // case when Plugger is killed
            case PATRON_HOSTILE:
                for (auto m_sBarPatronNpcGuid : m_sBarPatronNpcGuids)
                {
                    if (Creature* pPatron = instance->GetCreature(m_sBarPatronNpcGuid))
                    {
                        pPatron->SetFactionTemporary(FACTION_DARK_IRON, TEMPFACTION_RESTORE_RESPAWN);
                        pPatron->SetStandState(UNIT_STAND_STATE_STAND);
                        pPatron->HandleEmote(EMOTE_ONESHOT_NONE);
                        pPatron->GetMotionMaster()->MoveRandomAroundPoint(pPatron->GetPositionX(), pPatron->GetPositionY(), pPatron->GetPositionZ(), 2.0f);
                    }
                }
                // Mistress Nagmara and Private Rocknot despawn if the bar turns hostile
                if (Creature* pRocknot = GetSingleCreatureFromStorage(NPC_PRIVATE_ROCKNOT))
                {
                    DoScriptText(SAY_ROCKNOT_DESPAWN, pRocknot);
                    pRocknot->ForcedDespawn();
                }
                if (Creature* pNagmara = GetSingleCreatureFromStorage(NPC_MISTRESS_NAGMARA))
                {
                    pNagmara->CastSpell(pNagmara, SPELL_NAGMARA_VANISH, TRIGGERED_OLD_TRIGGERED);
                    pNagmara->ForcedDespawn();
                }
            }
        }

        void HandleBarPatrol(uint8 uiStep)
        {
            if (GetData(TYPE_BAR) == DONE)
                return;

            switch (uiStep)
            {
            case 0:
                if (Creature* pPlugger = GetSingleCreatureFromStorage(NPC_PLUGGER_SPAZZRING))
                {
                    // if relevant, open the bar door and tell the instance it is open
                    if (!m_bIsBarDoorOpen)
                    {
                        DoUseDoorOrButton(GO_BAR_DOOR);
                        SetBarDoorIsOpen();
                    }

                    // One Fireguard Destroyer and two Anvilrage Officers are spawned
                    for (unsigned int i : aBarPatrolId)
                    {
                        float fX, fY, fZ;
                        // spawn them behind the bar door
                        pPlugger->GetRandomPoint(aBarPatrolPositions[0][0], aBarPatrolPositions[0][1], aBarPatrolPositions[0][2], 2.0f, fX, fY, fZ);
                        if (Creature* pSummoned = pPlugger->SummonCreature(i, fX, fY, fZ, aBarPatrolPositions[0][3], TEMPSPAWN_DEAD_DESPAWN, 0))
                        {
                            m_sBarPatrolGuids.insert(pSummoned->GetObjectGuid());
                            // move them to the Grim Guzzler
                            pPlugger->GetRandomPoint(aBarPatrolPositions[1][0], aBarPatrolPositions[1][1], aBarPatrolPositions[1][2], 2.0f, fX, fY, fZ);
                            pSummoned->GetMotionMaster()->MoveIdle();
                            pSummoned->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                        }
                    }
                    // start timer to handle the yells
                    m_uiPatrolTimer = 4000;
                    break;
                }
            case 1:
                for (auto m_sBarPatrolGuid : m_sBarPatrolGuids)
                {
                    if (Creature* pTmp = instance->GetCreature(m_sBarPatrolGuid))
                    {
                        if (pTmp->GetEntry() == NPC_FIREGUARD_DESTROYER)
                        {
                            DoScriptText(YELL_PATROL_1, pTmp);
                            SetData(TYPE_BAR, SPECIAL); // temporary set the status to special before the next yell: event will then be complete
                            m_uiPatrolTimer = 2000;
                            break;
                        }
                    }
                }
                break;
            case 2:
                for (auto m_sBarPatrolGuid : m_sBarPatrolGuids)
                {
                    if (Creature* pTmp = instance->GetCreature(m_sBarPatrolGuid))
                    {
                        if (pTmp->GetEntry() == NPC_FIREGUARD_DESTROYER)
                        {
                            DoScriptText(YELL_PATROL_2, pTmp);
                            SetData(TYPE_BAR, DONE);
                            m_uiPatrolTimer = 0;
                            break;
                        }
                    }
                }
                break;
            }
        }

        void Update(uint32 uiDiff)
        {
            if (m_uiDwarfFightTimer)
            {
                if (m_uiDwarfFightTimer <= uiDiff)
                {
                    if (m_uiDwarfRound < MAX_DWARFS)
                    {
                        DoCallNextDwarf();
                        m_uiDwarfFightTimer = 30000;
                    }
                    else
                        m_uiDwarfFightTimer = 0;
                }
                else
                    m_uiDwarfFightTimer -= uiDiff;
            }

            if (m_uiDagranTimer)
            {
                if (m_uiDagranTimer <= uiDiff)
                    m_uiDagranTimer = 0;
                else
                    m_uiDagranTimer -= uiDiff;
            }

            // Every second some of the patrons will do one random emote if they are not hostile (i.e. Plugger event is not done/in progress)
            if (m_uiPatronEmoteTimer)
            {
                if (m_uiPatronEmoteTimer <= uiDiff)
                {
                    HandleBarPatrons(PATRON_EMOTE);
                    m_uiPatronEmoteTimer = 1000;
                }
                else
                    m_uiPatronEmoteTimer -= uiDiff;
            }

            if (m_uiPatrolTimer)
            {
                if (m_uiPatrolTimer <= uiDiff)
                {
                    switch (GetData(TYPE_BAR))
                    {
                    case IN_PROGRESS:
                        HandleBarPatrol(1);
                        break;
                    case SPECIAL:
                        HandleBarPatrol(2);
                        break;
                    default:
                        break;
                    }
                }
                else
                    m_uiPatrolTimer -= uiDiff;
            }
        }
    };

};
enum
{
    SAY_START_1 = -1230004,
    SAY_START_2 = -1230005,
    SAY_OPEN_EAST_GATE = -1230006,
    SAY_SUMMON_BOSS_1 = -1230007,
    SAY_SUMMON_BOSS_2 = -1230008,
    SAY_OPEN_NORTH_GATE = -1230009,

    NPC_GRIMSTONE = 10096,
    DATA_BANNER_BEFORE_EVENT = 5,

    MAX_THELDREN_ADDS = 4,
    MAX_POSSIBLE_THELDREN_ADDS = 8,

    SPELL_SUMMON_THELRIN_DND = 27517,
    // Other spells used by Grimstone
    SPELL_ASHCROMBES_TELEPORT_A = 15742,
    SPELL_ASHCROMBES_TELEPORT_B = 6422,
    SPELL_ARENA_FLASH_A = 15737,
    SPELL_ARENA_FLASH_B = 15739,
    SPELL_ARENA_FLASH_C = 15740,
    SPELL_ARENA_FLASH_D = 15741,

    QUEST_THE_CHALLENGE = 9015,
    NPC_THELDREN_QUEST_CREDIT = 16166,
};


enum SpawnPosition
{
    POS_EAST = 0,
    POS_NORTH = 1,
    POS_GRIMSTONE = 2,
};

static const float aSpawnPositions[3][4] =
{
    {608.960f, -235.322f, -53.907f, 1.857f},                // Ring mob spawn position
    {644.300f, -175.989f, -53.739f, 3.418f},                // Ring boss spawn position
    {625.559f, -205.618f, -52.735f, 2.609f}                 // Grimstone spawn position
};


static const uint32 aGladiator[MAX_POSSIBLE_THELDREN_ADDS] = { NPC_LEFTY, NPC_ROTFANG, NPC_SNOKH, NPC_MALGEN, NPC_KORV, NPC_REZZNIK, NPC_VAJASHNI, NPC_VOLIDA };
static const uint32 aRingMob[2][6] =
{
    {NPC_WORM, NPC_STINGER, NPC_SCREECHER, NPC_THUNDERSNOUT, NPC_CREEPER, NPC_BEETLE}, // NPC template entry
    {4, 2, 5, 3, 3, 7}                                                                 // Number of NPCs per wave (two waves)
};
static const uint32 aRingBoss[] = { NPC_GOROSH, NPC_GRIZZLE, NPC_EVISCERATOR, NPC_OKTHOR, NPC_ANUBSHIAH, NPC_HEDRUM };


class npc_theldren_trigger : public CreatureScript
{
public:
    npc_theldren_trigger() : CreatureScript("npc_theldren_trigger") { }

    bool OnEffectDummy(Unit* /*pCaster*/, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/) override
    {
        if (uiSpellId == SPELL_SUMMON_THELRIN_DND && uiEffIndex != EFFECT_INDEX_0)
        {
            ScriptedInstance* pInstance = (ScriptedInstance*)pCreatureTarget->GetInstanceData();
            if (pInstance && pInstance->GetData(TYPE_RING_OF_LAW) != DONE && pInstance->GetData(TYPE_RING_OF_LAW) != SPECIAL)
                pInstance->SetData(TYPE_RING_OF_LAW, pInstance->GetData(TYPE_RING_OF_LAW) == IN_PROGRESS ? uint32(SPECIAL) : uint32(DATA_BANNER_BEFORE_EVENT));

            return true;
        }
        return false;
    }



};class at_ring_of_law : public AreaTriggerScript
{
public:
    at_ring_of_law() : AreaTriggerScript("at_ring_of_law") { }

    bool OnTrigger(Player* pPlayer, AreaTriggerEntry const* pAt) override
    {
        if (instance_blackrock_depths::instance_blackrock_depthsAI* pInstance = (instance_blackrock_depths::instance_blackrock_depthsAI*)pPlayer->GetInstanceData())
        {
            if (pInstance->GetData(TYPE_RING_OF_LAW) == IN_PROGRESS || pInstance->GetData(TYPE_RING_OF_LAW) == DONE || pInstance->GetData(TYPE_RING_OF_LAW) == SPECIAL)
                return false;

            if (pPlayer->isGameMaster())
                return false;

            pInstance->SetData(TYPE_RING_OF_LAW, pInstance->GetData(TYPE_RING_OF_LAW) == DATA_BANNER_BEFORE_EVENT ? SPECIAL : IN_PROGRESS);

            pPlayer->SummonCreature(NPC_GRIMSTONE, aSpawnPositions[POS_GRIMSTONE][0], aSpawnPositions[POS_GRIMSTONE][1], aSpawnPositions[POS_GRIMSTONE][2], aSpawnPositions[POS_GRIMSTONE][3], TEMPSPAWN_DEAD_DESPAWN, 0);
            pInstance->SetArenaCenterCoords(pAt->x, pAt->y, pAt->z);

            return false;
        }
        return false;
    }



};

/*######
## npc_grimstone
######*/
enum Phases
{
    PHASE_MOBS = 0,
    PHASE_BOSS = 2,
    PHASE_GLADIATORS = 3,
};

class npc_grimstone : public CreatureScript
{
public:
    npc_grimstone() : CreatureScript("npc_grimstone") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new npc_grimstoneAI(pCreature);
    }



    struct npc_grimstoneAI : public npc_escortAI
    {
        npc_grimstoneAI(Creature* pCreature) : npc_escortAI(pCreature)
        {
            m_pInstance = (instance_blackrock_depths::instance_blackrock_depthsAI*)pCreature->GetInstanceData();
            // select which trash NPC will be released for this run
            m_uiMobSpawnId = urand(0, 5);
            // Select MAX_THELDREN_ADDS(4) random adds for Theldren encounter
            uint8 uiCount = 0;
            for (uint8 i = 0; i < MAX_POSSIBLE_THELDREN_ADDS && uiCount < MAX_THELDREN_ADDS; ++i)
            {
                if (urand(0, 1) || i >= MAX_POSSIBLE_THELDREN_ADDS - MAX_THELDREN_ADDS + uiCount)
                {
                    m_uiGladiatorId[uiCount] = aGladiator[i];
                    ++uiCount;
                }
            }

            Reset();
        }

        instance_blackrock_depths::instance_blackrock_depthsAI* m_pInstance;

        uint8 m_uiEventPhase;
        uint32 m_uiEventTimer;

        uint8 m_uiMobSpawnId;
        uint8 m_uiAliveSummonedMob;

        Phases m_uiPhase;

        uint32 m_uiGladiatorId[MAX_THELDREN_ADDS];

        GuidList m_lSummonedGUIDList;
        GuidSet m_lArenaCrowd;

        void Reset() override
        {
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            m_uiEventTimer = 1000;
            m_uiEventPhase = 0;
            m_uiAliveSummonedMob = 0;

            m_uiPhase = PHASE_MOBS;
        }

        void JustSummoned(Creature* pSummoned) override
        {
            if (!m_pInstance)
                return;

            // Ring mob or boss summoned
            float fX, fY, fZ;
            float fcX, fcY, fcZ;
            m_pInstance->GetArenaCenterCoords(fX, fY, fZ);
            m_creature->GetRandomPoint(fX, fY, fZ, 10.0f, fcX, fcY, fcZ);
            pSummoned->GetMotionMaster()->MovePoint(1, fcX, fcY, fcZ);

            ++m_uiAliveSummonedMob;
            m_lSummonedGUIDList.push_back(pSummoned->GetObjectGuid());
        }

        void DoChallengeQuestCredit()
        {
            Map::PlayerList const& PlayerList = m_creature->GetMap()->GetPlayers();

            for (const auto& itr : PlayerList)
            {
                Player* pPlayer = itr.getSource();
                if (pPlayer && pPlayer->GetQuestStatus(QUEST_THE_CHALLENGE) == QUEST_STATUS_INCOMPLETE)
                    pPlayer->KilledMonsterCredit(NPC_THELDREN_QUEST_CREDIT);
            }
        }

        void SummonedCreatureJustDied(Creature* /*pSummoned*/) override
        {
            --m_uiAliveSummonedMob;

            switch (m_uiPhase)
            {
            case PHASE_MOBS:                                // Ring mob killed
            case PHASE_BOSS:                                // Ring boss killed
                if (m_uiAliveSummonedMob == 0)
                    m_uiEventTimer = 5000;
                break;
            case PHASE_GLADIATORS:                          // Theldren and his band killed
                // Adds + Theldren
                if (m_uiAliveSummonedMob == 0)
                {
                    m_uiEventTimer = 5000;
                    DoChallengeQuestCredit();
                }
                break;
            }
        }

        void SummonRingMob(uint32 uiEntry, uint8 uiNpcPerWave, SpawnPosition uiPosition)
        {
            float fX, fY, fZ;
            for (uint8 i = 0; i < uiNpcPerWave; ++i)
            {
                m_creature->GetRandomPoint(aSpawnPositions[uiPosition][0], aSpawnPositions[uiPosition][1], aSpawnPositions[uiPosition][2], 2.0f, fX, fY, fZ);
                m_creature->SummonCreature(uiEntry, fX, fY, fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 0);
            }
        }

        void WaypointReached(uint32 uiPointId) override
        {
            switch (uiPointId)
            {
            case 1:                                         // Middle reached first time
                DoScriptText(SAY_START_1, m_creature);
                SetEscortPaused(true);
                m_uiEventTimer = 5000;
                break;
            case 2:                                         // Reached wall again
                DoScriptText(SAY_OPEN_EAST_GATE, m_creature);
                SetEscortPaused(true);
                m_uiEventTimer = 5000;
                break;
            case 3:                                         // walking along the wall, while door opened
                SetEscortPaused(true);
                break;
            case 4:                                         // Middle reached second time
                DoScriptText(SAY_SUMMON_BOSS_1, m_creature);
                break;
            case 5:                                         // Reached North Gate
                DoScriptText(SAY_OPEN_NORTH_GATE, m_creature);
                SetEscortPaused(true);
                m_uiEventTimer = 5000;
                break;
            case 6:
                if (m_pInstance)
                {
                    m_pInstance->SetData(TYPE_RING_OF_LAW, DONE);
                    // debug_log("SD2: npc_grimstone: event reached end and set complete.");
                }
                break;
            }
        }

        void UpdateEscortAI(const uint32 uiDiff) override
        {
            if (!m_pInstance)
                return;

            if (m_pInstance->GetData(TYPE_RING_OF_LAW) == FAIL)
            {
                // Reset Doors
                if (m_uiEventPhase >= 10)                       // North Gate is opened
                {
                    m_pInstance->DoUseDoorOrButton(GO_ARENA_2);
                    m_pInstance->DoUseDoorOrButton(GO_ARENA_4);
                }
                else if (m_uiEventPhase >= 4)                   // East Gate is opened
                {
                    m_pInstance->DoUseDoorOrButton(GO_ARENA_1);
                    m_pInstance->DoUseDoorOrButton(GO_ARENA_4);
                }

                // Despawn Summoned Mobs
                for (GuidList::const_iterator itr = m_lSummonedGUIDList.begin(); itr != m_lSummonedGUIDList.end(); ++itr)
                {
                    if (Creature* pSummoned = m_creature->GetMap()->GetCreature(*itr))
                        pSummoned->ForcedDespawn();
                }
                m_lSummonedGUIDList.clear();

                // Despawn NPC
                m_creature->ForcedDespawn();
                return;
            }

            if (m_uiEventTimer)
            {
                if (m_uiEventTimer <= uiDiff)
                {
                    switch (m_uiEventPhase)
                    {
                    case 0:
                        // Shortly after spawn, start walking
                        DoCastSpellIfCan(m_creature, SPELL_ASHCROMBES_TELEPORT_A, CAST_TRIGGERED);
                        DoScriptText(SAY_START_2, m_creature);
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_4);
                        // Some of the NPCs in the crowd do cheer emote at event start
                        // we randomly select 25% of the NPCs to do this
                        m_pInstance->GetArenaCrowdGuid(m_lArenaCrowd);
                        for (auto itr : m_lArenaCrowd)
                        {
                            if (Creature* pSpectator = m_creature->GetMap()->GetCreature(itr))
                            {
                                if (urand(0, 3) < 1)
                                    pSpectator->HandleEmote(EMOTE_ONESHOT_CHEER);
                            }
                        }
                        Start(false);
                        SetEscortPaused(false);
                        m_uiEventTimer = 0;
                        break;
                    case 1:
                        // Start walking towards wall
                        SetEscortPaused(false);
                        m_uiEventTimer = 0;
                        break;
                    case 2:
                        m_uiEventTimer = 2000;
                        break;
                    case 3:
                        // Open East Gate
                        DoCastSpellIfCan(m_creature, SPELL_ARENA_FLASH_A, CAST_TRIGGERED);
                        DoCastSpellIfCan(m_creature, SPELL_ARENA_FLASH_B, CAST_TRIGGERED);
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_1);
                        m_uiEventTimer = 3000;
                        break;
                    case 4:
                        // timer for teleport out spell which has 2000 ms cast time
                        DoCastSpellIfCan(m_creature, SPELL_ASHCROMBES_TELEPORT_B, CAST_TRIGGERED);
                        m_uiEventTimer = 2500;
                        break;
                    case 5:
                        m_creature->SetVisibility(VISIBILITY_OFF);
                        SetEscortPaused(false);
                        // Summon Ring Mob(s)
                        SummonRingMob(aRingMob[0][m_uiMobSpawnId], aRingMob[1][m_uiMobSpawnId], POS_EAST);
                        m_uiEventTimer = 16000;
                        break;
                    case 7:
                        // Summon Ring Mob(s)
                        SummonRingMob(aRingMob[0][m_uiMobSpawnId], aRingMob[1][m_uiMobSpawnId], POS_EAST);
                        m_uiEventTimer = 0;
                        break;
                    case 8:
                        // Summoned Mobs are dead, continue event
                        DoScriptText(SAY_SUMMON_BOSS_2, m_creature);
                        m_creature->SetVisibility(VISIBILITY_ON);
                        DoCastSpellIfCan(m_creature, SPELL_ASHCROMBES_TELEPORT_A, CAST_TRIGGERED);
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_1);
                        SetEscortPaused(false);
                        m_uiEventTimer = 0;
                        break;
                    case 9:
                        // Open North Gate
                        DoCastSpellIfCan(m_creature, SPELL_ARENA_FLASH_C, CAST_TRIGGERED);
                        DoCastSpellIfCan(m_creature, SPELL_ARENA_FLASH_D, CAST_TRIGGERED);
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_2);
                        m_uiEventTimer = 5000;
                        break;
                    case 10:
                        // timer for teleport out spell which has 2000 ms cast time
                        DoCastSpellIfCan(m_creature, SPELL_ASHCROMBES_TELEPORT_B, CAST_TRIGGERED);
                        m_uiEventTimer = 2500;
                        break;
                    case 11:
                        // Summon Boss
                        m_creature->SetVisibility(VISIBILITY_OFF);
                        // If banner summoned after start, then summon Thelden after the creatures are dead
                        if (m_pInstance->GetData(TYPE_RING_OF_LAW) == SPECIAL && m_uiPhase == PHASE_MOBS)
                        {
                            m_uiPhase = PHASE_GLADIATORS;
                            SummonRingMob(NPC_THELDREN, 1, POS_NORTH);
                            for (unsigned int i : m_uiGladiatorId)
                                SummonRingMob(i, 1, POS_NORTH);
                        }
                        else
                        {
                            m_uiPhase = PHASE_BOSS;
                            SummonRingMob(aRingBoss[urand(0, 5)], 1, POS_NORTH);
                        }
                        m_uiEventTimer = 0;
                        break;
                    case 12:
                        // Boss dead
                        m_lSummonedGUIDList.clear();
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_2);
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_3);
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_4);
                        SetEscortPaused(false);
                        m_uiEventTimer = 0;
                        break;
                    }
                    ++m_uiEventPhase;
                }
                else
                    m_uiEventTimer -= uiDiff;
            }
        }
    };



};


/*######
+## npc_phalanx
+######*/

enum
{
    YELL_PHALANX_AGGRO = -1230040,

    SPELL_THUNDERCLAP = 15588,
    SPELL_MIGHTY_BLOW = 14099,
    SPELL_FIREBALL_VOLLEY = 15285,
};
class npc_phalanx : public CreatureScript
{
public:
    npc_phalanx() : CreatureScript("npc_phalanx") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new npc_phalanxAI(pCreature);
    }



    struct npc_phalanxAI : public npc_escortAI
    {
        npc_phalanxAI(Creature* pCreature) : npc_escortAI(pCreature)
        {
            m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
            Reset();
        }

        ScriptedInstance* m_pInstance;

        float m_fKeepDoorOrientation;
        uint32 uiThunderclapTimer;
        uint32 uiMightyBlowTimer;
        uint32 uiFireballVolleyTimer;
        uint32 uiCallPatrolTimer;

        void Reset() override
        {
            // If reset after an fight, it means Phalanx has already started moving (if not already reached door)
            // so we made him restart right before reaching the door to guard it (again)
            if (HasEscortState(STATE_ESCORT_ESCORTING) || HasEscortState(STATE_ESCORT_PAUSED))
            {
                SetCurrentWaypoint(1);
                SetEscortPaused(false);
            }

            m_fKeepDoorOrientation = 2.06059f;
            uiThunderclapTimer = 0;
            uiMightyBlowTimer = 0;
            uiFireballVolleyTimer = 0;
            uiCallPatrolTimer = 0;
        }

        void Aggro(Unit* /*pWho*/) override
        {
            uiThunderclapTimer = 12000;
            uiMightyBlowTimer = 15000;
            uiFireballVolleyTimer = 1;
        }

        void WaypointReached(uint32 uiPointId) override
        {
            if (!m_pInstance)
                return;

            switch (uiPointId)
            {
            case 1:
                DoScriptText(YELL_PHALANX_AGGRO, m_creature);
                break;
            case 2:
                SetEscortPaused(true);
                // There are two ways of activating Phalanx: completing Rocknot event, making Phalanx hostile to anyone
                // killing Plugger making Phalanx hostile to Horde (do not ask why)
                // In the later case, Phalanx should also spawn the bar patrol with some delay and only then set the Plugger
                // event to DONE. In the weird case where Plugger was previously killed (event == DONE) but Phalanx is reactivated
                //  (like on reset after a wipe), do not spawn the patrol again
                if (m_pInstance->GetData(TYPE_PLUGGER) == DONE || m_pInstance->GetData(TYPE_PLUGGER) == IN_PROGRESS)
                {
                    m_creature->SetFactionTemporary(FACTION_IRONFORGE, TEMPFACTION_NONE);
                    if (m_pInstance->GetData(TYPE_PLUGGER) == IN_PROGRESS)
                    {
                        uiCallPatrolTimer = 10000;
                        m_pInstance->SetData(TYPE_PLUGGER, DONE);
                    }
                }
                else
                    m_creature->SetFactionTemporary(FACTION_DARK_IRON, TEMPFACTION_NONE);

                m_creature->SetFacingTo(m_fKeepDoorOrientation);
                break;
            }
        }

        void UpdateEscortAI(const uint32 uiDiff) override
        {
            if (!m_pInstance)
                return;

            if (uiCallPatrolTimer)
            {
                if (uiCallPatrolTimer < uiDiff && m_pInstance->GetData(TYPE_BAR) != DONE)
                {
                    m_pInstance->SetData(TYPE_BAR, IN_PROGRESS);
                    uiCallPatrolTimer = 0;
                }
                else
                    uiCallPatrolTimer -= uiDiff;
            }

            // Combat check
            if (m_creature->SelectHostileTarget() && m_creature->GetVictim())
            {
                if (uiThunderclapTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_THUNDERCLAP) == CAST_OK)
                        uiThunderclapTimer = 10000;
                }
                else
                    uiThunderclapTimer -= uiDiff;

                if (uiMightyBlowTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_MIGHTY_BLOW) == CAST_OK)
                        uiMightyBlowTimer = 10000;
                }
                else
                    uiMightyBlowTimer -= uiDiff;

                if (m_creature->GetHealthPercent() < 51.0f)
                {
                    if (uiFireballVolleyTimer < uiDiff)
                    {
                        if (DoCastSpellIfCan(m_creature, SPELL_FIREBALL_VOLLEY) == CAST_OK)
                            uiFireballVolleyTimer = 15000;
                    }
                    else
                        uiFireballVolleyTimer -= uiDiff;
                }

                DoMeleeAttackIfReady();
            }
        }
    };



};



/*######
## npc_mistress_nagmara
######*/

enum
{
    GOSSIP_ITEM_NAGMARA = -3230003,
    GOSSIP_ID_NAGMARA = 2727,
    GOSSIP_ID_NAGMARA_2 = 2729,
    SPELL_POTION_LOVE = 14928,
    SPELL_NAGMARA_ROCKNOT = 15064,

    SAY_NAGMARA_1 = -1230066,
    SAY_NAGMARA_2 = -1230067,
    TEXTEMOTE_NAGMARA = -1230068,
    TEXTEMOTE_ROCKNOT = -1230069,

    QUEST_POTION_LOVE = 4201
};
class npc_mistress_nagmara : public CreatureScript
{
public:
    npc_mistress_nagmara() : CreatureScript("npc_mistress_nagmara") { }

    bool OnQuestReward(Player* /*pPlayer*/, Creature* pCreature, Quest const* pQuest) override
    {
        ScriptedInstance* pInstance = (ScriptedInstance*)pCreature->GetInstanceData();

        if (!pInstance)
            return true;

        if (pQuest->GetQuestId() == QUEST_POTION_LOVE)
        {
            if (npc_mistress_nagmara::npc_mistress_nagmaraAI* pNagmaraAI = dynamic_cast<npc_mistress_nagmara::npc_mistress_nagmaraAI*>(pCreature->AI()))
                pNagmaraAI->DoPotionOfLoveIfCan();
        }

        return true;
    }



    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        switch (uiAction)
        {
        case GOSSIP_ACTION_INFO_DEF + 1:
            pPlayer->CLOSE_GOSSIP_MENU();
            if (npc_mistress_nagmara::npc_mistress_nagmaraAI* pNagmaraAI = dynamic_cast<npc_mistress_nagmara::npc_mistress_nagmaraAI*>(pCreature->AI()))
                pNagmaraAI->DoPotionOfLoveIfCan();
            break;
        }
        return true;
    }



    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {
        if (pCreature->isQuestGiver())
            pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

        if (pPlayer->GetQuestStatus(QUEST_POTION_LOVE) == QUEST_STATUS_COMPLETE)
        {
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NAGMARA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_ID_NAGMARA_2, pCreature->GetObjectGuid());
        }
        else
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_ID_NAGMARA, pCreature->GetObjectGuid());

        return true;
    }



    UnitAI* GetAI(Creature* pCreature)
    {
        return new npc_mistress_nagmaraAI(pCreature);
    }



    struct npc_mistress_nagmaraAI : public ScriptedAI
    {
        npc_mistress_nagmaraAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
            Reset();
        }

        ScriptedInstance* m_pInstance;
        Creature* pRocknot;

        uint8 m_uiPhase;
        uint32 m_uiPhaseTimer;

        void Reset() override
        {
            m_uiPhase = 0;
            m_uiPhaseTimer = 0;
        }

        void DoPotionOfLoveIfCan()
        {
            if (!m_pInstance)
                return;

            pRocknot = m_pInstance->GetSingleCreatureFromStorage(NPC_PRIVATE_ROCKNOT);
            if (!pRocknot)
                return;

            m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            pRocknot->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);

            m_creature->GetMotionMaster()->MoveIdle();
            m_creature->GetMotionMaster()->MoveFollow(pRocknot, 2.0f, 0);
            m_uiPhase = 1;
        }

        void UpdateAI(const uint32 uiDiff) override
        {
            if (m_uiPhaseTimer)
            {
                if (m_uiPhaseTimer <= uiDiff)
                    m_uiPhaseTimer = 0;
                else
                {
                    m_uiPhaseTimer -= uiDiff;
                    return;
                }
            }

            if (!pRocknot)
                return;

            switch (m_uiPhase)
            {
            case 0:     // Phase 0 : Nagmara patrols in the bar to serve patrons or is following Rocknot passively
                break;
            case 1:     // Phase 1 : Nagmara is moving towards Rocknot
                if (m_creature->IsWithinDist2d(pRocknot->GetPositionX(), pRocknot->GetPositionY(), 5.0f))
                {
                    m_creature->GetMotionMaster()->MoveIdle();
                    m_creature->SetFacingToObject(pRocknot);
                    pRocknot->SetFacingToObject(m_creature);
                    DoScriptText(SAY_NAGMARA_1, m_creature);
                    m_uiPhase++;
                    m_uiPhaseTimer = 5000;
                }
                else
                    m_creature->GetMotionMaster()->MoveFollow(pRocknot, 2.0f, 0);
                break;
            case 2:     // Phase 2 : Nagmara is "seducing" Rocknot
                DoScriptText(SAY_NAGMARA_2, m_creature);
                m_uiPhaseTimer = 4000;
                m_uiPhase++;
                break;
            case 3:     // Phase 3: Nagmara give potion to Rocknot and Rocknot escort AI will handle the next part of the event
                if (DoCastSpellIfCan(m_creature, SPELL_POTION_LOVE) == CAST_OK)
                {
                    m_uiPhase = 0;
                    m_pInstance->SetData(TYPE_NAGMARA, SPECIAL);
                }
                break;
            case 4:     // Phase 4 : make the lovers face each other
                m_creature->SetFacingToObject(pRocknot);
                pRocknot->SetFacingToObject(m_creature);
                m_uiPhaseTimer = 4000;
                m_uiPhase++;
                m_pInstance->SetData(TYPE_NAGMARA, DONE);
                break;
            case 5:     // Phase 5 : Nagmara and Rocknot are under the stair kissing (this phase repeats endlessly)
                DoScriptText(TEXTEMOTE_NAGMARA, m_creature);
                DoScriptText(TEXTEMOTE_ROCKNOT, pRocknot);
                DoCastSpellIfCan(m_creature, SPELL_NAGMARA_ROCKNOT);
                pRocknot->CastSpell(pRocknot, SPELL_NAGMARA_ROCKNOT, TRIGGERED_NONE);
                m_uiPhaseTimer = 12000;
                break;
            }
        }
    };



};




/*######
## npc_rocknot
######*/

enum
{
    SAY_GOT_BEER = -1230000,
    SAY_MORE_BEER = -1230036,
    SAY_BARREL_1 = -1230044,
    SAY_BARREL_2 = -1230045,
    SAY_BARREL_3 = -1230046,

    SPELL_DRUNKEN_RAGE = 14872,

    QUEST_ALE = 4295
};

static const float aPosNagmaraRocknot[3] = { 878.1779f, -222.0662f, -49.96714f };
class npc_rocknot : public CreatureScript
{
public:
    npc_rocknot() : CreatureScript("npc_rocknot") { }

    bool OnQuestReward(Player* pPlayer, Creature* pCreature, Quest const* pQuest) override
    {
        ScriptedInstance* pInstance = (ScriptedInstance*)pCreature->GetInstanceData();

        if (!pInstance)
            return true;

        if (pInstance->GetData(TYPE_ROCKNOT) == DONE || pInstance->GetData(TYPE_ROCKNOT) == SPECIAL)
            return true;

        if (pQuest->GetQuestId() == QUEST_ALE)
        {
            if (pInstance->GetData(TYPE_ROCKNOT) != IN_PROGRESS)
                pInstance->SetData(TYPE_ROCKNOT, IN_PROGRESS);

            pCreature->SetFacingToObject(pPlayer);
            DoScriptText(SAY_GOT_BEER, pCreature);
            if (npc_rocknot::npc_rocknotAI* pEscortAI = dynamic_cast<npc_rocknot::npc_rocknotAI*>(pCreature->AI()))
                pEscortAI->m_uiEmoteTimer = 1500;

            // We keep track of amount of beers given in the instance script by setting data to SPECIAL
            // Once the correct amount is reached, the script will also returns SPECIAL, if not, it returns IN_PROGRESS/DONE
            // the return state and the following of the script are handled in the Update->emote part of the Rocknot NPC escort AI script
            pInstance->SetData(TYPE_ROCKNOT, SPECIAL);
        }

        return true;
    }



    UnitAI* GetAI(Creature* pCreature)
    {
        return new npc_rocknotAI(pCreature);
    }



    struct npc_rocknotAI : public npc_escortAI
    {
        npc_rocknotAI(Creature* pCreature) : npc_escortAI(pCreature)
        {
            m_pInstance = (instance_blackrock_depths::instance_blackrock_depthsAI*)pCreature->GetInstanceData();
            Reset();
        }

        instance_blackrock_depths::instance_blackrock_depthsAI* m_pInstance;
        Creature* pNagmara;

        uint32 m_uiBreakKegTimer;
        uint32 m_uiBreakDoorTimer;
        uint32 m_uiEmoteTimer;
        uint32 m_uiBarReactTimer;

        bool m_bIsDoorOpen;
        float m_fInitialOrientation;

        void Reset() override
        {
            pNagmara = m_pInstance->GetSingleCreatureFromStorage(NPC_MISTRESS_NAGMARA);

            if (HasEscortState(STATE_ESCORT_ESCORTING))
                return;

            m_fInitialOrientation = 3.21141f;
            m_uiBreakKegTimer = 0;
            m_uiBreakDoorTimer = 0;
            m_uiEmoteTimer = 0;
            m_uiBarReactTimer = 0;
            m_bIsDoorOpen = false;
        }

        void DoGo(uint32 id, GOState state)
        {
            if (GameObject* pGo = m_pInstance->GetSingleGameObjectFromStorage(id))
                pGo->SetGoState(state);
        }

        void WaypointReached(uint32 uiPointId) override
        {
            if (!m_pInstance)
                return;

            switch (uiPointId)
            {
            case 1:     // if Nagmara and Potion of Love event is in progress, switch to second part of the escort
                SetEscortPaused(true);
                if (m_pInstance->GetData(TYPE_NAGMARA) == IN_PROGRESS)
                    SetCurrentWaypoint(10);

                SetEscortPaused(false);
                break;
            case 3:
                DoScriptText(SAY_BARREL_1, m_creature);
                break;
            case 4:
                DoScriptText(SAY_BARREL_2, m_creature);
                break;
            case 5:
                DoScriptText(SAY_BARREL_2, m_creature);
                break;
            case 6:
                DoScriptText(SAY_BARREL_1, m_creature);
                break;
            case 7:
                DoCastSpellIfCan(m_creature, SPELL_DRUNKEN_RAGE, false);
                m_uiBreakKegTimer = 2000;
                break;
            case 9:     // Back home stop here
                SetEscortPaused(true);
                m_creature->SetFacingTo(m_fInitialOrientation);
                break;
            case 10:     // This step is the start of the "alternate" waypoint path used with Nagmara
                // Make Nagmara follow Rocknot
                if (!pNagmara)
                {
                    SetEscortPaused(true);
                    SetCurrentWaypoint(9);
                }
                else
                    pNagmara->GetMotionMaster()->MoveFollow(m_creature, 2.0f, 0);
                break;
            case 17:
                // Open the bar back door if relevant
                m_pInstance->GetBarDoorIsOpen(m_bIsDoorOpen);
                if (!m_bIsDoorOpen)
                {
                    m_pInstance->DoUseDoorOrButton(GO_BAR_DOOR);
                    m_pInstance->SetBarDoorIsOpen();
                }
                if (pNagmara)
                    pNagmara->GetMotionMaster()->MoveFollow(m_creature, 2.0f, 0);
                break;
            case 34: // Reach under the stair, make Nagmara move to her position and give the handle back to Nagmara AI script
                if (!pNagmara)
                    break;

                pNagmara->GetMotionMaster()->MoveIdle();
                pNagmara->GetMotionMaster()->MovePoint(0, aPosNagmaraRocknot[0], aPosNagmaraRocknot[1], aPosNagmaraRocknot[2]);
                if (npc_mistress_nagmara::npc_mistress_nagmaraAI* pNagmaraAI = dynamic_cast<npc_mistress_nagmara::npc_mistress_nagmaraAI*>(pNagmara->AI()))
                {
                    pNagmaraAI->m_uiPhase = 4;
                    pNagmaraAI->m_uiPhaseTimer = 5000;
                }
                SetEscortPaused(true);
                break;
            }
        }

        void UpdateEscortAI(const uint32 uiDiff) override
        {
            if (!m_pInstance)
                return;

            // When Nagmara is in Potion of Love event and reach Rocknot, she set TYPE_NAGMARA to SPECIAL
            // in order to make Rocknot start the second part of his escort quest
            if (m_pInstance->GetData(TYPE_NAGMARA) == SPECIAL)
            {
                m_pInstance->SetData(TYPE_NAGMARA, IN_PROGRESS);
                Start(false, nullptr, nullptr, true);
                return;
            }

            if (m_uiBreakKegTimer)
            {
                if (m_uiBreakKegTimer <= uiDiff)
                {
                    DoGo(GO_BAR_KEG_SHOT, GO_STATE_ACTIVE);
                    m_uiBreakKegTimer = 0;
                    m_uiBreakDoorTimer = 1000;
                    m_uiBarReactTimer = 5000;
                }
                else
                    m_uiBreakKegTimer -= uiDiff;
            }

            if (m_uiBreakDoorTimer)
            {
                if (m_uiBreakDoorTimer <= uiDiff)
                {
                    // Open the bar back door if relevant
                    m_pInstance->GetBarDoorIsOpen(m_bIsDoorOpen);
                    if (!m_bIsDoorOpen)
                        DoGo(GO_BAR_DOOR, GO_STATE_ACTIVE_ALTERNATIVE);

                    DoScriptText(SAY_BARREL_3, m_creature);
                    DoGo(GO_BAR_KEG_TRAP, GO_STATE_ACTIVE);                   // doesn't work very well, leaving code here for future
                    // spell by trap has effect61

                    m_uiBreakDoorTimer = 0;
                }
                else
                    m_uiBreakDoorTimer -= uiDiff;
            }

            if (m_uiBarReactTimer)
            {
                if (m_uiBarReactTimer <= uiDiff)
                {
                    // Activate Phalanx and handle nearby patrons says
                    if (Creature* pPhalanx = m_pInstance->GetSingleCreatureFromStorage(NPC_PHALANX))
                    {
                        if (npc_phalanx::npc_phalanxAI* pEscortAI = dynamic_cast<npc_phalanx::npc_phalanxAI*>(pPhalanx->AI()))
                            pEscortAI->Start(false, nullptr, nullptr, true);
                    }
                    m_pInstance->SetData(TYPE_ROCKNOT, DONE);

                    m_uiBarReactTimer = 0;
                }
                else
                    m_uiBarReactTimer -= uiDiff;
            }

            // Several times Rocknot is supposed to perform an action (text, spell cast...) followed closely by an emote
            // we handle it here
            if (m_uiEmoteTimer)
            {
                if (m_uiEmoteTimer <= uiDiff)
                {
                    // If event is SPECIAL (Rocknot moving to barrel), then we want him to say a special text and start moving
                    // if not, he is still accepting beers, so we want him to cheer player
                    if (m_pInstance->GetData(TYPE_ROCKNOT) == SPECIAL)
                    {
                        DoScriptText(SAY_MORE_BEER, m_creature);
                        Start(false);
                    }
                    else
                        m_creature->HandleEmote(EMOTE_ONESHOT_CHEER);

                    m_uiEmoteTimer = 0;
                }
                else
                    m_uiEmoteTimer -= uiDiff;
            }
        }
    };



};


/*######
## boss_plugger_spazzring
######*/

enum
{
    SAY_OOC_1 = -1230050,
    SAY_OOC_2 = -1230051,
    SAY_OOC_3 = -1230052,
    SAY_OOC_4 = -1230053,

    YELL_STOLEN_1 = -1230054,
    YELL_STOLEN_2 = -1230055,
    YELL_STOLEN_3 = -1230056,
    YELL_AGRRO_1 = -1230057,
    YELL_AGRRO_2 = -1230058,
    YELL_PICKPOCKETED = -1230059,

    // spells
    SPELL_BANISH = 8994,
    SPELL_CURSE_OF_TONGUES = 13338,
    SPELL_DEMON_ARMOR = 13787,
    SPELL_IMMOLATE = 12742,
    SPELL_SHADOW_BOLT = 12739,
    SPELL_PICKPOCKET = 921,
};

static const int aRandomSays[] = { SAY_OOC_1, SAY_OOC_2, SAY_OOC_3, SAY_OOC_4 };

static const int aRandomYells[] = { YELL_STOLEN_1, YELL_STOLEN_2, YELL_STOLEN_3 };
class boss_plugger_spazzring : public CreatureScript
{
public:
    boss_plugger_spazzring() : CreatureScript("boss_plugger_spazzring") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new boss_plugger_spazzringAI(pCreature);
    }



    struct boss_plugger_spazzringAI : public ScriptedAI
    {

        boss_plugger_spazzringAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = (instance_blackrock_depths::instance_blackrock_depthsAI*)pCreature->GetInstanceData();
            Reset();
        }

        instance_blackrock_depths::instance_blackrock_depthsAI* m_pInstance;

        uint32 m_uiOocSayTimer;
        uint32 m_uiDemonArmorTimer;
        uint32 m_uiBanishTimer;
        uint32 m_uiImmolateTimer;
        uint32 m_uiShadowBoltTimer;
        uint32 m_uiCurseOfTonguesTimer;
        uint32 m_uiPickpocketTimer;

        void Reset() override
        {
            m_uiOocSayTimer = 10000;
            m_uiDemonArmorTimer = 1000;
            m_uiBanishTimer = 0;
            m_uiImmolateTimer = 0;
            m_uiShadowBoltTimer = 0;
            m_uiCurseOfTonguesTimer = 0;
            m_uiPickpocketTimer = 0;
        }

        void Aggro(Unit* /*pWho*/) override
        {
            m_uiBanishTimer = urand(8, 12) * 1000;
            m_uiImmolateTimer = urand(18, 20) * 1000;
            m_uiShadowBoltTimer = 1000;
            m_uiCurseOfTonguesTimer = 17000;
        }

        void JustDied(Unit* /*pKiller*/) override
        {
            if (!m_pInstance)
                return;

            // Activate Phalanx and handle patrons faction
            if (Creature* pPhalanx = m_pInstance->GetSingleCreatureFromStorage(NPC_PHALANX))
            {
                if (npc_phalanx::npc_phalanxAI* pEscortAI = dynamic_cast<npc_phalanx::npc_phalanxAI*>(pPhalanx->AI()))
                    pEscortAI->Start(false, nullptr, nullptr, true);
            }
            m_pInstance->HandleBarPatrons(PATRON_HOSTILE);
            m_pInstance->SetData(TYPE_PLUGGER, IN_PROGRESS); // The event is set IN_PROGRESS even if Plugger is dead because his death triggers more actions that are part of the event
        }

        void SpellHit(Unit* pCaster, const SpellEntry* pSpell) override
        {
            if (pCaster->GetTypeId() == TYPEID_PLAYER)
            {
                if (pSpell->Id == SPELL_PICKPOCKET)
                    m_uiPickpocketTimer = 5000;
            }
        }

        // Players stole one of the ale mug/roasted boar: warn them
        void WarnThief(Player* pPlayer)
        {
            DoScriptText(aRandomYells[urand(0, 2)], m_creature);
            m_creature->SetFacingToObject(pPlayer);
        }

        // Players stole too much of the ale mug/roasted boar: attack them
        void AttackThief(Player* pPlayer)
        {
            if (pPlayer)
            {
                DoScriptText(urand(0, 1) < 1 ? YELL_AGRRO_1 : YELL_AGRRO_2, m_creature);
                m_creature->SetFacingToObject(pPlayer);
                m_creature->SetFactionTemporary(FACTION_DARK_IRON, TEMPFACTION_RESTORE_RESPAWN);
                AttackStart(pPlayer);
            }
        }

        void UpdateAI(const uint32 uiDiff) override
        {
            // Combat check
            if (m_creature->SelectHostileTarget() && m_creature->GetVictim())
            {
                if (m_uiBanishTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    {
                        if (DoCastSpellIfCan(pTarget, SPELL_BANISH) == CAST_OK)
                            m_uiBanishTimer = urand(26, 28) * 1000;
                    }
                }
                else
                    m_uiBanishTimer -= uiDiff;

                if (m_uiImmolateTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_IMMOLATE) == CAST_OK)
                        m_uiImmolateTimer = 25000;
                }
                else
                    m_uiImmolateTimer -= uiDiff;

                if (m_uiShadowBoltTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SHADOW_BOLT) == CAST_OK)
                        m_uiShadowBoltTimer = urand(36, 63) * 100;
                }
                else
                    m_uiShadowBoltTimer -= uiDiff;

                if (m_uiCurseOfTonguesTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_CURSE_OF_TONGUES, SELECT_FLAG_POWER_MANA))
                    {
                        if (DoCastSpellIfCan(pTarget, SPELL_CURSE_OF_TONGUES) == CAST_OK)
                            m_uiCurseOfTonguesTimer = urand(19, 31) * 1000;
                    }
                }
                else
                    m_uiCurseOfTonguesTimer -= uiDiff;

                DoMeleeAttackIfReady();
            }
            // Out of Combat (OOC)
            else
            {
                if (m_uiOocSayTimer < uiDiff)
                {
                    DoScriptText(aRandomSays[urand(0, 3)], m_creature);
                    m_uiOocSayTimer = urand(10, 20) * 1000;
                }
                else
                    m_uiOocSayTimer -= uiDiff;

                if (m_uiPickpocketTimer)
                {
                    if (m_uiPickpocketTimer < uiDiff)
                    {
                        DoScriptText(YELL_PICKPOCKETED, m_creature);
                        m_creature->SetFactionTemporary(FACTION_DARK_IRON, TEMPFACTION_RESTORE_RESPAWN);
                        m_uiPickpocketTimer = 0;
                    }
                    else
                        m_uiPickpocketTimer -= uiDiff;
                }

                if (m_uiDemonArmorTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_DEMON_ARMOR) == CAST_OK)
                        m_uiDemonArmorTimer = 30 * MINUTE * IN_MILLISECONDS;
                }
                else
                    m_uiDemonArmorTimer -= uiDiff;
            }
        }
    };



};


/*######
## npc_hurley_blackbreath
######*/

enum
{
    YELL_HURLEY_SPAWN = -1230041,
    SAY_HURLEY_AGGRO = -1230042,

    // SPELL_DRUNKEN_RAGE      = 14872,
    SPELL_FLAME_BREATH = 9573,

    NPC_RIBBLY_SCREWSPIGOT = 9543,
    NPC_RIBBLY_CRONY = 10043,
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
                m_uiEventTimer = 1000;

            bIsEnraged = false;
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
            uiFlameBreathTimer = 7000;
            bIsEnraged = false;
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
## go_bar_ale_mug
######*/
class go_bar_ale_mug : public GameObjectScript
{
public:
    go_bar_ale_mug() : GameObjectScript("go_bar_ale_mug") { }

    bool OnGameObjectUse(Player* pPlayer, GameObject* pGo) override
    {
        if (ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData())
        {
            if (pInstance->GetData(TYPE_PLUGGER) == IN_PROGRESS || pInstance->GetData(TYPE_PLUGGER) == DONE) // GOs despawning on use, this check should never be true but this is proper to have it there
                return false;
            if (Creature* pPlugger = pInstance->GetSingleCreatureFromStorage(NPC_PLUGGER_SPAZZRING))
            {
                if (boss_plugger_spazzring::boss_plugger_spazzringAI* pPluggerAI = dynamic_cast<boss_plugger_spazzring::boss_plugger_spazzringAI*>(pPlugger->AI()))
                {
                    // Every time we set the event to SPECIAL, the instance script increments the number of stolen mugs/boars, capping at 3
                    pInstance->SetData(TYPE_PLUGGER, SPECIAL);
                    // If the cap is reached the instance script changes the type from SPECIAL to IN_PROGRESS
                    // Plugger then aggroes and engage players, else he just warns them
                    if (pInstance->GetData(TYPE_PLUGGER) == IN_PROGRESS)
                        pPluggerAI->AttackThief(pPlayer);
                    else
                        pPluggerAI->WarnThief(pPlayer);
                }
            }
        }
        return false;
    }



};

void AddSC_instance_blackrock_depths()
{
    new instance_blackrock_depths();
    new at_ring_of_law();
    new npc_grimstone();
    new npc_theldren_trigger();
    new npc_phalanx();
    new npc_mistress_nagmara();
    new npc_rocknot();

    new boss_plugger_spazzring();
    new go_bar_ale_mug();

}
