/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#include "include/sc_common.h"
#include "Policies/Singleton.h"
#include "Config/Config.h"
#include "Database/DatabaseEnv.h"
#include "Server/DBCStores.h"
#include "Globals/ObjectMgr.h"
#include "ProgressBar.h"
#include "system/system.h"
#include "ScriptDevAIMgr.h"
#include "include/sc_creature.h"

#ifdef BUILD_SCRIPTDEV
#include "system/ScriptLoader.h"
#endif

INSTANTIATE_SINGLETON_1(ScriptDevAIMgr);

// Utility macros to refer to the script registry.
#define SCR_REG_MAP(T) ScriptRegistry<T>::ScriptMap
#define SCR_REG_LST(T) ScriptRegistry<T>::ScriptPointerList

// Utility macros for looping over scripts.
#define FOR_SCRIPTS(T,C,E) \
    if (SCR_REG_LST(T).empty()) \
        return; \
    for (SCR_REG_MAP(T)::iterator C = SCR_REG_LST(T).begin(); \
        C != SCR_REG_LST(T).end(); ++C)
#define FOR_SCRIPTS_RET(T,C,E,R) \
    if (SCR_REG_LST(T).empty()) \
        return R; \
    for (SCR_REG_MAP(T)::iterator C = SCR_REG_LST(T).begin(); \
        C != SCR_REG_LST(T).end(); ++C)
#define FOREACH_SCRIPT(T) \
    FOR_SCRIPTS(T, itr, end) \
    itr->second

// Utility macros for finding specific scripts.
#define GET_SCRIPT(T,I,V) \
    T* V = ScriptRegistry<T>::GetScriptById(I); \
    if (!V) \
        return;
#define GET_SCRIPT_RET(T,I,V,R) \
    T* V = ScriptRegistry<T>::GetScriptById(I); \
    if (!V) \
        return R;

void FillSpellSummary();

void LoadDatabase()
{
    // Load content
    // pSystemMgr.LoadVersion(); // currently we are not checking for version; function to be completely removed in the future
    pSystemMgr.LoadScriptTexts();
    pSystemMgr.LoadScriptTextsCustom();
    pSystemMgr.LoadScriptGossipTexts();
    pSystemMgr.LoadScriptWaypoints();
}

//*********************************
//*** Functions used globally ***

/**
* Function that does script text
*
* @param iTextEntry Entry of the text, stored in SD2-database
* @param pSource Source of the text
* @param pTarget Can be nullptr (depending on CHAT_TYPE of iTextEntry). Possible target for the text
*/
void DoScriptText(int32 iTextEntry, WorldObject* pSource, Unit* pTarget)
{
    if (!pSource)
    {
        script_error_log("DoScriptText entry %i, invalid Source pointer.", iTextEntry);
        return;
    }

    if (iTextEntry >= 0)
    {
        script_error_log("DoScriptText with source entry %u (TypeId=%u, guid=%u) attempts to process text entry %i, but text entry must be negative.",
                         pSource->GetEntry(), pSource->GetTypeId(), pSource->GetGUIDLow(), iTextEntry);

        return;
    }

    DoDisplayText(pSource, iTextEntry, pTarget);
    // TODO - maybe add some call-stack like error output if above function returns false
}

