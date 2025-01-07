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
public class UFT_SmartTankBT : AITank
{

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




    /*
|| INC Jacks written BTress using state machine and rules.|| 

Hirakey Attack,consubale ,retreat ,search.
    


   Flee sequence - if fuel is low, if ammo is low, if health is low yet if enemey visable turn gun fire 

   Attack sequence -if enemy is visable or enemy base is visable, if ammo is high, if health is high, if fuel is high, turn gun,fire

   Search Selector - if no enemy is visable, if no consumable is visable, if no enemy base is visable,no enemy is visable if  health is high, if fuel is high, ,if ammo is high move to random point


*/


    public Dictionary<string, bool> facts = new Dictionary<string, bool>();
    List<string> shouldFollow = new List<string>();
    List<string> shouldRetreat = new List<string>();
    List<string> shouldAttack = new List<string>();
    List<string> shouldSearch = new List<string>();
    List<string> shouldWait = new List<string>();








    /// <summary>
    ///WARNING, do not use void <c>Start()</c> function, use this <c>AITankStart()</c> function instead if you want to use Start method from Monobehaviour.
    ///Use this function to initialise your tank variables etc.
    /// </summary>
    public override void AITankStart()
    {
        facts.Add("lowHealth", false);
        facts.Add("lowAmmo", false);
        facts.Add("lowFuel", false);

        facts.Add("consumableVisible", false);
        facts.Add("enemyBaseVisible", false);     
        facts.Add("enemyVisible", false);

        facts.Add("targetReached", false);

        facts.Add("goodHealth", false);
        facts.Add("goodFuel", false);
        facts.Add("goodAmmo", false);

        facts.Add("noConsumableVisible", false);
        facts.Add("noEnemyBaseVisible", false);
        facts.Add("noEnemyVisible", false);

        shouldFollow.Add("goodHealth");
        shouldFollow.Add("enemyVisible");        
        shouldFollow.Add("goodFuel");
  

        shouldRetreat.Add("lowHealth");
        shouldRetreat.Add("lowAmmo");
        shouldRetreat.Add("lowFuel");
        

        shouldAttack.Add("enemyVisible");
        shouldAttack.Add("goodAmmo");


        shouldSearch.Add("enemyVisible");
        shouldSearch.Add("consumableVisible");
        shouldSearch.Add("enemyBaseVisible");

        shouldWait.Add("lowFuel");

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


            
    

       facts["enemyVisible"] = enemyTanksFound.Count > 0 ;
       facts["consumableVisible"] = consumablesFound.Count > 0 ;
        facts["enemyBaseVisible"] = enemyBasesFound.Count > 0 ;

        facts["lowAmmo"] = TankCurrentAmmo < 8 ;
        facts["lowFuel"] = TankCurrentFuel < 31 ;     
        facts["lowHealth"] = TankCurrentHealth < 41 ;

        facts["goodHealth"] = TankCurrentHealth > 40;
        facts["goodFuel"] = TankCurrentFuel > 30;
        facts["goodAmmo"] = TankCurrentAmmo > 9;

        facts["noEnemyVisible"] = enemyTanksFound.Count == 0;
        facts["noConsumableVisible"] = consumablesFound.Count == 0;
        facts["noEnemyBaseVisible"] = enemyBasesFound.Count == 0;

        if (Sequence(shouldFollow))
        {
            Debug.Log("Follow");
            Follow();

        }
        else if (Selection(shouldWait))
        {
            Debug.Log("Wait");
            Wait();
        }
        else if (Selection(shouldRetreat))
        {Debug.Log("Retreat");
            
            Retreat();

        }
        else if (Sequence(shouldAttack))
        {
            Debug.Log("Attack");
           Attack();
        }
        else if (Selection(shouldSearch))
        {
            Debug.Log("Search");
           Search();
        }
        

    }

    void Search()
    {
        GenerateNewRandomWorldPoint();
        FollowPathToRandomWorldPoint(0.6f);
    }

    void Follow()
    {
        GeneratePathToWorldPoint(consumablesFound.First().Key);
        FollowPathToWorldPoint(consumablesFound.First().Key, 0.6f, heuristicMode);

        GeneratePathToWorldPoint(enemyTanksFound.First().Key);
        FollowPathToWorldPoint(enemyTanksFound.First().Key, 1f, heuristicMode);
    }
    void Retreat()
    {
        GeneratePathToWorldPoint(MyBases.First());
        FollowPathToWorldPoint(MyBases.First(), 0.6f, heuristicMode);
    }

    void Attack()
    {
        TurretFaceWorldPoint(enemyTanksFound.First().Key);
        TurretFireAtPoint(enemyTanksFound.First().Key);
    }

    void Wait()
    {
        TankStop();
    }

    bool Sequence(List<string> conditions)
    {
        foreach (var item in conditions)
        {
            if (facts[item])
            {
                continue;
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    bool Selection(List<string> conditions)
    {
        foreach (var item in conditions)
        {
            if (facts[item] == true)
            {
                return true;
            }
            else
            {
                continue;
            }
        }

        return false;
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
    protected List<GameObject> MyBases
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
