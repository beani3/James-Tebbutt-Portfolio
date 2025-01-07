using System.Collections;
using System.Collections.Generic;
using UnityEngine;


using System;
//inherit from BaseState
public class UFT_ChaseStateFSMRBS : UFT_BaseStateFSMRBS
{
    //UFT_Tank as an object
    private UFT_SmartTankFSMRBS UFT_Tank;
    //gets our tank
    public UFT_ChaseStateFSMRBS(UFT_SmartTankFSMRBS UFT_Tank)
    {
        this.UFT_Tank = UFT_Tank;
    }
    //enter state
    public override Type StateEnter()
    {
        UFT_Tank.stats["chaseState"] = true;
        return null;
    }
    //exit state
    public override Type StateExit()
    {
        UFT_Tank.stats["chaseState"] = false;
        return null;
    }
    //update state
    public override Type StateUpdate()
    {
        if (UFT_Tank != null)
        {
            if (Vector3.Distance(UFT_Tank.transform.position, UFT_Tank.enemyTank.transform.position) < 50)
            {
                Debug.Log("Chasing");           
                UFT_Tank.FollowPathToWorldPoint(UFT_Tank.enemyTank, 1f);
            } 
            else
            {
                Debug.Log("Switching to searching");
                return typeof(UFT_SearchStateFSMRBS);
            }
        }
        else
        {
            Debug.Log("Switching to searching");
            return typeof(UFT_SearchStateFSMRBS);
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
