#include "include/sc_common.h"
#include "Policies/Singleton.h"
#include "Config/Config.h"
#include "Database/DatabaseEnv.h"
#include "Server/DBCStores.h"
#include "Globals/ObjectMgr.h"
#include "ProgressBar.h"
#include "system/system.h"
#include "ScriptDevMgr.h"
#include "include/sc_creature.h"

#ifdef BUILD_SCRIPTDEV
#include "system/ScriptLoader.h"
#endif

INSTANTIATE_SINGLETON_1(ScriptDevMgr);

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

ScriptDevMgr::ScriptDevMgr()
{
}

ScriptDevMgr::~ScriptDevMgr()
{
#define SCR_CLEAR(T) \
        FOR_SCRIPTS(T, itr, end) \
            delete itr->second; \
        SCR_REG_LST(T).clear();

    // Clear scripts for every script type.
    /*SCR_CLEAR(SpellHandlerScript);
    SCR_CLEAR(AuraHandlerScript);
    SCR_CLEAR(ServerScript);
    
    SCR_CLEAR(GroupScript);
    SCR_CLEAR(FormulaScript);
    SCR_CLEAR(WorldMapScript);
    SCR_CLEAR(InstanceMapScript);
    SCR_CLEAR(BattlegroundMapScript);
    SCR_CLEAR(AreaTriggerScript);
    SCR_CLEAR(BattlegroundScript);
    SCR_CLEAR(OutdoorPvPScript);
    SCR_CLEAR(CommandScript);
    SCR_CLEAR(WeatherScript);
    SCR_CLEAR(AuctionHouseScript);
    SCR_CLEAR(ConditionScript);
    SCR_CLEAR(DynamicObjectScript);
    SCR_CLEAR(TransportScript);*/
    SCR_CLEAR(GameObjectScript);
    SCR_CLEAR(CreatureScript);
    SCR_CLEAR(PlayerScript);
    SCR_CLEAR(WorldScript);
   

#undef SCR_CLEAR
}

/* #############################################
   #                GameObjectScripts
   #
   ############################################# */

bool ScriptDevMgr::OnGossipHello(Player* player, GameObject* go)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(go);

    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnGossipHello(player, go);
}

bool ScriptDevMgr::OnGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(go);


    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnGossipSelect(player, go, sender, action);
}

bool ScriptDevMgr::OnGossipSelectCode(Player* player, GameObject* go, uint32 sender, uint32 action, const char* code)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(go);
    MANGOS_ASSERT(code);


    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnGossipSelectCode(player, go, sender, action, code);
}

bool ScriptDevMgr::OnQuestAccept(Player* player, GameObject* go, Quest const* quest)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(go);
    MANGOS_ASSERT(quest);


    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnQuestAccept(player, go, quest);
}

bool ScriptDevMgr::OnQuestReward(Player* player, GameObject* go, Quest const* quest, uint32 opt)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(go);
    MANGOS_ASSERT(quest);

    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnQuestReward(player, go, quest, opt);
}


bool ScriptDevMgr::OnQuestComplete(Player* player, GameObject* gameobject, Quest const* quest)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(gameobject);
    MANGOS_ASSERT(quest);

    GET_SCRIPT_RET(GameObjectScript, gameobject->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnQuestComplete(player, gameobject, quest);
}

uint32 ScriptDevMgr::GetDialogStatus(Player* player, GameObject* go)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(go);


    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, 100);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnDialogStatus(player, go);
}

void ScriptDevMgr::OnGameObjectDestroyed(Player* player, GameObject* go, uint32 eventId)
{
    MANGOS_ASSERT(go);

    GET_SCRIPT(GameObjectScript, go->GetScriptId(), tmpscript);
    tmpscript->OnDestroyed(player, go, eventId);
}

void ScriptDevMgr::OnGameObjectUpdate(GameObject* go, uint32 diff)
{
    MANGOS_ASSERT(go);

    GET_SCRIPT(GameObjectScript, go->GetScriptId(), tmpscript);
    tmpscript->OnUpdate(go, diff);
}

/* #############################################
   #                CreatureScripts
   #
   ############################################# */

bool ScriptDevMgr::OnDummyEffect(Unit* caster, uint32 spellId, uint32 effIndex, Creature* target)
{
    MANGOS_ASSERT(caster);
    MANGOS_ASSERT(target);

    GET_SCRIPT_RET(CreatureScript, target->GetScriptId(), tmpscript, false);
    return tmpscript->OnDummyEffect(caster, spellId, effIndex, target);
}

bool ScriptDevMgr::OnGossipHello(Player* player, Creature* creature)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(creature);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnGossipHello(player, creature);
}

