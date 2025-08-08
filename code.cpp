#include <SFML/Graphics.hpp> //for graphics
#include <SFML/Audio.hpp> //for audio
#include <vector> //it is a dynamic container that can grow or shrink in size during gameplay. It allows for efficient management of game objects 
#include <cstdlib> // MEMORY MANAGEMNET AND RANDOM NUMBER GENERATION
#include <ctime> //DELTA TIME
#include <string> //TO DISPLAY MSG

using namespace std;

// Constants for game configuration : CONSTANTS ARE GOOD FOR READABILITY AND GAME CONFIGURATION
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float PLAYER_SPEED = 300.0f;
const float BULLET_SPEED = 500.0f;
float ENEMY_SPEED = 100.0f;
const float SHOOT_COOLDOWN = 0.5f; // BULLETS DONT SHOOT CONTINIOUSLY RATHER THERE IS A GAP
 

// Bullet structure to represent player bullets
struct Bullet { //STRUCT IS COLLECTION OF VARIABLE AND IS A USER DEFINED DATA TYPE
    sf::RectangleShape shape;
};

// Enemy structure to represent enemy entities - TEXTURES
struct Enemy { 
    sf::Sprite sprite;
};
// WE HAVE 6 TO 7 FUNCTIONS AS SEPARATE FUNCTION FOR READABILITY AND MODULARITY
// Function to display the start menu

void showStartMenu(sf::RenderWindow& window, sf::Font& font) { //BUILT IN FUNCTION AND WE WILL GET FROM FILE alientft.
    // Title text configuration
    sf::Text titleText("Space Invaders", font, 50);
    titleText.setFillColor(sf::Color::Red);
    titleText.setPosition(WINDOW_WIDTH / 2 - titleText.getGlobalBounds().width / 2, WINDOW_HEIGHT / 3); //divided by 3 because we need it in the third part

    // Instruction text configuration
    sf::Text instructionText("Press Enter to Start", font, 30);
    instructionText.setFillColor(sf::Color::White);
    instructionText.setPosition(WINDOW_WIDTH / 2 - instructionText.getGlobalBounds().width / 2, WINDOW_HEIGHT / 2); // divided by 2 because we want it in the centre

    // Display the start menu until the player presses Enter
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close(); //window will close
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
            break; 
        }
//rendering the start menu on game
        window.clear();
        window.draw(titleText);
        window.draw(instructionText);
        window.display();
    }
}

// Function to update bullets
void updateBullets(std::vector<Bullet>& bullets, float dt) { //bullets are stored in vector
    for (size_t i = 0; i < bullets.size(); ++i) { 
        // Move bullets upward
        bullets[i].shape.move(0, -BULLET_SPEED * dt); // in SFML plane system is different as in this for up we use negative while in normal plane we use positive
// The origin of plane is top left corner
        // Remove bullets that go off-screen 
        if (bullets[i].shape.getPosition().y < 0) {
            bullets.erase(bullets.begin() + i);
            --i;
        }
    }
}

// Function to update enemies and handle direction changes
void updateEnemies(std::vector<Enemy>& enemies, float dt, float& direction) {
    float moveDistance = ENEMY_SPEED * dt * direction; // MULTIPLY BY dt to make the function frame rate independent
     //ENEMIES ARE STORED IN VECTOR

    for (size_t i = 0; i < enemies.size(); ++i) {
        // Move enemies horizontally 
        enemies[i].sprite.move(moveDistance, 0);

        // Change direction and move enemies down when they hit screen edges
        if (enemies[i].sprite.getPosition().x <= 0 ||
            enemies[i].sprite.getPosition().x >= WINDOW_WIDTH - enemies[i].sprite.getGlobalBounds().width) { 

            direction *= -1; //multiply by -1 so the direction changes 
            for (size_t j = 0; j < enemies.size(); ++j) {
                enemies[j].sprite.move(0, 10); // Move down when direction changes
            }
            break; //direction of last enemy chnages when all change
        }
    }
}

// Function to check for bullet-enemy collisions
//Bullet and ememy intersect then collision and both will disappear as we want to free memory to avpoid memory leaks 
void checkBulletEnemyCollisions(std::vector<Bullet>& bullets, std::vector<Enemy>& enemies, int& score, sf::Sound& hitSound, sf::Text& scoreText) {

    for (size_t i = 0; i < bullets.size();) {
        bool hit = false;
        for (size_t j = 0; j < enemies.size();) {
            // Check for collision between bullets and enemies
            if (bullets[i].shape.getGlobalBounds().intersects(enemies[j].sprite.getGlobalBounds())) {
                bullets.erase(bullets.begin() + i);
                enemies.erase(enemies.begin() + j);
//a sound will be played and score will be increased by 10
                hitSound.play();
                score += 10; // Increase score       
                scoreText.setString("Score: " + to_string(score)); //score will be changed from integer to string
                hit = true;
                break; 
            } else {
                ++j;
            }
        }
        if (!hit) { 
            ++i;
        }
    }
}

// Function to check if the player wins
void checkWinCondition(const std::vector<Enemy>& enemies, int score, sf::RenderWindow& window, sf::Text& scoreText) {
    if (enemies.empty()) {
        // Display win message
        scoreText.setString("    You Win!\nFinal Score: " + to_string(score));
        scoreText.setFillColor(sf::Color::Green);
        sf::FloatRect textBounds = scoreText.getGlobalBounds();
        scoreText.setPosition(WINDOW_WIDTH / 2 - textBounds.width / 2, WINDOW_HEIGHT / 2 - textBounds.height / 2);

        window.clear();
        window.draw(scoreText);
        window.display();
        sf::sleep(sf::seconds(3)); //3 sec gap for user to read the msg
        window.close();
    }
}

