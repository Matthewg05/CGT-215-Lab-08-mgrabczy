
// imported nuget packages
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFPhysics.h>
#include <vector>
#include <SFML/Audio.hpp>

// imported namespaces
using namespace std;
using namespace sf;
using namespace sfp;

// the speed at which the crossbow moves
const float KB_SPEED = 0.2;

// contingancy in case the code cannot find a texture
void LoadTex(Texture& tex, string filename) {
    if (!tex.loadFromFile(filename)) {
        cout << "Could not load " << filename << endl;
    }
}
// alows us the move the crossbow using the arrow keys
void MoveCrossbow(PhysicsSprite& crossbow, int elapsedMS) {
    if (Keyboard::isKeyPressed(Keyboard::Right)) {
        Vector2f newPos(crossbow.getCenter());
        newPos.x = newPos.x + (KB_SPEED * elapsedMS);
        crossbow.setCenter(newPos);
    }
    if (Keyboard::isKeyPressed(Keyboard::Left)) {
        Vector2f newPos(crossbow.getCenter());
        newPos.x = newPos.x - (KB_SPEED * elapsedMS);
        crossbow.setCenter(newPos);
    }
}

Vector2f GetTextSize(Text text) {
    FloatRect r = text.getGlobalBounds();
    return Vector2f(r.width, r.height);
}


int main()
{
// creates the game window
    RenderWindow window(VideoMode(800, 600), "Duck Hunt");
    World world(Vector2f(0, 0));
    int score(0);
    int arrows(5);

    //loads the sound effect and music files
    SoundBuffer popBuffer;
    if (!popBuffer.loadFromFile("duckpop.ogg")) {
        cout << "could not load duckpop.ogg" << endl;
        exit(5);
    }
    Sound popSound;
    popSound.setBuffer(popBuffer);

    Music music;
    if (!music.openFromFile("duckmusic.ogg")) {
        cout << "Failed to load duckmusic.ogg ";
        exit(6);
    }
    music.play();
    //makes the music loop
	music.setLoop(true);

    //loads the image textures for the crossbow and arrow
    PhysicsSprite& crossBow = *new PhysicsSprite();
    Texture cbowTex;
    LoadTex(cbowTex, "Lab08images/crossbow.png");
    crossBow.setTexture(cbowTex);
    Vector2f sz = crossBow.getSize();
    crossBow.setCenter(Vector2f(400,
        600 - (sz.y / 2)));

    PhysicsSprite arrow;
    Texture arrowTex;
    LoadTex(arrowTex, "Lab08images/arrow.png");
    arrow.setTexture(arrowTex);
    bool drawingArrow(false);

    //creates the bounding boxes of the game window
    PhysicsRectangle top;
    top.setSize(Vector2f(800, 10));
    top.setCenter(Vector2f(400, 5));
    top.setStatic(true);
    world.AddPhysicsBody(top);

    PhysicsRectangle left;
    left.setSize(Vector2f(10, 600));
    left.setCenter(Vector2f(5, 300));
    left.setStatic(true);
    world.AddPhysicsBody(left);

    PhysicsRectangle right;
    right.setSize(Vector2f(10, 600));
    right.setCenter(Vector2f(795, 300));
    right.setStatic(true);
    world.AddPhysicsBody(right);

    //loads the image of the duck
    Texture redTex;
    LoadTex(redTex, "Lab08images/gwr-duck.png");
    PhysicsShapeList<PhysicsSprite> ducks;
    
    // if the arrow hits the bounding box of the window, it will disapear
    top.onCollision = [&drawingArrow, &world, &arrow]
    (PhysicsBodyCollisionResult result) {
        drawingArrow = false;
        world.RemovePhysicsBody(arrow);
        };

    // loads the font for the score, ammo, and game over text
    Font fnt;
    if (!fnt.loadFromFile("arial.ttf")) {
        cout << "Could not load font." << endl;
        exit(3);
    }
    //creates the in game clock to keep track of millisecons
    Clock clock;
    Time lastTime(clock.getElapsedTime());
    Time currentTime(lastTime);

    //makes it so you can only shoot one arrow at a time
    long duckMS(0);
    while ((arrows > 0) || drawingArrow) {
        currentTime = clock.getElapsedTime();
        Time deltaTime = currentTime - lastTime;
        long deltaMS = deltaTime.asMilliseconds();
        if (deltaMS > 9) {
            duckMS = duckMS + deltaMS;

            lastTime = currentTime;
            world.UpdatePhysics(deltaMS);
            MoveCrossbow(crossBow, deltaMS);
            //pressing the space bar shoots the arrow
            if (Keyboard::isKeyPressed(Keyboard::Space) &&
                !drawingArrow) {
                drawingArrow = true;
                arrows = arrows - 1;
                arrow.setCenter(crossBow.getCenter());
                arrow.setVelocity(Vector2f(0, -1));
                world.AddPhysicsBody(arrow);

            }
            if (duckMS > 500) { //500 MS is half a second
                duckMS = 0;
                
                //continually creates a duck with velocity every 5 seconds
                for (int i(0); i < 1; i++) {
                    PhysicsSprite& duck = ducks.Create();
                    duck.setTexture(redTex);
                    int x = 50 + ((700 / 5) * i);
                    Vector2f sz = duck.getSize();
                    duck.setCenter(Vector2f(x, 20 + (sz.y / 2)));
                    duck.setVelocity(Vector2f(0.5, 0));
                    world.AddPhysicsBody(duck);
                    // if a duck colides with an arrow, it will be removed
                    duck.onCollision = [&drawingArrow, &world, &arrow, &duck, &ducks, &score, &popSound, &right]
                        (PhysicsBodyCollisionResult result) {
                            if (result.object2 == arrow) {
                                popSound.play();
                                drawingArrow = false;
                                world.RemovePhysicsBody(arrow);
                                world.RemovePhysicsBody(duck);
                                ducks.QueueRemove(duck);
                                score += 10;
                                //if a duck collides with the right side of the window, it will dissapear
                             }
                            else if (result.object2 == right) {
                                world.RemovePhysicsBody(duck);
                                ducks.QueueRemove(duck);
							}
                        };
                }
            }
            window.clear();
            if (drawingArrow) {
                window.draw(arrow);
            }
            ducks.DoRemovals();
            for (PhysicsShape& duck : ducks) {
                window.draw((PhysicsSprite&)duck);
            }
            //places the text for the score and ammo counter
            window.draw(crossBow);
            Text scoreText;
            scoreText.setString(to_string(score));
            scoreText.setFont(fnt);
            scoreText.setPosition(Vector2f(30 - GetTextSize(scoreText).x, 560));
            window.draw(scoreText);
            Text arrowCountText;
            arrowCountText.setString(to_string(arrows));
            arrowCountText.setFont(fnt);
            arrowCountText.setPosition(Vector2f(790 - GetTextSize(arrowCountText).x, 560));
            window.draw(arrowCountText);
            

            window.display();

        }
    }
    //This establishes the game over screen and text
    window.display(); 
    Text gameOverText;
    gameOverText.setString("GAME OVER");
    gameOverText.setFont(fnt);
    sz = GetTextSize(gameOverText);
    gameOverText.setPosition(400 - (sz.x / 2), 300 - (sz.y / 2));
    window.draw(gameOverText);
    window.display();
	music.stop();
    while (true);

}
