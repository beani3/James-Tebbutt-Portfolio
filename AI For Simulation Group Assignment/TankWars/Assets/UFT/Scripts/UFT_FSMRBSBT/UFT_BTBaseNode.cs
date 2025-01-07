using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public abstract class UFT_BTBaseNode
{
    //The current state of the node
    protected UFT_BTNodeStates btNodeState;

    //Return node state
    public UFT_BTNodeStates UFT_BTNodeState
    {
        get { return btNodeState; }
    }

    //Evaluate the desired set of conditions 
    public abstract UFT_BTNodeStates Evaluate();

}

