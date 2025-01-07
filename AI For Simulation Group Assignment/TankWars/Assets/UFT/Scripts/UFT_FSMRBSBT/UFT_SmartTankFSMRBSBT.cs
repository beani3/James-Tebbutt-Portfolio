// IMPORTANT PLEASE READ: Atach StateMachineFSMRBSBT.cs //
// to the tank as this has the FSM that this script uses//

using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using static AStar;
using System;
using System.Runtime.CompilerServices;


/// <summary>
///currently 5am 13-12-2024 -- Remember to create new StateMachine for this Script and correct the FSM variables ///
/// </summary>
public class UFT_SmartTankFSMRBSBT : AITank
{
    // FSM Variables
    public Dictionary<string, bool> stats = new();
    public UFT_RulesFSMRBSBT rules = new();
    public bool lowHealth, lowFuel, lowAmmo, targetSpotted; // States


    public Dictionary<GameObject, float> enemyTanksFound = new ();  // Stores all enemy tanks found
    public Dictionary<GameObject, float> consumablesFound = new(); // Stores all consumables found
    public Dictionary<GameObject, float> enemyBasesFound = new (); // Stores all enemy bases found

    public GameObject enemyTank; // Stores the enemy tank
    public GameObject consumable; // Stores the consumable
    public GameObject enemyBase;   // Stores the enemy base


    public HeuristicMode heuristicMode;    // Heuristic mode for pathfinding

    // Behavior Tree Components
    public UFT_BTAction healthCheck; // Checks if health is low
    public UFT_BTAction ammoCheck; // Checks if ammo is low
    public UFT_BTAction fuelCheck; // checks if fuel is low
    public UFT_BTAction tankSpottedCheck;// Checks if enemy tank is in view
    public UFT_BTAction enemyBaseCheck; // Checks if enemy base is in view
    public UFT_BTAction attackCheck; // Checks if tank is in attack
    public UFT_BTAction consumableCheck; // Checks if consumable is in view
    public UFT_BTAction chaseCheck; // Checks if tank is in chase

    public UFT_BTSequence retreatSeq; // Retreat sequence
    public UFT_BTSequence attackSeq; // Attack sequence
    public UFT_BTSequence gatherSeq; // Gather sequence
    public UFT_BTSequence searchSeq; // Search sequence
    public UFT_BTSequence chaseSeq; // Chase sequence

    public UFT_BTSelector seqSelector; // Selector sequence



    /// <summary>
    ///WARNING, do not use void <c>Start()</c> function, use this <c>AITankStart()</c> function instead if you want to use Start method from Monobehaviour.
    ///Use this function to initialise your tank variables etc.
    /// </summary>
    public override void AITankStart() // Use this instead of Start()
    {
        UFT_InitializeStateMachine();//Initialise the FSM
        UFT_InitializeRules(); //Initialise the rules
        UFT_InitializeStats(); //Initialise the stats
        UFT_InitializeBehaviorTree(); //Initialise the behavior tree
    }

    void UFT_InitializeRules() //Initialise the rules
    {
        // Changes to retreat state if low health and in attack state
        rules.UFT_AddRules(new UFT_RuleFSMRBSBT("attackState", "lowHealth", typeof(UFT_RetreatStateFSMRBSBT), UFT_RuleFSMRBSBT.Predicate.And));
        // Changes to retreat state if low health and in chase state
        rules.UFT_AddRules(new UFT_RuleFSMRBSBT("chaseState", "lowHealth", typeof(UFT_RetreatStateFSMRBSBT), UFT_RuleFSMRBSBT.Predicate.And));
        // Changes to chase if target spotted and in search state
        rules.UFT_AddRules(new UFT_RuleFSMRBSBT("searchState", "targetSpotted", typeof(UFT_ChaseStateFSMRBSBT), UFT_RuleFSMRBSBT.Predicate.And));
        // Changes to search state if not in search state and target hasn't been spotted
        rules.UFT_AddRules(new UFT_RuleFSMRBSBT("searchState", "targetSpotted", typeof(UFT_SearchStateFSMRBSBT), UFT_RuleFSMRBSBT.Predicate.nAnd));
        // Changes to attack state if target reached and in search state
        rules.UFT_AddRules(new UFT_RuleFSMRBSBT("targetReached", "chaseState", typeof(UFT_AttackStateFSMRBSBT), UFT_RuleFSMRBSBT.Predicate.And));
        // Changes to attack state if target spotted and in chase state
        rules.UFT_AddRules(new UFT_RuleFSMRBSBT("searchState", "lowHealth", typeof(UFT_SearchStateFSMRBSBT), UFT_RuleFSMRBSBT.Predicate.Or));
        // Changes to attack state if target spotted and in search state
        rules.UFT_AddRules(new UFT_RuleFSMRBSBT("searchState", "lowFuel", typeof(UFT_SearchStateFSMRBSBT), UFT_RuleFSMRBSBT.Predicate.Or));
        // Changes to attack state if target spotted and in search state
        rules.UFT_AddRules(new UFT_RuleFSMRBSBT("searchState", "lowAmmo", typeof(UFT_SearchStateFSMRBSBT), UFT_RuleFSMRBSBT.Predicate.Or));
        // Changes to search state if low ammo and in search state
        rules.UFT_AddRules(new UFT_RuleFSMRBSBT("collectState", "consumableVisible", typeof(UFT_CollectStateFSMRBSBT), UFT_RuleFSMRBSBT.Predicate.And));
        // Changes to colelct state if consuamble spotted and in chase state
    }


