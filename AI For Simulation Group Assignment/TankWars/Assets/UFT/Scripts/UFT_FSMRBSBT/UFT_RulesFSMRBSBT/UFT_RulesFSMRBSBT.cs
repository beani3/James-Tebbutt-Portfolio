using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UFT_RulesFSMRBSBT
{
    public void UFT_AddRules(UFT_RuleFSMRBSBT rule)
    {
        getRules.Add(rule);
    }

    public List<UFT_RuleFSMRBSBT> getRules { get; } = new List<UFT_RuleFSMRBSBT>();
}