/**
* Function that either simulates or does script text for a map
*
* @param iTextEntry Entry of the text, stored in SD2-database, only type CHAT_TYPE_ZONE_YELL supported
* @param uiCreatureEntry Id of the creature of whom saying will be simulated
* @param pMap Given Map on which the map-wide text is displayed
* @param pCreatureSource Can be nullptr. If pointer to Creature is given, then the creature does the map-wide text
* @param pTarget Can be nullptr. Possible target for the text
*/
void DoOrSimulateScriptTextForMap(int32 iTextEntry, uint32 uiCreatureEntry, Map* pMap, Creature* pCreatureSource /*=nullptr*/, Unit* pTarget /*=nullptr*/)
{
    if (!pMap)
    {
        script_error_log("DoOrSimulateScriptTextForMap entry %i, invalid Map pointer.", iTextEntry);
        return;
    }

    if (iTextEntry >= 0)
    {
        script_error_log("DoOrSimulateScriptTextForMap with source entry %u for map %u attempts to process text entry %i, but text entry must be negative.", uiCreatureEntry, pMap->GetId(), iTextEntry);
        return;
    }

    CreatureInfo const* pInfo = GetCreatureTemplateStore(uiCreatureEntry);
    if (!pInfo)
    {
        script_error_log("DoOrSimulateScriptTextForMap has invalid source entry %u for map %u.", uiCreatureEntry, pMap->GetId());
        return;
    }

    MangosStringLocale const* pData = GetMangosStringData(iTextEntry);
    if (!pData)
    {
        script_error_log("DoOrSimulateScriptTextForMap with source entry %u for map %u could not find text entry %i.", uiCreatureEntry, pMap->GetId(), iTextEntry);
        return;
    }

    debug_log("SD2: DoOrSimulateScriptTextForMap: text entry=%i, Sound=%u, Type=%u, Language=%u, Emote=%u",
              iTextEntry, pData->SoundId, pData->Type, pData->LanguageId, pData->Emote);

    if (pData->Type != CHAT_TYPE_ZONE_YELL && pData->Type != CHAT_TYPE_ZONE_EMOTE)
    {
        script_error_log("DoSimulateScriptTextForMap entry %i has not supported chat type %u.", iTextEntry, pData->Type);
        return;
    }

    if (pData->SoundId)
        pMap->PlayDirectSoundToMap(pData->SoundId);

    ChatMsg chatMsg = (pData->Type == CHAT_TYPE_ZONE_EMOTE ? CHAT_MSG_MONSTER_EMOTE : CHAT_MSG_MONSTER_YELL);

    if (pCreatureSource)                                // If provided pointer for sayer, use direct version
        pMap->MonsterYellToMap(pCreatureSource->GetObjectGuid(), iTextEntry, chatMsg, pData->LanguageId, pTarget);
    else                                                // Simulate yell
        pMap->MonsterYellToMap(pInfo, iTextEntry, chatMsg, pData->LanguageId, pTarget);
}

//*********************************
//*** Functions used internally ***

void Script::RegisterSelf(bool bReportError)
{
    if (uint32 id = sScriptDevAIMgr.GetScriptId(Name.c_str()))
        sScriptDevAIMgr.AddScript(id, this);
    else
    {
        if (bReportError)
            script_error_log("Script registering but ScriptName %s is not assigned in database. Script will not be used.", Name.c_str());

        delete this;
    }
}

//********************************
//*** Functions to be Exported ***
ScriptDevAIMgr::~ScriptDevAIMgr()
{
#define SCR_CLEAR(T) \
        FOR_SCRIPTS(T, itr, end) \
            delete itr->second; \
        SCR_REG_LST(T).clear();

    SCR_CLEAR(PlayerScript);
    SCR_CLEAR(ItemScript);
    SCR_CLEAR(GameObjectScript);
    SCR_CLEAR(CreatureScript);
    SCR_CLEAR(AreaTriggerScript);
    SCR_CLEAR(InstanceMapScript);
    SCR_CLEAR(SpellAuraScript);
    SCR_CLEAR(ObjectScript);

#undef SCR_CLEAR
}

void ScriptDevAIMgr::AddScript(uint32 id, Script* script)
{
    if (!id)
        return;

    m_scripts[id] = script;
    ++num_sc_scripts;
}

Script* ScriptDevAIMgr::GetScript(uint32 id) const
{
#ifdef BUILD_SCRIPTDEV
    if (!id || id < m_scripts.size())
        return m_scripts[id];
#endif
    return nullptr;
}

void ScriptDevAIMgr::Initialize()
{
#ifdef BUILD_SCRIPTDEV
    // ScriptDev startup
    outstring_log("  _____           _       _   _____ ");
    outstring_log(" / ____|         (_)     | | |  __ \\");
    outstring_log("| (___   ___ _ __ _ _ __ | |_| |  | | _____   __");
    outstring_log(" \\___ \\ / __| '__| | '_ \\| __| |  | |/ _ \\ \\ / /");
    outstring_log(" ____) | (__| |  | | |_) | |_| |__| |  __/\\ V / ");
    outstring_log("|_____/ \\___|_|  |_| .__/ \\__|_____/ \\___| \\_/ ");
    outstring_log("                   | |");
    outstring_log("                   |_| http://www.cmangos.net");



    // Load database (must be called after SD2Config.SetSource).
    LoadDatabase();

    outstring_log("SD2: Loading C++ scripts\n");
    BarGoLink bar(1);
    bar.step();

    // Resize script ids to needed amount of assigned ScriptNames (from core)
    m_scripts.resize(GetScriptIdsCount(), nullptr);

    FillSpellSummary();

    AddScripts();

    // Check existence scripts for all registered by core script names
    for (uint32 i = 1; i < GetScriptIdsCount(); ++i)
    {
        if (!m_scripts[i])
            script_error_log("No script found for ScriptName '%s'.", GetScriptName(i));
    }

    outstring_log(">> Loaded %i C++ Scripts.", num_sc_scripts);
#else
    outstring_log(">> ScriptDev is disabled!\n");
#endif
}

