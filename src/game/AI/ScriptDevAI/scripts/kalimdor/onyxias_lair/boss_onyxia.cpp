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
SDName: Boss_Onyxia
SD%Complete: 85
SDComment: Visual improvement needed in phase 2: flying animation while hovering. Quel'Serrar event is missing.
SDCategory: Onyxia's Lair
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "onyxias_lair.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SAY_AGGRO                   = -1249000,
    SAY_KILL                    = -1249001,
    SAY_PHASE_2_TRANS           = -1249002,
    SAY_PHASE_3_TRANS           = -1249003,
    EMOTE_BREATH                = -1249004,
    SAY_KITE                    = -1249005,

    SPELL_WINGBUFFET            = 18500,
    SPELL_FLAMEBREATH           = 18435,
    SPELL_CLEAVE                = 19983,
    SPELL_TAILSWEEP             = 15847,
    SPELL_KNOCK_AWAY            = 19633,
    SPELL_FIREBALL              = 18392,

    SPELL_DRAGON_HOVER          = 18430,
    SPELL_SPEED_BURST           = 18391,
    SPELL_PACIFY_SELF           = 19951,

    // Not much choise about these. We have to make own defintion on the direction/start-end point
    SPELL_BREATH_NORTH_TO_SOUTH = 17086,                    // 20x in "array"
    SPELL_BREATH_SOUTH_TO_NORTH = 18351,                    // 11x in "array"

    SPELL_BREATH_EAST_TO_WEST   = 18576,                    // 7x in "array"
    SPELL_BREATH_WEST_TO_EAST   = 18609,                    // 7x in "array"

    SPELL_BREATH_SE_TO_NW       = 18564,                    // 12x in "array"
    SPELL_BREATH_NW_TO_SE       = 18584,                    // 12x in "array"
    SPELL_BREATH_SW_TO_NE       = 18596,                    // 12x in "array"
    SPELL_BREATH_NE_TO_SW       = 18617,                    // 12x in "array"

    SPELL_VISUAL_BREATH_A       = 4880,                     // Only and all of the above Breath spells (and their triggered spells) have these visuals
    SPELL_VISUAL_BREATH_B       = 4919,

    SPELL_BREATH_ENTRANCE       = 21131,                    // 8x in "array", different initial cast than the other arrays

    SPELL_BELLOWINGROAR         = 18431,
    SPELL_HEATED_GROUND         = 22191,                    // Prevent players from hiding in the tunnels when it is time for Onyxia's breath

    // SPELL_SUMMON_ONYXIAN_WHELPS = 20171,                    // Periodic spell triggering SPELL_SUMMON_ONYXIAN_WHELP every 2 secs. Possibly used in Phase 2 for summoning whelps but data like duration or caster are unkown
    // SPELL_SUMMON_ONYXIAN_WHELP  = 20172,                    // Triggered by SPELL_SUMMON_ONYXIAN_WHELPS

    POINT_ID_NORTH              = 0,
    POINT_ID_SOUTH              = 4,
    NUM_MOVE_POINT              = 8,
    MAX_POINTS                  = 10,
    POINT_ID_LIFTOFF            = 1 + NUM_MOVE_POINT,
    POINT_ID_INIT_NORTH         = 2 + NUM_MOVE_POINT,
    POINT_ID_LAND               = 3 + NUM_MOVE_POINT,

    PHASE_START                 = 1,                        // Health above 65%, normal ground abilities
    PHASE_BREATH                = 2,                        // Breath phase (while health above 40%)
    PHASE_END                   = 3,                        // normal ground abilities + some extra abilities
    PHASE_BREATH_POST           = 4,                        // Landing and initial fearing
    PHASE_TO_LIFTOFF            = 5,                        // Movement to south-entrance of room and liftoff there
    PHASE_BREATH_PRE            = 6,                        // lifting off + initial flying to north side (summons also first pack of whelps)

    POINT_MID_AIR_LAND          = 4 + NUM_MOVE_POINT,

};

