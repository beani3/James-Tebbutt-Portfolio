using System.Collections;
using System.Collections.Generic;
using UnityEngine;


using System;
//inherit from BaseState
public class UFT_SearchStateFSMRBS : UFT_BaseStateFSMRBS
{
    //UFT_Tank as an object
    private UFT_SmartTankFSMRBS UFT_Tank;
    //gets our tank
    public UFT_SearchStateFSMRBS(UFT_SmartTankFSMRBS UFT_Tank)
    {
        this.UFT_Tank = UFT_Tank;
    }
    //enter state
    public override Type StateEnter()
    {
        UFT_Tank.stats["searchState"] = true;
        return null;
    }
    //exit state
    public override Type StateExit()
    {
        UFT_Tank.stats["searchState"] = false;
        return null;
    }
    //update state
    public override Type StateUpdate()
    {
        UFT_Tank.FollowPathToRandomWorldPoint(1f);
        Debug.Log("Searching");

        if (UFT_Tank.stats["lowFuel"] == true)
        {
            return typeof(UFT_WaitStateFSMRBS);
        }

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
