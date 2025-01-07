using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UFT_Rules 
{
    public void UFT_AddRules(UFT_Rule rule)
    {
        getRules.Add(rule);
    }

    public List<UFT_Rule> getRules { get; } = new List<UFT_Rule>();
}