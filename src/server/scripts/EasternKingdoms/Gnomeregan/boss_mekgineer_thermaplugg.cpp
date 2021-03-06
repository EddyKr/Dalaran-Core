/*
* Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
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

//@author: Lothloryen.

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "gnomeregan.h"
#include "GameObjectAI.h"
#include "GridNotifiers.h"

enum Yells
{
    SAY_AGGRO         = 0,
    SAY_MACHINES      = 1,
    SAY_EXPLOSIONS    = 2,
    SAY_DEAD          = 3,
};

enum Events
{
    EVENT_POUND          = 1,
    EVENT_STEAM_BLAST    = 2,
    EVENT_KNOCK_AWAY     = 3,
    EVENT_ACTIVATE_BOMBS = 4,
    EVENT_WELDING_BEAM   = 5,

    EVENT_SUMMON_BOMB    = 6,
    EVENT_ATTACK_START   = 7,    
};

enum Spells
{
    SPELL_POUND                 = 35049,
    SPELL_STEAM_BLAST           = 50375,
    SPELL_KNOCK_AWAY            = 10101,
    SPELL_ACTIVATE_BOMB_VISUAL  = 11511,
    SPELL_ACTIVATE_BOMB         = 11518,
    SPELL_WELDING_BEAM          = 35919,

    SPELL_BOMB_EFFECT           = 11504,
};

enum Actions
{
    ACTION_ACTIVATE = 0,
};

Position BombPositions[MAX_GNOME_FACES] =
{
    { -550.36f, 695.84f, -316.64f, 5.29f }, // Gnome Face 01
    { -522.94f, 701.96f, -315.95f, 4.48f }, // Gnome Face 02
    { -501.40f, 683.53f, -315.77f, 3.55f }, // Gnome Face 03
    { -501.87f, 655.21f, -315.75f, 2.62f }, // Gnome Face 04
    { -524.90f, 638.13f, -315.07f, 1.79f }, // Gnome Face 05
    { -552.53f, 645.29f, -314.61f, 1.00f }, // Gnome Face 06
};

class boss_mekgineer_thermaplugg : public CreatureScript
{
public:
    boss_mekgineer_thermaplugg() : CreatureScript("boss_mekgineer_thermaplugg") { }

    struct boss_mekgineer_thermapluggAI : public BossAI
    {
        boss_mekgineer_thermapluggAI(Creature* creature) : BossAI(creature, DATA_MEKGINEER_THERMAPLUGG)
        {            
        }

        void AddFaceAvailable(uint32 entry)
        {
            _availableFacesList.push_back(entry);
        }

        void RemoveFaceAvailable(uint32 entry)
        {
            _availableFacesList.remove(entry);            
        }

        void DespawnBombs()
        {
            std::list<Creature*> pCreatureList;

            Trinity::AllCreaturesOfEntryInRange checker(me, NPC_WALKING_BOMB, 125.0f);
            Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(me, pCreatureList, checker);

            me->VisitNearbyObject(125.0f, searcher);

            if (pCreatureList.empty())
                return;

            if (!pCreatureList.empty())
            {
                for (std::list<Creature*>::iterator i = pCreatureList.begin(); i != pCreatureList.end(); ++i)
                    (*i)->DespawnOrUnsummon();
            }
        }

        void Initialize()
        {            
            _availableFacesList.clear();

            for (uint8 i = 0; i < MAX_GNOME_FACES; i++)            
                if (GameObject* face = me->FindNearestGameObject(GnomeFaces[i], 130.0f))
                    face->AI()->Reset();           
        }        

        void Reset() override
        {
            _events.Reset();
            Initialize();
            DespawnBombs();
        }

        void KilledUnit(Unit* who)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_DEAD);
        }

        void JustDied(Unit* /*killer*/) override
        {            
            _JustDied();            
            DespawnBombs();
            for (uint8 i = 0; i < MAX_GNOME_FACES; i++)
                if (GameObject* face = me->FindNearestGameObject(GnomeFaces[i], 130.0f))
                    face->AI()->Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {            
            for (uint8 i = 0; i < MAX_GNOME_FACES; i++)            
                if (GameObject* face = me->FindNearestGameObject(GnomeFaces[i], 130.0f))                
                    AddFaceAvailable(GnomeFaces[i]);                            

            Talk(SAY_AGGRO);
            _events.ScheduleEvent(EVENT_POUND, 6 * IN_MILLISECONDS);
            _events.ScheduleEvent(EVENT_STEAM_BLAST, 10 * IN_MILLISECONDS);
            _events.ScheduleEvent(EVENT_ACTIVATE_BOMBS, 8 * IN_MILLISECONDS);
            _events.ScheduleEvent(EVENT_WELDING_BEAM, 14 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;           

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_POUND:     
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 5.0f))                    
                        DoCast(target, SPELL_POUND);                                      
                    else                    
                        DoCastVictim(SPELL_POUND);                                         
                    
                    _events.RescheduleEvent(EVENT_POUND, 10 * IN_MILLISECONDS);
                    _events.RescheduleEvent(EVENT_STEAM_BLAST, 4 * IN_MILLISECONDS);
                    break;
                case EVENT_STEAM_BLAST:
                {
                    uint8 rand = urand(1, 4);
                    if (rand == 1)
                        Talk(SAY_MACHINES);
                    DoCastVictim(SPELL_STEAM_BLAST);
                    if (DoGetThreat(me->GetVictim()))
                        DoModifyThreatPercent(me->GetVictim(), -50);
                }
                    break;
                case EVENT_ACTIVATE_BOMBS:
                {                    
                    uint32 faceEntry = Trinity::Containers::SelectRandomContainerElement(_availableFacesList);
                    if (GameObject* face = me->FindNearestGameObject(faceEntry, 125.0f))
                    {
                        Talk(SAY_EXPLOSIONS);
                        face->AI()->DoAction(ACTION_ACTIVATE);
                        DoCast(SPELL_ACTIVATE_BOMB_VISUAL);
                    }                        

                    _events.RescheduleEvent(EVENT_ACTIVATE_BOMBS, urand(10 * IN_MILLISECONDS, 18 * IN_MILLISECONDS));
                }
                    break;
                case EVENT_WELDING_BEAM:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30, true))                   
                        DoCast(target, SPELL_WELDING_BEAM, true);
                   
                    _events.RescheduleEvent(EVENT_WELDING_BEAM, urand(10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS));
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }

        EventMap _events;
        std::list<uint32> _availableFacesList;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetInstanceAI<boss_mekgineer_thermapluggAI>(creature);
    }

};

