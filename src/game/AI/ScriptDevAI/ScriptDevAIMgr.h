/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_SCRIPTMGR_H
#define SC_SCRIPTMGR_H

#include "Common.h"
#include "Server/DBCStructure.h"
#include "Server/SQLStorages.h"
#include "Spells/SpellMgr.h"

class Player;
class Creature;
class UnitAI;
class InstanceData;
class Quest;
class Item;
class GameObject;
class SpellCastTargets;
class Map;
class Unit;
class WorldObject;
class Aura;
class Object;
class ObjectGuid;
class GameObjectAI;

// *********************************************************
// ************** Some defines used globally ***************

// Basic defines
#define VISIBLE_RANGE       (166.0f)                        // MAX visible range (size of grid)
#define DEFAULT_TEXT        "<ScriptDev2 Text Entry Missing!>"

/* Escort Factions
 * TODO: find better naming and definitions.
 * N=Neutral, A=Alliance, H=Horde.
 * NEUTRAL or FRIEND = Hostility to player surroundings (not a good definition)
 * ACTIVE or PASSIVE = Hostility to environment surroundings.
 */
enum EscortFaction
{
    FACTION_ESCORT_A_NEUTRAL_PASSIVE    = 10,
    FACTION_ESCORT_H_NEUTRAL_PASSIVE    = 33,
    FACTION_ESCORT_N_NEUTRAL_PASSIVE    = 113,

    FACTION_ESCORT_A_NEUTRAL_ACTIVE     = 231,
    FACTION_ESCORT_H_NEUTRAL_ACTIVE     = 232,
    FACTION_ESCORT_N_NEUTRAL_ACTIVE     = 250,

    FACTION_ESCORT_N_FRIEND_PASSIVE     = 290,
    FACTION_ESCORT_N_FRIEND_ACTIVE      = 495,

    FACTION_ESCORT_A_PASSIVE            = 774,
    FACTION_ESCORT_H_PASSIVE            = 775,

    FACTION_ESCORT_N_ACTIVE             = 1986,
    FACTION_ESCORT_H_ACTIVE             = 2046
};

// *********************************************************
// ************* Some structures used globally *************

template <typename T>
UnitAI* GetNewAIInstance(Creature* creature)
{
    return new T(creature);
}

template <typename T>
GameObjectAI* GetNewAIInstance(GameObject* gameobject)
{
    return new T(gameobject);
}

template <typename T>
InstanceData* GetNewInstanceScript(Map* map)
{
    return new T(map);
}

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

template<class TMap> class MapScript : public UpdatableScript<TMap>
{
    MapEntry const* _mapEntry;

protected:

    MapScript(uint32 mapId)
        : _mapEntry(sMapStore.LookupEntry(mapId))
    {
        if (!_mapEntry)
            sLog.outError("Invalid MapScript for %u; no such map ID.", mapId);
    }

public:

    // Gets the MapEntry structure associated with this script. Can return NULL.
    MapEntry const* GetEntry() { return _mapEntry; }

    // Called when the map is created.
    virtual void OnCreate(TMap* map) { }

    // Called just before the map is destroyed.
    virtual void OnDestroy(TMap* map) { }

    // Called when a grid map is loaded.
    virtual void OnLoadGridMap(TMap* map, uint32 gx, uint32 gy) { }

    // Called when a grid map is unloaded.
    virtual void OnUnloadGridMap(TMap* map, uint32 gx, uint32 gy) { }

    // Called when a player enters the map.
    virtual void OnPlayerEnter(TMap* map, Player* player) { }

    // Called when a player leaves the map.
    virtual void OnPlayerLeave(TMap* map, Player* player) { }

    // Called on every map update tick.
    virtual void OnUpdate(TMap* map, uint32 diff) { }
};

struct Script
{
    Script(){}

    std::string Name;

    void RegisterSelf(bool bReportError = true);
};


class ScriptDevAIMgr
{
    friend class MaNGOS::Singleton<ScriptDevAIMgr>;

    public:
        ScriptDevAIMgr() : num_sc_scripts(0) {}
        ~ScriptDevAIMgr();

        void Initialize();
        void LoadScriptNames();
        void LoadAreaTriggerScripts();
        void LoadEventIdScripts();