bool ScriptDevMgr::OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(creature);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
        
    return tmpscript->OnGossipSelect(player, creature, sender, action);
}

bool ScriptDevMgr::OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(creature);
    MANGOS_ASSERT(code);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnGossipSelectCode(player, creature, sender, action, code);
}

bool ScriptDevMgr::OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
{
    MANGOS_ASSERT(player);

    MANGOS_ASSERT(creature);
    MANGOS_ASSERT(quest);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnQuestAccept(player, creature, quest);
}

bool ScriptDevMgr::OnQuestSelect(Player* player, Creature* creature, Quest const* quest)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(creature);
    MANGOS_ASSERT(quest);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnQuestSelect(player, creature, quest);
}

bool ScriptDevMgr::OnQuestComplete(Player* player, Creature* creature, Quest const* quest)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(creature);
    MANGOS_ASSERT(quest);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnQuestComplete(player, creature, quest);
}

bool ScriptDevMgr::OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(creature);
    MANGOS_ASSERT(quest);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnQuestReward(player, creature, quest, opt);
}

uint32 ScriptDevMgr::GetDialogStatus(const Player* player, const Creature* creature)
{
    MANGOS_ASSERT(player);
    MANGOS_ASSERT(creature);

    // TODO: 100 is a funny magic number to have hanging around here...
    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, 100);
    player->GetPlayerMenu()->ClearMenus();
    return tmpscript->OnDialogStatus(player, creature);
}

CreatureAI* ScriptDevMgr::GetCreatureAI(Creature* creature)
{
    MANGOS_ASSERT(creature);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, NULL);
    return tmpscript->GetAI(creature);
}

void ScriptDevMgr::OnCreatureUpdate(Creature* creature, uint32 diff)
{
    MANGOS_ASSERT(creature);

    GET_SCRIPT(CreatureScript, creature->GetScriptId(), tmpscript);
    tmpscript->OnUpdate(creature, diff);
}


CreatureScript::CreatureScript(const char* name)
    : ScriptObject(name)
{
    ScriptDevMgr::ScriptRegistry<CreatureScript>::AddScript(this);
}

/* #############################################
   #                PlayerScripts
   #
   ############################################# */

void ScriptDevMgr::OnPlayerLogin(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnLogin(player);
}

void ScriptDevMgr::OnGivePlayerXP(Player* player, uint32& amount, Creature* victim)
{
    FOREACH_SCRIPT(PlayerScript)->OnGiveXP(player, amount, victim);
}

void ScriptDevMgr::OnLootMoney(Player* player, uint32 amount)
{
    FOREACH_SCRIPT(PlayerScript)->OnLootMoney(player, amount);
}

void ScriptDevMgr::OnBeforePlayerUpdate(Player* player, uint32 p_time)
{
    FOREACH_SCRIPT(PlayerScript)->OnBeforeUpdate(player, p_time);
}

void ScriptDevMgr::OnLootItem(Player* player, Item* item, uint32 count, uint64 lootGUID)
{
    FOREACH_SCRIPT(PlayerScript)->OnLootItem(player, item, count, lootGUID);
}

void ScriptDevMgr::OnCreateItem(Player* player, Item* item, uint32 count)
{
    FOREACH_SCRIPT(PlayerScript)->OnCreateItem(player, item, count);
}

void ScriptDevMgr::OnQuestRewardItem(Player* player, Item* item, uint32 count)
{
    FOREACH_SCRIPT(PlayerScript)->OnQuestRewardItem(player, item, count);
}

void ScriptDevMgr::OnPlayerLoadFromDB(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerLoadFromDB(player);
}

void ScriptDevMgr::OnPVPKill(Player* killer, Player* killed)
{
    FOREACH_SCRIPT(PlayerScript)->OnPVPKill(killer, killed);
}

void ScriptDevMgr::OnCreatureKill(Player* killer, Creature* killed)
{
    FOREACH_SCRIPT(PlayerScript)->OnCreatureKill(killer, killed);
}

void ScriptDevMgr::OnPlayerKilledByCreature(Creature* killer, Player* killed)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerKilledByCreature(killer, killed);
}

void ScriptDevMgr::OnPlayerLevelChanged(Player* player, uint8 oldLevel, uint8 newLevel)
{
    FOREACH_SCRIPT(PlayerScript)->OnLevelChanged(player, oldLevel, newLevel);
}

void ScriptDevMgr::OnPlayerFreeTalentPointsChanged(Player* player, uint32 points)
{
    FOREACH_SCRIPT(PlayerScript)->OnFreeTalentPointsChanged(player, points);
}