void ScriptDevAIMgr::LoadScriptNames()
{
    m_scriptNames.push_back("");
    QueryResult* result = WorldDatabase.Query(
                              "SELECT DISTINCT(ScriptName) FROM creature_template WHERE ScriptName <> '' "
                              "UNION "
                              "SELECT DISTINCT(ScriptName) FROM gameobject_template WHERE ScriptName <> '' "
                              "UNION "
                              "SELECT DISTINCT(ScriptName) FROM item_template WHERE ScriptName <> '' "
                              "UNION "
                              "SELECT DISTINCT(ScriptName) FROM scripted_areatrigger WHERE ScriptName <> '' "
                              "UNION "
                              "SELECT DISTINCT(ScriptName) FROM scripted_event_id WHERE ScriptName <> '' "
                              "UNION "
                              "SELECT DISTINCT(ScriptName) FROM instance_template WHERE ScriptName <> '' "
                              "UNION "
                              "SELECT DISTINCT(ScriptName) FROM world_template WHERE ScriptName <> ''");

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outErrorDb(">> Loaded empty set of Script Names!");
        sLog.outString();
        return;
    }

    BarGoLink bar(result->GetRowCount());
    uint32 count = 0;

    do
    {
        bar.step();
        m_scriptNames.push_back((*result)[0].GetString());
        ++count;
    }
    while (result->NextRow());
    delete result;

    std::sort(m_scriptNames.begin(), m_scriptNames.end());

    sLog.outString(">> Loaded %d Script Names", count);
    sLog.outString();
}

uint32 ScriptDevAIMgr::GetScriptId(const char* name) const
{
    // use binary search to find the script name in the sorted vector
    // assume "" is the first element
    if (!name)
        return 0;

    ScriptNameMap::const_iterator itr = std::lower_bound(m_scriptNames.begin(), m_scriptNames.end(), name);

    if (itr == m_scriptNames.end() || *itr != name)
        return 0;

    return uint32(itr - m_scriptNames.begin());
}

void ScriptDevAIMgr::LoadAreaTriggerScripts()
{
    m_AreaTriggerScripts.clear();                           // need for reload case
    QueryResult* result = WorldDatabase.Query("SELECT entry, ScriptName FROM scripted_areatrigger");

    uint32 count = 0;

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded %u scripted areatrigger", count);
        sLog.outString();
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        ++count;
        bar.step();

        Field* fields = result->Fetch();

        uint32 triggerId = fields[0].GetUInt32();
        const char* scriptName = fields[1].GetString();

        if (!sAreaTriggerStore.LookupEntry(triggerId))
        {
            sLog.outErrorDb("Table `scripted_areatrigger` has area trigger (ID: %u) not listed in `AreaTrigger.dbc`.", triggerId);
            continue;
        }

        m_AreaTriggerScripts[triggerId] = GetScriptId(scriptName);
    }
    while (result->NextRow());

    delete result;

    sLog.outString(">> Loaded %u areatrigger scripts", count);
    sLog.outString();

}

