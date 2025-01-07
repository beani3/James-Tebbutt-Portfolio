using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System;
//inherit from BaseState
public class UFT_AttackStateFSMRBS : UFT_BaseStateFSMRBS
{
    //UFT_Tank as an object
    private UFT_SmartTankFSMRBS UFT_Tank;
    //gets our tank
    public UFT_AttackStateFSMRBS(UFT_SmartTankFSMRBS UFT_Tank)
    {
        this.UFT_Tank = UFT_Tank;
    }

    // tracks time in state
    public float time;
    public float attackTime;

    //enter state
    public override Type StateEnter()
    {
        UFT_Tank.stats["attackState"] = true;
        return null;
    }
    //exit state
    public override Type StateExit()
    {
        UFT_Tank.stats["attackState"] = false;
        return null;
    }
    //update state
    public override Type StateUpdate() {
        time += Time.deltaTime;

            if (UFT_Tank.stats["lowHealth"] == true)
            {
                Debug.Log("Switching to retreating");
                return typeof(UFT_RetreatStateFSMRBS);
            }
            if (UFT_Tank.enemyTank != null)
            {              
                Debug.Log("Attacking");
                UFT_Tank.TurretFaceWorldPoint(UFT_Tank.enemyTank);
                UFT_Tank.TurretFireAtPoint(UFT_Tank.enemyTank);
            }
            else
            {
                Debug.Log("Switching to searching");
                return typeof(UFT_SearchStateFSMRBS);
            }

        return null;
    }
}
