using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using static AStar;
using System;
using System.Runtime.CompilerServices;


/// <summary>
/// Class <c>DumbTank</c> is an example class used to demonstrate how to use the functions available from the <c>AITank</c> base class. 
/// Copy this class when creating your smart tank class.
/// </summary>
public class UFT_SmartTankFSMRBS : AITank
{
    // RBS Variables
    public Dictionary<string, bool> stats = new Dictionary<string, bool>();
    public UFT_RulesFSMRBS rules = new UFT_RulesFSMRBS();
    public bool lowHealth;
    public bool lowFuel;
    public bool lowAmmo;

    public Dictionary<GameObject, float> enemyTanksFound = new Dictionary<GameObject, float>();     /*!< <c>enemyTanksFound</c> stores all tanks that are visible within the tanks sensor. */
    public Dictionary<GameObject, float> consumablesFound = new Dictionary<GameObject, float>();    /*!< <c>consumablesFound</c> stores all consumables that are visible within the tanks sensor. */
    public Dictionary<GameObject, float> enemyBasesFound = new Dictionary<GameObject, float>();     /*!< <c>enemyBasesFound</c> stores all enemybases that are visible within the tanks sensor. */

    public GameObject enemyTank;        /*!< <c>enemyTank</c> stores a reference to a target enemy tank. 
                                        * This should be taken from <c>enemyTanksFound</c>, only whilst within the tank sensor. 
                                        * Reference should be removed and refreshed every update. */

    public GameObject consumable;       /*!< <c>consumable</c> stores a reference to a target consumable. 
                                        * This should be taken from <c>consumablesFound</c>, only whilst within the tank sensor. 
                                        * Reference should be removed and refreshed every update. */

    public GameObject enemyBase;        /*!< <c>enemyBase</c> stores a reference to a target enemy base. 
                                         * This should be taken from <c>enemyBasesFound</c>, only whilst within the tank sensor. 
                                        * Reference should be removed and refreshed every update. */

    float t;    /*!< <c>t</c> stores timer value */
    public HeuristicMode heuristicMode; /*!< <c>heuristicMode</c> Which heuristic used for find path. */

    /// <summary>
    ///WARNING, do not use void <c>Start()</c> function, use this <c>AITankStart()</c> function instead if you want to use Start method from Monobehaviour.
    ///Use this function to initialise your tank variables etc.
    /// </summary>
    public override void AITankStart()
    {
        UFT_InitializeStateMachine();
        UFT_InitializeRules();
        UFT_InitializeStats();
    }

    private void UFT_InitializeStateMachine()
    {
        //dictionary is created for the states
        Dictionary<Type, UFT_BaseStateFSMRBS> states = new Dictionary<Type, UFT_BaseStateFSMRBS>();

        //add to dictionary type of states(IsClass), and name of states
        states.Add(typeof(UFT_SearchStateFSMRBS), new UFT_SearchStateFSMRBS(this));
        states.Add(typeof(UFT_ChaseStateFSMRBS), new UFT_ChaseStateFSMRBS(this));
        states.Add(typeof(UFT_AttackStateFSMRBS), new UFT_AttackStateFSMRBS(this));
        states.Add(typeof(UFT_RetreatStateFSMRBS), new UFT_RetreatStateFSMRBS(this));
        states.Add(typeof(UFT_WaitStateFSMRBS), new UFT_WaitStateFSMRBS(this));

        //sets the state machine with the dictionary
        GetComponent<UFT_StateMachineFSMRBS>().SetStates(states);
    }

    void UFT_InitializeStats()
    {
        stats.Add("lowHealth", lowHealth);
        stats.Add("lowFuel", lowFuel);
        stats.Add("lowAmmo", lowAmmo);
        stats.Add("targetSpotted", false);
        stats.Add("targetReached", false);
        stats.Add("retreatState", false);
        stats.Add("chaseState", false);
        stats.Add("searchState", false);
        stats.Add("attackState", false);
    }

    public void CheckTargetSpotted()
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

    public void CheckTargetReached()
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

    void UpdateStats()
    {
        stats["lowHealth"] = TankCurrentHealth < 30 ? true : false;
        stats["lowFuel"] = TankCurrentFuel < 50 ? true : false;
        stats["lowAmmp"] = TankCurrentAmmo < 5 ? true : false;

        CheckTargetSpotted();
        CheckTargetReached();
    }

    void UFT_InitializeRules()
    {
        // Changes to retreat state if low health and in attack state
        rules.UFT_AddRules(new UFT_RuleFSMRBS("attackState", "lowHealth", typeof(UFT_RetreatStateFSMRBS), UFT_RuleFSMRBS.Predicate.And));
        // Changes to retreat state if low health and in chase state
        rules.UFT_AddRules(new UFT_RuleFSMRBS("chaseState", "lowHealth", typeof(UFT_RetreatStateFSMRBS), UFT_RuleFSMRBS.Predicate.And));
        // Changes to chase state if target spotted and in search state
        rules.UFT_AddRules(new UFT_RuleFSMRBS("searchState", "targetSpotted", typeof(UFT_ChaseStateFSMRBS), UFT_RuleFSMRBS.Predicate.And));
        // Changes to search state if not in search state and target hasn't been spotted
        rules.UFT_AddRules(new UFT_RuleFSMRBS("searchState", "targetSpotted", typeof(UFT_SearchStateFSMRBS), UFT_RuleFSMRBS.Predicate.nAnd));
        // Changes to attack state state if target reached and in chase state
        rules.UFT_AddRules(new UFT_RuleFSMRBS("targetReached", "chaseState", typeof(UFT_AttackStateFSMRBS), UFT_RuleFSMRBS.Predicate.And));
        // Changes to wait state if low fuel and low ammo
        rules.UFT_AddRules(new UFT_RuleFSMRBS("lowFuel", "lowFuel", typeof(UFT_WaitStateFSMRBS), UFT_RuleFSMRBS.Predicate.And));
        // ^^ Still sorting out the implementation of this
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

    /// <summary>
    ///WARNING, do not use void <c>OnCollisionEnter()</c> function, use this <c>AIOnCollisionEnter()</c> function instead if you want to use OnColiisionEnter function from Monobehaviour.
    ///Use this function to see if tank has collided with anything.
    /// </summary>
    public override void AIOnCollisionEnter(Collision collision)
    {
        if (collision.gameObject.name == "DumbTank - DO NOT MODIFY -")
        {
            Debug.Log("Collided with enemy tank");
        }
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
