using System.Collections;
using System.Collections.Generic;
using UnityEngine;


using System;
using UnityEditor.UIElements;
using JetBrains.Annotations;
using System.Threading;
using System.Linq;
//inherit from BaseState


public class UFT_CollectStateFSMRBSBT : UFT_BaseStateFSMRBSBT
{
    //UFT_Tank as an object
    private UFT_SmartTankFSMRBSBT UFT_Tank;
    //gets our tank
    public UFT_CollectStateFSMRBSBT(UFT_SmartTankFSMRBSBT UFT_Tank)
    {
        this.UFT_Tank = UFT_Tank;
    }

    //enter state
    public override Type StateEnter()
    {
        UFT_Tank.stats["collectState"] = true;
        return null;
    }

    //exit state
    public override Type StateExit()
    {
        UFT_Tank.stats["collectState"] = false;
        return null;
    }

    //update state
    public override Type StateUpdate()
    {
        if (UFT_Tank.consumable != null)
        {
            UFT_Tank.FollowPathToWorldPoint(UFT_Tank.consumable, 1f, UFT_Tank.heuristicMode);
            return null;
        }
        else if (UFT_Tank.consumablesFound.Count == 0)
        {

            UFT_Tank.stats["consumableVisible"] = false;
            return typeof(UFT_SearchStateFSMRBSBT);
        }
        else
        {
            return null;
        }
    }
}