void ScriptDevAIMgr::LoadEventIdScripts()
{
    m_EventIdScripts.clear();                           // need for reload case
    QueryResult* result = WorldDatabase.Query("SELECT id, ScriptName FROM scripted_event_id");

    uint32 count = 0;

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded %u scripted event id", count);
        sLog.outString();
        return;
    }

    BarGoLink bar(result->GetRowCount());

    std::set<uint32> eventIds;                              // Store possible event ids
    ScriptMgr::CollectPossibleEventIds(eventIds);

    do
    {
        ++count;
        bar.step();

        Field* fields = result->Fetch();

        uint32 eventId = fields[0].GetUInt32();
        const char* scriptName = fields[1].GetString();

        std::set<uint32>::const_iterator itr = eventIds.find(eventId);
        if (itr == eventIds.end())
            sLog.outErrorDb("Table `scripted_event_id` has id %u not referring to any gameobject_template type 10 data2 field, type 3 data6 field, type 13 data 2 field, type 29 or any spell effect %u or path taxi node data",
                            eventId, SPELL_EFFECT_SEND_EVENT);

        m_EventIdScripts[eventId] = GetScriptId(scriptName);
    }
    while (result->NextRow());

    delete result;

    sLog.outString(">> Loaded %u scripted event id", count);
    sLog.outString();
}

uint32 ScriptDevAIMgr::GetAreaTriggerScriptId(uint32 triggerId) const
{
    AreaTriggerScriptMap::const_iterator itr = m_AreaTriggerScripts.find(triggerId);
    if (itr != m_AreaTriggerScripts.end())
        return itr->second;

    return 0;
}

uint32 ScriptDevAIMgr::GetEventIdScriptId(uint32 eventId) const
{
    EventIdScriptMap::const_iterator itr = m_EventIdScripts.find(eventId);
    if (itr != m_EventIdScripts.end())
        return itr->second;

    return 0;
}

template<class TScript>
void ScriptDevAIMgr::ScriptRegistry<TScript>::AddScript(TScript* const script)
{
    MANGOS_ASSERT(script);

    // See if the script is using the same memory as another script. If this happens, it means that
    // someone forgot to allocate new memory for a script.
    typedef typename ScriptMap::iterator ScriptMapIterator;
    for (ScriptMapIterator it = ScriptPointerList.begin(); it != ScriptPointerList.end(); ++it)
    {
        if (it->second == script)
        {
            sLog.outError("Script '%s' forgot to allocate memory, so this script and/or the script before that can't work.",
                script->ToString());

            return;
        }
    }
    // Get an ID for the script. An ID only exists if it's a script that is assigned in the database
    // through a script name (or similar).
    uint32 id = sScriptDevAIMgr.GetScriptId(script->ToString());
    if (id)
    {
        // Try to find an existing script.
        bool existing = false;
        typedef typename ScriptMap::iterator ScriptMapIterator;
        for (ScriptMapIterator it = ScriptPointerList.begin(); it != ScriptPointerList.end(); ++it)
        {
            // If the script names match...
            if (it->second->GetName() == script->GetName())
            {
                // ... It exists.
                existing = true;
                break;
            }
        }

        // If the script isn't assigned -> assign it!
        if (!existing)
        {
            ScriptPointerList[id] = script;
        }
        else
        {
            // If the script is already assigned -> delete it!
            sLog.outError("Script '%s' already assigned with the same script name, so the script can't work.",
                script->ToString());

            delete script;
        }
    }
    else if (script->IsDatabaseBound())
    {
        // The script uses a script name from database, but isn't assigned to anything.
        if (script->GetName().find("example") == std::string::npos)
            sLog.outErrorDb("Script named '%s' does not have a script name assigned in database.",
                script->ToString());

        delete script;
    }
    else
    {
        // We're dealing with a code-only script; just add it.
        ScriptPointerList[_scriptIdCounter++] = script;
    }
}

/*#################################################
#                 GameObjectScript
#
###################################################*/
GameObjectScript::GameObjectScript(const char* name)
    : ScriptObject(name)
{
    ScriptDevAIMgr::ScriptRegistry<GameObjectScript>::AddScript(this);
}

bool ScriptDevAIMgr::OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, GameObject* pTarget, ObjectGuid originalCasterGuid)
{
    MANGOS_ASSERT(pCaster);
    MANGOS_ASSERT(pTarget);

    GET_SCRIPT_RET(GameObjectScript, pTarget->GetScriptId(), tmpscript, false);
    return tmpscript->OnEffectDummy(pCaster, spellId, effIndex, pTarget, originalCasterGuid);
}

std::function<bool(Unit*)>* ScriptDevAIMgr::OnTrapSearch(GameObject* go)
{
    MANGOS_ASSERT(go);

    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, nullptr);
    return tmpscript->OnTrapSearch(go);
}