// Function to check if the game is over
void checkGameOver(const std::vector<Enemy>& enemies, const sf::Sprite& player, int score, sf::RenderWindow& window, sf::Text& scoreText) {
    for (size_t i = 0; i < enemies.size(); ++i) {
        // Check if any enemy reaches the player's position
        if (enemies[i].sprite.getPosition().y > WINDOW_HEIGHT - player.getGlobalBounds().height) {
            scoreText.setFillColor(sf::Color::Red);
            scoreText.setString("   Game Over!\nFinal Score: " + to_string(score));
            sf::FloatRect textBounds = scoreText.getGlobalBounds();
            scoreText.setPosition(WINDOW_WIDTH / 2 - textBounds.width / 2, WINDOW_HEIGHT / 2 - textBounds.height / 2);

            window.clear();
            window.draw(scoreText);
            window.display();
            sf::sleep(sf::seconds(3));
            window.close();
            break;
        }
    }
}

int main() {
    // Create the game window 
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Space Invaders");
    window.setFramerateLimit(60);

    // Load textures for the player, enemies, and background
    sf::Texture playerTexture, enemyTexture, backgroundTexture;
    if (!playerTexture.loadFromFile("player.png") || //portable network graphics
        !enemyTexture.loadFromFile("enemy.png") ||
        !backgroundTexture.loadFromFile("background.png")) {    //DT DIFFERENCE BETWEEN TWO TIMES
        return -1;
    }

    // Background setup
    sf::Sprite background;
    background.setTexture(backgroundTexture);

    // Player setup
    sf::Sprite player(playerTexture); 
    player.setPosition(WINDOW_WIDTH / 2 - player.getGlobalBounds().width / 2, WINDOW_HEIGHT - player.getGlobalBounds().height - 10); //GET GLOCBAL BOUND IS A BOUNDARY OF WINDOW

    // Font and score setup
    sf::Font font; 
    if (!font.loadFromFile("alien.ttf")) { //TRUE TYPE FONT
        return -1;
    }
    sf::Text scoreText("Score: 0", font, 20); 
    scoreText.setPosition(10, 10);
    int score = 0;

    // Sound setup
    sf::SoundBuffer shootBuffer, hitBuffer; // we dont load it directly first we convert it in .mp3
    if (!shootBuffer.loadFromFile("shoot.mp3") || !hitBuffer.loadFromFile("hit.mp3")) {
        return -1;
    }
    sf::Sound shootSound(shootBuffer);
    sf::Sound hitSound(hitBuffer);

    // Music setup
    sf::Music music;
    if (!music.openFromFile("music.mp3")) {
        return -1;
    }
    music.setLoop(true);
    music.play();

    // Vectors to hold bullets and enemies
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;

    // Enemy setup
    int rows = 4, cols = 8; 
    float enemyStartX = 100, enemyStartY = 50, enemySpacing = 60; 
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            Enemy enemy;
            enemy.sprite.setTexture(enemyTexture);
            enemy.sprite.setPosition(enemyStartX + col * enemySpacing, enemyStartY + row * 40);
            enemies.push_back(enemy);
        }
    }

    // Show start menu
    showStartMenu(window, font);

    sf::Clock clock;
    float shootTimer = 0.0f;
    float enemyDirection = 1.0f;

    // Main game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) { //poll.event is for event handling
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
//multiply with dt as we want to make it frame rate independent
        float dt = clock.restart().asSeconds();
        shootTimer -= dt;

        // Player movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && player.getPosition().x > 0) {
            player.move(-PLAYER_SPEED * dt, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) &&
            player.getPosition().x < WINDOW_WIDTH - player.getGlobalBounds().width) {
            player.move(PLAYER_SPEED * dt, 0);
        }

        // Shooting bullets
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && shootTimer <= 0) {
            Bullet bullet;
            bullet.shape.setSize({ 5, 20 });
            bullet.shape.setFillColor(sf::Color::Red);
            bullet.shape.setPosition(player.getPosition().x + player.getGlobalBounds().width / 2 - 2.5f, player.getPosition().y);
            bullets.push_back(bullet);
            shootSound.play();
            shootTimer = SHOOT_COOLDOWN;
        }

        // Update bullets and enemies
        updateBullets(bullets, dt);
        updateEnemies(enemies, dt, enemyDirection);

        // Check for bullet-enemy collisions
        checkBulletEnemyCollisions(bullets, enemies, score, hitSound, scoreText);

        // Increase difficulty as score increases
        if (score % 50 == 0 && score > 0) {
            ENEMY_SPEED += 0.60f;
        }

        // Check win condition
        checkWinCondition(enemies, score, window, scoreText);

        // Check game over condition
        checkGameOver(enemies, player, score, window, scoreText);

        // Render game objects
        window.clear();
        window.draw(background);
        window.draw(player);

        for (size_t i = 0; i < bullets.size(); ++i) {
            window.draw(bullets[i].shape);
        }
        for (size_t i = 0; i < enemies.size(); ++i) {
            window.draw(enemies[i].sprite);
        }

        window.draw(scoreText);
        window.display();
    }

    return 0;
}
