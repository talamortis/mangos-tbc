#ifndef SC_SCRIPTDEVMGR_H
#define SC_SCRIPTDEVMGR_H

#include "Common.h"
#include "BattleGround.h"
#include "Chat/Chat.h"
#include "Entities/Player.h"
#include "World/World.h"
#include "Guilds/Guild.h"
#include "Guilds/GuildMgr.h"

class Player;
class BattlegroundScript;
class Creature;
class CreatureAI;
class InstanceData;
class SpellScript;
class Quest;
class Item;
class Transport;
class GameObject;
class SpellCastTargets;
class Map;
class Unit;
class WorldObject;
struct ItemPrototype;
class Spell;
class ScriptMgr;
class WorldSocket;

class ScriptObject
{
    friend class ScriptMgr;

public:

    // Called when the script is initialized. Use it to initialize any properties of the script.
    virtual void OnInitialize() { }

    // Called when the script is deleted. Use it to free memory, etc.
    virtual void OnTeardown() { }

    // Do not override this in scripts; it should be overridden by the various script type classes. It indicates
    // whether or not this script type must be assigned in the database.
    virtual bool IsDatabaseBound() const { return false; }

    const std::string& GetName() const { return _name; }

    const char* ToString() const { return _name.c_str(); }

protected:

    ScriptObject(const char* name)
        : _name(std::string(name))
    {
        // Allow the script to do startup routines.
        OnInitialize();
    }

    virtual ~ScriptObject()
    {
        // Allow the script to do cleanup routines.
        OnTeardown();
    }

private:

    const std::string _name;
};

template<class TObject> class UpdatableScript
{
protected:

    UpdatableScript()
    {
    }

public:

    virtual void OnUpdate(TObject* obj, uint32 diff) { }
};

/* #############################################
   #                PlayerScripts
   #
   #############################################*/

class PlayerScript : public ScriptObject
{
protected:
    PlayerScript(char const* name);

public:
    // Called when player completes quest
    virtual void OnPlayerCompleteQuest(Player* /*player*/, Quest const* /*quest*/) { }

    // Called when player loots money
    virtual void OnLootMoney(Player* /*player*/, uint32 /*amount*/) { }

    // Called When a player Loots an item
    virtual void OnLootItem(Player* /*player*/, Item* /*item*/, uint32 /*count*/, uint64 /*itemGUID*/) { }

    // Called when player creates an item
    virtual void OnCreateItem(Player* /*player*/, Item* /*item*/, uint32 /*count*/) { }

    // Called when player recieves item from quest reward
    virtual void OnQuestRewardItem(Player* /*player*/, Item* /*item*/, uint32 /*count*/) { }

    // Called for player::update
    virtual void OnBeforeUpdate(Player* /*player*/, uint32 /*p_time*/) { }

    // Called when a player kills another player
    virtual void OnPVPKill(Player* /*killer*/, Player* /*killed*/) { }

    // Called when a player kills a creature
    virtual void OnCreatureKill(Player* /*killer*/, Creature* /*killed*/) { }

    // Called when a player is killed by a creature
    virtual void OnPlayerKilledByCreature(Creature* /*killer*/, Player* /*killed*/) { }

    // Called when a player's level changes (after the level is applied)
    virtual void OnLevelChanged(Player* /*player*/, uint8 /*oldLevel*/, uint8 /*newLevel*/) { }

    // Called when a player's free talent points change (right before the change is applied)
    virtual void OnFreeTalentPointsChanged(Player* /*player*/, uint32 /*points*/) { }

    // Called when a player's talent points are reset (right before the reset is done)
    virtual void OnTalentsReset(Player* /*player*/, bool /*noCost*/) { }

    // Called when a player's money is modified (before the modification is done)
    virtual void OnMoneyChanged(Player* /*player*/, int32& /*amount*/) { }

    // Called when a player's money is at limit (amount = money tried to add)
    virtual void OnMoneyLimit(Player* /*player*/, int32 /*amount*/) { }

    // Called when a player gains XP (before anything is given)
    virtual void OnGiveXP(Player* /*player*/, uint32& /*amount*/, Creature* /*victim*/) { }

    // Called when a player's reputation changes (before it is actually changed)
    virtual void OnReputationChange(Player* /*player*/, uint32 /*factionId*/, int32& /*standing*/, bool /*incremental*/) { }

    // Called when a duel is requested
    virtual void OnDuelRequest(Player* /*target*/, Player* /*challenger*/) { }

    // Called when a duel starts (after 3s countdown)
    virtual void OnDuelStart(Player* /*player1*/, Player* /*player2*/) { }

    // Called when a duel ends
    virtual void OnDuelEnd(Player* /*winner*/, Player* /*loser*/, DuelCompleteType /*type*/) { }

