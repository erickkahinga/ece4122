/*
Author: Erick Kahinga
Class: ECE4122
Last Date Modified: 10/2/24
Description: Main file tracks state for centipede game with Centipede implementation
*/

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <deque>
#include <random>
#include <cmath> 
#include "Bullet.h"

using namespace sf;

// Be careful changing window width and height and DO NOT maximize the window the scaling will be thrown off. Adjust window size here.
const float WINDOW_WIDTH = 1280.0f; // needs to be bigger than 800.0f
const float WINDOW_HEIGHT = 720.0f; // needs to be bigger than 600.0f
const float PLAYER_SPEED = 0.25f;
const float MIN_DISTANCE = 50.0f;



// Function to calculate the distance between two points
float distance(const sf::Vector2f& point1, const sf::Vector2f& point2) {
    return std::sqrt(std::pow(point1.x - point2.x, 2) + std::pow(point1.y - point2.y, 2));
}

bool checkCollisionBtwBullMush(const sf::RectangleShape& bullet, const sf::Sprite& mushroom) {
    return bullet.getGlobalBounds().intersects(mushroom.getGlobalBounds());
}

bool checkCollisionBtwBulletAndSpider(const sf::RectangleShape& bullet, const sf::Sprite& spider) {
    return bullet.getGlobalBounds().intersects(spider.getGlobalBounds());
}


// BETWEEN THESE DASHES ARE CLASSES/ENUMS USED IN THE MAIN FUNCTION. HAD TROUBLE IMPLEMENTING THEM IN SEPERATE FILES AND I WAS LOSING TIME. BUT THE BULLET CLASS WORKED JUST FINE HAHA //
/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


enum GameState { START_MENU, PLAYING };

class Mushroom {
private:
    sf::Sprite mushroomSprite;
    int health;

public:
    Mushroom(float x, float y, sf::Texture& fullTexture) : health(4) {
        mushroomSprite.setTexture(fullTexture);  // Set the texture passed to the constructor
        mushroomSprite.setPosition(x, y);    // Set the position of the mushroom
    }

    void update() {
        mushroomSprite.setPosition(mushroomSprite.getPosition());  // Allows for me to update position later
    }

    void takeDamage() {
        health--;

        if (health <= 0) {
            mushroomSprite.setPosition(-100, -100);  // If dead move off screen
        }
        else if (health == 1) {
            mushroomSprite.setColor(sf::Color::Red);
        }
        else if (health == 2) {
            mushroomSprite.setColor(sf::Color::Yellow);
        }
        else if (health == 3) {
            mushroomSprite.setColor(sf::Color::Green);
        }
    }

    const sf::Sprite& getSprite() const {
        return mushroomSprite;
    }

    bool isAlive() const {
        return health > 0;
    }

    // Get position for mushroom
    sf::Vector2f getPosition() const {
        return mushroomSprite.getPosition();
    }
};

class Spider {
private:
    sf::Sprite spiderSprite;
    float hopTimer;  // Timer for controlling hop intervals
    float directionX;  // Controls the horizontal direction (-1 for left, 1 for right)
    float hopDistance;  // The distance the spider hops each time
    float windowWidth;  // To keep track of the window's width
    float baseY;  // The base Y position
    float hopHeight;  // How high the spider hops vertically
    float hopDuration;  // Time for a complete hop up and down

public:
    Spider(float startX, float startY, float windowWidth, sf::Texture& spiderTexture) : directionX(1.0f), hopDistance(50.0f), windowWidth(windowWidth), baseY(startY), hopHeight(150.0f), hopDuration(2.0f) {
        spiderSprite.setTexture(spiderTexture);
        spiderSprite.setPosition(startX, startY);

        hopTimer = 0.0f;
    }