    private void UFT_InitializeStateMachine() //Initialise the FSM
    {
        Dictionary<Type, UFT_BaseStateFSMRBSBT> states = new() // Dictionary of states
        {
            { typeof(UFT_SearchStateFSMRBSBT), new UFT_SearchStateFSMRBSBT(this) },
            // Add Search states to the dictionary
            { typeof(UFT_ChaseStateFSMRBSBT), new UFT_ChaseStateFSMRBSBT(this) },
            // Add Chase states to the dictionary
            { typeof(UFT_AttackStateFSMRBSBT), new UFT_AttackStateFSMRBSBT(this) },
            // Add Attack states to the dictionary
            { typeof(UFT_RetreatStateFSMRBSBT), new UFT_RetreatStateFSMRBSBT(this) },
            // Add Retreat states to the dictionary
            { typeof(UFT_WaitStateFSMRBSBT), new UFT_WaitStateFSMRBSBT(this) },
            //  Add Wait states to the dictionary
            { typeof(UFT_CollectStateFSMRBSBT), new UFT_CollectStateFSMRBSBT(this) }
            // Add Collect states to the dictionary
        };

        GetComponent<UFT_StateMachineFSMRBSBT>().SetStates(states);  // Set the states
    }

    private void UFT_InitializeStats() // Initialise the stats
    {
        stats.Add("lowHealth", lowHealth);  // Add low health to the stats
        stats.Add("lowFuel", lowFuel);    // Add low fuel to the stats
        stats.Add("lowAmmo", lowAmmo);// Add low ammo to the stats
        stats.Add("targetSpotted", false);// Add target spotted to the stats
        stats.Add("targetReached", false); //  Add target reached to the stats
        stats.Add("consumableVisible", false);// Add consumable visible to the stats
        stats.Add("retreatState", false);// Add retreat state to the stats
        stats.Add("chaseState", false);// Add chase state to the stats
        stats.Add("searchState", false);//  Add search state to the stats
        stats.Add("attackState", false);// Add attack state to the stats
        stats.Add("collectState", false);// Add collect state to the stats
    }

    private void UFT_InitializeBehaviorTree() // Initialise the behavior tree
    {
        healthCheck = new UFT_BTAction(CheckHealth);
        // Check health
        ammoCheck = new UFT_BTAction(CheckAmmo);
        // Check ammo
        fuelCheck = new UFT_BTAction(CheckFuel); // Check fuel
        tankSpottedCheck = new UFT_BTAction(CheckTargetSpotted);// Check if tank is spotted
        enemyBaseCheck = new UFT_BTAction(CheckTargetSpotted); // Check if enemy base is spotted
        consumableCheck = new UFT_BTAction(CheckConsumable); // Check if consumable is spotted
        chaseCheck = new UFT_BTAction(ChaseCheck); // Check if tank is in chase

        gatherSeq = new UFT_BTSequence(new List<UFT_BTBaseNode> { consumableCheck, healthCheck, fuelCheck, ammoCheck }); // Gather sequence
        retreatSeq = new UFT_BTSequence(new List<UFT_BTBaseNode> { healthCheck, fuelCheck, ammoCheck });// Retreat sequence
        attackSeq = new UFT_BTSequence(new List<UFT_BTBaseNode> { tankSpottedCheck,enemyBaseCheck, healthCheck, fuelCheck, ammoCheck }); // Attack sequence
        searchSeq = new UFT_BTSequence(new List<UFT_BTBaseNode> { tankSpottedCheck,enemyBaseCheck, healthCheck, fuelCheck, ammoCheck }); // Search sequence
        chaseSeq = new UFT_BTSequence(new List<UFT_BTBaseNode> { tankSpottedCheck, healthCheck, fuelCheck, ammoCheck }); // Chase sequence

        seqSelector = new UFT_BTSelector(new List<UFT_BTBaseNode> { gatherSeq, chaseSeq, retreatSeq, attackSeq }); // Selector sequenceorder

    }
    