    // The following methods are called when a player sends a chat message.
    virtual void OnChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/) { }

    virtual void OnChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Player* /*receiver*/) { }

    virtual void OnChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Group* /*group*/) { }

    virtual void OnChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Guild* /*guild*/) { }

    virtual void OnChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Channel* /*channel*/) { }

    // Both of the below are called on emote opcodes.
    virtual void OnEmote(Player* /*player*/, uint32 /*emote*/) { }

    virtual void OnTextEmote(Player* /*player*/, uint32 /*textEmote*/, uint32 /*emoteNum*/, ObjectGuid /*guid*/) { }

    // Called in Spell::Cast.
    virtual void OnSpellCast(Player* /*player*/, Spell* /*spell*/, bool /*skipCheck*/) { }

    // Called when a player logs in.
    virtual void OnLogin(Player* /*player*/) { }

    // Called when a player logs out.
    virtual void OnLogout(Player* /*player*/) { }

    // Called when a player is created.
    virtual void OnCreate(Player* /*player*/) { }

    // Called When player is loading data from DB
    virtual void OnPlayerLoadFromDB(Player* /*player*/) { }

    // Called when a player is deleted.
    virtual void OnDelete(ObjectGuid /*guid*/, uint32 /*accountId*/) { }

    // Called when a player delete failed
    virtual void OnFailedDelete(ObjectGuid /*guid*/, uint32 /*accountId*/) { }

    // Called when a player is about to be saved.
    virtual void OnSave(Player* /*player*/) { }

    // Called when a player is bound to an instance
    virtual void OnBindToInstance(Player* /*player*/, DungeonDifficulty /*difficulty*/, uint32 /*mapId*/, bool /*permanent*/, uint8 /*extendState*/) { }

    // Called when a player switches to a new zone
    virtual void OnUpdateZone(Player* /*player*/, uint32 /*newZone*/, uint32 /*newArea*/) { }

    // Called when a player changes to a new map (after moving to new map)
    virtual void OnMapChanged(Player* /*player*/) { }

    // Called when a player obtains progress on a quest's objective
    virtual void OnQuestObjectiveProgress(Player* /*player*/, Quest const* /*quest*/, uint32 /*objectiveIndex*/, uint16 /*progress*/) { }

    // Called after a player's quest status has been changed
    virtual void OnQuestStatusChange(Player* /*player*/, uint32 /*questId*/) { }

    // Called when a player presses release when he died
    virtual void OnPlayerRepop(Player* /*player*/) { }

    // Called when a player selects an option in a player gossip window
    virtual void OnGossipSelect(Player* /*player*/, uint32 /*menu_id*/, uint32 /*sender*/, uint32 /*action*/) { }

    // Called when a player selects an option in a player gossip window
    virtual void OnGossipSelectCode(Player* /*player*/, uint32 /*menu_id*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/) { }

};


class ScriptDevMgr
{

    friend class ScriptDevAImgr;
    friend class ScriptObject;

public:
    ScriptDevMgr();
    virtual ~ScriptDevMgr();

public: /* PlayerScript */
    void OnPlayerCompleteQuest(Player* player, Quest const* quest);
    void OnPVPKill(Player* killer, Player* killed);
    void OnCreatureKill(Player* killer, Creature* killed);
    void OnPlayerKilledByCreature(Creature* killer, Player* killed);
    void OnPlayerLevelChanged(Player* player, uint8 oldLevel, uint8 newLevel);
    void OnPlayerFreeTalentPointsChanged(Player* player, uint32 newPoints);
    void OnPlayerTalentsReset(Player* player, bool noCost);
    void OnPlayerMoneyChanged(Player* player, int32& amount);
    void OnPlayerMoneyLimit(Player* player, int32 amount);
    void OnGivePlayerXP(Player* player, uint32& amount, Creature* victim);
    void OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental);
    void OnPlayerDuelRequest(Player* target, Player* challenger);
    void OnPlayerDuelStart(Player* player1, Player* player2);
    void OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type);
    void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg);
    void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver);
    void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group);
    void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild);
    void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Channel* channel);
    void OnPlayerEmote(Player* player, uint32 emote);
    void OnPlayerTextEmote(Player* player, uint32 textEmote, uint32 emoteNum, ObjectGuid guid);
    void OnPlayerSpellCast(Player* player, Spell* spell, bool skipCheck);
    void OnPlayerLogin(Player* player);
    void OnPlayerLogout(Player* player);
    void OnPlayerCreate(Player* player);
    void OnPlayerLoadFromDB(Player* player);
    void OnBeforePlayerUpdate(Player* player, uint32 p_time);
    void OnLootMoney(Player* player, uint32 amount);
    void OnLootItem(Player* player, Item* item, uint32 count, uint64 lootGUID);
    void OnCreateItem(Player* player, Item* item, uint32 count);
    void OnQuestRewardItem(Player* player, Item* item, uint32 count);
    void OnPlayerDelete(ObjectGuid guid, uint32 accountId);
    void OnPlayerFailedDelete(ObjectGuid guid, uint32 accountId);
    void OnPlayerSave(Player* player);
    void OnPlayerBindToInstance(Player* player, DungeonDifficulty difficulty, uint32 mapid, bool permanent, uint8 extendState);
    void OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 newArea);
    void OnQuestObjectiveProgress(Player* player, Quest const* quest, uint32 objectiveIndex, uint16 progress);
    void OnQuestStatusChange(Player* player, uint32 questId);
    void OnPlayerRepop(Player* player);
    void OnGossipSelectCode(Player* player, uint32 menu_id, uint32 sender, uint32 action, const char* code);
    void OnGossipSelect(Player* player, uint32 menu_id, uint32 sender, uint32 action);


public: /* ScriptRegistry */

// This is the global static registry of scripts.
    template<class TScript> class ScriptRegistry
    {
        // Counter used for code-only scripts.
        static uint32 _scriptIdCounter;

    public:

        typedef std::map<uint32, TScript*> ScriptMap;
        typedef typename ScriptMap::iterator ScriptMapIterator;
        // The actual list of scripts. This will be accessed concurrently, so it must not be modified
        // after server startup.
        static ScriptMap ScriptPointerList;

        // Gets a script by its ID (assigned by ObjectMgr).
        static TScript* GetScriptById(uint32 id)
        {
            for (ScriptMapIterator it = ScriptPointerList.begin(); it != ScriptPointerList.end(); ++it)
                if (it->first == id)
                    return it->second;

            return NULL;
        }

        // Attempts to add a new script to the list.
        static void AddScript(TScript* const script);
    };
};

#define sScriptDevMgr MaNGOS::Singleton<ScriptDevMgr>::Instance()

#endif