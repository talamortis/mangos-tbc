/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_SHADOW_LABYRINTH_H
#define DEF_SHADOW_LABYRINTH_H

enum
{
    MAX_ENCOUNTER           = 4,

    TYPE_HELLMAW            = 1,
    // TYPE_OVERSEER        = 2,                            // obsolete id used by acid
    TYPE_INCITER            = 3,
    TYPE_VORPIL             = 4,
    TYPE_MURMUR             = 5,

    DATA_CABAL_RITUALIST    = 1,                            // DO NOT CHANGE! Used by Acid. - used to check the Cabal Ritualists alive

    NPC_HELLMAW             = 18731,
    NPC_VORPIL              = 18732,
    NPC_CABAL_RITUALIST     = 18794,
    NPC_CONTAINMENT_BEAM    = 21159,

    GO_REFECTORY_DOOR       = 183296,                       // door opened when blackheart the inciter dies
    GO_SCREAMING_HALL_DOOR  = 183295,                       // door opened when grandmaster vorpil dies and player comes in range

    SAY_HELLMAW_INTRO       = -1555000,

    SPELL_BANISH            = 30231,                        // spell is handled in creature_template_addon;
};

#endif
