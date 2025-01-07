using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UFT_RuleFSMRBSBT
{
    public string atecedentA;
    public string atecedentB;
    public Type consequent;
    public Predicate compare;
    public enum Predicate
    { And, Or, nAnd, nOr }

    public UFT_RuleFSMRBSBT(string atecedentA, string atecedentB, Type consequent, Predicate compare)
    {
        this.atecedentA = atecedentA;
        this.atecedentB = atecedentB;
        this.consequent = consequent;
        this.compare = compare;
    }

    public Type CheckRule(Dictionary<string, bool> stats)
    {
        bool atecedentABool = stats[atecedentA];
        bool atecedentBBool = stats[atecedentB];

        switch (compare)
        {
            case Predicate.And:
                if (atecedentABool && atecedentBBool)
                {
                    return consequent;
                }
                else
                {
                    return null;  
                }

            case Predicate.Or:
                if (atecedentABool || atecedentBBool)
                {
                    return consequent;
                }
                else
                {
                    return null;
                }

            case Predicate.nAnd:
                if (!atecedentABool && !atecedentBBool)
                {
                    return consequent;
                }
                else
                {
                    return null;
                }

            case Predicate.nOr:
                if (!atecedentABool || !atecedentBBool)
                {
                    return consequent;
                }
                else
                {
                    return null;
                }

            default:
                return null;
         }
    }
}
