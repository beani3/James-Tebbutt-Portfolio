using System.Collections;
using System.Collections.Generic;
using UnityEngine;


using System;
using System.Xml;
//inherit from BaseState
public class UFT_SearchStateFSMRBSBT : UFT_BaseStateFSMRBSBT
{
    //UFT_Tank as an object
    private UFT_SmartTankFSMRBSBT UFT_Tank;
    //gets our tank
    public UFT_SearchStateFSMRBSBT(UFT_SmartTankFSMRBSBT UFT_Tank)
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
        if (UFT_Tank.stats["collectState"] == false)
            {
            UFT_Tank.FollowPathToRandomWorldPoint(1f);
            Debug.Log("Searching");

            foreach (var item in UFT_Tank.rules.getRules)
            {
                if (item.CheckRule(UFT_Tank.stats) != null)
                {
                    return item.CheckRule(UFT_Tank.stats);
                }
            }
        }
        return null;
    }
}
