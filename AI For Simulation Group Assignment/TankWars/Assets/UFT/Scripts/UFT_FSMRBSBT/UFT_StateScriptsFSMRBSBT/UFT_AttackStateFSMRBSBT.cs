using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System;
//inherit from BaseState
public class UFT_AttackStateFSMRBSBT : UFT_BaseStateFSMRBSBT
{
    //UFT_Tank as an object
    private UFT_SmartTankFSMRBSBT UFT_Tank;
    //gets our tank
    public UFT_AttackStateFSMRBSBT(UFT_SmartTankFSMRBSBT UFT_Tank)
    {
        this.UFT_Tank = UFT_Tank;
    }

    // tracks time in state
    public float time;

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

        if (time > 1f)
        {
            if (UFT_Tank.enemyTank != null)
            {
                Debug.Log("Attacking");
                UFT_Tank.TurretFaceWorldPoint(UFT_Tank.enemyTank);
                UFT_Tank.TurretFireAtPoint(UFT_Tank.enemyTank);               
            }
           
            else if (UFT_Tank.enemyBase != null)
            {
                Debug.Log("Attacking");
                UFT_Tank.TurretFaceWorldPoint(UFT_Tank.enemyBase);
                UFT_Tank.TurretFireAtPoint(UFT_Tank.enemyBase);
            }
            else if (UFT_Tank.enemyTank == null && UFT_Tank.enemyBase == null)
            {
             
                return typeof(UFT_SearchStateFSMRBSBT);
            }
        }

        return null;
    }
}