    void update(float deltaTime) {
        hopTimer += deltaTime;

        // Horizontal movement
        sf::Vector2f newPosition = spiderSprite.getPosition();
        newPosition.x += directionX * hopDistance * deltaTime;  // Moves smoother horizontally when I use deltaTime

        // Check if the spider hit the horizontal edges
        if (newPosition.x <= 0) {
            newPosition.x = 0;  // Clamp to the left edge
            directionX = 1.0f;  // Change direction to right
        }
        else if (newPosition.x + spiderSprite.getLocalBounds().width >= windowWidth) {
            newPosition.x = windowWidth - spiderSprite.getLocalBounds().width;  // Clamp to the right edge
            directionX = -1.0f;  // Change direction to left
        }

        // Moves smoother vertically when I use a sine wave
        float verticalOffset = std::sin((hopTimer / hopDuration) * 3.14159f) * hopHeight;
        newPosition.y = baseY - verticalOffset;

        spiderSprite.setPosition(newPosition);  // Update the spider's position

        // Reset the hopTimer if it's greater than the hop duration. Loops sine wave
        if (hopTimer >= hopDuration) {
            hopTimer = 0.0f;
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(spiderSprite);
    }

    sf::Sprite& getSprite() {
        return spiderSprite;
    }

    void setPosition(float x, float y) {
        spiderSprite.setPosition(x, y);
    }
};



class CentipedeSegment {
public:
    sf::Sprite sprite;
    bool alive;

    CentipedeSegment(sf::Texture& texture, float x, float y) : alive(true) {
        sprite.setTexture(texture);
        sprite.setPosition(x, y);
    }
};

class Centipede {
private:
    std::vector<CentipedeSegment> segments;
    sf::Texture& headTexture;
    sf::Texture& bodyTexture;
    float speed;
    int direction;              // 1 for right, -1 for left
    float moveDownDistance;
    float moveDownInterval;
    float moveDownTimer;
    std::deque<sf::Vector2f> path; // Path buffer to store head's historical positions
    float spacing;               // Required spacing between segments
    sf::Vector2f lastPathPoint;  // Last recorded path point

public:
    Centipede(sf::Texture& headTex, sf::Texture& bodyTex, int initialLength, float startX, float startY, float speed = 150.0f, int initialDirection = 1) : headTexture(headTex), bodyTexture(bodyTex), speed(speed), direction(initialDirection), moveDownDistance(20.0f), moveDownInterval(0.5f), moveDownTimer(0.0f), spacing(static_cast<float>(bodyTex.getSize().x) + 5.0f), lastPathPoint(startX, startY)
    {
        // Initialize head segment with head texture
        segments.emplace_back(headTexture, startX, startY);
        path.emplace_back(startX, startY);

        // Initialize body segments with body texture
        for (int i = 1; i < initialLength; ++i) {
            float x = startX - i * spacing;
            float y = startY;
            segments.emplace_back(bodyTexture, x, y);
            path.emplace_back(x, y);
        }
    }

    void update(float deltaTime, const std::vector<Mushroom>& mushrooms) {
        // Move head based on current direction and speed
        CentipedeSegment& head = segments[0];
        float distanceToMove = speed * deltaTime * direction;
        head.sprite.move(distanceToMove, 0.0f);

        // Check if head has moved enough to add a new path point
        if (distance(head.sprite.getPosition(), lastPathPoint) >= spacing) {
            path.emplace_back(head.sprite.getPosition());
            lastPathPoint = head.sprite.getPosition();
        }

        // Check for window boundaries and handle direction change
        if (head.sprite.getPosition().x <= 0 || head.sprite.getPosition().x + head.sprite.getGlobalBounds().width >= WINDOW_WIDTH) {
            direction *= -1;
            moveDown();
            // After moving down, reset the lastPathPoint to the new head position to avoid duplicate path points
            lastPathPoint = head.sprite.getPosition();
        }

        // Check for collision with mushrooms
        for (const auto& mushroom : mushrooms) {
            if (mushroom.isAlive() && head.sprite.getGlobalBounds().intersects(mushroom.getSprite().getGlobalBounds())) {
                direction *= -1;
                moveDown();
                // Adjust head position to prevent sticking
                if (direction == 1)
                    head.sprite.setPosition(mushroom.getPosition().x + mushroom.getSprite().getGlobalBounds().width, head.sprite.getPosition().y);
                else
                    head.sprite.setPosition(mushroom.getPosition().x - head.sprite.getGlobalBounds().width, head.sprite.getPosition().y);
                // Reset the lastPathPoint after collision to ensure correct path following
                lastPathPoint = head.sprite.getPosition();
                break;
            }
        }

        // Update the rest of the segments to follow the path buffer
        for (size_t i = 1; i < segments.size(); ++i) {
            if (path.size() > i) {
                // Each segment follows a point in the path buffer based on its order
                segments[i].sprite.setPosition(path[path.size() - i - 1]);
            }
        }

        // Limit the path buffer size to prevent it from growing indefinitely
        if (path.size() > segments.size() * 2) {
            path.pop_front();
        }
    }

