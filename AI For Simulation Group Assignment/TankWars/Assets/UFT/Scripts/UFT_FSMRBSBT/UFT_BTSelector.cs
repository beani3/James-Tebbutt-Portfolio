using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UFT_BTSelector : UFT_BTBaseNode
{
    /** The child nodes for this selector */
    protected List<UFT_BTBaseNode> btNodes = new List<UFT_BTBaseNode>();

    /** The constructor requires a lsit of child nodes to be  
     * passed in*/
    public UFT_BTSelector(List<UFT_BTBaseNode> btNodes)
    {
        this.btNodes = btNodes;
    }

    /* If any of the children reports a success, the selector will 
     * immediately report a success upwards. If all children fail, 
     * it will report a failure instead.*/
    public override UFT_BTNodeStates Evaluate()
    {
        foreach (UFT_BTBaseNode btNode in btNodes)
        {
            switch (btNode.Evaluate())
            {
                case UFT_BTNodeStates.FAILURE:
                    continue;
                case UFT_BTNodeStates.SUCCESS:
                    btNodeState = UFT_BTNodeStates.SUCCESS;
                    return btNodeState;
                default:
                    continue;
            }
        }
        btNodeState = UFT_BTNodeStates.FAILURE;
        return btNodeState;
    }
}

