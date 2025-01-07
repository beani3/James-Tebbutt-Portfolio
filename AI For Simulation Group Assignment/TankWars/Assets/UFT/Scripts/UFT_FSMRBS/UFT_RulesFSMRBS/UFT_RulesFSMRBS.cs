using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UFT_RulesFSMRBS 
{
    public void UFT_AddRules(UFT_RuleFSMRBS rule)
    {
        getRules.Add(rule);
    }

    public List<UFT_RuleFSMRBS> getRules { get; } = new List<UFT_RuleFSMRBS>();
}
