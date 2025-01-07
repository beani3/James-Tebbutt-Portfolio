using System.Collections;
using System.Collections.Generic;
using UnityEngine;

//use classes from the system namespace
using System;

//inherits from another class, abstract class cannot exist on its own
public abstract class UFT_BaseStateFSMRBS
{
    //state update
    public abstract Type StateUpdate();
    //enter state
    public abstract Type StateEnter();
    //leave state
    public abstract Type StateExit();
}