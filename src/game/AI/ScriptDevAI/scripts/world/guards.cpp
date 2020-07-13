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
SDName: Guards
SD%Complete: 100
SDComment: CombatAI should be organized better for future.
SDCategory: Guards
EndScriptData */

/* ContentData
guard_azuremyst
guard_bluffwatcher
guard_contested
guard_darnassus
guard_dunmorogh
guard_durotar
guard_elwynnforest
guard_eversong
guard_exodar
guard_ironforge
guard_mulgore
guard_orgrimmar
guard_shattrath
guard_shattrath_aldor
guard_shattrath_scryer
guard_silvermoon
guard_stormwind
guard_teldrassil
guard_tirisfal
guard_undercity
EndContentData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/guard_ai.h"
class guard_azuremyst : public CreatureScript
{
public:
    guard_azuremyst() : CreatureScript("guard_azuremyst") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_bluffwatcher : public CreatureScript
{
public:
    guard_bluffwatcher() : CreatureScript("guard_bluffwatcher") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_contested : public CreatureScript
{
public:
    guard_contested() : CreatureScript("guard_contested") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_darnassus : public CreatureScript
{
public:
    guard_darnassus() : CreatureScript("guard_darnassus") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_dunmorogh : public CreatureScript
{
public:
    guard_dunmorogh() : CreatureScript("guard_dunmorogh") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_durotar : public CreatureScript
{
public:
    guard_durotar() : CreatureScript("guard_durotar") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_elwynnforest : public CreatureScript
{
public:
    guard_elwynnforest() : CreatureScript("guard_elwynnforest") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_eversong : public CreatureScript
{
public:
    guard_eversong() : CreatureScript("guard_eversong") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_exodar : public CreatureScript
{
public:
    guard_exodar() : CreatureScript("guard_exodar") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_ironforge : public CreatureScript
{
public:
    guard_ironforge() : CreatureScript("guard_ironforge") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_mulgore : public CreatureScript
{
public:
    guard_mulgore() : CreatureScript("guard_mulgore") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_orgrimmar : public CreatureScript
{
public:
    guard_orgrimmar() : CreatureScript("guard_orgrimmar") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI_orgrimmar(pCreature);
    }



};
class guard_shattrath : public CreatureScript
{
public:
    guard_shattrath() : CreatureScript("guard_shattrath") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};

/*******************************************************
 * guard_shattrath_aldor
 *******************************************************/
class guard_shattrath_aldor : public CreatureScript
{
public:
    guard_shattrath_aldor() : CreatureScript("guard_shattrath_aldor") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guard_shattrath_aldorAI(pCreature);
    }



    struct guard_shattrath_aldorAI : public guardAI
    {
        guard_shattrath_aldorAI(Creature* pCreature) : guardAI(pCreature) { Reset(); }

        uint32 m_uiExile_Timer;
        uint32 m_uiBanish_Timer;
        ObjectGuid m_playerGuid;
        bool m_bCanTeleport;

        void Reset() override
        {
            m_uiBanish_Timer = 5000;
            m_uiExile_Timer = 8500;
            m_playerGuid.Clear();
            m_bCanTeleport = false;
        }

        void UpdateAI(const uint32 uiDiff) override
        {
            if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
                return;

            if (m_bCanTeleport)
            {
                if (m_uiExile_Timer < uiDiff)
                {
                    if (Player* pTarget = m_creature->GetMap()->GetPlayer(m_playerGuid))
                    {
                        pTarget->CastSpell(pTarget, SPELL_EXILE, TRIGGERED_OLD_TRIGGERED);
                        pTarget->CastSpell(pTarget, SPELL_BANISH_TELEPORT, TRIGGERED_OLD_TRIGGERED);
                    }

                    m_playerGuid.Clear();
                    m_uiExile_Timer = 8500;
                    m_bCanTeleport = false;
                }
                else
                    m_uiExile_Timer -= uiDiff;
            }
            else if (m_uiBanish_Timer < uiDiff)
            {
                Unit* pVictim = m_creature->GetVictim();

                if (pVictim && pVictim->GetTypeId() == TYPEID_PLAYER)
                {
                    DoCastSpellIfCan(pVictim, SPELL_BANISHED_SHATTRATH_A);
                    m_uiBanish_Timer = 9000;
                    m_playerGuid = pVictim->GetObjectGuid();
                    m_bCanTeleport = true;
                }
            }
            else
                m_uiBanish_Timer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };



};


/*******************************************************
 * guard_shattrath_scryer
 *******************************************************/