void ScriptDevMgr::OnPlayerTalentsReset(Player* player, bool noCost)
{
    FOREACH_SCRIPT(PlayerScript)->OnTalentsReset(player, noCost);
}

void ScriptDevMgr::OnPlayerMoneyChanged(Player* player, int32& amount)
{
    FOREACH_SCRIPT(PlayerScript)->OnMoneyChanged(player, amount);
}

void ScriptDevMgr::OnPlayerMoneyLimit(Player* player, int32 amount)
{
    FOREACH_SCRIPT(PlayerScript)->OnMoneyLimit(player, amount);
}

void ScriptDevMgr::OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental)
{
    FOREACH_SCRIPT(PlayerScript)->OnReputationChange(player, factionID, standing, incremental);
}

void ScriptDevMgr::OnPlayerDuelRequest(Player* target, Player* challenger)
{
    FOREACH_SCRIPT(PlayerScript)->OnDuelRequest(target, challenger);
}

void ScriptDevMgr::OnPlayerDuelStart(Player* player1, Player* player2)
{
    FOREACH_SCRIPT(PlayerScript)->OnDuelStart(player1, player2);
}

void ScriptDevMgr::OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type)
{
    FOREACH_SCRIPT(PlayerScript)->OnDuelEnd(winner, loser, type);
}

void ScriptDevMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg)
{
    FOREACH_SCRIPT(PlayerScript)->OnChat(player, type, lang, msg);
}

void ScriptDevMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver)
{
    FOREACH_SCRIPT(PlayerScript)->OnChat(player, type, lang, msg, receiver);
}

void ScriptDevMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group)
{
    FOREACH_SCRIPT(PlayerScript)->OnChat(player, type, lang, msg, group);
}

void ScriptDevMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild)
{
    FOREACH_SCRIPT(PlayerScript)->OnChat(player, type, lang, msg, guild);
}

void ScriptDevMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Channel* channel)
{
    FOREACH_SCRIPT(PlayerScript)->OnChat(player, type, lang, msg, channel);
}

void ScriptDevMgr::OnPlayerEmote(Player* player, uint32 emote)
{
    FOREACH_SCRIPT(PlayerScript)->OnEmote(player, emote);
}

void ScriptDevMgr::OnPlayerTextEmote(Player* player, uint32 textEmote, uint32 emoteNum, ObjectGuid guid)
{
    FOREACH_SCRIPT(PlayerScript)->OnTextEmote(player, textEmote, emoteNum, guid);
}

void ScriptDevMgr::OnPlayerSpellCast(Player* player, Spell* spell, bool skipCheck)
{
    FOREACH_SCRIPT(PlayerScript)->OnSpellCast(player, spell, skipCheck);
}

void ScriptDevMgr::OnPlayerCompleteQuest(Player* player, Quest const* quest)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerCompleteQuest(player, quest);
}

void ScriptDevMgr::OnPlayerLogout(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnLogout(player);
}

void ScriptDevMgr::OnPlayerCreate(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnCreate(player);
}

void ScriptDevMgr::OnPlayerDelete(ObjectGuid guid, uint32 accountId)
{
    FOREACH_SCRIPT(PlayerScript)->OnDelete(guid, accountId);
}

void ScriptDevMgr::OnPlayerFailedDelete(ObjectGuid guid, uint32 accountId)
{
    FOREACH_SCRIPT(PlayerScript)->OnFailedDelete(guid, accountId);
}

void ScriptDevMgr::OnPlayerSave(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnSave(player);
}

void ScriptDevMgr::OnPlayerBindToInstance(Player* player, Difficulty difficulty, uint32 mapid, bool permanent)
{
    FOREACH_SCRIPT(PlayerScript)->OnBindToInstance(player, difficulty, mapid, permanent);
}

void ScriptDevMgr::OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 newArea)
{
    FOREACH_SCRIPT(PlayerScript)->OnUpdateZone(player, newZone, newArea);
}

void ScriptDevMgr::OnQuestStatusChange(Player* player, uint32 questId)
{
    FOREACH_SCRIPT(PlayerScript)->OnQuestStatusChange(player, questId);
}

void ScriptDevMgr::OnGossipSelect(Player* player, uint32 menu_id, uint32 sender, uint32 action)
{
    FOREACH_SCRIPT(PlayerScript)->OnGossipSelect(player, menu_id, sender, action);
}

void ScriptDevMgr::OnGossipSelectCode(Player* player, uint32 menu_id, uint32 sender, uint32 action, const char* code)
{
    FOREACH_SCRIPT(PlayerScript)->OnGossipSelectCode(player, menu_id, sender, action, code);
}

void ScriptDevMgr::OnPlayerRepop(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerRepop(player);
}