struct OnyxiaMove
{
    uint32 uiSpellId;
    float fX, fY, fZ;
};

static const OnyxiaMove aMoveData[NUM_MOVE_POINT] =
{
    {SPELL_BREATH_NORTH_TO_SOUTH,  24.16332f, -216.0808f, -58.98009f},  // north (coords verified in wotlk)
    {SPELL_BREATH_NE_TO_SW,        10.2191f,  -247.912f,  -60.896f},    // north-east
    {SPELL_BREATH_EAST_TO_WEST,   -15.00505f, -244.4841f, -60.40087f},  // east (coords verified in wotlk)
    {SPELL_BREATH_SE_TO_NW,       -63.5156f,  -240.096f,  -60.477f},    // south-east
    {SPELL_BREATH_SOUTH_TO_NORTH, -66.3589f,  -215.928f,  -64.23904f},  // south (coords verified in wotlk)
    {SPELL_BREATH_SW_TO_NE,       -58.2509f,  -189.020f,  -60.790f},    // south-west
    {SPELL_BREATH_WEST_TO_EAST,   -16.70134f, -181.4501f, -61.98513f},  // west (coords verified in wotlk)
    {SPELL_BREATH_NW_TO_SE,        12.26687f, -181.1084f, -60.23914f},  // north-west (coords verified in wotlk)
};

static const float landPoints[1][3] =
{
    {-1.060547f, -229.9293f, -86.14094f},
};

static const float afSpawnLocations[3][3] =
{
    { -30.127f, -254.463f, -89.440f},                       // whelps
    { -30.817f, -177.106f, -89.258f},                       // whelps
};

enum OnyxiaActions
{
    ONYXIA_PHASE_2_TRANSITION,
    ONYXIA_PHASE_3_TRANSITION,
    ONYXIA_CHECK_IN_LAIR,
    ONYXIA_BELLOWING_ROAR,
    ONYXIA_FLAME_BREATH,
    ONYXIA_CLEAVE,
    ONYXIA_TAIL_SWEEP,
    ONYXIA_WING_BUFFET,
    ONYXIA_KNOCK_AWAY,
    ONYXIA_MOVEMENT,
    ONYXIA_FIREBALL,
    ONYXIA_ACTION_MAX,
    ONYXIA_SUMMON_WHELPS,
    ONYXIA_PHASE_TRANSITIONS,
};
class boss_onyxia : public CreatureScript
{
public:
    boss_onyxia() : CreatureScript("boss_onyxia") { }


    struct boss_onyxiaAI : public CombatAI
    {
        boss_onyxiaAI(Creature* creature) :
            CombatAI(creature, ONYXIA_ACTION_MAX),
            m_instance(static_cast<instance_onyxias_lair*>(creature->GetInstanceData())),
            m_uiPhase(0),
            m_uiMovePoint(POINT_ID_NORTH),
            m_uiSummonCount(0),
            m_uiWhelpsPerWave(20),
            m_bIsSummoningWhelps(false),
            m_bHasYelledLured(false),
            m_HasSummonedFirstWave(false)
        {
            AddTimerlessCombatAction(ONYXIA_PHASE_2_TRANSITION, true);
            AddTimerlessCombatAction(ONYXIA_PHASE_3_TRANSITION, false);
            AddCombatAction(ONYXIA_BELLOWING_ROAR, true);
            AddCombatAction(ONYXIA_CHECK_IN_LAIR, 3000u);
            AddCombatAction(ONYXIA_FLAME_BREATH, 10000, 20000);
            AddCombatAction(ONYXIA_CLEAVE, 2000, 5000);
            AddCombatAction(ONYXIA_TAIL_SWEEP, 15000, 20000);
            AddCombatAction(ONYXIA_WING_BUFFET, 10000, 20000);
            AddCombatAction(ONYXIA_KNOCK_AWAY, 20000, 30000);
            AddCombatAction(ONYXIA_FIREBALL, true);
            AddCombatAction(ONYXIA_MOVEMENT, true);
            AddCustomAction(ONYXIA_SUMMON_WHELPS, true, [&]() { SummonWhelps(); });
            AddCustomAction(ONYXIA_PHASE_TRANSITIONS, true, [&]() { PhaseTransition(); });
            m_creature->SetWalk(false); // onyxia should run when flying
        }