    public UFT_BTNodeStates CheckConsumable()// Check if consumable is visible
    {
        if (stats["consumableVisible"])
        {
            return UFT_BTNodeStates.SUCCESS; 
        }
        else
        {
            return UFT_BTNodeStates.FAILURE; 
        }
    }

    public UFT_BTNodeStates CheckFuel() //
    {
        if (stats["lowFuel"])
        {
            return UFT_BTNodeStates.FAILURE;
        }
        else 
        { 
            return UFT_BTNodeStates.SUCCESS; 
        }
    }

    public UFT_BTNodeStates CheckAmmo() // Check if ammo is low
    {
        if (stats["lowAmmo"])
        {
            return UFT_BTNodeStates.FAILURE;
        }
        else
        {
            return UFT_BTNodeStates.SUCCESS;
        }
    }

    public UFT_BTNodeStates CheckHealth() // Check if health is low
    {
        if (stats["lowHealth"])
            {
                return UFT_BTNodeStates.FAILURE;
            }
            else
            {
                return UFT_BTNodeStates.SUCCESS;
            }
        }

    public UFT_BTNodeStates CheckTargetSpotted()// Check if target is spotted
    {
        if (stats["targetSpotted"])
        {
            return UFT_BTNodeStates.SUCCESS;
        }
        else
        {
            return UFT_BTNodeStates.FAILURE;
        }
    }

    public UFT_BTNodeStates CheckTargetReached() // Check if target is reached
    {
        if (stats["targetReached"])
        {
            return UFT_BTNodeStates.SUCCESS;
        }
        else
        {
            return UFT_BTNodeStates.FAILURE;
        }

    }

    public UFT_BTNodeStates ChaseCheck() // Check if tank is in chase
    {
        if (stats["chaseState"])
        {
            return UFT_BTNodeStates.SUCCESS;
        }
        else
        {
            return UFT_BTNodeStates.FAILURE;
        }
    }



    /// <summary>
    ///WARNING, do not use void <c>Update()</c> function, use this <c>AITankUpdate()</c> function instead if you want to use Start method from Monobehaviour.
    ///Function checks to see what is currently visible in the tank sensor and updates the Founds list. <code>First().Key</code> is used to get the first
    ///element found in that dictionary list and is set as the target, based on tank health, ammo and fuel. 
    /// </summary>
    public override void AITankUpdate()
    {
        //Update all currently visible.
        enemyTanksFound = VisibleEnemyTanks;
        consumablesFound = VisibleConsumables;
        enemyBasesFound = VisibleEnemyBases;

        enemyTank = enemyTanksFound.Count > 0 ? enemyTanksFound.First().Key : null;
        consumable = consumablesFound.Count > 0 ? consumablesFound.First().Key : null;
        enemyBase = enemyBasesFound.Count > 0 ? enemyBasesFound.First().Key : null;

        // Checks if enemy is in view
        if (enemyTanksFound.Count > 0 && enemyTanksFound.First().Key != null)
        {
            enemyTank = enemyTanksFound.First().Key;
        }
        else
        {
            enemyTank = null;
        }

        // Checks if consumable is in view
        if (consumablesFound.Count() > 0 && consumablesFound.First().Key != null)
        {
            consumable = consumablesFound.First().Key;
            FollowPathToWorldPoint(consumable, 1f, heuristicMode);
        }
        else
        {
            consumable = null;
        }

        // Checks if enemy base is in view 
        if (enemyBasesFound.Count() > 0)
        {
            enemyBase = enemyBasesFound.First().Key;
            if (enemyBase != null)
            {
                if (Vector3.Distance(transform.position, enemyBase.transform.position) < 25f)
                {
                    TurretFireAtPoint(enemyBase);
                }
                else
                {
                    FollowPathToWorldPoint(enemyBase, 1f, heuristicMode);
                }
            }
        }
        else
        {
            enemyBase = null;
        }

        UpdateStats();
        
    }

    public UFT_BTNodeStates CollectConsumable()
    {
        if (consumable != null)
        {
            stats["collectState"] = true;
            FollowPathToWorldPoint(consumable, 1f, heuristicMode);
            return UFT_BTNodeStates.SUCCESS;
        }

        // If no consumable is found, fail this action
        return UFT_BTNodeStates.FAILURE;
    }