void ScriptDevMgr::OnQuestObjectiveProgress(Player* player, Quest const* quest, uint32 objectiveIndex, uint16 progress)
{
    FOREACH_SCRIPT(PlayerScript)->OnQuestObjectiveProgress(player, quest, objectiveIndex, progress);
}

PlayerScript::PlayerScript(const char* name)
    : ScriptObject(name)
{
    ScriptDevMgr::ScriptRegistry<PlayerScript>::AddScript(this);
}


/* #############################################
   #                WorldScript
   #
   ############################################# */

void ScriptDevMgr::OnLoadCustomDatabaseTable()
{
    FOREACH_SCRIPT(WorldScript)->OnLoadCustomDatabaseTable();
}

void ScriptDevMgr::OnOpenStateChange(bool open)
{
    FOREACH_SCRIPT(WorldScript)->OnOpenStateChange(open);
}

void ScriptDevMgr::OnConfigLoad(bool reload)
{
    FOREACH_SCRIPT(WorldScript)->OnConfigLoad(reload);
}

void ScriptDevMgr::OnMotdChange(std::string& newMotd)
{
    FOREACH_SCRIPT(WorldScript)->OnMotdChange(newMotd);
}

void ScriptDevMgr::OnStartup()
{
    FOREACH_SCRIPT(WorldScript)->OnStartup();
}

void ScriptDevMgr::OnShutdown(ShutdownExitCode code, ShutdownMask mask)
{
    FOREACH_SCRIPT(WorldScript)->OnShutdown(code, mask);
}

void ScriptDevMgr::OnShutdownCancel()
{
    FOREACH_SCRIPT(WorldScript)->OnShutdownCancel();
}

void ScriptDevMgr::OnWorldUpdate(uint32 diff)
{
    FOREACH_SCRIPT(WorldScript)->OnUpdate(NULL, diff);
}

WorldScript::WorldScript(const char* name)
    : ScriptObject(name)
{
    ScriptDevMgr::ScriptRegistry<WorldScript>::AddScript(this);
}

template<class TScript>
void ScriptDevMgr::ScriptRegistry<TScript>::AddScript(TScript* const script)
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

// Instantiate static members of ScriptMgr::ScriptRegistry.
template<class TScript> std::map<uint32, TScript*> ScriptDevMgr::ScriptRegistry<TScript>::ScriptPointerList;
template<class TScript> uint32 ScriptDevMgr::ScriptRegistry<TScript>::_scriptIdCounter;

// Specialize for each script type class like so:
template class ScriptDevMgr::ScriptRegistry<CreatureScript>;
template class ScriptDevMgr::ScriptRegistry<GameObjectScript>;
template class ScriptDevMgr::ScriptRegistry<PlayerScript>;
template class ScriptDevMgr::ScriptRegistry<WorldScript>;
/*template class ScriptDevMgr::ScriptRegistry<SpellHandlerScript>;
template class ScriptDevMgr::ScriptRegistry<AuraHandlerScript>;
template class ScriptDevMgr::ScriptRegistry<ServerScript>;
template class ScriptDevMgr::ScriptRegistry<GroupScript>;
template class ScriptDevMgr::ScriptRegistry<FormulaScript>;
template class ScriptDevMgr::ScriptRegistry<WorldMapScript>;
template class ScriptDevMgr::ScriptRegistry<InstanceMapScript>;
template class ScriptDevMgr::ScriptRegistry<BattlegroundMapScript>;
template class ScriptDevMgr::ScriptRegistry<ItemScript>;
template class ScriptDevMgr::ScriptRegistry<AreaTriggerScript>;
template class ScriptDevMgr::ScriptRegistry<BattlegroundScript>;
template class ScriptDevMgr::ScriptRegistry<OutdoorPvPScript>;
template class ScriptDevMgr::ScriptRegistry<CommandScript>;
template class ScriptDevMgr::ScriptRegistry<WeatherScript>;
template class ScriptDevMgr::ScriptRegistry<AuctionHouseScript>;
template class ScriptDevMgr::ScriptRegistry<ConditionScript>;
template class ScriptDevMgr::ScriptRegistry<DynamicObjectScript>;
template class ScriptDevMgr::ScriptRegistry<TransportScript>;
template class ScriptDevMgr::ScriptRegistry<MovementHandlerScript>;
template class ScriptDevMgr::ScriptRegistry<BGScript>; */


// Undefine utility macros.
#undef GET_SCRIPT_RET
#undef GET_SCRIPT
#undef FOREACH_SCRIPT
#undef FOR_SCRIPTS_RET
#undef FOR_SCRIPTS
#undef SCR_REG_LST
#undef SCR_REG_MAP