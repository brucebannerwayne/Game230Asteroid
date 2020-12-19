#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Main.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <random>
#include <string>

#define PI 3.1415926
const int WIDTH = 1000;
const int HEIGHT = 1000;
const float BUCKET_WIDTH = 250.0f;
const float BUCKET_HEIGHT = 250.0f;
const int COLUMNS = 4;
const int ROWS = 4;
sf::Texture bigAvoid;
sf::Texture midAvoid;
sf::Texture smallAvoid;
sf::Texture bigChaser;
sf::Texture midChaser;
sf::Texture smallChaser;
sf::Texture bigSlow;
sf::Texture midSlow;
sf::Texture smallSlow;

sf::SoundBuffer shootSnd;
sf::Sound shoot;
sf::SoundBuffer levelSnd;
sf::Sound levelChange;
sf::SoundBuffer shipExplodeSnd;
sf::Sound shipExplode;
sf::SoundBuffer asteroidSnd;
sf::Sound asteroidExplode;
sf::SoundBuffer thrustSnd;
sf::Sound thrustSound;
std::vector<sf::Vector2i> neighbor;
std::random_device rd;
std::default_random_engine generator{ rd() };
std::uniform_real_distribution<double> dist(0, 1.0);
sf::Vector2f Normalize(sf::Vector2f vector) {
	float mag = sqrtf(pow(vector.x, 2) + pow(vector.y, 2));
	return sf::Vector2f(vector.x / mag, vector.y / mag);
}
float Magnitude(sf::Vector2f a, sf::Vector2f b) {
	return sqrtf(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}
class VisualEffect {
public:
	sf::CircleShape shape;
	bool alive;
	sf::Clock remain;
	void Create(sf::Vector2f pos, float radius) {
		this->shape.setFillColor(sf::Color::Yellow);
		this->shape = sf::CircleShape(radius);
		this->shape.setPosition(pos);
		this->alive = true;
		remain.restart();
	}
	void draw(sf::RenderWindow* window) {
		if (this->alive) {
			window->draw(this->shape);
		}
		if (remain.getElapsedTime().asSeconds() > 1.0f) {
			this->alive = false;
		}
	}
};
std::vector<VisualEffect> asteroExplo;

class Ship {
public:
	sf::CircleShape shape;
	sf::Vector2f velocity;
	sf::Vector2f inertia;
	sf::Sprite sprite;
	sf::Clock respawnClock;
	bool alive;
	bool canThrust;
	int lives;
	void Move(float deltaTime, int dir) {
		auto position = this->shape.getPosition() + sf::Vector2f(this->shape.getRadius(), this->shape.getRadius());
		switch (dir)
		{
		case 1:
				this->shape.setPosition(position.x - this->shape.getRadius() + abs(this->velocity.x) * deltaTime, position.y - this->shape.getRadius());
				this->sprite.setPosition(position.x - this->shape.getRadius() + abs(this->velocity.x) * deltaTime, position.y - this->shape.getRadius());
				if (position.x >= WIDTH ) {
					this->shape.setPosition(fmodf(position.x, WIDTH - this->shape.getRadius()), position.y - this->shape.getRadius());
					this->sprite.setPosition(fmodf(position.x, WIDTH) - this->shape.getRadius(), position.y - this->shape.getRadius());
				}
				break;
		case 2: 
				this->shape.setPosition(position.x - this->shape.getRadius() - abs(this->velocity.x) * deltaTime, position.y - this->shape.getRadius());
				this->sprite.setPosition(position.x - this->shape.getRadius() - abs(this->velocity.x) * deltaTime, position.y - this->shape.getRadius());

				if (position.x <= 0) {
					this->shape.setPosition(WIDTH - this->shape.getRadius(), position.y - this->shape.getRadius());
					this->sprite.setPosition(WIDTH - this->shape.getRadius(), position.y - this->shape.getRadius());
				}
				break;
		case 3:
				this->shape.setPosition(position.x - this->shape.getRadius(), position.y - this->shape.getRadius() - abs(this->velocity.y) * deltaTime);
				this->sprite.setPosition(position.x - this->shape.getRadius(), position.y - this->shape.getRadius() - abs(this->velocity.y) * deltaTime);
				if (position.y <= 0) {
					this->shape.setPosition(position.x - this->shape.getRadius(), HEIGHT - this->shape.getRadius());
					this->sprite.setPosition(position.x - this->shape.getRadius(), HEIGHT - this->shape.getRadius());
				}
				break;
		case 4: 
				this->shape.setPosition(position.x - this->shape.getRadius(), position.y - this->shape.getRadius() + abs(this->velocity.y) * deltaTime);
				this->sprite.setPosition(position.x - this->shape.getRadius(), position.y - this->shape.getRadius() + abs(this->velocity.y) * deltaTime);
				if (position.y >= HEIGHT) {
					this->shape.setPosition(position.x - this->shape.getRadius(), fmodf(position.y, HEIGHT) - this->shape.getRadius());
					this->sprite.setPosition(position.x - this->shape.getRadius(), fmodf(position.y, HEIGHT) - this->shape.getRadius());
				}
				break;
		}
	}
	void thrust(float mX, float mY) {
		sf::Vector2f mP = sf::Vector2f(mX, mY);
		thrustSound.play();
		auto position = this->shape.getPosition() + sf::Vector2f(this->shape.getRadius(), this->shape.getRadius());
		sf::Vector2f dir = Normalize(mP - position);
		
		this->inertia = dir * 40.0f;
		this->shape.setPosition(position.x - this->shape.getRadius() + dir.x * 100.0f, position.y - this->shape.getRadius() + dir.y * 100.0f);
		this->sprite.setPosition(position.x - this->shape.getRadius() + dir.x * 100.0f, position.y - this->shape.getRadius() + dir.y * 100.0f);
		if (position.x >= WIDTH) {
			this->shape.setPosition(fmodf(position.x, WIDTH), position.y);
			this->sprite.setPosition(fmodf(position.x, WIDTH), position.y);
		}
		if (position.x <= 0) {
			this->shape.setPosition(WIDTH - this->shape.getRadius(), position.y);
			this->sprite.setPosition(WIDTH - this->shape.getRadius(), position.y);
		}
		if (position.y <= 0) {
			this->shape.setPosition(position.x, HEIGHT - this->shape.getRadius());
			this->sprite.setPosition(position.x, HEIGHT - this->shape.getRadius());
		}
		if (position.y >= HEIGHT) {
			this->shape.setPosition(position.x, fmodf(position.y, HEIGHT));
			this->sprite.setPosition(position.x, fmodf(position.y, HEIGHT));
		}
		this->canThrust = false;
	
		
	}

	void RotateAndMomentum(float mX, float mY, float deltaTime) {
		sf::Vector2f mP = sf::Vector2f(mX, mY );
		auto position = this->shape.getPosition() + sf::Vector2f(this->shape.getRadius(), this->shape.getRadius());
		sf::Vector2f dir = Normalize(mP - position);
		/*auto offset = position = this->shape.getOrigin();
		this->shape.setOrigin(position);
		this->shape.move(offset);*/
	
		this->shape.setRotation(atan2f(dir.y, dir.x) * 180 / PI  + 90);
		this->sprite.setRotation(atan2f(dir.y, dir.x) * 180 / PI  + 90);
		this->shape.setPosition(position.x - this->shape.getRadius() + this->inertia.x * deltaTime, position.y - this->shape.getRadius() + this->inertia.y * deltaTime );
		this->sprite.setPosition(position.x - this->shape.getRadius() + this->inertia.x * deltaTime, position.y - this->shape.getRadius() + this->inertia.y * deltaTime);
		if (position.x >= WIDTH) {
			this->shape.setPosition(fmodf(position.x, WIDTH), position.y);
			this->sprite.setPosition(fmodf(position.x, WIDTH), position.y);
		}
		if (position.x <= 0) {
			this->shape.setPosition(WIDTH - this->shape.getRadius(), position.y);
			this->sprite.setPosition(WIDTH - this->shape.getRadius(), position.y);
		}
		if (position.y <= 0) {
			this->shape.setPosition(position.x, HEIGHT - this->shape.getRadius());
			this->sprite.setPosition(position.x, HEIGHT - this->shape.getRadius());
		}
		if (position.y >= HEIGHT) {
			this->shape.setPosition(position.x, fmodf(position.y, HEIGHT));
			this->sprite.setPosition(position.x, fmodf(position.y, HEIGHT));
		}
		if (!this->alive) {
			if (this->respawnClock.getElapsedTime().asSeconds() > 3.0f) {
				this->shape.setFillColor(sf::Color::Black);
				this->alive = true;
			}
		}
		//this->inertia = this->inertia * 0.9f;
	
	}

	void Respawn() {
		this->alive = false;
		lives--;
		respawnClock.restart();
		this->shape.setFillColor(sf::Color::Red);
	}
	void draw(sf::RenderWindow* window) {
		window->draw(this->shape);
		window->draw(this->sprite);
	}
};
class Bullet {
public:
	sf::CircleShape shape;
	sf::Vector2f mouseDir;
	sf::Vector2f velocity;
	float speed;
	bool alive;
	sf::Clock life;
	void Init() {
		auto position = this->shape.getPosition() + sf::Vector2f(this->shape.getRadius(), this->shape.getRadius());
		velocity = Normalize(mouseDir - position);
	}
	void Update(float deltaTime) {
		auto position = this->shape.getPosition() + sf::Vector2f(this->shape.getRadius(), this->shape.getRadius());
		this->shape.setPosition(position - sf::Vector2f(this->shape.getRadius(), this->shape.getRadius()) + velocity * speed * deltaTime);
		if (position.x >= WIDTH) {
			this->shape.setPosition(fmodf(position.x, WIDTH) - this->shape.getRadius(), position.y - this->shape.getRadius());

		}
		if (position.x <= 0) {
			this->shape.setPosition(WIDTH - this->shape.getRadius() * 2.0f, position.y - this->shape.getRadius());

		}
		if (position.y <= 0) {
			this->shape.setPosition(position.x - this->shape.getRadius(), HEIGHT - this->shape.getRadius() * 2.0f);

		}
		if (position.y >= HEIGHT) {
			this->shape.setPosition(position.x - this->shape.getRadius(), fmodf(position.y, HEIGHT) - this->shape.getRadius());

		}
		if (life.getElapsedTime().asSeconds() >= 4.0f) {
			this->alive = false;
		}

	}
	void draw(sf::RenderWindow* window) {
		window->draw(this->shape);
	}
};
class PowerUp {
public:
	sf::CircleShape shape = sf::CircleShape(16.0f);
	sf::Sprite sprite;
	bool alive;
	int type;
	sf::Clock lifeTime;
	bool Update(Ship* ship) {
		auto position = this->shape.getPosition() + sf::Vector2f(this->shape.getRadius(), this->shape.getRadius());
		auto radius = this->shape.getRadius();
		auto shipRadius = ship->shape.getRadius();
		if (ship->alive) {
			if (Magnitude(position, ship->shape.getPosition() + sf::Vector2f(ship->shape.getRadius(), ship->shape.getRadius())) <= radius + shipRadius) {
				this->alive = false;
				return true;
			}
		}
		if (this->alive) {
			if (lifeTime.getElapsedTime().asSeconds() >= 5.0f) {
				this->alive = false;
			}
		}
		return false;
	}
	void draw(sf::RenderWindow * window) {
		window->draw(this->shape);
		window->draw(this->sprite);
	}
};
std::vector<PowerUp> powerupList;
class Asteroid {
public:
	sf::CircleShape shape;
	sf::Sprite sprite;
	sf::Vector2f velocity;
	float speed;
	bool alive;
	int size;
	int type;
	void Move(float deltaTime) {
		auto position = this->shape.getPosition() + sf::Vector2f(this->shape.getRadius(), this->shape.getRadius());
		auto radius = this->shape.getRadius();
		this->shape.setPosition(this->shape.getPosition() + Normalize(velocity) * speed * deltaTime);
		this->sprite.setPosition(this->shape.getPosition());
		if (position.x > WIDTH) {
			this->shape.setPosition(fmodf(position.x, WIDTH - this->shape.getRadius()), position.y - this->shape.getRadius());
			this->sprite.setPosition(fmodf(position.x, WIDTH) - this->shape.getRadius(), position.y - this->shape.getRadius());
		}
		if (position.x < 0) {
			this->shape.setPosition(WIDTH - this->shape.getRadius(), position.y - this->shape.getRadius());
			this->sprite.setPosition(WIDTH - this->shape.getRadius(), position.y - this->shape.getRadius());
		}
		if (position.y < 0) {
			this->shape.setPosition(position.x - this->shape.getRadius(), HEIGHT - this->shape.getRadius());
			this->sprite.setPosition(position.x - this->shape.getRadius(), HEIGHT - this->shape.getRadius());
		}
		if (position.y > HEIGHT) {
			this->shape.setPosition(position.x - this->shape.getRadius(), fmodf(position.y, HEIGHT) - this->shape.getRadius());
			this->sprite.setPosition(position.x - this->shape.getRadius(), fmodf(position.y, HEIGHT) - this->shape.getRadius());
		}
	}
	bool Update(Bullet *bullet) {
		auto position = this->shape.getPosition() + sf::Vector2f(this->shape.getRadius(), this->shape.getRadius());
		auto radius = this->shape.getRadius();
		
		
		if (bullet != nullptr) {
				auto bulletRadius = bullet->shape.getRadius();
				if (this->type == 4) {
					if (Magnitude(position, bullet->shape.getPosition() + sf::Vector2f(bullet->shape.getRadius(), bullet->shape.getRadius())) <= 15.0f * radius + bulletRadius) {
						bullet->speed -= 10.0f;
						if (bullet->speed <= 50.0f) {
							bullet->speed = 50.0f;
						}
					}
				}
				if (Magnitude(position, bullet->shape.getPosition() + sf::Vector2f(bullet->shape.getRadius(), bullet->shape.getRadius())) <= radius + bulletRadius) {
					bullet->alive = false;
					this->alive = false;
					VisualEffect explode;
					explode.Create(this->shape.getPosition(), this->shape.getRadius());
					asteroExplo.push_back(explode);
					asteroidExplode.play();
					double chance = dist(generator);
					if (chance > 0.9f) {
						PowerUp extLife;
						extLife.shape.setPosition(this->shape.getPosition());
						extLife.sprite.setPosition(extLife.shape.getPosition());
						extLife.alive = true;
						extLife.shape.setFillColor(sf::Color::Black);
						extLife.type = 1;
						powerupList.push_back(extLife);
					}
					else if (chance < 0.2f) {
						PowerUp snip;
						snip.shape.setPosition(this->shape.getPosition());
						snip.sprite.setPosition(snip.shape.getPosition());
						snip.alive = true;
						snip.shape.setFillColor(sf::Color::Black);
						snip.type = 2;
						powerupList.push_back(snip);
					}
					else if (chance >= 0.2f && chance < 0.4f) {
						PowerUp speedup;
						speedup.shape.setPosition(this->shape.getPosition());
						speedup.sprite.setPosition(speedup.shape.getPosition());
						speedup.alive = true;
						speedup.shape.setFillColor(sf::Color::Black);
						speedup.type = 3;
						powerupList.push_back(speedup);
					}
					return true;
				}
		}
		
		return false;
	}
	bool CollideWithShip(Ship* ship) {
		auto position = this->shape.getPosition() + sf::Vector2f(this->shape.getRadius(), this->shape.getRadius());
		auto radius = this->shape.getRadius();
		auto shipRadius = ship->shape.getRadius();
		if (ship->alive) {
			if (this->type == 2) {
				this->velocity = ship->shape.getPosition() - this->shape.getPosition();
				this->speed += 0.5f;
				if (this->speed >= 300.0f) {
					this->speed = 300.0f;
				}
				switch (this->size)
				{
				case 1:
					this->sprite.setTexture(smallChaser);
					break;
				case 2:
					this->sprite.setTexture(midChaser);
					break;
				case 3:
					this->sprite.setTexture(bigChaser);
					break;
				default:
					break;
				}
			}
			else if (this->type == 3) {
				if (Magnitude(position, ship->shape.getPosition() + sf::Vector2f(ship->shape.getRadius(), ship->shape.getRadius())) <= 10.0f * radius + shipRadius) {
					this->velocity = this->shape.getPosition() - ship->shape.getPosition();
					this->speed += 0.5f;
					if (this->speed >= 500.0f) {
						this->speed = 500.0f;
					}
				}
					switch (this->size)
					{
					case 1:
						this->sprite.setTexture(smallAvoid);
						break;
					case 2:
						this->sprite.setTexture(midAvoid);
						break;
					case 3:
						this->sprite.setTexture(bigAvoid);
						break;
					default:
						break;
					}
			}
			else if (type == 4) {
				switch (this->size)
					{
					case 1:
						this->sprite.setTexture(smallSlow);
						break;
					case 2:
						this->sprite.setTexture(midSlow);
						break;
					case 3:
						this->sprite.setTexture(bigSlow);
						break;
					default:
						break;
					}
			}
			if (Magnitude(position, ship->shape.getPosition() + sf::Vector2f(ship->shape.getRadius(), ship->shape.getRadius())) <= radius + shipRadius) {
				ship->Respawn();
				ship->alive = false;
				shipExplode.play();
				this->alive = false;
				VisualEffect explode;
				explode.Create(this->shape.getPosition(), this->shape.getRadius());
				asteroExplo.push_back(explode);
				asteroidExplode.play();
				return true;
			}
		}
		return false;
	}
	void draw(sf::RenderWindow* window) {
		window->draw(this->shape);
		window->draw(this->sprite);
	}
};
int random(int a) {
	srand(time(0));
	return rand() % a;
}
float random() {
	srand(time(0));
	return rand() / double(RAND_MAX);
}
sf::Vector2i getBucket(sf::Vector2f pos) {
	int col = int(pos.x / BUCKET_WIDTH);
	if (col < 0) {
		col = 0;
	}
	else if (col >= COLUMNS) {
		col = COLUMNS - 1;
	}
	int row = int(pos.y / BUCKET_HEIGHT);
	if (row < 0) {
		row = 0;
	}
	else if(row >= ROWS)
	{
		row = ROWS - 1;
	}
	return sf::Vector2i(row, col);
}
void getNeighbour(sf::Vector2i pos) {
	neighbor.clear();
	int a = pos.x + 1;
	if (a >= COLUMNS) {
		a = 0;
	}
	int b = pos.x - 1;
	if (b < 0) {
		b = COLUMNS - 1;
	}
	int c = pos.y - 1;
	if (c < 0) {
		c = ROWS - 1;
	}
	int d = pos.y + 1;
	if (d >= ROWS) {
		d = 0;
	}
	neighbor.push_back(sf::Vector2i(a, pos.y));
	neighbor.push_back(sf::Vector2i(a, d));
	neighbor.push_back(sf::Vector2i(pos.x, d));
	neighbor.push_back(sf::Vector2i(b, d));
	neighbor.push_back(sf::Vector2i(b, pos.y));
	neighbor.push_back(sf::Vector2i(b, c));
	neighbor.push_back(sf::Vector2i(pos.x, c));
	neighbor.push_back(sf::Vector2i(a,c));
}

int main()
{
	std::random_device rd;
	std::default_random_engine generator{rd()};
	std::uniform_real_distribution<double> distribution(-1.0, 1.0);

	if (!shootSnd.loadFromFile("Data/Sounds/Shoot.wav")) {
		return 1;
	}
	shoot.setBuffer(shootSnd);
	if (!levelSnd.loadFromFile("Data/Sounds/Level.wav")) {
		return 1;
	}
	levelChange.setBuffer(levelSnd);
	if (!thrustSnd.loadFromFile("Data/Sounds/Thrust.wav")) {
		return 1;
	}
	thrustSound.setBuffer(thrustSnd);
	if (!asteroidSnd.loadFromFile("Data/Sounds/AsteroidExplode.wav")) {
		return 1;
	}
	asteroidExplode.setBuffer(asteroidSnd);
	if (!shipExplodeSnd.loadFromFile("Data/Sounds/ShipExplode.wav")) {
		return 1;
	}
	shipExplode.setBuffer(shipExplodeSnd);
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Asteroids");
	window.setVerticalSyncEnabled(true);
	sf::Texture shipTexture;
	if (!shipTexture.loadFromFile("Data/Sprites/ship.png")) {
		return 1;
	}
	shipTexture.setSmooth(true);
	sf::Texture BigAsterTexture;
	if (!BigAsterTexture.loadFromFile("Data/Sprites/BigAster.png")) {
		return 1;
	}
	BigAsterTexture.setSmooth(true);
	sf::Texture MidAsterTexture;
	if (!MidAsterTexture.loadFromFile("Data/Sprites/MidAster.png")) {
		return 1;
	}
	MidAsterTexture.setSmooth(true);
	sf::Texture SmallAsterTexture;
	if (!SmallAsterTexture.loadFromFile("Data/Sprites/SmallAster.png")) {
		return 1;
	}
	SmallAsterTexture.setSmooth(true);
	sf::Texture extralife;
	if (!extralife.loadFromFile("Data/Sprites/ExtraLife.png")) {
		return 1;
	}
	extralife.setSmooth(true);
	sf::Texture snipe;
	if (!snipe.loadFromFile("Data/Sprites/Snipe.png")) {
		return 1;
	}
	snipe.setSmooth(true);
	sf::Texture speedUp;
	if (!speedUp.loadFromFile("Data/Sprites/SpeedUp.png")) {
		return 1;
	}
	speedUp.setSmooth(true);
	if (!bigAvoid.loadFromFile("Data/Sprites/BigAvoid.png")) {
		return 1;
	}
	bigAvoid.setSmooth(true);
	if (!midAvoid.loadFromFile("Data/Sprites/MidAvoid.png")) {
		return 1;
	}
	midAvoid.setSmooth(true);
	if (!smallAvoid.loadFromFile("Data/Sprites/SmallAvoid.png")) {
		return 1;
	}
	smallAvoid.setSmooth(true);
	if (!bigChaser.loadFromFile("Data/Sprites/BigChaser.png")) {
		return 1;
	}
	bigChaser.setSmooth(true);
	if (!midChaser.loadFromFile("Data/Sprites/MidChaser.png")) {
		return 1;
	}
	midChaser.setSmooth(true);
	if (!smallChaser.loadFromFile("Data/Sprites/SmallChaser.png")) {
		return 1;
	}
	smallChaser.setSmooth(true);
	if (!bigSlow.loadFromFile("Data/Sprites/BigSlow.png")) {
		return 1;
	}
	bigSlow.setSmooth(true);
	if (!midSlow.loadFromFile("Data/Sprites/MidSlow.png")) {
		return 1;
	}
	midSlow.setSmooth(true);
	if (!smallSlow.loadFromFile("Data/Sprites/SmallSlow.png")) {
		return 1;
	}
	smallSlow.setSmooth(true);

	sf::Font font;
	if (!font.loadFromFile("Data/Lobster/Lobster-Regular.ttf")) {
		return 1;
	}
	sf::String scoreString;
	sf::String lifeString;
	sf::String levelString;
	sf::Text scoreText;
	scoreText.setFont(font);
	scoreText.setCharacterSize(24);
	scoreText.setFillColor(sf::Color::Red);
	scoreText.setPosition(10, 0);
	sf::Text lifeText;
	lifeText.setFont(font);
	lifeText.setCharacterSize(24);
	lifeText.setFillColor(sf::Color::Red);
	lifeText.setPosition(WIDTH - 100, 0);
	sf::Text levelText;
	levelText.setFont(font);
	levelText.setCharacterSize(24);
	levelText.setFillColor(sf::Color::Red);
	levelText.setPosition(WIDTH/2 - 100, 0);
	
	sf::Text title;
	title.setFont(font);
	title.setCharacterSize(64);
	title.setFillColor(sf::Color::White);
	title.setPosition(200, 300);
	title.setString("Asteroid");
	sf::Text playGame;
	playGame.setFont(font);
	playGame.setCharacterSize(24);
	playGame.setFillColor(sf::Color::White);
	playGame.setPosition(700, 300);
	playGame.setString("PLAY !");
	sf::Text exitGame;
	exitGame.setFont(font);
	exitGame.setCharacterSize(24);
	exitGame.setFillColor(sf::Color::White);
	exitGame.setPosition(700, 400);
	exitGame.setString("Exit");
	sf::CircleShape choice;
	choice.setRadius(10);
	choice.setFillColor(sf::Color::Green);
	choice.setPosition(650, 300);
	sf::Text gameoverText;
	gameoverText.setFont(font);
	gameoverText.setCharacterSize(24);
	gameoverText.setFillColor(sf::Color::Red);
	gameoverText.setPosition(110, HEIGHT / 2);
	Ship ship;
	ship.shape = sf::CircleShape(32.0f);
	ship.shape.setPosition(WIDTH/2 - ship.shape.getRadius(), HEIGHT/2 - ship.shape.getRadius());
	ship.shape.setFillColor(sf::Color::Black);
	ship.velocity = sf::Vector2f(200.0f, 200.0f);
	ship.inertia = sf::Vector2f(0.0, 0.0);
	ship.sprite.setTexture(shipTexture);
	ship.sprite.setPosition(WIDTH / 2 - ship.shape.getRadius(), HEIGHT / 2 - ship.shape.getRadius());
	ship.lives = 3;
	ship.canThrust = true;
	ship.alive = true;

	std::vector<Bullet> BulletList;
	
	std::vector<Asteroid> AsteroidList;
	std::vector<Asteroid*> grid[COLUMNS][ROWS];
	std::vector<Bullet*> gridBullet[COLUMNS][ROWS];
	
	sf::Clock clock;
	sf::Clock clockthrust;
	sf::Clock fireRate;
	bool levelStarted = false;
	bool canFire = true;
	VisualEffect thrustEffect;
	thrustEffect.shape.setFillColor(sf::Color::Yellow);
	float deltaTime = 0.0f;
	int life;
	int score = 0;
	int level = 1;
	bool gameStarted = false;
	bool gameOver = false;
	float bullSpeed = 200.0f;

	while (window.isOpen())
	{
		deltaTime = clock.getElapsedTime().asSeconds();
		clock.restart();
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		if(!gameStarted){
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
				choice.setPosition(650, 300);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
				choice.setPosition(650, 400);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
				if (choice.getPosition().y == 300) {
					gameStarted = true;
				}
				else if (choice.getPosition().y == 400) {
					event.type = sf::Event::Closed;
					window.close();
				}
			}
			window.clear();
			window.draw(title);
			window.draw(playGame);
			window.draw(exitGame);
			window.draw(choice);
			window.display();
		}
		else
		{
			if (!gameOver) {
				switch (level)
				{

				case 1: {
					if (!levelStarted) {
						powerupList.clear();
						asteroExplo.clear();
						AsteroidList.clear();
						BulletList.clear();
						bullSpeed = 200.0f;
						Asteroid aster;
						aster.size = 3;
						aster.shape = sf::CircleShape(32.0f);
						aster.shape.setPosition(sf::Vector2f(200.0f, 200.0f));
						aster.shape.setFillColor(sf::Color::Black);
						aster.sprite.setPosition(aster.shape.getPosition());
						aster.type = 1;
						aster.sprite.setTexture(BigAsterTexture);
						aster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster.speed = 30.0f;
						aster.alive = true;
						AsteroidList.push_back(aster);
						Asteroid aster1;
						aster1.size = 3;
						aster1.shape = sf::CircleShape(32.0f);
						aster1.shape.setPosition(sf::Vector2f(800.0f, 800.0f));
						aster1.shape.setFillColor(sf::Color::Black);
						aster1.sprite.setPosition(aster.shape.getPosition());
						aster1.sprite.setTexture(BigAsterTexture);
						aster1.type = 1;
						aster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster1.speed = 30.0f;
						aster1.alive = true;
						AsteroidList.push_back(aster1);
						levelStarted = true;
						Asteroid aster2;
						aster2.size = 3;
						aster2.shape = sf::CircleShape(32.0f);
						aster2.shape.setPosition(sf::Vector2f(300.0f, 700.0f));
						aster2.shape.setFillColor(sf::Color::Black);
						aster2.sprite.setPosition(aster.shape.getPosition());
						aster2.sprite.setTexture(bigAvoid);
						aster2.type = 3;
						aster2.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster2.speed = 30.0f;
						aster2.alive = true;
						AsteroidList.push_back(aster2);
					}
					else {
						int counter = 0;
						life = ship.lives;
						if (life <= 0) {
							gameOver = true;
						}
						lifeString = sf::String("Life: " + std::to_string(life));
						lifeText.setString(lifeString);
						scoreString = sf::String("Score: " + std::to_string(score));
						scoreText.setString(scoreString);
						levelString = sf::String("Level: " + std::to_string(level));
						levelText.setString(levelString);
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
							gameStarted = false;
							levelStarted = false;
							score = 0;
						}
						//clear the grid
						for (int i = 0; i < COLUMNS; i++) {
							for (int j = 0; j < COLUMNS; j++) {
								grid[i][j].clear();
							}
						}
						for (int i = 0; i < COLUMNS; i++) {
							for (int j = 0; j < COLUMNS; j++) {
								gridBullet[i][j].clear();
							}
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
							ship.Move(deltaTime, 3);
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
							ship.Move(deltaTime, 4);
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
							ship.Move(deltaTime, 1);
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
							ship.Move(deltaTime, 2);
						}
						if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
							if (ship.canThrust == true) {
								thrustEffect.Create(ship.shape.getPosition(), 20.0f);
								ship.thrust(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
								clockthrust.restart();
								ship.canThrust = false;
							}
						}
						if (clockthrust.getElapsedTime().asSeconds() > 2.0f) {
							ship.canThrust = true;
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
							if (ship.alive) {
								if (canFire) {
									Bullet bullet;
									bullet.shape = sf::CircleShape(18.0f);
									bullet.shape.setPosition(ship.shape.getPosition());
									bullet.shape.setFillColor(sf::Color::Red);
									bullet.mouseDir = sf::Vector2f(sf::Mouse::getPosition(window));
									bullet.speed = bullSpeed;
									bullet.alive = true;
									bullet.Init();
									bullet.life.restart();
									BulletList.push_back(bullet);
									shoot.play();
									fireRate.restart();
									canFire = false;
								}
							}
						}
						if (fireRate.getElapsedTime().asSeconds() > 1.0f) {
							canFire = true;
						}

						ship.RotateAndMomentum(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, deltaTime);
						if (powerupList.size() > 0) {
							for (int i = 0; i < powerupList.size(); i++) {
								if (powerupList[i].alive) {
									switch (powerupList[i].type)
									{
									case 1:
										powerupList[i].sprite.setTexture(extralife);
										break;
									case 2:
										powerupList[i].sprite.setTexture(snipe);
										break;
									case 3:
										powerupList[i].sprite.setTexture(speedUp);
										break;
									default:
										break;
									}
									if (powerupList[i].Update(&ship)) {
										switch (powerupList[i].type)
										{
										case 1:
											ship.lives ++;
											break;
										case 2:
											bullSpeed += 40.0f;
											break;
										case 3:
											ship.velocity += sf::Vector2f(20.0f, 20.0f);
											break;
										default:
											break;
										}
									}
								}
							}
						}
						//assgin the bullet grid
						for (int i = 0; i < BulletList.size(); i++) {
							if (BulletList[i].alive) {
								auto pos = BulletList[i].shape.getPosition();
								sf::Vector2i gridID = getBucket(pos);
								gridBullet[gridID.x][gridID.y].push_back(&BulletList[i]);
								getNeighbour(gridID);
								auto rad = BulletList[i].shape.getRadius();
								if (pos.x - gridID.x * 250.0f <= rad) {
									if (pos.y - gridID.y * 250.0f <= rad) {
										gridBullet[neighbor[5].x][neighbor[5].y].push_back(&BulletList[i]);
									}
									else if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
										gridBullet[neighbor[3].x][neighbor[3].y].push_back(&BulletList[i]);
									}
									else {
										gridBullet[neighbor[4].x][neighbor[4].y].push_back(&BulletList[i]);
									}
								}
								if ((gridID.x + 1) * 250.0f - pos.x <= rad) {
									if (pos.y - gridID.y * 250.0f <= rad) {
										gridBullet[neighbor[7].x][neighbor[7].y].push_back(&BulletList[i]);
									}
									else if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
										gridBullet[neighbor[1].x][neighbor[1].y].push_back(&BulletList[i]);
									}
									else {
										gridBullet[neighbor[0].x][neighbor[0].y].push_back(&BulletList[i]);
									}
								}
								if (pos.y - gridID.y * 250.0f <= rad) {
									if (pos.x - gridID.x * 250.0f > rad && (gridID.x + 1) * 250.0f - pos.x > rad) {
										gridBullet[neighbor[6].x][neighbor[6].y].push_back(&BulletList[i]);
									}
								}
								if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
									if (pos.x - gridID.x * 250.0f > rad && (gridID.x + 1) * 250.0f - pos.x > rad) {
										gridBullet[neighbor[2].x][neighbor[2].y].push_back(&BulletList[i]);
									}
								}
							}
						}
						//assign the grid
						for (int i = 0; i < AsteroidList.size(); i++) {
							if (AsteroidList[i].alive) {
								auto pos = AsteroidList[i].shape.getPosition();
								sf::Vector2i gridID = getBucket(pos);
								grid[gridID.x][gridID.y].push_back(&AsteroidList[i]);
								getNeighbour(gridID);
								auto rad = AsteroidList[i].shape.getRadius();
								if (pos.x - gridID.x * 250.0f <= rad) {
									if (pos.y - gridID.y * 250.0f <= rad) {
										grid[neighbor[5].x][neighbor[5].y].push_back(&AsteroidList[i]);
									}
									else if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
										grid[neighbor[3].x][neighbor[3].y].push_back(&AsteroidList[i]);
									}
									else {
										grid[neighbor[4].x][neighbor[4].y].push_back(&AsteroidList[i]);
									}
								}
								if ((gridID.x + 1) * 250.0f - pos.x <= rad) {
									if (pos.y - gridID.y * 250.0f <= rad) {
										grid[neighbor[7].x][neighbor[7].y].push_back(&AsteroidList[i]);
									}
									else if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
										grid[neighbor[1].x][neighbor[1].y].push_back(&AsteroidList[i]);
									}
									else {
										grid[neighbor[0].x][neighbor[0].y].push_back(&AsteroidList[i]);
									}
								}
								if (pos.y - gridID.y * 250.0f <= rad) {
									if (pos.x - gridID.x * 250.0f > rad && (gridID.x + 1) * 250.0f - pos.x > rad) {
										grid[neighbor[6].x][neighbor[6].y].push_back(&AsteroidList[i]);
									}
								}
								if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
									if (pos.x - gridID.x * 250.0f > rad && (gridID.x + 1) * 250.0f - pos.x > rad) {
										grid[neighbor[2].x][neighbor[2].y].push_back(&AsteroidList[i]);
									}
								}
							}
						}
						for (int i = 0; i < COLUMNS; i++) {
							for (int j = 0; j < COLUMNS; j++) {
								if (grid[i][j].size() > 0) {
									for (int a = 0; a < grid[i][j].size(); a++) {
										if (gridBullet[i][j].size() > 0) {
											for (int b = 0; b < gridBullet[i][j].size(); b++) {
												if (grid[i][j][a]->alive && gridBullet[i][j][b]->alive) {
													if (grid[i][j][a]->Update(gridBullet[i][j][b])) {
														score += 10;
														switch (grid[i][j][a]->size)
														{
														case 1:
															grid[i][j][a]->alive = false;
															break;
														case 2: {
															grid[i][j][a]->alive = false;
															Asteroid subaster;

															subaster.size = 1;
															subaster.shape = sf::CircleShape(16.0f);
															subaster.shape.setFillColor(sf::Color::Black);
															subaster.shape.setPosition(grid[i][j][a]->shape.getPosition() + sf::Vector2f(subaster.shape.getRadius(), subaster.shape.getRadius()));
															subaster.sprite.setPosition(subaster.shape.getPosition());
															subaster.sprite.setTexture(SmallAsterTexture);
															subaster.type = grid[i][j][a]->type;
															subaster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
															subaster.speed = 30.0f;
															subaster.alive = true;
															AsteroidList.push_back(subaster);
															Asteroid subaster1;

															subaster1.size = 1;
															subaster1.shape = sf::CircleShape(16.0f);
															subaster1.shape.setFillColor(sf::Color::Black);
															subaster1.shape.setPosition(subaster.shape.getPosition() - 2.0f * sf::Vector2f(subaster1.shape.getRadius(), subaster1.shape.getRadius()));
															subaster1.sprite.setPosition(subaster1.shape.getPosition());
															subaster1.sprite.setTexture(SmallAsterTexture);
															subaster1.type = subaster.type;
															subaster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
															subaster1.speed = 30.0f;
															subaster1.alive = true;
															AsteroidList.push_back(subaster1);
															break;
														}
														case 3: {
															grid[i][j][a]->alive = false;
															Asteroid subaster;
															subaster.type = grid[i][j][a]->type;
															subaster.size = 2;
															subaster.shape = sf::CircleShape(24.0f);
															subaster.shape.setFillColor(sf::Color::Black);
															float r1 = subaster.shape.getRadius();
															subaster.shape.setPosition(grid[i][j][a]->shape.getPosition() + sf::Vector2f(subaster.shape.getRadius(), subaster.shape.getRadius()));
															subaster.sprite.setPosition(subaster.shape.getPosition());
															subaster.sprite.setTexture(MidAsterTexture);
															float a1 = subaster.shape.getPosition().x;
															float b1 = subaster.shape.getPosition().y;
															subaster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
															subaster.speed = 30.0f;
															subaster.alive = true;
															AsteroidList.push_back(subaster);
															Asteroid subaster1;
															subaster1.type = subaster.type;
															subaster1.size = 2;
															subaster1.shape = sf::CircleShape(24.0f);
															subaster1.shape.setFillColor(sf::Color::Black);
															float r2 = subaster1.shape.getRadius();
															subaster1.shape.setPosition(subaster.shape.getPosition() - 2.0f * sf::Vector2f(subaster1.shape.getRadius(), subaster1.shape.getRadius()));
															subaster1.sprite.setPosition(subaster1.shape.getPosition());
															subaster1.sprite.setTexture(MidAsterTexture);
															float c1 = grid[i][j][a]->shape.getPosition().x;
															float d1 = grid[i][j][a]->shape.getPosition().y;
															subaster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
															subaster1.speed = 30.0f;
															subaster1.alive = true;
															AsteroidList.push_back(subaster1);
															break;
														}
														default:
															break;
														}
														
														break;
													}
												}
											}
										}
										if (grid[i][j][a]->alive) {
											for (int c = 0; c < grid[i][j].size(); c++) {
												if (a != c) {
													sf::Vector2f dir = grid[i][j][a]->shape.getPosition() - grid[i][j][c]->shape.getPosition();
													if (Magnitude(dir, sf::Vector2f(0.0f, 0.0f)) <= grid[i][j][a]->shape.getRadius() + grid[i][j][c]->shape.getRadius()) {
														grid[i][j][a]->velocity = Normalize(dir);
														grid[i][j][c]->velocity = Normalize(-dir);
													}
												}
											}
											if (grid[i][j][a]->CollideWithShip(&ship)) {
												score += 10;
												switch (grid[i][j][a]->size)
												{
												case 1:
													grid[i][j][a]->alive = false;
													break;
												case 2: {
													grid[i][j][a]->alive = false;
													Asteroid subaster;

													subaster.size = 1;
													subaster.shape = sf::CircleShape(16.0f);
													subaster.shape.setFillColor(sf::Color::Black);
													subaster.shape.setPosition(grid[i][j][a]->shape.getPosition() + sf::Vector2f(subaster.shape.getRadius(), subaster.shape.getRadius()));
													subaster.sprite.setPosition(subaster.shape.getPosition());
													subaster.sprite.setTexture(SmallAsterTexture);
													subaster.type = grid[i][j][a]->type;
													subaster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
													subaster.speed = 30.0f;
													subaster.alive = true;
													AsteroidList.push_back(subaster);
													Asteroid subaster1;

													subaster1.size = 1;
													subaster1.shape = sf::CircleShape(16.0f);
													subaster1.shape.setFillColor(sf::Color::Black);
													subaster1.shape.setPosition(subaster.shape.getPosition() - 2.0f * sf::Vector2f(subaster1.shape.getRadius(), subaster1.shape.getRadius()));
													subaster1.sprite.setPosition(subaster1.shape.getPosition());
													subaster1.sprite.setTexture(SmallAsterTexture);
													subaster1.type = subaster.type;
													subaster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
													subaster1.speed = 30.0f;
													subaster1.alive = true;
													AsteroidList.push_back(subaster1);
													break;
												}
												case 3: {
													grid[i][j][a]->alive = false;
													Asteroid subaster;

													subaster.size = 2;
													subaster.shape = sf::CircleShape(24.0f);
													subaster.shape.setFillColor(sf::Color::Black);
													subaster.shape.setPosition(grid[i][j][a]->shape.getPosition() + sf::Vector2f(subaster.shape.getRadius(), subaster.shape.getRadius()));
													subaster.sprite.setPosition(subaster.shape.getPosition());
													subaster.sprite.setTexture(MidAsterTexture);
													subaster.type = grid[i][j][a]->type;
													subaster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
													subaster.speed = 30.0f;
													subaster.alive = true;
													AsteroidList.push_back(subaster);
													Asteroid subaster1;

													subaster1.size = 2;
													subaster1.shape = sf::CircleShape(24.0f);
													subaster1.shape.setFillColor(sf::Color::Black);
													subaster1.shape.setPosition(subaster.shape.getPosition() - 2.0f * sf::Vector2f(subaster1.shape.getRadius(), subaster1.shape.getRadius()));
													subaster1.sprite.setPosition(subaster1.shape.getPosition());
													subaster1.sprite.setTexture(MidAsterTexture);
													subaster1.type = subaster.type;
													subaster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
													subaster1.speed = 30.0f;
													subaster1.alive = true;
													AsteroidList.push_back(subaster1);
													break;
												}
												default:
													break;
												}
												break;
											}
										}
									}
								}
							}
						}
						for (int i = 0; i < BulletList.size(); i++) {
							if (BulletList[i].alive) {
								BulletList[i].Update(deltaTime);
							}
						}

						for (int i = 0; i < AsteroidList.size(); i++) {
							if (AsteroidList[i].alive) {
								AsteroidList[i].Move(deltaTime);
							}
						}
						window.clear();
						if (asteroExplo.size() > 0) {
							for (int i = 0; i < asteroExplo.size(); i++) {
								asteroExplo[i].draw(&window);
							}
						}
						for (int i = 0; i < BulletList.size(); i++) {
							if (BulletList[i].alive) {
								BulletList[i].draw(&window);
							}
						}
						for (int i = 0; i < AsteroidList.size(); i++) {
							if (AsteroidList[i].alive) {
								counter++;
								AsteroidList[i].draw(&window);
							}
						}
						if (counter == 0) {
							level++;
							levelChange.play();
							levelStarted = false;
						}
						if (powerupList.size() > 0) {
							for (int i = 0; i < powerupList.size(); i++) {
								if (powerupList[i].alive) {
									powerupList[i].draw(&window);
								}
							}
						}
						ship.draw(&window);
						thrustEffect.draw(&window);
						window.draw(scoreText);
						window.draw(lifeText);
						window.draw(levelText);
						window.display();
					}
					break;
				}
				case 2: {
					if (!levelStarted) {
						powerupList.clear();
						asteroExplo.clear();
						AsteroidList.clear();
						BulletList.clear();
						Asteroid aster;
						aster.size = 3;
						aster.shape = sf::CircleShape(32.0f);
						aster.shape.setPosition(sf::Vector2f(200.0f, 200.0f));
						aster.shape.setFillColor(sf::Color::Black);
						aster.sprite.setPosition(aster.shape.getPosition());
						aster.sprite.setTexture(BigAsterTexture);
						aster.type = 1;
						aster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster.speed = 40.0f;
						aster.alive = true;
						AsteroidList.push_back(aster);
						Asteroid aster1;
						aster1.size = 3;
						aster1.shape = sf::CircleShape(32.0f);
						aster1.shape.setPosition(sf::Vector2f(800.0f, 800.0f));
						aster1.shape.setFillColor(sf::Color::Black);
						aster1.sprite.setPosition(aster.shape.getPosition());
						aster1.sprite.setTexture(BigAsterTexture);
						aster1.type = 2;
						aster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster1.speed = 40.0f;
						aster1.alive = true;
						AsteroidList.push_back(aster1);
						levelStarted = true;
						Asteroid aster2;
						aster2.size = 3;
						aster2.shape = sf::CircleShape(32.0f);
						aster2.shape.setPosition(sf::Vector2f(300.0f, 700.0f));
						aster2.shape.setFillColor(sf::Color::Black);
						aster2.sprite.setPosition(aster.shape.getPosition());
						aster2.sprite.setTexture(BigAsterTexture);
						aster2.type = 3;
						aster2.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster2.speed = 40.0f;
						aster2.alive = true;
						AsteroidList.push_back(aster2);
						Asteroid aster3;
						aster3.size = 3;
						aster3.shape = sf::CircleShape(32.0f);
						aster3.shape.setPosition(sf::Vector2f(700.0f, 300.0f));
						aster3.shape.setFillColor(sf::Color::Black);
						aster3.sprite.setPosition(aster.shape.getPosition());
						aster3.sprite.setTexture(BigAsterTexture);
						aster3.type = 4;
						aster3.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster3.speed = 40.0f;
						aster3.alive = true;
						AsteroidList.push_back(aster3);
						Asteroid aster4;
						aster4.size = 3;
						aster4.shape = sf::CircleShape(32.0f);
						aster4.shape.setPosition(sf::Vector2f(100.0f, 400.0f));
						aster4.shape.setFillColor(sf::Color::Black);
						aster4.sprite.setPosition(aster.shape.getPosition());
						aster4.sprite.setTexture(BigAsterTexture);
						aster4.type = 3;
						aster4.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster4.speed = 40.0f;
						aster4.alive = true;
						AsteroidList.push_back(aster4);
					}
					else {
						int counter = 0;
						life = ship.lives;
						if (life <= 0) {
							gameOver = true;
						}
						lifeString = sf::String("Life: " + std::to_string(life));
						lifeText.setString(lifeString);
						scoreString = sf::String("Score: " + std::to_string(score));
						scoreText.setString(scoreString);
						levelString = sf::String("Level: " + std::to_string(level));
						levelText.setString(levelString);
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
							gameStarted = false;
							levelStarted = false;
							score = 0;
						}
						//clear the grid
						for (int i = 0; i < COLUMNS; i++) {
							for (int j = 0; j < COLUMNS; j++) {
								grid[i][j].clear();
							}
						}
						for (int i = 0; i < COLUMNS; i++) {
							for (int j = 0; j < COLUMNS; j++) {
								gridBullet[i][j].clear();
							}
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
							ship.Move(deltaTime, 3);
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
							ship.Move(deltaTime, 4);
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
							ship.Move(deltaTime, 1);
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
							ship.Move(deltaTime, 2);
						}
						if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
							if (ship.canThrust == true) {
								thrustEffect.Create(ship.shape.getPosition(), 20.0f);
								ship.thrust(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
								clockthrust.restart();
								ship.canThrust = false;
							}
						}
						if (clockthrust.getElapsedTime().asSeconds() > 2.0f) {
							ship.canThrust = true;
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
							if (ship.alive) {
								if (canFire) {
									Bullet bullet;
									bullet.shape = sf::CircleShape(18.0f);
									bullet.shape.setPosition(ship.shape.getPosition());
									bullet.shape.setFillColor(sf::Color::Red);
									bullet.mouseDir = sf::Vector2f(sf::Mouse::getPosition(window));
									bullet.speed = bullSpeed;
									bullet.alive = true;
									bullet.Init();
									bullet.life.restart();
									BulletList.push_back(bullet);
									shoot.play();
									fireRate.restart();
									canFire = false;
								}
							}
						}
						if (fireRate.getElapsedTime().asSeconds() > 1.0f) {
							canFire = true;
						}

						ship.RotateAndMomentum(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, deltaTime);
						if (powerupList.size() > 0) {
							for (int i = 0; i < powerupList.size(); i++) {
								if (powerupList[i].alive) {
									switch (powerupList[i].type)
									{
									case 1:
										powerupList[i].sprite.setTexture(extralife);
										break;
									case 2:
										powerupList[i].sprite.setTexture(snipe);
										break;
									case 3:
										powerupList[i].sprite.setTexture(speedUp);
										break;
									default:
										break;
									}
									if (powerupList[i].Update(&ship)) {
										switch (powerupList[i].type)
										{
										case 1:
											ship.lives++;
											break;
										case 2:
											bullSpeed += 40.0f;
											break;
										case 3:
											ship.velocity += sf::Vector2f(20.0f, 20.0f);
											break;
										default:
											break;
										}
									}
								}
							}
						}

						//assgin the bullet grid
						for (int i = 0; i < BulletList.size(); i++) {
							if (BulletList[i].alive) {
								auto pos = BulletList[i].shape.getPosition();
								sf::Vector2i gridID = getBucket(pos);
								gridBullet[gridID.x][gridID.y].push_back(&BulletList[i]);
								getNeighbour(gridID);
								auto rad = BulletList[i].shape.getRadius();
								if (pos.x - gridID.x * 250.0f <= rad) {
									if (pos.y - gridID.y * 250.0f <= rad) {
										gridBullet[neighbor[5].x][neighbor[5].y].push_back(&BulletList[i]);
									}
									else if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
										gridBullet[neighbor[3].x][neighbor[3].y].push_back(&BulletList[i]);
									}
									else {
										gridBullet[neighbor[4].x][neighbor[4].y].push_back(&BulletList[i]);
									}
								}
								if ((gridID.x + 1) * 250.0f - pos.x <= rad) {
									if (pos.y - gridID.y * 250.0f <= rad) {
										gridBullet[neighbor[7].x][neighbor[7].y].push_back(&BulletList[i]);
									}
									else if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
										gridBullet[neighbor[1].x][neighbor[1].y].push_back(&BulletList[i]);
									}
									else {
										gridBullet[neighbor[0].x][neighbor[0].y].push_back(&BulletList[i]);
									}
								}
								if (pos.y - gridID.y * 250.0f <= rad) {
									if (pos.x - gridID.x * 250.0f > rad && (gridID.x + 1) * 250.0f - pos.x > rad) {
										gridBullet[neighbor[6].x][neighbor[6].y].push_back(&BulletList[i]);
									}
								}
								if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
									if (pos.x - gridID.x * 250.0f > rad && (gridID.x + 1) * 250.0f - pos.x > rad) {
										gridBullet[neighbor[2].x][neighbor[2].y].push_back(&BulletList[i]);
									}
								}
							}
						}
						//assign the grid
						for (int i = 0; i < AsteroidList.size(); i++) {
							if (AsteroidList[i].alive) {
								auto pos = AsteroidList[i].shape.getPosition();
								sf::Vector2i gridID = getBucket(pos);
								grid[gridID.x][gridID.y].push_back(&AsteroidList[i]);
								getNeighbour(gridID);
								auto rad = AsteroidList[i].shape.getRadius();
								if (pos.x - gridID.x * 250.0f <= rad) {
									if (pos.y - gridID.y * 250.0f <= rad) {
										grid[neighbor[5].x][neighbor[5].y].push_back(&AsteroidList[i]);
									}
									else if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
										grid[neighbor[3].x][neighbor[3].y].push_back(&AsteroidList[i]);
									}
									else {
										grid[neighbor[4].x][neighbor[4].y].push_back(&AsteroidList[i]);
									}
								}
								if ((gridID.x + 1) * 250.0f - pos.x <= rad) {
									if (pos.y - gridID.y * 250.0f <= rad) {
										grid[neighbor[7].x][neighbor[7].y].push_back(&AsteroidList[i]);
									}
									else if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
										grid[neighbor[1].x][neighbor[1].y].push_back(&AsteroidList[i]);
									}
									else {
										grid[neighbor[0].x][neighbor[0].y].push_back(&AsteroidList[i]);
									}
								}
								if (pos.y - gridID.y * 250.0f <= rad) {
									if (pos.x - gridID.x * 250.0f > rad && (gridID.x + 1) * 250.0f - pos.x > rad) {
										grid[neighbor[6].x][neighbor[6].y].push_back(&AsteroidList[i]);
									}
								}
								if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
									if (pos.x - gridID.x * 250.0f > rad && (gridID.x + 1) * 250.0f - pos.x > rad) {
										grid[neighbor[2].x][neighbor[2].y].push_back(&AsteroidList[i]);
									}
								}
							}
						}
						for (int i = 0; i < COLUMNS; i++) {
							for (int j = 0; j < COLUMNS; j++) {
								if (grid[i][j].size() > 0) {
									for (int a = 0; a < grid[i][j].size(); a++) {
										if (gridBullet[i][j].size() > 0) {
											for (int b = 0; b < gridBullet[i][j].size(); b++) {
												if (grid[i][j][a]->alive && gridBullet[i][j][b]->alive) {
													if (grid[i][j][a]->Update(gridBullet[i][j][b])) {
														score += 10;
														switch (grid[i][j][a]->size)
														{
														case 1:
															grid[i][j][a]->alive = false;
															break;
														case 2: {
															grid[i][j][a]->alive = false;
															Asteroid subaster;

															subaster.size = 1;
															subaster.shape = sf::CircleShape(16.0f);
															subaster.shape.setFillColor(sf::Color::Black);
															subaster.shape.setPosition(grid[i][j][a]->shape.getPosition() + sf::Vector2f(subaster.shape.getRadius(), subaster.shape.getRadius()));
															subaster.sprite.setPosition(subaster.shape.getPosition());
															subaster.sprite.setTexture(SmallAsterTexture);
															subaster.type = grid[i][j][a]->type;
															subaster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
															subaster.speed = 30.0f;
															subaster.alive = true;
															AsteroidList.push_back(subaster);
															Asteroid subaster1;

															subaster1.size = 1;
															subaster1.shape = sf::CircleShape(16.0f);
															subaster1.shape.setFillColor(sf::Color::Black);
															subaster1.shape.setPosition(subaster.shape.getPosition() - 2.0f * sf::Vector2f(subaster1.shape.getRadius(), subaster1.shape.getRadius()));
															subaster1.sprite.setPosition(subaster1.shape.getPosition());
															subaster1.sprite.setTexture(SmallAsterTexture);
															subaster1.type = subaster.type;
															subaster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
															subaster1.speed = 30.0f;
															subaster1.alive = true;
															AsteroidList.push_back(subaster1);
															break;
														}
														case 3: {
															grid[i][j][a]->alive = false;
															Asteroid subaster;
															subaster.type = grid[i][j][a]->type;
															subaster.size = 2;
															subaster.shape = sf::CircleShape(24.0f);
															subaster.shape.setFillColor(sf::Color::Black);
															float r1 = subaster.shape.getRadius();
															subaster.shape.setPosition(grid[i][j][a]->shape.getPosition() + sf::Vector2f(subaster.shape.getRadius(), subaster.shape.getRadius()));
															subaster.sprite.setPosition(subaster.shape.getPosition());
															subaster.sprite.setTexture(MidAsterTexture);
															float a1 = subaster.shape.getPosition().x;
															float b1 = subaster.shape.getPosition().y;
															subaster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
															subaster.speed = 30.0f;
															subaster.alive = true;
															AsteroidList.push_back(subaster);
															Asteroid subaster1;
															subaster1.type = subaster.type;
															subaster1.size = 2;
															subaster1.shape = sf::CircleShape(24.0f);
															subaster1.shape.setFillColor(sf::Color::Black);
															float r2 = subaster1.shape.getRadius();
															subaster1.shape.setPosition(subaster.shape.getPosition() - 2.0f * sf::Vector2f(subaster1.shape.getRadius(), subaster1.shape.getRadius()));
															subaster1.sprite.setPosition(subaster1.shape.getPosition());
															subaster1.sprite.setTexture(MidAsterTexture);
															float c1 = grid[i][j][a]->shape.getPosition().x;
															float d1 = grid[i][j][a]->shape.getPosition().y;
															subaster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
															subaster1.speed = 30.0f;
															subaster1.alive = true;
															AsteroidList.push_back(subaster1);
															break;
														}
														default:
															break;
														}
														break;
													}
												}
											}
										}
										if (grid[i][j][a]->alive) {
											for (int c = 0; c < grid[i][j].size(); c++) {
												if (a != c) {
													sf::Vector2f dir = grid[i][j][a]->shape.getPosition() - grid[i][j][c]->shape.getPosition();
													if (Magnitude(dir, sf::Vector2f(0.0f, 0.0f)) <= grid[i][j][a]->shape.getRadius() + grid[i][j][c]->shape.getRadius()) {
														grid[i][j][a]->velocity = Normalize(dir);
														grid[i][j][c]->velocity = Normalize(-dir);
													}
												}
											}
											if (grid[i][j][a]->CollideWithShip(&ship)) {
												score += 10;
												switch (grid[i][j][a]->size)
												{
												case 1:
													grid[i][j][a]->alive = false;
													break;
												case 2: {
													grid[i][j][a]->alive = false;
													Asteroid subaster;

													subaster.size = 1;
													subaster.shape = sf::CircleShape(16.0f);
													subaster.shape.setFillColor(sf::Color::Black);
													subaster.shape.setPosition(grid[i][j][a]->shape.getPosition() + sf::Vector2f(subaster.shape.getRadius(), subaster.shape.getRadius()));
													subaster.sprite.setPosition(subaster.shape.getPosition());
													subaster.sprite.setTexture(SmallAsterTexture);
													subaster.type = grid[i][j][a]->type;
													subaster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
													subaster.speed = 30.0f;
													subaster.alive = true;
													AsteroidList.push_back(subaster);
													Asteroid subaster1;

													subaster1.size = 1;
													subaster1.shape = sf::CircleShape(16.0f);
													subaster1.shape.setFillColor(sf::Color::Black);
													subaster1.shape.setPosition(subaster.shape.getPosition() - 2.0f * sf::Vector2f(subaster1.shape.getRadius(), subaster1.shape.getRadius()));
													subaster1.sprite.setPosition(subaster1.shape.getPosition());
													subaster1.sprite.setTexture(SmallAsterTexture);
													subaster1.type = subaster.type;
													subaster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
													subaster1.speed = 30.0f;
													subaster1.alive = true;
													AsteroidList.push_back(subaster1);
													break;
												}
												case 3: {
													grid[i][j][a]->alive = false;
													Asteroid subaster;

													subaster.size = 2;
													subaster.shape = sf::CircleShape(24.0f);
													subaster.shape.setFillColor(sf::Color::Black);
													subaster.shape.setPosition(grid[i][j][a]->shape.getPosition() + sf::Vector2f(subaster.shape.getRadius(), subaster.shape.getRadius()));
													subaster.sprite.setPosition(subaster.shape.getPosition());
													subaster.sprite.setTexture(MidAsterTexture);
													subaster.type = grid[i][j][a]->type;
													subaster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
													subaster.speed = 30.0f;
													subaster.alive = true;
													AsteroidList.push_back(subaster);
													Asteroid subaster1;

													subaster1.size = 2;
													subaster1.shape = sf::CircleShape(24.0f);
													subaster1.shape.setFillColor(sf::Color::Black);
													subaster1.shape.setPosition(subaster.shape.getPosition() - 2.0f * sf::Vector2f(subaster1.shape.getRadius(), subaster1.shape.getRadius()));
													subaster1.sprite.setPosition(subaster1.shape.getPosition());
													subaster1.sprite.setTexture(MidAsterTexture);
													subaster1.type = subaster.type;
													subaster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
													subaster1.speed = 30.0f;
													subaster1.alive = true;
													AsteroidList.push_back(subaster1);
													break;
												}
												default:
													break;
												}
												break;
											}
										}
									}
								}
							}
						}
						for (int i = 0; i < BulletList.size(); i++) {
							if (BulletList[i].alive) {
								BulletList[i].Update(deltaTime);
							}
						}

						for (int i = 0; i < AsteroidList.size(); i++) {
							if (AsteroidList[i].alive) {
								AsteroidList[i].Move(deltaTime);
							}
						}
						
						window.clear();
						if (asteroExplo.size() > 0) {
							for (int i = 0; i < asteroExplo.size(); i++) {
								asteroExplo[i].draw(&window);
							}
						}
						for (int i = 0; i < BulletList.size(); i++) {
							if (BulletList[i].alive) {
								BulletList[i].draw(&window);
							}
						}
						for (int i = 0; i < AsteroidList.size(); i++) {
							if (AsteroidList[i].alive) {
								counter++;
								AsteroidList[i].draw(&window);
							}
						}
						if (counter == 0) {
							level++;
							levelChange.play();
							levelStarted = false;
						}
						if (powerupList.size() > 0) {
							for (int i = 0; i < powerupList.size(); i++) {
								if (powerupList[i].alive) {
									powerupList[i].draw(&window);
								}
							}
						}
						ship.draw(&window);
						thrustEffect.draw(&window);
						window.draw(scoreText);
						window.draw(lifeText);
						window.draw(levelText);
						window.display();
					}
					break;
				}
				case 3: {
					if (!levelStarted) {
						powerupList.clear();
						asteroExplo.clear();
						AsteroidList.clear();
						BulletList.clear();
						Asteroid aster;
						aster.size = 3;
						aster.shape = sf::CircleShape(32.0f);
						aster.shape.setPosition(sf::Vector2f(200.0f, 200.0f));
						aster.shape.setFillColor(sf::Color::Black);
						aster.sprite.setPosition(aster.shape.getPosition());
						aster.sprite.setTexture(BigAsterTexture);
						aster.type = 4;
						aster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster.speed = 50.0f;
						aster.alive = true;
						AsteroidList.push_back(aster);
						Asteroid aster1;
						aster1.size = 3;
						aster1.shape = sf::CircleShape(32.0f);
						aster1.shape.setPosition(sf::Vector2f(800.0f, 800.0f));
						aster1.shape.setFillColor(sf::Color::Black);
						aster1.sprite.setPosition(aster.shape.getPosition());
						aster1.sprite.setTexture(BigAsterTexture);
						aster1.type = 2;
						aster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster1.speed = 50.0f;
						aster1.alive = true;
						AsteroidList.push_back(aster1);
						levelStarted = true;
						Asteroid aster2;
						aster2.size = 3;
						aster2.shape = sf::CircleShape(32.0f);
						aster2.shape.setPosition(sf::Vector2f(300.0f, 700.0f));
						aster2.shape.setFillColor(sf::Color::Black);
						aster2.sprite.setPosition(aster.shape.getPosition());
						aster2.sprite.setTexture(BigAsterTexture);
						aster2.type = 3;
						aster2.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster2.speed = 50.0f;
						aster2.alive = true;
						AsteroidList.push_back(aster2);
						Asteroid aster3;
						aster3.size = 3;
						aster3.shape = sf::CircleShape(32.0f);
						aster3.shape.setPosition(sf::Vector2f(700.0f, 300.0f));
						aster3.shape.setFillColor(sf::Color::Black);
						aster3.sprite.setPosition(aster.shape.getPosition());
						aster3.sprite.setTexture(BigAsterTexture);
						aster3.type = 3;
						aster3.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster3.speed = 50.0f;
						aster3.alive = true;
						AsteroidList.push_back(aster3);
						Asteroid aster4;
						aster4.size = 3;
						aster4.shape = sf::CircleShape(32.0f);
						aster4.shape.setPosition(sf::Vector2f(800.0f, 500.0f));
						aster4.shape.setFillColor(sf::Color::Black);
						aster4.sprite.setPosition(aster.shape.getPosition());
						aster4.sprite.setTexture(BigAsterTexture);
						aster4.type = 2;
						aster4.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster4.speed = 50.0f;
						aster4.alive = true;
						AsteroidList.push_back(aster4);
						Asteroid aster5;
						aster5.size = 3;
						aster5.shape = sf::CircleShape(32.0f);
						aster5.shape.setPosition(sf::Vector2f(200.0f, 500.0f));
						aster5.shape.setFillColor(sf::Color::Black);
						aster5.sprite.setPosition(aster.shape.getPosition());
						aster5.sprite.setTexture(BigAsterTexture);
						aster5.type = 4;
						aster5.velocity = sf::Vector2f(distribution(generator), distribution(generator));
						aster5.speed = 50.0f;
						aster5.alive = true;
						AsteroidList.push_back(aster5);
					}
					else {
						int counter = 0;
						life = ship.lives;
						if (life <= 0) {
							gameOver = true;
						}
						lifeString = sf::String("Life: " + std::to_string(life));
						lifeText.setString(lifeString);
						scoreString = sf::String("Score: " + std::to_string(score));
						scoreText.setString(scoreString);
						levelString = sf::String("Level: " + std::to_string(level));
						levelText.setString(levelString);
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
							gameStarted = false;
							levelStarted = false;
							score = 0;
						}
						//clear the grid
						for (int i = 0; i < COLUMNS; i++) {
							for (int j = 0; j < COLUMNS; j++) {
								grid[i][j].clear();
							}
						}
						for (int i = 0; i < COLUMNS; i++) {
							for (int j = 0; j < COLUMNS; j++) {
								gridBullet[i][j].clear();
							}
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
							ship.Move(deltaTime, 3);
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
							ship.Move(deltaTime, 4);
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
							ship.Move(deltaTime, 1);
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
							ship.Move(deltaTime, 2);
						}
						if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
							if (ship.canThrust == true) {
								thrustEffect.Create(ship.shape.getPosition(), 20.0f);
								ship.thrust(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
								clockthrust.restart();
								ship.canThrust = false;
							}
						}
						if (clockthrust.getElapsedTime().asSeconds() > 2.0f) {
							ship.canThrust = true;
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
							if (ship.alive) {
								if (canFire) {
									Bullet bullet;
									bullet.shape = sf::CircleShape(18.0f);
									bullet.shape.setPosition(ship.shape.getPosition());
									bullet.shape.setFillColor(sf::Color::Red);
									bullet.mouseDir = sf::Vector2f(sf::Mouse::getPosition(window));
									bullet.speed = bullSpeed;
									bullet.alive = true;
									bullet.Init();
									bullet.life.restart();
									BulletList.push_back(bullet);
									shoot.play();
									fireRate.restart();
									canFire = false;
								}
							}
						}
						if (fireRate.getElapsedTime().asSeconds() > 1.0f) {
							canFire = true;
						}

						ship.RotateAndMomentum(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, deltaTime);
						if (powerupList.size() > 0) {
							for (int i = 0; i < powerupList.size(); i++) {
								if (powerupList[i].alive) {
									switch (powerupList[i].type)
									{
									case 1:
										powerupList[i].sprite.setTexture(extralife);
										break;
									case 2:
										powerupList[i].sprite.setTexture(snipe);
										break;
									case 3:
										powerupList[i].sprite.setTexture(speedUp);
										break;
									default:
										break;
									}
									if (powerupList[i].Update(&ship)) {
										switch (powerupList[i].type)
										{
										case 1:
											ship.lives++;
											break;
										case 2:
											bullSpeed += 40.0f;
											break;
										case 3:
											ship.velocity += sf::Vector2f(20.0f, 20.0f);
											break;
										default:
											break;
										}
									}
								}
							}
						}

						//assgin the bullet grid
						for (int i = 0; i < BulletList.size(); i++) {
							if (BulletList[i].alive) {
								auto pos = BulletList[i].shape.getPosition();
								sf::Vector2i gridID = getBucket(pos);
								gridBullet[gridID.x][gridID.y].push_back(&BulletList[i]);
								getNeighbour(gridID);
								auto rad = BulletList[i].shape.getRadius();
								if (pos.x - gridID.x * 250.0f <= rad) {
									if (pos.y - gridID.y * 250.0f <= rad) {
										gridBullet[neighbor[5].x][neighbor[5].y].push_back(&BulletList[i]);
									}
									else if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
										gridBullet[neighbor[3].x][neighbor[3].y].push_back(&BulletList[i]);
									}
									else {
										gridBullet[neighbor[4].x][neighbor[4].y].push_back(&BulletList[i]);
									}
								}
								if ((gridID.x + 1) * 250.0f - pos.x <= rad) {
									if (pos.y - gridID.y * 250.0f <= rad) {
										gridBullet[neighbor[7].x][neighbor[7].y].push_back(&BulletList[i]);
									}
									else if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
										gridBullet[neighbor[1].x][neighbor[1].y].push_back(&BulletList[i]);
									}
									else {
										gridBullet[neighbor[0].x][neighbor[0].y].push_back(&BulletList[i]);
									}
								}
								if (pos.y - gridID.y * 250.0f <= rad) {
									if (pos.x - gridID.x * 250.0f > rad && (gridID.x + 1) * 250.0f - pos.x > rad) {
										gridBullet[neighbor[6].x][neighbor[6].y].push_back(&BulletList[i]);
									}
								}
								if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
									if (pos.x - gridID.x * 250.0f > rad && (gridID.x + 1) * 250.0f - pos.x > rad) {
										gridBullet[neighbor[2].x][neighbor[2].y].push_back(&BulletList[i]);
									}
								}
							}
						}
						//assign the grid
						for (int i = 0; i < AsteroidList.size(); i++) {
							if (AsteroidList[i].alive) {
								auto pos = AsteroidList[i].shape.getPosition();
								sf::Vector2i gridID = getBucket(pos);
								grid[gridID.x][gridID.y].push_back(&AsteroidList[i]);
								getNeighbour(gridID);
								auto rad = AsteroidList[i].shape.getRadius();
								if (pos.x - gridID.x * 250.0f <= rad) {
									if (pos.y - gridID.y * 250.0f <= rad) {
										grid[neighbor[5].x][neighbor[5].y].push_back(&AsteroidList[i]);
									}
									else if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
										grid[neighbor[3].x][neighbor[3].y].push_back(&AsteroidList[i]);
									}
									else {
										grid[neighbor[4].x][neighbor[4].y].push_back(&AsteroidList[i]);
									}
								}
								if ((gridID.x + 1) * 250.0f - pos.x <= rad) {
									if (pos.y - gridID.y * 250.0f <= rad) {
										grid[neighbor[7].x][neighbor[7].y].push_back(&AsteroidList[i]);
									}
									else if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
										grid[neighbor[1].x][neighbor[1].y].push_back(&AsteroidList[i]);
									}
									else {
										grid[neighbor[0].x][neighbor[0].y].push_back(&AsteroidList[i]);
									}
								}
								if (pos.y - gridID.y * 250.0f <= rad) {
									if (pos.x - gridID.x * 250.0f > rad && (gridID.x + 1) * 250.0f - pos.x > rad) {
										grid[neighbor[6].x][neighbor[6].y].push_back(&AsteroidList[i]);
									}
								}
								if ((gridID.y + 1) * 250.0 - pos.y <= rad) {
									if (pos.x - gridID.x * 250.0f > rad && (gridID.x + 1) * 250.0f - pos.x > rad) {
										grid[neighbor[2].x][neighbor[2].y].push_back(&AsteroidList[i]);
									}
								}
							}
						}
						for (int i = 0; i < COLUMNS; i++) {
							for (int j = 0; j < COLUMNS; j++) {
								if (grid[i][j].size() > 0) {
									for (int a = 0; a < grid[i][j].size(); a++) {
										if (gridBullet[i][j].size() > 0) {
											for (int b = 0; b < gridBullet[i][j].size(); b++) {
												if (grid[i][j][a]->alive && gridBullet[i][j][b]->alive) {
													if (grid[i][j][a]->Update(gridBullet[i][j][b])) {
														score += 10;
														switch (grid[i][j][a]->size)
														{
														case 1:
															grid[i][j][a]->alive = false;
															break;
														case 2: {
															grid[i][j][a]->alive = false;
															Asteroid subaster;

															subaster.size = 1;
															subaster.shape = sf::CircleShape(16.0f);
															subaster.shape.setFillColor(sf::Color::Black);
															subaster.shape.setPosition(grid[i][j][a]->shape.getPosition() + sf::Vector2f(subaster.shape.getRadius(), subaster.shape.getRadius()));
															subaster.sprite.setPosition(subaster.shape.getPosition());
															subaster.sprite.setTexture(SmallAsterTexture);
															subaster.type = grid[i][j][a]->type;
															subaster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
															subaster.speed = 30.0f;
															subaster.alive = true;
															AsteroidList.push_back(subaster);
															Asteroid subaster1;

															subaster1.size = 1;
															subaster1.shape = sf::CircleShape(16.0f);
															subaster1.shape.setFillColor(sf::Color::Black);
															subaster1.shape.setPosition(subaster.shape.getPosition() - 2.0f * sf::Vector2f(subaster1.shape.getRadius(), subaster1.shape.getRadius()));
															subaster1.sprite.setPosition(subaster1.shape.getPosition());
															subaster1.sprite.setTexture(SmallAsterTexture);
															subaster1.type = subaster.type;
															subaster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
															subaster1.speed = 30.0f;
															subaster1.alive = true;
															AsteroidList.push_back(subaster1);
															break;
														}
														case 3: {
															grid[i][j][a]->alive = false;
															Asteroid subaster;
															subaster.type = grid[i][j][a]->type;
															subaster.size = 2;
															subaster.shape = sf::CircleShape(24.0f);
															subaster.shape.setFillColor(sf::Color::Black);
															float r1 = subaster.shape.getRadius();
															subaster.shape.setPosition(grid[i][j][a]->shape.getPosition() + sf::Vector2f(subaster.shape.getRadius(), subaster.shape.getRadius()));
															subaster.sprite.setPosition(subaster.shape.getPosition());
															subaster.sprite.setTexture(MidAsterTexture);
															float a1 = subaster.shape.getPosition().x;
															float b1 = subaster.shape.getPosition().y;
															subaster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
															subaster.speed = 30.0f;
															subaster.alive = true;
															AsteroidList.push_back(subaster);
															Asteroid subaster1;
															subaster1.type = subaster.type;
															subaster1.size = 2;
															subaster1.shape = sf::CircleShape(24.0f);
															subaster1.shape.setFillColor(sf::Color::Black);
															float r2 = subaster1.shape.getRadius();
															subaster1.shape.setPosition(subaster.shape.getPosition() - 2.0f * sf::Vector2f(subaster1.shape.getRadius(), subaster1.shape.getRadius()));
															subaster1.sprite.setPosition(subaster1.shape.getPosition());
															subaster1.sprite.setTexture(MidAsterTexture);
															float c1 = grid[i][j][a]->shape.getPosition().x;
															float d1 = grid[i][j][a]->shape.getPosition().y;
															subaster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
															subaster1.speed = 30.0f;
															subaster1.alive = true;
															AsteroidList.push_back(subaster1);
															break;
														}
														default:
															break;
														}
														break;
													}
												}
											}
										}
										if (grid[i][j][a]->alive) {
											for (int c = 0; c < grid[i][j].size(); c++) {
												if (a != c) {
													sf::Vector2f dir = grid[i][j][a]->shape.getPosition() - grid[i][j][c]->shape.getPosition();
													if (Magnitude(dir, sf::Vector2f(0.0f, 0.0f)) <= grid[i][j][a]->shape.getRadius() + grid[i][j][c]->shape.getRadius()) {
														grid[i][j][a]->velocity = Normalize(dir);
														grid[i][j][c]->velocity = Normalize(-dir);
													}
												}
											}
											if (grid[i][j][a]->CollideWithShip(&ship)) {
												score += 10;
												switch (grid[i][j][a]->size)
												{
												case 1:
													grid[i][j][a]->alive = false;
													break;
												case 2: {
													grid[i][j][a]->alive = false;
													Asteroid subaster;

													subaster.size = 1;
													subaster.shape = sf::CircleShape(16.0f);
													subaster.shape.setFillColor(sf::Color::Black);
													subaster.shape.setPosition(grid[i][j][a]->shape.getPosition() + sf::Vector2f(subaster.shape.getRadius(), subaster.shape.getRadius()));
													subaster.sprite.setPosition(subaster.shape.getPosition());
													subaster.sprite.setTexture(SmallAsterTexture);
													subaster.type = grid[i][j][a]->type;
													subaster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
													subaster.speed = 30.0f;
													subaster.alive = true;
													AsteroidList.push_back(subaster);
													Asteroid subaster1;

													subaster1.size = 1;
													subaster1.shape = sf::CircleShape(16.0f);
													subaster1.shape.setFillColor(sf::Color::Black);
													subaster1.shape.setPosition(subaster.shape.getPosition() - 2.0f * sf::Vector2f(subaster1.shape.getRadius(), subaster1.shape.getRadius()));
													subaster1.sprite.setPosition(subaster1.shape.getPosition());
													subaster1.sprite.setTexture(SmallAsterTexture);
													subaster1.type = subaster.type;
													subaster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
													subaster1.speed = 30.0f;
													subaster1.alive = true;
													AsteroidList.push_back(subaster1);
													break;
												}
												case 3: {
													grid[i][j][a]->alive = false;
													Asteroid subaster;

													subaster.size = 2;
													subaster.shape = sf::CircleShape(24.0f);
													subaster.shape.setFillColor(sf::Color::Black);
													subaster.shape.setPosition(grid[i][j][a]->shape.getPosition() + sf::Vector2f(subaster.shape.getRadius(), subaster.shape.getRadius()));
													subaster.sprite.setPosition(subaster.shape.getPosition());
													subaster.sprite.setTexture(MidAsterTexture);
													subaster.type = grid[i][j][a]->type;
													subaster.velocity = sf::Vector2f(distribution(generator), distribution(generator));
													subaster.speed = 30.0f;
													subaster.alive = true;
													AsteroidList.push_back(subaster);
													Asteroid subaster1;

													subaster1.size = 2;
													subaster1.shape = sf::CircleShape(24.0f);
													subaster1.shape.setFillColor(sf::Color::Black);
													subaster1.shape.setPosition(subaster.shape.getPosition() - 2.0f * sf::Vector2f(subaster1.shape.getRadius(), subaster1.shape.getRadius()));
													subaster1.sprite.setPosition(subaster1.shape.getPosition());
													subaster1.sprite.setTexture(MidAsterTexture);
													subaster1.type = subaster.type;
													subaster1.velocity = sf::Vector2f(distribution(generator), distribution(generator));
													subaster1.speed = 30.0f;
													subaster1.alive = true;
													AsteroidList.push_back(subaster1);
													break;
												}
												default:
													break;
												}
												break;
											}
										}
									}
								}
							}
						}
						for (int i = 0; i < BulletList.size(); i++) {
							if (BulletList[i].alive) {
								BulletList[i].Update(deltaTime);
							}
						}

						for (int i = 0; i < AsteroidList.size(); i++) {
							if (AsteroidList[i].alive) {
								AsteroidList[i].Move(deltaTime);
							}
						}

						window.clear();
						if (asteroExplo.size() > 0) {
							for (int i = 0; i < asteroExplo.size(); i++) {
								asteroExplo[i].draw(&window);
							}
						}
						for (int i = 0; i < BulletList.size(); i++) {
							if (BulletList[i].alive) {
								BulletList[i].draw(&window);
							}
						}
						for (int i = 0; i < AsteroidList.size(); i++) {
							if (AsteroidList[i].alive) {
								counter++;
								AsteroidList[i].draw(&window);
							}
						}
						if (counter == 0) {
							gameOver = true;
							levelChange.play();
							levelStarted = false;
						}
						if (powerupList.size() > 0) {
							for (int i = 0; i < powerupList.size(); i++) {
								if (powerupList[i].alive) {
									powerupList[i].draw(&window);
								}
							}
						}
						ship.draw(&window);
						thrustEffect.draw(&window);
						window.draw(scoreText);
						window.draw(lifeText);
						window.draw(levelText);
						window.display();
					}
					break;
				}
				default:
					break;
				}
			}
			else {
				gameoverText.setString("Game End, Press Space to go back to the start menu, Press Esc to exit the game");
				if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
					gameOver = false;
					gameStarted = false;
					levelStarted = false;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
					event.type = sf::Event::Closed;
					window.close();
				}
				window.clear();
				window.draw(gameoverText);
				window.draw(scoreText);
				window.display();
			}
		}
	}

	return 0;
}