GameObjectAI* ScriptDevAIMgr::GetGameObjectAI(GameObject* gameobject) const
{
    MANGOS_ASSERT(gameobject);

    GET_SCRIPT_RET(GameObjectScript, gameobject->GetScriptId(), tmpscript, NULL);
    return tmpscript->GetGameObjectAI(gameobject);
}

bool ScriptDevAIMgr::OnGameObjectUse(Player* pPlayer, GameObject* pGo)
{
    MANGOS_ASSERT(pPlayer);
    MANGOS_ASSERT(pGo);

    GET_SCRIPT_RET(GameObjectScript, pGo->GetScriptId(), tmpscript, false);
    return tmpscript->OnGameObjectUse(pPlayer, pGo);
}

bool ScriptDevAIMgr::OnQuestRewarded(Player* pPlayer, GameObject* pGo, Quest const* pQuest)
{
    MANGOS_ASSERT(pPlayer);
    MANGOS_ASSERT(pGo);
    MANGOS_ASSERT(pQuest);

    GET_SCRIPT_RET(GameObjectScript, pGo->GetScriptId(), tmpscript, false);
    pPlayer->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestReward(pPlayer, pGo, pQuest);
}

bool ScriptDevAIMgr::OnQuestAccept(Player* player, GameObject* go, Quest const* quest)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(go);
    MANGOS_ASSERT(quest);

    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestAccept(player, go, quest);
}

bool ScriptDevAIMgr::OnGossipHello(Player* pPlayer, GameObject* pGo)
{
    MANGOS_ASSERT(pPlayer);
    MANGOS_ASSERT(pGo);

    GET_SCRIPT_RET(GameObjectScript, pGo->GetScriptId(), tmpscript, false);
    pPlayer->PlayerTalkClass->ClearMenus();
    return tmpscript->OnGossipHello(pPlayer, pGo);
}

bool ScriptDevAIMgr::OnGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(go);

    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnGossipSelect(player, go, sender, action);
}

bool ScriptDevAIMgr::OnGossipSelectCode(Player* player, GameObject* go, uint32 sender, uint32 action, const char* code)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(go);
    MANGOS_ASSERT(code);

    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnGossipSelectCode(player, go, sender, action, code);
}

uint32 ScriptDevAIMgr::GetDialogStatus(const Player* player, const GameObject* go) const
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(go);
    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, 100);

    if (!tmpscript)
        return DIALOG_STATUS_UNDEFINED;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnDialogStatus(player, go);
}

/*#################################################
#                 CreatureScript
#
###################################################*/
CreatureScript::CreatureScript(const char* name)
    : ScriptObject(name)
{
    ScriptDevAIMgr::ScriptRegistry<CreatureScript>::AddScript(this);
}

bool ScriptDevAIMgr::OnEffectScriptEffect(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Creature* pTarget, ObjectGuid originalCasterGuid)
{
    MANGOS_ASSERT(pTarget);
    MANGOS_ASSERT(pCaster);

    GET_SCRIPT_RET(CreatureScript, pTarget->GetScriptId(), tmpscript, false);
    return tmpscript->OnEffectScriptEffect(pCaster, spellId, effIndex, pTarget, originalCasterGuid);
}

bool ScriptDevAIMgr::OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Creature* pTarget, ObjectGuid originalCasterGuid)
{
    MANGOS_ASSERT(pCaster);
    MANGOS_ASSERT(pTarget);

    GET_SCRIPT_RET(CreatureScript, pTarget->GetScriptId(), tmpscript, false);
    return tmpscript->OnEffectDummy(pCaster, spellId, effIndex, pTarget, originalCasterGuid);
}

UnitAI* ScriptDevAIMgr::GetCreatureAI(Creature* pCreature) const
{
    MANGOS_ASSERT(pCreature);

    GET_SCRIPT_RET(CreatureScript, pCreature->GetScriptId(), tmpscript, NULL);
    return tmpscript->GetAI(pCreature);
}

bool ScriptDevAIMgr::OnQuestRewarded(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    MANGOS_ASSERT(pPlayer);
    MANGOS_ASSERT(pCreature);
    MANGOS_ASSERT(pQuest);

    GET_SCRIPT_RET(CreatureScript, pCreature->GetScriptId(), tmpscript, false);
    pPlayer->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestReward(pPlayer, pCreature, pQuest);
}

