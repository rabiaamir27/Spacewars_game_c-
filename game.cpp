#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float PLAYER_SPEED = 300.0f;
const float BULLET_SPEED = 500.0f;
float ENEMY_SPEED = 100.0f;
const float SHOOT_COOLDOWN = 0.5f;

struct Bullet {
    sf::RectangleShape shape;
};

struct Enemy {
    sf::Sprite sprite;
};

void showStartMenu(sf::RenderWindow& window, sf::Font& font) {
    sf::Text titleText("Space Invaders", font, 50);
    titleText.setFillColor(sf::Color::Red);
    titleText.setPosition(WINDOW_WIDTH / 2 - titleText.getGlobalBounds().width / 2, WINDOW_HEIGHT / 3);

    sf::Text instructionText("Press Enter to Start", font, 30);
    instructionText.setFillColor(sf::Color::White);
    instructionText.setPosition(WINDOW_WIDTH / 2 - instructionText.getGlobalBounds().width / 2, WINDOW_HEIGHT / 2);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
            break;
        }

        window.clear();
        window.draw(titleText);
        window.draw(instructionText);
        window.display();
    }
}

void updateBullets(std::vector<Bullet>& bullets, float dt) {
    for (size_t i = 0; i < bullets.size(); ++i) {
        bullets[i].shape.move(0, -BULLET_SPEED * dt);

        if (bullets[i].shape.getPosition().y < 0) {
            bullets.erase(bullets.begin() + i);
            --i;
        }
    }
}

void updateEnemies(std::vector<Enemy>& enemies, float dt, float& direction) {
    float moveDistance = ENEMY_SPEED * dt * direction;

    for (size_t i = 0; i < enemies.size(); ++i) {
        enemies[i].sprite.move(moveDistance, 0);

        if (enemies[i].sprite.getPosition().x <= 0 ||
            enemies[i].sprite.getPosition().x >= WINDOW_WIDTH - enemies[i].sprite.getGlobalBounds().width) {
            direction *= -1;
            for (size_t j = 0; j < enemies.size(); ++j) {
                enemies[j].sprite.move(0, 10); // Move down when direction changes
            }
            break;
        }
    }
}

void checkBulletEnemyCollisions(std::vector<Bullet>& bullets, std::vector<Enemy>& enemies, int& score, sf::Sound& hitSound, sf::Text& scoreText) {
    for (size_t i = 0; i < bullets.size();) {
        bool hit = false;
        for (size_t j = 0; j < enemies.size();) {
            if (bullets[i].shape.getGlobalBounds().intersects(enemies[j].sprite.getGlobalBounds())) {
                bullets.erase(bullets.begin() + i);
                enemies.erase(enemies.begin() + j);
                hitSound.play();
                score += 10;
                scoreText.setString("Score: " + to_string(score));
                hit = true;
                break;
            }
            else {
                ++j;
            }
        }
        if (!hit) {
            ++i;
        }
    }
}

void checkWinCondition(const std::vector<Enemy>& enemies, int score, sf::RenderWindow& window, sf::Text& scoreText) {
    if (enemies.empty()) {
        scoreText.setString("    You Win!\nFinal Score: " + to_string(score));
        scoreText.setFillColor(sf::Color::Green);
        sf::FloatRect textBounds = scoreText.getGlobalBounds();
        scoreText.setPosition(WINDOW_WIDTH / 2 - textBounds.width / 2, WINDOW_HEIGHT / 2 - textBounds.height / 2);

        window.clear();
        window.draw(scoreText);
        window.display();
        sf::sleep(sf::seconds(3));
        window.close();
    }
}

void checkGameOver(const std::vector<Enemy>& enemies, const sf::Sprite& player, int score, sf::RenderWindow& window, sf::Text& scoreText) {
    for (size_t i = 0; i < enemies.size(); ++i) {
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
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Space Invaders");
    window.setFramerateLimit(60);

    // Load textures
    sf::Texture playerTexture, enemyTexture, backgroundTexture;
    if (!playerTexture.loadFromFile("player.png") ||
        !enemyTexture.loadFromFile("enemy.png") ||
        !backgroundTexture.loadFromFile("background.png")) {
        return -1;
    }

    // Background setup
    sf::Sprite background;
    background.setTexture(backgroundTexture);

    // Player setup
    sf::Sprite player(playerTexture);
    player.setPosition(WINDOW_WIDTH / 2 - player.getGlobalBounds().width / 2, WINDOW_HEIGHT - player.getGlobalBounds().height - 10);

    // Font and score setup
    sf::Font font;
    if (!font.loadFromFile("alien.ttf")) {
        return -1;
    }
    sf::Text scoreText("Score: 0", font, 20);
    scoreText.setPosition(10, 10);
    int score = 0;

    // Sound setup
    sf::SoundBuffer shootBuffer, hitBuffer;
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

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

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

        // Render
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