/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#ifndef TRINITY_TARGETEDMOVEMENTGENERATOR_H
#define TRINITY_TARGETEDMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
#include "FollowerReference.h"
#include "Timer.h"
#include "Unit.h"
#include "PathGenerator.h"

class TC_GAME_API TargetedMovementGeneratorBase
{
    public:
        TargetedMovementGeneratorBase(Unit* target) { i_target.link(target, this); }
        void stopFollowing() { }
    protected:
        FollowerReference i_target;
};

template<class T, typename D>
class TC_GAME_API TargetedMovementGenerator : public MovementGeneratorMedium< T, D >, public TargetedMovementGeneratorBase
{
    public:
        /**
        offset: Base distance between unit and target
        */
        explicit TargetedMovementGenerator(Unit* target, float offset, float angle) :
            TargetedMovementGeneratorBase(target), i_path(nullptr),
            i_recheckDistance(0), i_offset(offset), i_angle(angle),
            i_recalculatePath(false), i_targetReached(false), i_speedChanged(false),
            lastTargetXYZ(0.0f, 0.0f, 0.0f), lastOwnerXYZ(0.0f, 0.0f, 0.0f)
        {
        }

        ~TargetedMovementGenerator()
		{
			delete i_path;
		}

        bool DoUpdate(T*, uint32);
        Unit* GetTarget() const { return i_target.getTarget(); }

        void UnitSpeedChanged() { i_speedChanged = true; }
        bool IsReachable() const { return (i_path) ? (i_path->GetPathType() & PATHFIND_NORMAL) : true; }
        /** return true target position is within allowed distance of the owner */
        bool IsWithinAllowedDist(T* owner, float x, float y, float z);
        float GetAllowedDist(T* owner);
        float GetOffset() { return i_offset; }
        void SetOffset(float offset);
    protected:
        /* Update target locaton */
        void _setTargetLocation(T* owner);

        PathGenerator* i_path;
        TimeTrackerSmall i_recheckDistance;
        float i_offset;
        float i_angle;
        bool i_recalculatePath : 1;
        bool i_speedChanged : 1;
        bool i_targetReached : 1;

        Position lastOwnerXYZ;
        Position lastTargetXYZ;
        float lastOwnerSpeed;
};

template<class T>
class TC_GAME_API ChaseMovementGenerator : public TargetedMovementGenerator<T, ChaseMovementGenerator<T> >
{
    public:
        ChaseMovementGenerator(Unit* target)
            : TargetedMovementGenerator<T, ChaseMovementGenerator<T> >(target), _restoreWalking(false) { }
        ChaseMovementGenerator(Unit* target, float offset, float angle)
            : TargetedMovementGenerator<T, ChaseMovementGenerator<T> >(target, offset, angle), _restoreWalking(false) { }
        ~ChaseMovementGenerator() { }

        MovementGeneratorType GetMovementGeneratorType() { return CHASE_MOTION_TYPE; }

        bool DoInitialize(T*);
        void DoFinalize(T*);
        void DoReset(T*);
        void MovementInform(T*);

        static void _clearUnitStateMove(T* u) { u->ClearUnitState(UNIT_STATE_CHASE_MOVE); }
        static void _addUnitStateMove(T* u)  { u->AddUnitState(UNIT_STATE_CHASE_MOVE); }
        bool EnableWalking() const { return false;}
        bool _lostTarget(T* u) const { return u->GetVictim() != this->GetTarget(); }
        void _reachTarget(T*);
    private:
        bool _restoreWalking; //sunstrider addition
};

template<class T>
class TC_GAME_API FollowMovementGenerator : public TargetedMovementGenerator<T, FollowMovementGenerator<T> >
{
    public:
        FollowMovementGenerator(Unit* target)
            : TargetedMovementGenerator<T, FollowMovementGenerator<T> >(target){ }
        FollowMovementGenerator(Unit* target, float offset, float angle)
            : TargetedMovementGenerator<T, FollowMovementGenerator<T> >(target, offset, angle) { }
        ~FollowMovementGenerator() { }

        MovementGeneratorType GetMovementGeneratorType() { return FOLLOW_MOTION_TYPE; }

        bool DoInitialize(T*);
        void DoFinalize(T*);
        void DoReset(T*);
        void MovementInform(T*);

        static void _clearUnitStateMove(T* u) { u->ClearUnitState(UNIT_STATE_FOLLOW_MOVE); }
        static void _addUnitStateMove(T* u)  { u->AddUnitState(UNIT_STATE_FOLLOW_MOVE); }
        bool EnableWalking() const;
        bool _lostTarget(T*) const { return false; }
        void _reachTarget(T*) { }
    private:
        void _updateSpeed(T* owner);
};

#endif