bool ScriptDevAIMgr::OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(creature);
    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnGossipSelect(player, creature, sender, action);
}

bool ScriptDevAIMgr::OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(creature);
    MANGOS_ASSERT(code);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnGossipSelectCode(player, creature, sender, action, code);
}

bool ScriptDevAIMgr::OnGossipHello(Player* pPlayer, Creature* pCreature)
{
    MANGOS_ASSERT(pPlayer);
    MANGOS_ASSERT(pCreature);

    GET_SCRIPT_RET(CreatureScript, pCreature->GetScriptId(), tempscript, false);
    pPlayer->PlayerTalkClass->ClearMenus();

    return tempscript->OnGossipHello(pPlayer, pCreature);
}

uint32 ScriptDevAIMgr::GetDialogStatus(const Player* pPlayer, const Creature* pCreature) const
{
    MANGOS_ASSERT(pPlayer);
    MANGOS_ASSERT(pCreature);

    // TODO: 100 is a funny magic number to have hanging around here...
    GET_SCRIPT_RET(CreatureScript, pCreature->GetScriptId(), tmpscript, DIALOG_STATUS_UNDEFINED);
    pPlayer->PlayerTalkClass->ClearMenus();
    return tmpscript->OnDialogStatus(pPlayer, pCreature);
}

bool ScriptDevAIMgr::OnQuestAccept(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    MANGOS_ASSERT(pPlayer);
    MANGOS_ASSERT(pCreature);
    MANGOS_ASSERT(pQuest);

    GET_SCRIPT_RET(CreatureScript, pCreature->GetScriptId(), tmpscript, false);
    pPlayer->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestAccept(pPlayer, pCreature, pQuest);
}

/*#################################################
#                  PlayerScript
#
###################################################*/

PlayerScript::PlayerScript(const char* name)
    : ScriptObject(name)
{
    ScriptDevAIMgr::ScriptRegistry<PlayerScript>::AddScript(this);
}

void ScriptDevAIMgr::OnGivePlayerXP(Player* player, uint32& amount, Unit* victim)
{
    FOREACH_SCRIPT(PlayerScript)->OnGiveXP(player, amount, victim);
}

/*#################################################
#                  ItemScript
#
###################################################*/
ItemScript::ItemScript(const char* name)
    : ScriptObject(name)
{
    ScriptDevAIMgr::ScriptRegistry<ItemScript>::AddScript(this);
}

bool ScriptDevAIMgr::OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Item* pTarget, ObjectGuid originalCasterGuid)
{
    MANGOS_ASSERT(pTarget);
    MANGOS_ASSERT(pCaster);

    GET_SCRIPT_RET(ItemScript, pTarget->GetProto()->ScriptId, tmpscript, false);
    return tmpscript->OnEffectDummy(pCaster, spellId, effIndex, pTarget, originalCasterGuid);
}

bool ScriptDevAIMgr::OnQuestAccept(Player* pPlayer, Item* pItem, Quest const* pQuest)
{
    MANGOS_ASSERT(pPlayer);
    MANGOS_ASSERT(pItem);
    MANGOS_ASSERT(pQuest);

    GET_SCRIPT_RET(ItemScript, pItem->GetProto()->ScriptId, tmpscript, false);
    pPlayer->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestAccept(pPlayer, pItem, pQuest);
}

bool ScriptDevAIMgr::OnItemUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets)
{
    MANGOS_ASSERT(pPlayer);
    MANGOS_ASSERT(pItem);

    GET_SCRIPT_RET(ItemScript, pItem->GetProto()->ScriptId, tmpscript, false);
    return tmpscript->OnItemUse(pPlayer, pItem, targets);
}

bool ScriptDevAIMgr::OnItemLoot(Player* pPlayer, Item* pItem, bool apply)
{
    MANGOS_ASSERT(pPlayer);
    MANGOS_ASSERT(pItem);

    GET_SCRIPT_RET(ItemScript, pItem->GetProto()->ScriptId, tmpscript, false);
    return tmpscript->OnItemLoot(pPlayer, pItem, apply);
}

/*#################################################
#               AreaTriggerScript
#
###################################################*/

AreaTriggerScript::AreaTriggerScript(const char* name)
    : ScriptObject(name)
{
    ScriptDevAIMgr::ScriptRegistry<AreaTriggerScript>::AddScript(this);
}

