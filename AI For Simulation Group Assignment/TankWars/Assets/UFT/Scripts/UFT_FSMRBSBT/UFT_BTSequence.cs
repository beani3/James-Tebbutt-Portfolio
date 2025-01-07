using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UFT_BTSequence : UFT_BTBaseNode
{
    //The children of the sequencer
    protected List<UFT_BTBaseNode> btNodes = new List<UFT_BTBaseNode>();

    //Children set through constructor
    public UFT_BTSequence(List<UFT_BTBaseNode> btNodes)
    {
        this.btNodes = btNodes;
    }

    //If any child node returns a failure, the entire node fails. 
    public override UFT_BTNodeStates Evaluate()
    {
        bool failed = false;
        foreach (UFT_BTBaseNode btNode in btNodes)
        {
            if (failed == true)
            {
                break;
            }

            switch (btNode.Evaluate())
            {
                case UFT_BTNodeStates.FAILURE:
                    btNodeState = UFT_BTNodeStates.FAILURE;
                    failed = true;
                    break;
                case UFT_BTNodeStates.SUCCESS:
                    btNodeState = UFT_BTNodeStates.SUCCESS;
                    continue;
                default:
                    btNodeState = UFT_BTNodeStates.FAILURE;
                    failed = true;
                    break;
            }
        }
        return btNodeState;
    }
}