        void AddScript(uint32 id, Script* script);
        Script* GetScript(uint32 id) const;
        const char* GetScriptName(uint32 id) const { return id < m_scriptNames.size() ? m_scriptNames[id].c_str() : ""; }
        uint32 GetScriptId(const char* name) const;
        uint32 GetScriptIdsCount() const { return m_scriptNames.size(); }

        
        uint32 GetAreaTriggerScriptId(uint32 triggerId) const;
        uint32 GetEventIdScriptId(uint32 eventId) const;

public: /* PlayerScript*/
    void OnGivePlayerXP(Player* player, uint32& amount, Unit* victim);

public: /* CreatureScript */
    bool OnGossipHello(Player* pPlayer, Creature* pCreature);
    uint32 GetDialogStatus(const Player* player, const Creature* creature) const;
    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action);
    bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code);
    bool OnQuestAccept(Player* pPlayer, Creature* pCreature, const Quest* pQuest);
    bool OnQuestRewarded(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
    UnitAI* GetCreatureAI(Creature* pCreature) const;
    bool OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Creature* pTarget, ObjectGuid originalCasterGuid);
    bool OnEffectScriptEffect(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Creature* pTarget, ObjectGuid originalCasterGuid);

public: /* GameObjectScript */
    bool OnGossipHello(Player* pPlayer, GameObject* pGo);
    bool OnGossipSelect(Player* player, GameObject* pGo, uint32 sender, uint32 action);
    bool OnGossipSelectCode(Player* player, GameObject* pGo, uint32 sender, uint32 action, const char* code);
    uint32 GetDialogStatus(const Player* pPlayer, const GameObject* pGo) const;
    bool OnQuestAccept(Player* pPlayer, GameObject* pGo, Quest const* pQuest);
    bool OnQuestRewarded(Player* pPlayer, GameObject* pGo, Quest const* pQuest);
    bool OnGameObjectUse(Player* pPlayer, GameObject* pGo);
    GameObjectAI* GetGameObjectAI(GameObject* gameobject) const;
    std::function<bool(Unit*)>* OnTrapSearch(GameObject* go);
    bool OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, GameObject* pTarget, ObjectGuid originalCasterGuid);

public: /* ItemScript */
    bool OnQuestAccept(Player* pPlayer, Item* pItem, Quest const* pQuest);
    bool OnItemUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets);
    bool OnItemLoot(Player* pPlayer, Item* pItem, bool apply);
    bool OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Item* pTarget, ObjectGuid originalCasterGuid);

public: /* AreaTriggerScript */
    bool OnAreaTrigger(Player* pPlayer, AreaTriggerEntry const* atEntry);

public: /* InstaceMapScript */
    InstanceData* ScriptDevAIMgr::CreateInstanceData(Map* pMap);

public: /* SpellAuraScript */
    bool OnAuraDummy(Aura const* pAura, bool bApply);

public: /* ObjectScript */
    bool OnProcessEvent(uint32 uiEventId, Object* pSource, Object* pTarget, bool bIsStart);

public:/* ScriptRegistry */

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

    private:
        typedef std::vector<Script*> SDScriptVec;
        typedef std::vector<std::string> ScriptNameMap;
        typedef std::unordered_map<uint32, uint32> AreaTriggerScriptMap;
        typedef std::unordered_map<uint32, uint32> EventIdScriptMap;

        int num_sc_scripts;
        SDScriptVec m_scripts;

        AreaTriggerScriptMap    m_AreaTriggerScripts;
        EventIdScriptMap        m_EventIdScripts;

        ScriptNameMap           m_scriptNames;
};

class InstanceMapScript : public ScriptObject, public MapScript<DungeonMap>
{
protected:
    InstanceMapScript(const char* name, uint32 mapId = 0);

public:
    bool IsDatabaseBound() const { return true; }

    // Gets an InstanceData object for this instance.
    virtual InstanceData* GetInstanceScript(Map* map) const { return NULL; }
};

class SpellAuraScript : public ScriptObject
{
protected:
    SpellAuraScript(char const* name);
public:
    virtual bool OnAuraDummy(Aura const* pAura, bool bApply) { return false; }
};

class ObjectScript : public ScriptObject
{
protected:
    ObjectScript(char const* name);
public:
    virtual bool OnProcessEvent(uint32 uiEventId, Object* pSource, Object* pTarget, bool bIsStart) { return false; }
};

class PlayerScript : public ScriptObject
{
protected:
    PlayerScript(char const* name);
public:

    // Called when a player gains XP (before anything is given)
    virtual void OnGiveXP(Player* /*player*/, uint32& /*amount*/, Unit* /*victim*/) { }
};

class CreatureScript : public ScriptObject, public UpdatableScript<Creature>
{
protected:

    CreatureScript(const char* name);

public:
    bool IsDatabaseBound() const { return true; }

    // Called when a player opens a gossip dialog with the creature.
    virtual bool OnGossipHello(Player* player, Creature* creature) { return false; }

    // Called when a player selects a gossip item in the creature's gossip menu.
    virtual bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) { return false; }

    // Called when a player selects a gossip with a code in the Creature's gossip menu.
    virtual bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code) { return false; }

    // Called when the dialog status between a player and the creature is requested.
    virtual uint32 OnDialogStatus(const Player* player, const Creature* creature) { return 0; }

    // Called when a player accepts a quest from the creature.
    virtual bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) { return false; }

    // Called when a player selects a quest reward.
    virtual bool OnQuestReward(Player* player, Creature* creature, Quest const* quest) { return false; }

    // Called when a CreatureAI object is needed for the creature.
    virtual UnitAI* GetAI(Creature* creature) const { return NULL; }

    // Called when a dummy spell effect is triggered on the creature.
    virtual bool OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Creature* pTarget, ObjectGuid originalCasterGuid) { return false; }

    virtual bool OnEffectScriptEffect(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Creature* pTarget, ObjectGuid originalCasterGuid) { return false; }

};

class GameObjectScript : public ScriptObject, public UpdatableScript<GameObject>
{
protected:
    
    GameObjectScript(const char* name);
public:
    bool IsDatabaseBound() const { return true; }

    // Called when a player opens a gossip dialog with the gameobject.
    virtual bool OnGossipHello(Player* player, GameObject* go) { return false; }

    // Called when a player selects a gossip item in the gameobject's gossip menu.
    virtual bool OnGossipSelect(Player* player, GameObject* pGo, uint32 sender, uint32 action) { return false; }

    // Called when a player selects a gossip with a code in the gameobject's gossip menu.
    virtual bool OnGossipSelectCode(Player* player, GameObject* pGo, uint32 sender, uint32 action, const char* code) { return false; }

    // Called when the dialog status between a player and the gameobject is requested.
    virtual uint32 OnDialogStatus(const Player* player, const GameObject* pGo) { return 0; }

    // Called when a player accepts a quest from the gameobject.
    virtual bool OnQuestAccept(Player* player, GameObject* go, Quest const* quest) { return false; }

    // Called when a player selects a quest reward.
    virtual bool OnQuestReward(Player* player, GameObject* pGo, Quest const* quest) { return false; }

    // Called when Gameobject has been used.
    virtual bool OnGameObjectUse(Player* pPlayer, GameObject* pGo) { return false; }

    // Called when GameObjectAI is needed.
    virtual GameObjectAI* GetGameObjectAI(GameObject* gameobject) const { return NULL; };

    std::function<bool(Unit*)>* OnTrapSearch(GameObject* go) { return nullptr; }

    // Called when a dummy spell effect is triggered on the Gameobject.
    virtual bool OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, GameObject* pTarget, ObjectGuid originalCasterGuid) { return false; }
};

class ItemScript : public ScriptObject
{
protected:
    ItemScript(const char* name);
public:
    bool IsDatabaseBound() const { return true; }

    // Called when a player accepts a quest from the item.
    virtual bool OnQuestAccept(Player* pPlayer, Item* pItem, Quest const* pQuest) { return false; }

    // Called when an item has been used.
    virtual bool OnItemUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets) { return false; }

    // Called when an item has been looted.
    virtual bool OnItemLoot(Player* pPlayer, Item* pItem, bool apply) { return false; }

    // Called when a dummy spell effect is triggered on the Item.
    virtual bool OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Item* pTarget, ObjectGuid originalCasterGuid) { return false; }
};

class AreaTriggerScript : public ScriptObject
{
protected:
    AreaTriggerScript(const char* name);
public:
    bool IsDatabaseBound() const { return true; }

    // Called when the area trigger is activated by a player.
    virtual bool OnTrigger(Player* player, AreaTriggerEntry const* trigger) { return false; }
};

// *********************************************************
// ************* Some functions used globally **************

// Generic scripting text function
void DoScriptText(int32 iTextEntry, WorldObject* pSource, Unit* pTarget = nullptr);
void DoOrSimulateScriptTextForMap(int32 iTextEntry, uint32 uiCreatureEntry, Map* pMap, Creature* pCreatureSource = nullptr, Unit* pTarget = nullptr);

#define sScriptDevAIMgr MaNGOS::Singleton<ScriptDevAIMgr>::Instance()

#endif