bool ScriptDevAIMgr::OnAreaTrigger(Player* pPlayer, AreaTriggerEntry const* atEntry)
{
    MANGOS_ASSERT(pPlayer);
    MANGOS_ASSERT(atEntry);

    GET_SCRIPT_RET(AreaTriggerScript, sScriptDevAIMgr.GetAreaTriggerScriptId(atEntry->id), tmpscript, false);
    return tmpscript->OnTrigger(pPlayer, atEntry);
}

/*#################################################
#               InstanceMapScript
#
###################################################*/
InstanceMapScript::InstanceMapScript(const char* name, uint32 mapId)
    : ScriptObject(name), MapScript(mapId)
{
    if (GetEntry() && !GetEntry()->IsDungeon())
        sLog.outError("InstanceMapScript for map %u is invalid.", mapId);

    ScriptDevAIMgr::ScriptRegistry<InstanceMapScript>::AddScript(this);
}

InstanceData* ScriptDevAIMgr::CreateInstanceData(Map* pMap)
{
    MANGOS_ASSERT(pMap);
    GET_SCRIPT_RET(InstanceMapScript, pMap->GetScriptId(), tmpscript, NULL);
    return tmpscript->GetInstanceScript(pMap);
}

/*#################################################
#               SpellAuraScripts
#
###################################################*/
SpellAuraScript::SpellAuraScript(const char* name)
    : ScriptObject(name)
{
    ScriptDevAIMgr::ScriptRegistry<SpellAuraScript>::AddScript(this);
}

bool ScriptDevAIMgr::OnAuraDummy(Aura const* pAura, bool bApply)
{
    MANGOS_ASSERT(pAura);

    GET_SCRIPT_RET(SpellAuraScript, ((Creature*)pAura->GetTarget())->GetScriptId(), tmpscript, false);
    return tmpscript->OnAuraDummy(pAura, bApply);
}

/*#################################################
#                 ObjectScript
#
###################################################*/
ObjectScript::ObjectScript(const char* name)
    : ScriptObject(name)
{
    ScriptDevAIMgr::ScriptRegistry<ObjectScript>::AddScript(this);
}

bool ScriptDevAIMgr::OnProcessEvent(uint32 uiEventId, Object* pSource, Object* pTarget, bool bIsStart)
{
    /*Script* pTempScript = GetScript(GetEventIdScriptId(uiEventId));

    if (!pTempScript || !pTempScript->pProcessEventId)
        return false; */

    //GET_SCRIPT_RET(ObjectScript, sScriptDevAI.GetScript(GetEventIdScriptId(uiEventId)), tmpscript, false);
    GET_SCRIPT_RET(ObjectScript, ((uint32)(GetScript(GetEventIdScriptId(uiEventId)))), tmpscript, false);
    // bIsStart may be false, when event is from taxi node events (arrival=false, departure=true)
    return tmpscript->OnProcessEvent(uiEventId, pSource, pTarget, bIsStart);
}

// Instantiate static members of ScriptMgr::ScriptRegistry.
template<class TScript> std::map<uint32, TScript*> ScriptDevAIMgr::ScriptRegistry<TScript>::ScriptPointerList;
template<class TScript> uint32 ScriptDevAIMgr::ScriptRegistry<TScript>::_scriptIdCounter;

template class ScriptDevAIMgr::ScriptRegistry<PlayerScript>;
template class ScriptDevAIMgr::ScriptRegistry<CreatureScript>;
template class ScriptDevAIMgr::ScriptRegistry<GameObjectScript>;
template class ScriptDevAIMgr::ScriptRegistry<ItemScript>;
template class ScriptDevAIMgr::ScriptRegistry<AreaTriggerScript>;
template class ScriptDevAIMgr::ScriptRegistry<InstanceMapScript>;
template class ScriptDevAIMgr::ScriptRegistry<ObjectScript>;
template class ScriptDevAIMgr::ScriptRegistry<SpellAuraScript>;

// Undefine utility macros.
#undef GET_SCRIPT_RET
#undef GET_SCRIPT
#undef FOREACH_SCRIPT
#undef FOR_SCRIPTS_RET
#undef FOR_SCRIPTS
#undef SCR_REG_LST
#undef SCR_REG_MAP