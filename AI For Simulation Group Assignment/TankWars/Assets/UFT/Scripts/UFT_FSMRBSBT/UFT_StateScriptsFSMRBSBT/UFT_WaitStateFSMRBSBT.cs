using System.Collections;
using System.Collections.Generic;
using UnityEngine;


using System;
//inherit from BaseState
public class UFT_WaitStateFSMRBSBT : UFT_BaseStateFSMRBSBT
{
    //UFT_Tank as an object
    private UFT_SmartTankFSMRBSBT UFT_Tank;

    //gets our tank
    public UFT_WaitStateFSMRBSBT(UFT_SmartTankFSMRBSBT UFT_Tank)
    {
        this.UFT_Tank = UFT_Tank;
    }
    //enter state
    public override Type StateEnter()
    {
        UFT_Tank.stats["waitState"] = true;
        return null;
    }
    //exit state
    public override Type StateExit()
    {
        UFT_Tank.stats["waitState"] = false;
        return null;
    }
    //update state
    public override Type StateUpdate()
    {
        UFT_Tank.TankStop();
        Debug.Log("Waiting");

        foreach (var item in UFT_Tank.rules.getRules)
        {
            if (item.CheckRule(UFT_Tank.stats) != null)
            {
                return item.CheckRule(UFT_Tank.stats);
            }
        }

        return null;
    }
}