class guard_shattrath_scryer : public CreatureScript
{
public:
    guard_shattrath_scryer() : CreatureScript("guard_shattrath_scryer") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guard_shattrath_scryerAI(pCreature);
    }



    struct guard_shattrath_scryerAI : public guardAI
    {
        guard_shattrath_scryerAI(Creature* pCreature) : guardAI(pCreature) { Reset(); }

        uint32 m_uiExile_Timer;
        uint32 m_uiBanish_Timer;
        ObjectGuid m_playerGuid;
        bool m_bCanTeleport;

        void Reset() override
        {
            m_uiBanish_Timer = 5000;
            m_uiExile_Timer = 8500;
            m_playerGuid.Clear();
            m_bCanTeleport = false;
        }

        void UpdateAI(const uint32 uiDiff) override
        {
            if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
                return;

            if (m_bCanTeleport)
            {
                if (m_uiExile_Timer < uiDiff)
                {
                    if (Player* pTarget = m_creature->GetMap()->GetPlayer(m_playerGuid))
                    {
                        pTarget->CastSpell(pTarget, SPELL_EXILE, TRIGGERED_OLD_TRIGGERED);
                        pTarget->CastSpell(pTarget, SPELL_BANISH_TELEPORT, TRIGGERED_OLD_TRIGGERED);
                    }

                    m_playerGuid.Clear();
                    m_uiExile_Timer = 8500;
                    m_bCanTeleport = false;
                }
                else
                    m_uiExile_Timer -= uiDiff;
            }
            else if (m_uiBanish_Timer < uiDiff)
            {
                Unit* pVictim = m_creature->GetVictim();

                if (pVictim && pVictim->GetTypeId() == TYPEID_PLAYER)
                {
                    DoCastSpellIfCan(pVictim, SPELL_BANISHED_SHATTRATH_S);
                    m_uiBanish_Timer = 9000;
                    m_playerGuid = pVictim->GetObjectGuid();
                    m_bCanTeleport = true;
                }
            }
            else
                m_uiBanish_Timer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };



};

class guard_silvermoon : public CreatureScript
{
public:
    guard_silvermoon() : CreatureScript("guard_silvermoon") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};

enum{
    SPELL_WINDSOR_INSPIRATION_EFFECT = 20275,
    MAX_GUARD_SALUTES                = 7,
};

static const int32 aGuardSalute[MAX_GUARD_SALUTES] = { -1000842, -1000843, -1000844, -1000845, -1000846, -1000847, -1000848};

struct guardAI_stormwind : public guardAI
{
    guardAI_stormwind(Creature* creature) : guardAI(creature)
    {
        Reset();
    }

    uint32 m_saluteWaitTimer;

    void Reset() override
    {
        m_saluteWaitTimer = 0;
    }

    void SpellHit(Unit* /*caster*/, const SpellEntry* spell) override
    {
        if (spell->Id == SPELL_WINDSOR_INSPIRATION_EFFECT && !m_saluteWaitTimer)
        {
            DoScriptText(aGuardSalute[urand(0, MAX_GUARD_SALUTES - 1)], m_creature);
            m_saluteWaitTimer = 2 * MINUTE * IN_MILLISECONDS;
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        if (m_saluteWaitTimer)
        {
            if (m_saluteWaitTimer < diff)
                m_saluteWaitTimer = 0;
            else
                m_saluteWaitTimer -= diff;
        }

        guardAI::UpdateAI(diff);
    }

    void  ReceiveEmote(Player* player, uint32 textEmote) override
    {
        if (player->GetTeam() == ALLIANCE)
            DoReplyToTextEmote(textEmote);
    }
};
class guard_stormwind : public CreatureScript
{
public:
    guard_stormwind() : CreatureScript("guard_stormwind") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI_stormwind(pCreature);
    }



};
class guard_teldrassil : public CreatureScript
{
public:
    guard_teldrassil() : CreatureScript("guard_teldrassil") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_tirisfal : public CreatureScript
{
public:
    guard_tirisfal() : CreatureScript("guard_tirisfal") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};
class guard_undercity : public CreatureScript
{
public:
    guard_undercity() : CreatureScript("guard_undercity") { }

    UnitAI* GetAI(Creature* pCreature)
    {
        return new guardAI(pCreature);
    }



};

void AddSC_guards()
{
    new guard_azuremyst();
    new guard_bluffwatcher();
    new guard_contested();
    new guard_darnassus();
    new guard_dunmorogh();
    new guard_durotar();
    new guard_elwynnforest();
    new guard_eversong();
    new guard_exodar();
    new guard_ironforge();
    new guard_mulgore();
    new guard_orgrimmar();
    new guard_shattrath();
    new guard_shattrath_aldor();
    new guard_shattrath_scryer();
    new guard_silvermoon();
    new guard_stormwind();
    new guard_teldrassil();
    new guard_tirisfal();
    new guard_undercity();

}