        instance_onyxias_lair* m_instance;

        uint8 m_uiPhase;

        uint32 m_uiMovePoint;

        uint8 m_uiSummonCount;
        uint8 m_uiWhelpsPerWave;

        bool m_bIsSummoningWhelps;
        bool m_bHasYelledLured;
        bool m_HasSummonedFirstWave;

        ObjectGuid m_fireballVictim;

        void Reset() override
        {
            CombatAI::Reset();
            if (!IsCombatMovement())
                SetCombatMovement(true);

            m_uiPhase = PHASE_START;

            m_uiMovePoint = POINT_ID_NORTH;                     // First point reached by the flying Onyxia

            m_uiSummonCount = 0;
            m_uiWhelpsPerWave = 20;                               // Twenty whelps on first summon, 4 - 10 on the nexts

            m_bHasYelledLured = false;
            m_HasSummonedFirstWave = false;

            m_creature->SetStandState(UNIT_STAND_STATE_SLEEP);
            m_creature->SetSheath(SHEATH_STATE_MELEE);
            SetMeleeEnabled(true);
        }

        void Aggro(Unit* /*who*/) override
        {
            DoScriptText(SAY_AGGRO, m_creature);

            if (m_instance)
                m_instance->SetData(TYPE_ONYXIA, IN_PROGRESS);

            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
        }

        void JustReachedHome() override
        {
            // in case evade in phase 2, see comments for hack where phase 2 is set

            if (m_instance)
                m_instance->SetData(TYPE_ONYXIA, FAIL);
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (m_instance)
                m_instance->SetData(TYPE_ONYXIA, DONE);
        }

        void JustSummoned(Creature* summoned) override
        {
            if (!m_instance)
                return;

            if (summoned->GetEntry() == NPC_ONYXIA_WHELP)
                ++m_uiSummonCount;
        }

        void SummonedMovementInform(Creature* summoned, uint32 motionType, uint32 pointId) override
        {
            if (motionType != POINT_MOTION_TYPE || pointId != 1 || !m_creature->GetVictim())
                return;

            summoned->SetInCombatWithZone();
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            DoScriptText(SAY_KILL, m_creature);
        }

        void SpellHit(Unit* /*caster*/, const SpellEntry* spellInfo) override
        {
            if (spellInfo->Id == SPELL_BREATH_EAST_TO_WEST ||
                    spellInfo->Id == SPELL_BREATH_WEST_TO_EAST ||
                    spellInfo->Id == SPELL_BREATH_SE_TO_NW ||
                    spellInfo->Id == SPELL_BREATH_NW_TO_SE ||
                    spellInfo->Id == SPELL_BREATH_SW_TO_NE ||
                    spellInfo->Id == SPELL_BREATH_NE_TO_SW ||
                    spellInfo->Id == SPELL_BREATH_SOUTH_TO_NORTH ||
                    spellInfo->Id == SPELL_BREATH_NORTH_TO_SOUTH)
            {
                // This was sent with SendMonsterMove - which resulted in better speed than now
                m_creature->GetMotionMaster()->MovePoint(m_uiMovePoint, aMoveData[m_uiMovePoint].fX, aMoveData[m_uiMovePoint].fY, aMoveData[m_uiMovePoint].fZ);
                DoCastSpellIfCan(m_creature, SPELL_HEATED_GROUND, CAST_TRIGGERED);
            }
        }