    private void UpdateStats()
    {
        CheckFuel();
        CheckAmmo();
        CheckHealth();
        CheckTargetSpotted();
        CheckTargetReached();
        FuelCheck();
        AmmoCheck();
        HealthCheck();
        TargetReachedCheck();
        TargetSpottedCheck();
        if (lowFuel || lowAmmo || lowHealth)
        {
            ConsumableCheck();
        }
    }
        public void FuelCheck()
        {
            if (TankCurrentFuel < 50)
            {
                stats["lowFuel"] = true;
            }
            else
            {
                stats["lowFuel"] = false;
            }
        }
        public void AmmoCheck()
        {
            if (TankCurrentAmmo < 5)
            {
                stats["lowAmmo"] = true;
            }
            else
            {
                stats["lowAmmo"] = false;
            }
        }

        public void HealthCheck()
        {
            if (TankCurrentHealth < 35)
            {
                stats["lowHealth"] = true;
            }
            else
            {
                stats["lowHealth"] = false;
            }
        }

        public void TargetReachedCheck()
        {
            if (enemyTank != null)
            {
                if (Vector3.Distance(transform.position, enemyTank.transform.position) < 35)
                {
                    stats["targetReached"] = true;
                }
                else
                {
                    stats["targetReached"] = false;
                }
            }
            else
            {
                stats["targetReached"] = false;
            }
        }

        public void TargetSpottedCheck()
        {
            if (enemyTank != null)
            {
                if (Vector3.Distance(transform.position, enemyTank.transform.position) < 50)
                {
                    stats["targetSpotted"] = true;
                }
                else
                {
                    stats["targetSpotted"] = false;
                }
            }
            else
            {
                stats["targetSpotted"] = false;
            }
        }

    public void ConsumableCheck()
    {
        if (consumable != null)
        {
            if (Vector3.Distance(transform.position, consumable.transform.position) < 50)
            {
                stats["consumableVisible"] = true;
            }
            else
            {
                stats["consumableVisible"] = false;
            }
        }
        else
        {
            stats["consumableVisible"] = false;
        }

    }

    /// <summary>
    ///WARNING, do not use void <c>OnCollisionEnter()</c> function, use this <c>AIOnCollisionEnter()</c> function instead if you want to use OnColiisionEnter function from Monobehaviour.
    ///Use this function to see if tank has collided with anything.
    /// </summary>
    public override void AIOnCollisionEnter(Collision collision)
    {

    }



    /*******************************************************************************************************       
    Below are a set of functions you can use. These reference the functions in the AITank Abstract class
    and are protected. These are simply to make access easier if you an not familiar with inheritance and modifiers
    when dealing with reference to this class. This does mean you will have two similar function names, one in this
    class and one from the AIClass. 
    *******************************************************************************************************/


    /// <summary>
    /// Generate a path from current position to pointInWorld (GameObject). If no heuristic mode is set, default is Euclidean,
    /// </summary>
    /// <param name="pointInWorld">This is a gameobject that is in the scene.</param>
    public void GeneratePathToWorldPoint(GameObject pointInWorld)
    {
        a_FindPathToPoint(pointInWorld);
    }

    /// <summary>
    /// Generate a path from current position to pointInWorld (GameObject)
    /// </summary>
    /// <param name="pointInWorld">This is a gameobject that is in the scene.</param>
    /// <param name="heuristic">Chosen heuristic for path finding</param>
    public void GeneratePathToWorldPoint(GameObject pointInWorld, HeuristicMode heuristic)
    {
        a_FindPathToPoint(pointInWorld, heuristic);
    }

    /// <summary>
    ///Generate and Follow path to pointInWorld (GameObject) at normalizedSpeed (0-1). If no heuristic mode is set, default is Euclidean,
    /// </summary>
    /// <param name="pointInWorld">This is a gameobject that is in the scene.</param>
    /// <param name="normalizedSpeed">This is speed the tank should go at. Normalised speed between 0f,1f.</param>
    public void FollowPathToWorldPoint(GameObject pointInWorld, float normalizedSpeed)
    {
        a_FollowPathToPoint(pointInWorld, normalizedSpeed);
    }

    /// <summary>
    ///Generate and Follow path to pointInWorld (GameObject) at normalizedSpeed (0-1). 
    /// </summary>
    /// <param name="pointInWorld">This is a gameobject that is in the scene.</param>
    /// <param name="normalizedSpeed">This is speed the tank should go at. Normalised speed between 0f,1f.</param>
    /// <param name="heuristic">Chosen heuristic for path finding</param>
    public void FollowPathToWorldPoint(GameObject pointInWorld, float normalizedSpeed, HeuristicMode heuristic)
    {
        a_FollowPathToPoint(pointInWorld, normalizedSpeed, heuristic);
    }

