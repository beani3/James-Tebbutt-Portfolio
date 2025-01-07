using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System;
using System.Linq;


public class UFT_StateMachineFSMRBS : MonoBehaviour
{
    //creates a dictionary for the states for the FSM
    private Dictionary<Type, UFT_BaseStateFSMRBS> states;
    //BaseState = currentState
    public UFT_BaseStateFSMRBS currentState;

    public UFT_BaseStateFSMRBS CurrentState
    {
        //CurrentState returns the current state from BaseState
        get
        {
            return currentState;
        }
        private set
        {
            currentState = value;
        }
    }

    //sets the states from the dictionary with type and BaseState
    public void SetStates(Dictionary<Type, UFT_BaseStateFSMRBS> states)
    {
        this.states = states;
    }
    //check every frame, constantly.
    void Update()
    {
        if (CurrentState == null)
        {
            //sets current state as the first state if current state is not set
            CurrentState = states.Values.First();
            CurrentState.StateEnter();
        }
        else
        {
            //nextstate = update the base state
            var nextState = CurrentState.StateUpdate();

            //if the next state contains an update and if next state
            //does not have a return type
            if (nextState != null && nextState != CurrentState.GetType())
            {
                //switch the state to the next state
                SwitchToState(nextState);
            }
        }
    }
    //switches the state to the next state
    void SwitchToState(Type nextState)
    {
        //exits the current state
        CurrentState.StateExit();
        //gets the next state to use
        CurrentState = states[nextState];
        //enter the next state
        CurrentState.StateEnter();
    }
}