class npc_walking_bomb : public CreatureScript
{
public:
    npc_walking_bomb() : CreatureScript("npc_walking_bomb") { }

    struct npc_walking_bombAI : public ScriptedAI
    {
        npc_walking_bombAI(Creature* creature) : ScriptedAI(creature)
        {            
            _instance = me->GetInstanceScript();
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
            me->SetReactState(REACT_PASSIVE);
        }

        void Reset() override
        {
            _events.ScheduleEvent(EVENT_ATTACK_START, 2 * IN_MILLISECONDS);
            despawn = false;
        }

        void KilledUnit(Unit* who) override
        {
            if (who->GetTypeId() == TYPEID_PLAYER)            
                if (Creature* boss = ObjectAccessor::GetCreature(*me, _instance->GetGuidData(DATA_MEKGINEER_THERMAPLUGG)))                
                    boss->AI()->Talk(SAY_DEAD);                                                
        }

        void UpdateAI(uint32 diff) override
        {
            if (!despawn)
            if (Player* player = me->SelectNearestPlayer(3.0f))
            {
                despawn = true;
                me->CastSpell(me, SPELL_BOMB_EFFECT, true);
                me->StopMoving();                
            }               
            
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ATTACK_START:
                    me->SetInCombatWithZone();
                    if (Player* player = me->SelectNearestPlayer(125.0f))
                    {
                        AttackStart(player);
                        me->AddThreat(player, 10000);                        
                    }                        
                    break;
                default:
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        EventMap _events;
        bool despawn;
        InstanceScript* _instance;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetInstanceAI<npc_walking_bombAI>(creature);
    }
};

class go_button : public GameObjectScript
{
public:
    go_button() : GameObjectScript("go_button") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (InstanceScript* instance = go->GetInstanceScript())
        {
            if (GameObject* gnomeFace = go->FindNearestGameObject(instance->GetData(go->GetEntry()), 20.0f))
            {                
                gnomeFace->SetLootState(GO_READY);
                gnomeFace->SetGoState(GO_STATE_READY);
            }
        }
        return false;
    }
};

