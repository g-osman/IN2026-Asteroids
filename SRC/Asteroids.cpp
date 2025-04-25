#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"


// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char* argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */


//In this fucntion, i update it to integerate with Menu 
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation* explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation* asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation* spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");

	
	mGameState = MENU;
	CreateMenu();
	CreateGUI();
	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////


//Updated this method to make the main menu interactable 
void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	switch (mGameState) {
	case MENU:
		switch (tolower(key)) {
		case '1': StartGame(); break;
		case '2': /* Difficulty - will be implement later */ break;
		case '3': ShowInstructions(); break;
		case '4': /* High scores - will be implement later */ break;
		}
		break;

	case INSTRUCTIONS:
		if (tolower(key) == 'b') {
			ReturnToMenu();
		}
		break;

	case PLAYING:
		if (key == ' ') {
			mSpaceship->Shoot();
		}
		break;
	}
}


void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	switch (key)
	{
		// If up arrow key is pressed start applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
		// If left arrow key is pressed start rotating anti-clockwise
	case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
		// If right arrow key is pressed start rotating clockwise
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
		// Default case - do nothing
	default: break;
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	switch (key)
	{
		// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
		// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
		// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
		// Default case - do nothing
	default: break;
	}
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAsteroidCount--;
		if (mAsteroidCount <= 0)
		{
			SetTimer(500, START_NEXT_LEVEL);
		}
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
	}

	if (value == SHOW_GAME_OVER)
	{
		mGameOverLabel->SetVisible(true);
	}

}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));

	// Create score label but keep it hidden initially
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mScoreLabel->SetVisible(false);
	shared_ptr<GUIComponent> score_component = static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create lives label but keep it hidden initially
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mLivesLabel->SetVisible(false);
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Game over label
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameOverLabel->SetVisible(false);
	shared_ptr<GUIComponent> game_over_component = static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));
}

void Asteroids::OnScoreChanged(int score)
{
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	mLivesLabel->SetText("Lives: " + std::to_string(lives_left));

	if (lives_left > 0) {
		SetTimer(1000, CREATE_NEW_PLAYER);
	}
	else {
		mGameState = GAME_OVER;
		SetTimer(500, SHOW_GAME_OVER);
	}
}

// New function to create main menu at the start 
void Asteroids::CreateMenu()
{
	// Creating menu asteroids background 
	for (int i = 0; i < 15; i++) {
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite = make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.15f);
		mMenuAsteroids.push_back(asteroid);
		mGameWorld->AddObject(asteroid);
	}

	// Creating menu labels 
	mTitleLabel = make_shared<GUILabel>("ASTEROIDS");
	mTitleLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mTitleLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mGameDisplay->GetContainer()->AddComponent(static_pointer_cast<GUIComponent>(mTitleLabel), GLVector2f(0.5f, 0.9f));

	mStartLabel = make_shared<GUILabel>("1. Start Game");
	mStartLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameDisplay->GetContainer()->AddComponent(static_pointer_cast<GUIComponent>(mStartLabel), GLVector2f(0.5f, 0.6f));

	mDifficultyLabel = make_shared<GUILabel>("2. Difficulty");
	mDifficultyLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameDisplay->GetContainer()->AddComponent(static_pointer_cast<GUIComponent>(mDifficultyLabel), GLVector2f(0.5f, 0.5f));

	mInstructionsLabel = make_shared<GUILabel>("3. Instructions");
	mInstructionsLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameDisplay->GetContainer()->AddComponent(static_pointer_cast<GUIComponent>(mInstructionsLabel), GLVector2f(0.5f, 0.4f));

	mHighScoreLabel = make_shared<GUILabel>("4. High Scores");
	mHighScoreLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameDisplay->GetContainer()->AddComponent(static_pointer_cast<GUIComponent>(mHighScoreLabel), GLVector2f(0.5f, 0.3f));

	// instruction lines
	vector<string> instructions = {
		"HOW TO PLAY:",
		"Move: Arrow Keys",
		"Shoot: Spacebar",
		"Pause: P",
		"Destroy asteroids to score!",
		"Avoid asteroid collisions!"
	};

	float yPos = 0.7f; 
	float yStep = 0.08f; 

	for (const auto& line : instructions) {
		auto instructionLine = make_shared<GUILabel>(line);
		instructionLine->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
		instructionLine->SetVisible(false);
		mGameDisplay->GetContainer()->AddComponent(
			static_pointer_cast<GUIComponent>(instructionLine),
			GLVector2f(0.5f, yPos));
		mInstructionLines.push_back(instructionLine);
		yPos -= yStep;
	}

	// Create back button 
	mBackLabel = make_shared<GUILabel>("Press B to go back");
	mBackLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mBackLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mBackLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(static_pointer_cast<GUIComponent>(mBackLabel), GLVector2f(0.5f, 0.1f));
}

//A new start game method which swtiches game state from other others to PLAYING 
void Asteroids::StartGame()
{
	mGameState = PLAYING;
	ClearMenuAsteroids();

	mTitleLabel->SetVisible(false);
	mStartLabel->SetVisible(false);
	mDifficultyLabel->SetVisible(false);
	mInstructionsLabel->SetVisible(false);
	mHighScoreLabel->SetVisible(false);
	mBackLabel->SetVisible(false);

	for (auto& line : mInstructionLines) {
		line->SetVisible(false);
	}

	mScoreLabel->SetVisible(true);
	mLivesLabel->SetVisible(true);

	mGameWorld->AddObject(CreateSpaceship());
	CreateAsteroids(10);

	mGameWorld->AddListener(&mScoreKeeper);
	mScoreKeeper.AddListener(shared_ptr<Asteroids>(this));
	mGameWorld->AddListener(&mPlayer);
	mPlayer.AddListener(shared_ptr<Asteroids>(this));
}
// A new method which clears menu elements and show instructions 

void Asteroids::ShowInstructions()
{
	mGameState = INSTRUCTIONS;

	mTitleLabel->SetVisible(false);
	mStartLabel->SetVisible(false);
	mDifficultyLabel->SetVisible(false);
	mInstructionsLabel->SetVisible(false);
	mHighScoreLabel->SetVisible(false);

	for (auto& line : mInstructionLines) {
		line->SetVisible(true);
	}
	mBackLabel->SetVisible(true);
}
// A method to clear elements 
void Asteroids::ClearMenuAsteroids()
{
	for (auto asteroid : mMenuAsteroids) {
		mGameWorld->FlagForRemoval(asteroid);
	}
	mMenuAsteroids.clear();
}
// A method to return back to Main Menu 
void Asteroids::ReturnToMenu()
{
	mGameState = MENU;

	mTitleLabel->SetVisible(true);
	mStartLabel->SetVisible(true);
	mDifficultyLabel->SetVisible(true);
	mInstructionsLabel->SetVisible(true);
	mHighScoreLabel->SetVisible(true);

	for (auto& line : mInstructionLines) {
		line->SetVisible(false);
	}
	mBackLabel->SetVisible(false);
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}