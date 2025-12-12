#include <gtest/gtest.h>
#include "../../GameEngine.h"
#include "../../PlayerCharacter.h"
#include "../../EnemyNPC.h"
#include "../../Tilemap.h"
#include "../../StaticObject.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

TEST(InputTest, Player_Movement_Test) {
    sf::RenderWindow window(sf::VideoMode(640, 640), "Test");
    PlayerCharacter player(100, 100, sf::Vector2f(0, 0), 0, sf::Vector2f(0, 0));
    GameEngine engine(window, player); 
    float dt;
    sf::Clock c_clock;
    dt = c_clock.restart().asSeconds();
    std::vector<sf::Keyboard::Key> v_cardinalInputs;
    // Adds cardinal inputs to a vector
    v_cardinalInputs.push_back(sf::Keyboard::W);
    v_cardinalInputs.push_back(sf::Keyboard::S);
    v_cardinalInputs.push_back(sf::Keyboard::A);
    v_cardinalInputs.push_back(sf::Keyboard::D);

    /* Iterates through the vector, testing whether the 
     * inputs have been registered 
     */
    for (int i = 0; i < v_cardinalInputs.size(); i++) {
        if (sf::Keyboard::isKeyPressed(v_cardinalInputs[i])) {
            engine.handleMovement(dt);
            EXPECT_TRUE(engine.getMoved());
        }
    }
}

 TEST(CollisionTest, Player_Enemy_Collision) {
     sf::Sprite s_sprite;
     sf::FloatRect fr_collider;
     EnemyBuilder enemyBuilder;
     sf::RenderWindow window(sf::VideoMode(640, 640), "Test");
     /* Creates an instance of the player and enemy at the same position 
      * and checking whether a collision has been detected 
      */
     PlayerCharacter player(100, 100, sf::Vector2f(0, 0), 0, sf::Vector2f(0, 0));
     std::shared_ptr<EnemyNPC> enemy = enemyBuilder
         .buildHealth(100)
         .buildSpeed(100)
         .buildPos(sf::Vector2f(0, 0))
         .buildScale(sf::Vector2f(1, 1))
         .buildSprite("../Assets/TinySwords/Factions/Goblins/Troops/TNT/Blue/TNT_Blue-Idle.png", 0, 0, sf::Vector2f(192, 192))
         .buildCollider(enemyBuilder.getSprite().getPosition(), 64, 64)
         .build();
     GameEngine engine(window, player, enemy); 
     float dt;
     sf::Clock c_clock;
     dt = c_clock.restart().asSeconds();
 
     engine.handleCollisions(dt);
     
     /* If the test were to fail, I'd want to abort the
      * tests as the player would not take damage if a collision
      * is not detected
      */
     ASSERT_TRUE(engine.getEnemyCol()); 
 }

 TEST(CollisionTest, Damage_Player) {
     PlayerCharacter player(100, 100, sf::Vector2f(0, 0), 0, sf::Vector2f(0, 0));

     player.takeDamage(25);
 
     EXPECT_EQ(player.getHealth(), 75);
 }

TEST(SpriteTest, Load_Texture) {
    sf::Texture t_testTexture;
    bool b_loaded = false;

    if (!t_testTexture.loadFromFile("../Assets/TinySwords/Factions/Knights/Troops/Warrior/Purple/Warrior_Purple-Idle.png")) {
        b_loaded = false;
    } else {
        b_loaded = true;
    }

    /* Want test to abort if failed as not loading the texture
     * would result in an incorrectly loaded sprite. 
     */
    ASSERT_TRUE(b_loaded);
}

TEST(SpriteTest, Load_Sprite) {
    StaticObject staticObj;
   
    staticObj.createObject("../Assets/TinySwords/Factions/Knights/Troops/Warrior/Purple/Warrior_Purple-Idle.png", 0, 0, sf::Vector2f(192, 192));
    
    ASSERT_TRUE(staticObj.getLoaded());
}

TEST(SpriteTest, Check_Sprite_Vector) {
    AnimationManager animManager;

    animManager.playAnimation("../Assets/TinySwords/Factions/Knights/Troops/Warrior/Purple/Warrior_Purple-Idle.png", 6, 0, 0, sf::Vector2f(192, 192));

    /* Abort if test fails as if the vector is filled incorrectly, the
     * animation will likely look wrong
     */
    ASSERT_EQ(animManager.getSpriteVec().size(), 6.0f);
}

/* Integration Test
 */
class BuilderMethodTest : public ::testing::Test {
protected:
    EnemyBuilder enemyBuilder;
    std::shared_ptr<EnemyNPC> enemy;

    /* Creates instance of enemy using the builder method. Integration test 
     * for the enemy variables and enemy shared pointer
     */
    void SetUp() override {
        enemy = enemyBuilder
            .buildHealth(100)
            .buildSpeed(100)
            .buildPos(sf::Vector2f(0, 0))
            .buildScale(sf::Vector2f(1, 1))
            .buildSprite("../Assets/TinySwords/Factions/Goblins/Troops/TNT/Blue/TNT_Blue-Idle.png", 0, 0, sf::Vector2f(192, 192))
            .buildCollider(enemyBuilder.getSprite().getPosition(), 64, 64)
            .build();      
    }

    void TearDown() override {
        // shared pointers handle memory management on their own so
        // not sure what to put here :)  
    }
};

/* Compares the enemies variables with their expected
 * values. Assures they are giving the intended outcomes 
 */
TEST_F(BuilderMethodTest, Check_Enemy_Health) {
    EXPECT_EQ(enemyBuilder.getHealth(), 100);
}

TEST_F(BuilderMethodTest, Check_Enemy_Speed) {
    EXPECT_EQ(enemyBuilder.getSpeed(), 100);
}

TEST_F(BuilderMethodTest, Check_Enemy_Pos) {
    EXPECT_EQ(enemyBuilder.getPos().x, 0);
    EXPECT_EQ(enemyBuilder.getPos().y, 0);
}

TEST_F(BuilderMethodTest, Check_Enemy_Scale) {
    EXPECT_EQ(enemyBuilder.getScale().x, 1);
    EXPECT_EQ(enemyBuilder.getScale().y, 1);
}

TEST_F(BuilderMethodTest, Check_Enemy_Sprite) {
    EXPECT_TRUE(enemyBuilder.getLoadSprite());
}

TEST_F(BuilderMethodTest, Check_Enemy_Collider) {
    EXPECT_TRUE(enemyBuilder.getCreateCollider());
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}