class go_gnome_face : public GameObjectScript
{
public:
    go_gnome_face() : GameObjectScript("go_gnome_face") { }

    struct go_gnome_faceAI : public GameObjectAI
    {
        go_gnome_faceAI(GameObject* go) : GameObjectAI(go), _instance(go->GetInstanceScript()), _isActive(false) { }

        void OnStateChanged(uint32 state, Unit* who)
        {
            switch (state)
            {
            case GO_STATE_ACTIVE_ALTERNATIVE:
                if (Creature* boss = ObjectAccessor::GetCreature(*go, _instance->GetGuidData(DATA_MEKGINEER_THERMAPLUGG)))                
                    CAST_AI(boss_mekgineer_thermaplugg::boss_mekgineer_thermapluggAI, boss->AI())->RemoveFaceAvailable(go->GetEntry());
                
                _isActive = true;
                _events.RescheduleEvent(EVENT_SUMMON_BOMB, 2 * IN_MILLISECONDS);
                break;
            case GO_STATE_READY:
                if (_isActive)                
                    if (Creature* boss = ObjectAccessor::GetCreature(*go, _instance->GetGuidData(DATA_MEKGINEER_THERMAPLUGG)))                    
                        CAST_AI(boss_mekgineer_thermaplugg::boss_mekgineer_thermapluggAI, boss->AI())->AddFaceAvailable(go->GetEntry());                                                                 

                _isActive = false;
                _events.CancelEvent(EVENT_SUMMON_BOMB);
                break;
            default:
                break;
            }
        }

        void Reset() override
        {
            go->SetLootState(GO_READY);
            go->SetGoState(GO_STATE_READY);
        }

        void DoAction(int32 param)
        {
            if (param == ACTION_ACTIVATE) 
                if (Creature* trigger = go->FindNearestCreature(NPC_WORLD_TRIGGER, 30.0f, true))                
                    trigger->CastSpell(trigger, SPELL_ACTIVATE_BOMB, true);                                                        
        }

        Position GetBombPosition()
        {
            switch (go->GetEntry())
            {
            case GO_GNOME_FACE_01:
                return BombPositions[0];          
            case GO_GNOME_FACE_02:
                return BombPositions[1];
            case GO_GNOME_FACE_03:
                return BombPositions[2];
            case GO_GNOME_FACE_04:
                return BombPositions[3];
            case GO_GNOME_FACE_05:
                return BombPositions[4];
            case GO_GNOME_FACE_06:
                return BombPositions[5];
            default:
                return BombPositions[0];
                break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!_isActive)
                return;

            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SUMMON_BOMB:
                    if (Creature* bomb = go->SummonCreature(NPC_WALKING_BOMB, GetBombPosition()))
                    {
                        bomb->SetCorpseDelay(5);
                        bomb->GetMotionMaster()->MoveFall();                        
                    }                        
                    
                    _events.RescheduleEvent(EVENT_SUMMON_BOMB, 10 * IN_MILLISECONDS);
                    break;
                default:
                    break;
                }
            }
        }

        InstanceScript* _instance;
        EventMap _events;
        bool _isActive;
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return GetInstanceAI<go_gnome_faceAI>(go);
    }

};

void AddSC_boss_mekgineer_thermaplugg()
{
    new boss_mekgineer_thermaplugg();
    new npc_walking_bomb();
    new go_button();
    new go_gnome_face();
}