    void moveDown() {
        for (auto& segment : segments) {
            segment.sprite.move(0.0f, moveDownDistance);
        }
    }

    void draw(sf::RenderWindow& window) {
        for (auto& segment : segments) {
            if (segment.alive)
                window.draw(segment.sprite);
        }
    }

    // Handle bullet collisions, returns number of segments hit
    int handleBulletCollision(const sf::RectangleShape& bullet) {
        int hitCount = 0;
        for (size_t i = 0; i < segments.size(); ++i) {
            if (segments[i].alive && bullet.getGlobalBounds().intersects(segments[i].sprite.getGlobalBounds())) {
                segments[i].alive = false;
                hitCount++;
                break; // Only one segment can be hit by a single bullet
            }
        }
        return hitCount;
    }

    // Check if centipede is alive
    bool isAlive() const {
        for (const auto& segment : segments) {
            if (segment.alive)
                return true;
        }
        return false;
    }
};



/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


int main()
{
    // Create a video mode object
    sf::VideoMode vm(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Create and open a window for the game
    RenderWindow window(vm, "Centipede Game", Style::Default);

    // Create a texture to hold a graphic on the GPU
    sf::Texture startMenuBackground;

    // Load a graphic into the texture
    if (!startMenuBackground.loadFromFile("assests/StartMenuBackground.png")) {
        printf("ERROR: main.cpp | Failed to load StartMenuBackground.png \n");
    }

    // Create a sprite
    sf::Sprite startMenuBackgroundSprite;

    // Attach the texture to the sprite
    startMenuBackgroundSprite.setTexture(startMenuBackground);

    // Define the target rectangle dimensions
    sf::FloatRect targetRect(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT); // Position (x, y) and size (width, height)

    // Get the original size of the sprite
    sf::FloatRect originalSize = startMenuBackgroundSprite.getGlobalBounds();

    // Calculate scale factors
    float scaleX = targetRect.width / originalSize.width;
    float scaleY = targetRect.height / originalSize.height;

    // Set the scale of the sprite to fit the target rectangle
    startMenuBackgroundSprite.setScale(scaleX, scaleY);

    // Set the position of the sprite to the target rectangle's position
    startMenuBackgroundSprite.setPosition(targetRect.left, targetRect.top);

    // Load player texture
    sf::Texture playerTexture;
    if (!playerTexture.loadFromFile("assests/StarShip.png")) {
        printf("ERROR: main.cpp | Failed to load StarShip.png \n");
    }

    // Create player sprite and apply the texture
    sf::Sprite player(playerTexture);
    player.setScale(1.0f, 1.0f);
    sf::Vector2f playerStartPos(WINDOW_WIDTH / 2, WINDOW_HEIGHT - 50);  // Player's starting position
    player.setPosition(playerStartPos);

    // Bullets
    std::vector<Bullet> bullets;

    sf::Texture fullMushroom;
    if (!fullMushroom.loadFromFile("assests/FullMushroom.jpg")) {
        printf("ERROR: main.cpp | Failed to load FullMushroom.jpg \n");
    }

    std::vector<Mushroom> mushrooms;

    // Random number generator for mushroom placement
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distX(0, WINDOW_WIDTH);
    std::uniform_real_distribution<float> distY(10, WINDOW_HEIGHT - 100);

    // Function to reset mushrooms
    auto resetMushrooms = [&]() {
        mushrooms.clear();
        for (int i = 0; i < 30; ++i) {
            sf::Vector2f newPosition;
            bool validPosition = false;

            while (!validPosition) {
                newPosition = sf::Vector2f(distX(gen), distY(gen));
                validPosition = true;

                // Check that the new mushroom is far enough from existing mushrooms
                for (const auto& mushroom : mushrooms) {
                    if (distance(mushroom.getPosition(), newPosition) < MIN_DISTANCE) {
                        validPosition = false;
                        break;
                    }
                }
            }

            mushrooms.emplace_back(Mushroom(newPosition.x, newPosition.y, fullMushroom));
        }
        };

    // Initialize mushrooms
    resetMushrooms();

    // Used to update the spider man animation
    sf::Clock clock;

    sf::Texture spiderTexture;
    if (!spiderTexture.loadFromFile("assests/SpiderMan.jpg")) {
        printf("ERROR: main.cpp | Failed to load SpiderMan.jpg \n");
    }

    Spider spider(WINDOW_WIDTH / 2, WINDOW_HEIGHT - 25, WINDOW_WIDTH, spiderTexture);
    bool spiderAlive = true;
    sf::Clock respawnClock;

    int playerLives = 3;  // Player starts with 3 lives
    int score = 0; // Score starts at 0
    GameState gameState = START_MENU;

    // Load font and create text object for displaying lives and score
    sf::Font font;
    if (!font.loadFromFile("./assests/KOMIKAP.ttf")) {
        printf("ERROR: main.cpp | Failed to load KOMIKAP.ttf \n");
    }

    sf::Text livesText;
    livesText.setFont(font);
    livesText.setCharacterSize(30);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition(WINDOW_WIDTH - (WINDOW_WIDTH - 100), WINDOW_HEIGHT - (WINDOW_HEIGHT - 5));

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(WINDOW_WIDTH - (WINDOW_WIDTH - 1000), WINDOW_HEIGHT - (WINDOW_HEIGHT - 5));


    sf::Texture centipedeBodyTexture;
    if (!centipedeBodyTexture.loadFromFile("assests/CentipedeBody.png")) {
        printf("ERROR: main.cpp | Failed to load CentipedeBody.png \n");
    }


    sf::Texture centipedeHeadTexture;
    if (!centipedeHeadTexture.loadFromFile("assests/CentipedeHead.png")) {
        printf("ERROR: main.cpp | Failed to load CentipedeHead.png \n");
    }

    Centipede* centipede = new Centipede(centipedeHeadTexture, centipedeBodyTexture, 10, WINDOW_WIDTH / 2, 50);

    // Main loop
    while (window.isOpen()) {

        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (gameState == START_MENU) {
            // Display the start menu background
            window.clear();
            window.draw(startMenuBackgroundSprite);
            window.display();

            // Start the game on Enter press
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
                gameState = PLAYING;
                playerLives = 3;  // Reset player lives
                score = 0; // Reset score
                player.setPosition(playerStartPos);  // Reset player position
                bullets.clear();  // Clear bullets
                resetMushrooms();  // Reset mushrooms
                // Reset centipede
                delete centipede;
                centipede = new Centipede(centipedeHeadTexture, centipedeBodyTexture, 10, WINDOW_WIDTH / 2, 50);
                spiderAlive = true;
                respawnClock.restart();
            }
        }


        if (gameState == PLAYING) {
            // Player movement
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && player.getPosition().x > 0) {
                player.move(-PLAYER_SPEED, 0);
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && player.getPosition().x + player.getGlobalBounds().width < WINDOW_WIDTH) {
                player.move(PLAYER_SPEED, 0);
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && player.getPosition().y + player.getGlobalBounds().height < WINDOW_HEIGHT) {
                player.move(0, PLAYER_SPEED);
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && player.getPosition().y + player.getGlobalBounds().height > WINDOW_HEIGHT - 100) {
                player.move(0, -PLAYER_SPEED);
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                if (bullets.empty() || bullets.back().shape.getPosition().y < player.getPosition().y - 100) {
                    // Create a new bullet at the player's current position
                    bullets.push_back(Bullet(player.getPosition().x + player.getGlobalBounds().width / 2 - 2.5f, player.getPosition().y));
                }
            }

            livesText.setString("LIVES: " + std::to_string(playerLives));
            scoreText.setString("SCORE: " + std::to_string(score));

            // Use a separate vector to track bullets to be removed
            std::vector<int> bulletsToRemove;

            // Update bullets and check for collisions
            for (size_t i = 0; i < bullets.size(); i++) {
                bullets[i].update();
                if (bullets[i].shape.getPosition().y < 0) {
                    bulletsToRemove.push_back(i);  // Mark bullet for removal
                    continue;
                }

                // Check for collisions with mushrooms
                for (auto& mushroom : mushrooms) {
                    if (mushroom.isAlive() && checkCollisionBtwBullMush(bullets[i].shape, mushroom.getSprite())) {
                        mushroom.takeDamage();  // Subtract health from the mushroom
                        bulletsToRemove.push_back(i);  // Mark bullet for removal
                        if (!mushroom.isAlive()) {
                            score += 4;
                        }
                        break;  // Exit inner loop after collision
                    }
                }

                // Check if a bullet hits the spider
                if (spiderAlive && checkCollisionBtwBulletAndSpider(bullets[i].shape, spider.getSprite())) {
                    spiderAlive = false;
                    score += 600;
                    respawnClock.restart();  // Start respawn timer
                    bulletsToRemove.push_back(i);
                }

                // Check if a bullet hits the centipede
                if (centipede->isAlive()) {
                    int hits = centipede->handleBulletCollision(bullets[i].shape);
                    if (hits > 0) {
                        score += 10 * hits; // Increment score for each hit segment
                        bulletsToRemove.push_back(i);
                    }
                }
            }

            // Remove bullets marked for deletion
            for (int i = bulletsToRemove.size() - 1; i >= 0; --i) {
                bullets.erase(bullets.begin() + bulletsToRemove[i]);
            }

            // Respawn spider after 1.5 seconds
            if (!spiderAlive && respawnClock.getElapsedTime().asSeconds() >= 1.5) {
                spiderAlive = true;
                spider.setPosition(distX(gen), WINDOW_HEIGHT - 25);  // Respawn spider
            }

            float deltaTime = clock.restart().asSeconds();

            // Update spider movement
            if (spiderAlive) {
                spider.update(deltaTime);
            }

            // Update centipede movement
            if (centipede->isAlive()) {
                centipede->update(deltaTime, mushrooms);
            }

            // Check for centipede reaching the bottom or colliding with player
            // (Optional: Implement game over or player damage)

            // Check for spider collision with mushrooms
            auto removeMushroom = mushrooms.begin();
            while (removeMushroom != mushrooms.end()) {
                if (spider.getSprite().getGlobalBounds().intersects(removeMushroom->getSprite().getGlobalBounds())) {
                    removeMushroom = mushrooms.erase(removeMushroom);  // Remove mushroom if spider collides with it
                }
                else {
                    ++removeMushroom;
                }
            }

            // Check for spider collision with player
            if (spider.getSprite().getGlobalBounds().intersects(player.getGlobalBounds())) {
                playerLives--;  // Decrease player lives by 1
                player.setPosition(playerStartPos);  // Reset player position to the start

                if (playerLives <= 0) {
                    gameState = START_MENU;  // Go back to start menu
                }
            }

            // Clear screen
            window.clear();

            // Draw mushrooms
            for (Mushroom& mushroom : mushrooms) {
                if (mushroom.isAlive()) {
                    window.draw(mushroom.getSprite());
                }
            }

            // Draw centipede
            if (centipede->isAlive()) {
                centipede->draw(window);
            }

            // Draw player
            window.draw(player);

            // Draw bullets
            for (auto& bullet : bullets) {
                window.draw(bullet.shape);
            }

            if (spiderAlive) {
                spider.draw(window);
            }

            // Draw the lives text
            window.draw(livesText);

            // Draw the score text
            window.draw(scoreText);

            // Display everything we drew
            window.display();
        }

    }

    delete centipede;

    return 0;
}