        void MovementInform(uint32 motionType, uint32 pointId) override
        {
            if (motionType != POINT_MOTION_TYPE || !m_instance)
                return;

            switch (pointId)
            {
                case POINT_ID_LAND:
                    // undo flying
                    m_creature->HandleEmote(EMOTE_ONESHOT_LAND);
                    m_creature->SetHover(false);
                    m_creature->RemoveAurasDueToSpell(SPELL_PACIFY_SELF);
                    ResetTimer(ONYXIA_PHASE_TRANSITIONS, 2000);                          // Start PHASE_END shortly delayed
                    return;
                case POINT_ID_LIFTOFF:
                    ResetTimer(ONYXIA_PHASE_TRANSITIONS, 3500);                           // Start Flying shortly delayed
                    m_creature->SetHover(true);
                    m_creature->HandleEmote(EMOTE_ONESHOT_LIFTOFF);
                    break;
                case POINT_ID_INIT_NORTH:                           // Start PHASE_BREATH
                    m_uiPhase = PHASE_BREATH;
                    m_creature->CastSpell(nullptr, SPELL_DRAGON_HOVER, TRIGGERED_OLD_TRIGGERED);
                    m_creature->CastSpell(nullptr, SPELL_PACIFY_SELF, TRIGGERED_OLD_TRIGGERED);
                    SetCombatScriptStatus(false);
                    HandlePhaseTransition();
                    break;
                default:
                    m_creature->CastSpell(nullptr, SPELL_DRAGON_HOVER, TRIGGERED_OLD_TRIGGERED);
                    break;
            }

            if (Creature* pTrigger = m_instance->GetSingleCreatureFromStorage(NPC_ONYXIA_TRIGGER))
                m_creature->SetFacingToObject(pTrigger);
        }

        void AttackStart(Unit* who) override
        {
            if (m_uiPhase == PHASE_START || m_uiPhase == PHASE_END)
                ScriptedAI::AttackStart(who);
        }