    /// <summary>
    ///Generate and Follow path to a randome point at normalizedSpeed (0-1). Go to a randon spot in the playfield. 
    ///If no heuristic mode is set, default is Euclidean,
    /// </summary>
    /// <param name="normalizedSpeed">This is speed the tank should go at. Normalised speed between 0f,1f.</param>
    public void FollowPathToRandomWorldPoint(float normalizedSpeed)
    {
        a_FollowPathToRandomPoint(normalizedSpeed);
    }

    /// <summary>
    ///Generate and Follow path to a randome point at normalizedSpeed (0-1). Go to a randon spot in the playfield
    /// </summary>
    /// <param name="normalizedSpeed">This is speed the tank should go at. Normalised speed between 0f,1f.</param>
    /// <param name="heuristic">Chosen heuristic for path finding</param>
    public void FollowPathToRandomWorldPoint(float normalizedSpeed, HeuristicMode heuristic)
    {
        a_FollowPathToRandomPoint(normalizedSpeed, heuristic);
    }

    /// <summary>
    ///Generate new random point
    /// </summary>
    public void GenerateNewRandomWorldPoint()
    {
        a_GenerateRandomPoint();
    }

    /// <summary>
    /// Stop Tank at current position.
    /// </summary>
    public void TankStop()
    {
        a_StopTank();
    }

    /// <summary>
    /// Continue Tank movement at last know speed and pointInWorld path.
    /// </summary>
    public void TankGo()
    {
        a_StartTank();
    }

    /// <summary>
    /// Face turret to pointInWorld (GameObject)
    /// </summary>
    /// <param name="pointInWorld">This is a gameobject that is in the scene.</param>
    public void TurretFaceWorldPoint(GameObject pointInWorld)
    {
        a_FaceTurretToPoint(pointInWorld);
    }

    /// <summary>
    /// Reset turret to forward facing position
    /// </summary>
    public void TurretReset()
    {
        a_ResetTurret();
    }

    /// <summary>
    /// Face turret to pointInWorld (GameObject) and fire (has delay).
    /// </summary>
    /// <param name="pointInWorld">This is a gameobject that is in the scene.</param>
    public void TurretFireAtPoint(GameObject pointInWorld)
    {
        a_FireAtPoint(pointInWorld);
    }

    /// <summary>
    /// Returns true if the tank is currently in the process of firing.
    /// </summary>
    public bool TankIsFiring()
    {
        return a_IsFiring;
    }

    /// <summary>
    /// Returns float value of remaining health.
    /// </summary>
    /// <returns>Current health.</returns>
    public float TankCurrentHealth
    {
        get
        {
            return a_GetHealthLevel;
        }
    }

    /// <summary>
    /// Returns float value of remaining ammo.
    /// </summary>
    /// <returns>Current ammo.</returns>
    public float TankCurrentAmmo
    {
        get
        {
            return a_GetAmmoLevel;
        }
    }

    /// <summary>
    /// Returns float value of remaining fuel.
    /// </summary>
    /// <returns>Current fuel level.</returns>
    public float TankCurrentFuel
    {
        get
        {
            return a_GetFuelLevel;
        }
    }

    /// <summary>
    /// Returns list of friendly bases. Does not include bases which have been destroyed.
    /// </summary>
    /// <returns>List of your own bases which are. </returns>
    public List<GameObject> MyBases
    {
        get
        {
            return a_GetMyBases;
        }
    }

    /// <summary>
    /// Returns Dictionary(GameObject target, float distance) of visible targets (tanks in TankMain LayerMask).
    /// </summary>
    /// <returns>All enemy tanks currently visible.</returns>
    protected Dictionary<GameObject, float> VisibleEnemyTanks
    {
        get
        {
            return a_TanksFound;
        }
    }

    /// <summary>
    /// Returns Dictionary(GameObject consumable, float distance) of visible consumables (consumables in Consumable LayerMask).
    /// </summary>
    /// <returns>All consumables currently visible.</returns>
    protected Dictionary<GameObject, float> VisibleConsumables
    {
        get
        {
            return a_ConsumablesFound;
        }
    }

    /// <summary>
    /// Returns Dictionary(GameObject base, float distance) of visible enemy bases (bases in Base LayerMask).
    /// </summary>
    /// <returns>All enemy bases currently visible.</returns>
    protected Dictionary<GameObject, float> VisibleEnemyBases
    {
        get
        {
            return a_BasesFound;
        }
    }

}
