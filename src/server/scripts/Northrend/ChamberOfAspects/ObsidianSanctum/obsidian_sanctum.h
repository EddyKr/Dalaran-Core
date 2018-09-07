/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEF_OBSIDIAN_SANCTUM_H
#define DEF_OBSIDIAN_SANCTUM_H

enum DataTypes
{
    TYPE_SARTHARION_EVENT       = 1,
    TYPE_TENEBRON_PREKILLED     = 2,
    TYPE_SHADRON_PREKILLED      = 3,
    TYPE_VESPERON_PREKILLED     = 4,

    DATA_SARTHARION             = 10,
    DATA_TENEBRON               = 11,
    DATA_SHADRON                = 12,
    DATA_VESPERON               = 13,

	DATA_ACOLYTE_VESPERON		= 14,
	DATA_ACOLYTE_SHADRON		= 15
};

enum CreaturesIds
{
    NPC_SARTHARION              = 28860,
    NPC_TENEBRON                = 30452,
    NPC_SHADRON                 = 30451,
    NPC_VESPERON                = 30449,
	
	NPC_ACOLYTE_VESPERON		= 31219,
	NPC_ACOLYTE_SHADRON			= 31218
};

enum GameObjectIds
{
    GO_TWILIGHT_PORTAL          = 193988
};

#endif