        // Onyxian Whelps summoning during phase 2
        void SummonWhelps()
        {
            if (m_uiSummonCount >= m_uiWhelpsPerWave)
            {
                ResetTimer(ONYXIA_SUMMON_WHELPS, 90 * IN_MILLISECONDS - m_uiWhelpsPerWave * (m_HasSummonedFirstWave ? 3000 : 1750));
                m_HasSummonedFirstWave = true;
                m_uiWhelpsPerWave = urand(4, 10);
                m_uiSummonCount = 0;
                return;
            }

            for (uint8 i = 0; i < 2; i++)
            {
                // Should probably make use of SPELL_SUMMON_ONYXIAN_WHELPS instead. Correct caster and removal of the spell is unkown, so make Onyxia do the summoning
                if (Creature* pWhelp =  m_creature->SummonCreature(NPC_ONYXIA_WHELP, afSpawnLocations[i][0], afSpawnLocations[i][1], afSpawnLocations[i][2], 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 3 * IN_MILLISECONDS))
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0))
                        pWhelp->AI()->AttackStart(pTarget);
                }
            }
            ResetTimer(ONYXIA_SUMMON_WHELPS, m_HasSummonedFirstWave ? urand(2000, 4000) : urand(1000, 2500));
        }

        void PhaseTransition()
        {
            switch (m_uiPhase)
            {
                case PHASE_TO_LIFTOFF:
                    m_uiPhase = PHASE_BREATH_PRE;
                    // Initial Onyxian Whelps spawn
                    ResetTimer(ONYXIA_SUMMON_WHELPS, 3000);
                    if (m_instance)
                        m_instance->SetData(TYPE_ONYXIA, DATA_LIFTOFF);
                    m_creature->GetMotionMaster()->MovePoint(POINT_ID_INIT_NORTH, aMoveData[POINT_ID_NORTH].fX, aMoveData[POINT_ID_NORTH].fY, aMoveData[POINT_ID_NORTH].fZ, true, FORCED_MOVEMENT_FLIGHT);
                    break;
                case PHASE_BREATH_POST:
                    m_uiPhase = PHASE_END;
                    m_creature->SetTarget(m_creature->GetVictim());
                    SetCombatMovement(true, true);
                    SetMeleeEnabled(true);
                    SetCombatScriptStatus(false);
                    HandlePhaseTransition();
                    break;
            }
        }

        void HandlePhaseTransition()
        {
            switch (m_uiPhase)
            {
                case PHASE_BREATH:
                {
                    ResetCombatAction(ONYXIA_FIREBALL, 0);
                    ResetCombatAction(ONYXIA_MOVEMENT, 25000);
                    DisableCombatAction(ONYXIA_FLAME_BREATH);
                    DisableCombatAction(ONYXIA_CLEAVE);
                    DisableCombatAction(ONYXIA_TAIL_SWEEP);
                    DisableCombatAction(ONYXIA_WING_BUFFET);
                    DisableCombatAction(ONYXIA_KNOCK_AWAY);
                    DisableCombatAction(ONYXIA_CHECK_IN_LAIR);
                    SetActionReadyStatus(ONYXIA_PHASE_3_TRANSITION, true);
                    break;
                }
                case PHASE_END:
                {
                    DisableCombatAction(ONYXIA_FIREBALL);
                    DisableCombatAction(ONYXIA_MOVEMENT);
                    ResetCombatAction(ONYXIA_BELLOWING_ROAR, 0);
                    ResetCombatAction(ONYXIA_FLAME_BREATH, urand(10000, 20000));
                    ResetCombatAction(ONYXIA_CLEAVE, urand(2000, 5000));
                    ResetCombatAction(ONYXIA_TAIL_SWEEP, urand(15000, 20000));
                    ResetCombatAction(ONYXIA_KNOCK_AWAY, urand(10000, 20000));
                    ResetCombatAction(ONYXIA_CHECK_IN_LAIR, 3000);
                    break;
                }
            }
        }

        void OnSpellCooldownAdded(SpellEntry const* spellInfo) override
        {
            CombatAI::OnSpellCooldownAdded(spellInfo);
            if (spellInfo->Id == SPELL_FIREBALL)
                if (Unit* target = m_creature->GetMap()->GetUnit(m_fireballVictim))
                    m_creature->getThreatManager().modifyThreatPercent(target, -100);
        }

        void ExecuteAction(uint32 action) override
        {
            switch (action)
            {
                case ONYXIA_PHASE_2_TRANSITION:
                {
                    if (m_creature->GetHealthPercent() > 65.0f)
                        return;

                    m_uiPhase = PHASE_TO_LIFTOFF;
                    DoScriptText(SAY_PHASE_2_TRANS, m_creature);
                    SetCombatMovement(false);
                    SetMeleeEnabled(false);
                    m_creature->SetTarget(nullptr);
                    SetCombatScriptStatus(true);

                    m_creature->GetMotionMaster()->MovePoint(POINT_ID_LIFTOFF, aMoveData[POINT_ID_SOUTH].fX, aMoveData[POINT_ID_SOUTH].fY, -84.25523f, 6.248279f);
                    SetActionReadyStatus(action, false);
                    break;
                }
                case ONYXIA_PHASE_3_TRANSITION:
                {
                    if (m_creature->GetHealthPercent() > 40.0f)
                        return;

                    m_uiPhase = PHASE_BREATH_POST;
                    DoScriptText(SAY_PHASE_3_TRANS, m_creature);
                    DisableTimer(ONYXIA_SUMMON_WHELPS);

                    SetCombatScriptStatus(true);
                    m_creature->SetTarget(nullptr);
                    m_creature->RemoveAurasDueToSpell(SPELL_DRAGON_HOVER);
                    m_creature->GetMotionMaster()->MovePoint(POINT_ID_LAND, landPoints[0][0], landPoints[0][1], landPoints[0][2], true, FORCED_MOVEMENT_FLIGHT);
                    SetActionReadyStatus(action, false);
                    break;
                }
                case ONYXIA_CHECK_IN_LAIR:
                {
                    if (m_creature->IsNonMeleeSpellCasted(false))
                        return;
                    if (m_instance)
                    {
                        Creature* onyxiaTrigger = m_instance->GetSingleCreatureFromStorage(NPC_ONYXIA_TRIGGER);
                        if (onyxiaTrigger && !m_creature->IsWithinDistInMap(onyxiaTrigger, 90.0f, false))
                        {
                            if (!m_bHasYelledLured)
                            {
                                m_bHasYelledLured = true;
                                DoScriptText(SAY_KITE, m_creature);
                            }
                            DoCastSpellIfCan(nullptr, SPELL_BREATH_ENTRANCE);
                        }
                        else
                            m_bHasYelledLured = false;
                    }
                    ResetCombatAction(action, 3000);
                    break;
                }
                case ONYXIA_SUMMON_WHELPS:
                {
                    SummonWhelps();
                    break;
                }
                case ONYXIA_BELLOWING_ROAR:
                {
                    if (DoCastSpellIfCan(nullptr, SPELL_BELLOWINGROAR) == CAST_OK)
                        ResetCombatAction(action, urand(15000, 45000));
                    break;
                }
                case ONYXIA_FLAME_BREATH:
                {
                    if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_FLAMEBREATH) == CAST_OK)
                        ResetCombatAction(action, urand(10000, 20000));
                    break;
                }
                case ONYXIA_CLEAVE:
                {
                    if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CLEAVE) == CAST_OK)
                        ResetCombatAction(action, urand(5000, 10000));
                    break;
                }
                case ONYXIA_TAIL_SWEEP:
                {
                    if (DoCastSpellIfCan(nullptr, SPELL_TAILSWEEP) == CAST_OK)
                        ResetCombatAction(action, urand(15000, 20000));
                    break;
                }
                case ONYXIA_WING_BUFFET:
                {
                    if (DoCastSpellIfCan(nullptr, SPELL_WINGBUFFET) == CAST_OK)
                        ResetCombatAction(action, urand(15000, 30000));
                    break;
                }
                case ONYXIA_KNOCK_AWAY:
                {
                    if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_KNOCK_AWAY) == CAST_OK)
                        ResetCombatAction(action, urand(25000, 40000));
                    break;
                }
                case ONYXIA_FIREBALL:
                {
                    if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_FIREBALL, SELECT_FLAG_PLAYER))
                    {
                        if (DoCastSpellIfCan(target, SPELL_FIREBALL) == CAST_OK)
                        {
                            ResetCombatAction(action, urand(3000, 5000));
                            m_fireballVictim = target->GetObjectGuid();
                        }
                    }
                    break;
                }
                case ONYXIA_MOVEMENT:
                {
                    if (m_creature->IsNonMeleeSpellCasted(false))
                        return;
                    // 3 possible actions
                    switch (urand(0, 2))
                    {
                        case 0:                             // breath
                            DoScriptText(EMOTE_BREATH, m_creature);
                            DoCastSpellIfCan(m_creature, aMoveData[m_uiMovePoint].uiSpellId, CAST_INTERRUPT_PREVIOUS);
                            m_uiMovePoint += NUM_MOVE_POINT / 2;
                            m_uiMovePoint %= NUM_MOVE_POINT;
                            ResetCombatAction(action, 25000);
                            return;
                        case 1:                             // a point on the left side
                        {
                            // C++ is stupid, so add -1 with +7
                            m_uiMovePoint += NUM_MOVE_POINT - 1;
                            m_uiMovePoint %= NUM_MOVE_POINT;
                            break;
                        }
                        case 2:                             // a point on the right side
                            m_uiMovePoint = (m_uiMovePoint + 1) % NUM_MOVE_POINT;
                            break;
                    }

                    ResetCombatAction(action, urand(15000, 25000));
                    m_creature->RemoveAurasDueToSpell(SPELL_DRAGON_HOVER);
                    m_creature->GetMotionMaster()->MovePoint(m_uiMovePoint, aMoveData[m_uiMovePoint].fX, aMoveData[m_uiMovePoint].fY, aMoveData[m_uiMovePoint].fZ, true, FORCED_MOVEMENT_FLIGHT);
                    break;
                }
            }
        }
    };



};

void AddSC_boss_onyxia()
{
    new boss_onyxia();

}
