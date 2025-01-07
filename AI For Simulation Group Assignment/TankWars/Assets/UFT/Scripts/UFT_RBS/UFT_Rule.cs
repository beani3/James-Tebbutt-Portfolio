using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UFT_Rule
{
    public string atecedentA;
    public string atecedentB;
    public string consequent;
    public Predicate compare;
    public enum Predicate
    { And, Or, nAnd, nOr }

    public UFT_Rule(string atecedentA, string atecedentB, string consequent, Predicate compare)
    {
        this.atecedentA = atecedentA;
        this.atecedentB = atecedentB;
        this.consequent = consequent;
        this.compare = compare;
    }

    public Dictionary<string, bool> CheckRule(Dictionary<string, bool> facts)
    {
        bool atecedentABool = facts[atecedentA];
        bool atecedentBBool = facts[atecedentB];

        switch (compare)
        {
            case Predicate.And:
                if (atecedentABool && atecedentBBool)
                {
                    facts[consequent] = true;
                }
                else
                {
                    facts[consequent] = false;  
                }
                return facts;

            case Predicate.Or:
                if (atecedentABool || atecedentBBool)
                {
                    facts[consequent] = true;
                }
                else
                {
                    facts[consequent] = false;
                }
                return facts;

            case Predicate.nAnd:
                if (!atecedentABool && !atecedentBBool)
                {
                    facts[consequent] = true;
                }
                else
                {
                    facts[consequent] = false;
                }
                return facts;

            case Predicate.nOr:
                if (!atecedentABool || !atecedentBBool)
                {
                    facts[consequent] = true;
                }
                else
                {
                    facts[consequent] = true;
                }
                return facts;

            default:
                return null;
         }
    }
}
