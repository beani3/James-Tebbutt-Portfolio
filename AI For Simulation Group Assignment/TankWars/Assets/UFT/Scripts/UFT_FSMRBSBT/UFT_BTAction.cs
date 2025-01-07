using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UFT_BTAction : UFT_BTBaseNode
{
    //Stores the function signature for the action
    public delegate UFT_BTNodeStates ActionNodeFunction();

    //Called to evaluate this node
    private ActionNodeFunction btAction;

    //The function is passed in and stored upon creating the action node
    public UFT_BTAction(ActionNodeFunction btAction)
    {
        this.btAction = btAction;
    }

    // Evaluates the action node
    public override UFT_BTNodeStates Evaluate()
    {
        switch (btAction())
        {
            case UFT_BTNodeStates.SUCCESS:
                btNodeState = UFT_BTNodeStates.SUCCESS;
                return btNodeState;
            case UFT_BTNodeStates.FAILURE:
                btNodeState = UFT_BTNodeStates.FAILURE;
                return btNodeState;
            default:
                btNodeState = UFT_BTNodeStates.FAILURE;
                return btNodeState;
        }
    }
}

