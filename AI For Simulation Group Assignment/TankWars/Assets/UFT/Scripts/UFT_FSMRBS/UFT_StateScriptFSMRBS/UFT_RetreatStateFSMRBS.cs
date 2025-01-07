using System.Collections;
using System.Collections.Generic;
using UnityEngine;


using System;
using UnityEditor.UIElements;
using JetBrains.Annotations;
using System.Threading;
using System.Linq;
//inherit from BaseState


public class UFT_RetreatStateFSMRBS : UFT_BaseStateFSMRBS
{
    //UFT_Tank as an object
    private UFT_SmartTankFSMRBS UFT_Tank;

    public UFT_RetreatStateFSMRBS(UFT_SmartTankFSMRBS UFT_Tank)
    {
        this.UFT_Tank = UFT_Tank;
    }

    //enter state
    public override Type StateEnter()
    {
        UFT_Tank.stats["retreatState"] = true;
        return null;
    }

    //exit state
    public override Type StateExit()
    {
        UFT_Tank.stats["retreatState"] = false;
        return null;
    }

    //update state
    public override Type StateUpdate()
    {
        if (UFT_Tank.MyBases[0] != null)
        {
            if (Vector3.Distance(UFT_Tank.transform.position, UFT_Tank.MyBases[0].transform.position) < 25)
            {
                Debug.Log("Switching to searching");
                return typeof(UFT_SearchStateFSMRBS);
            }
            else
            {
                Debug.Log("Retreating");
                UFT_Tank.FollowPathToWorldPoint(UFT_Tank.MyBases[0], 1f);
            }
        }
        else
        {
            UFT_Tank.FollowPathToRandomWorldPoint(1f);
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
