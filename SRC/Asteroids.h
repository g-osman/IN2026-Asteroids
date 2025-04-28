#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h" 
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"

//New header includes
#include "vector"
#include "GUIComponent.h" 
#include "GUILabel.h"     
#include "GLVector.h"      
#include <algorithm>
#include <fstream>


class GameObject;
class Spaceship;
class GUILabel;

//  this enum is to keep track of different game states 
enum GameState {
	MENU,
	PLAYING,
	GAME_OVER,
	INSTRUCTIONS,
	HIGH_SCORES,
	NAME_ENTRY,
	DIFFICULTY
};


// this enum is for difficulty options
enum Difficulty {
	EASY,       
	NORMAL,     
	HARD       
};



class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener, public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char* argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	virtual void Stop(void);

	// Declaration of IKeyboardListener interface ////////////////////////////////

	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// Declaration of IScoreListener interface //////////////////////////////////

	void OnScoreChanged(int score);

	// Declaration of the IPlayerLister interface //////////////////////////////

	void OnPlayerKilled(int lives_left);

	// Declaration of IGameWorldListener interface //////////////////////////////

	void OnWorldUpdated(GameWorld* world) {}
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// Override the default implementation of ITimerListener ////////////////////
	void OnTimer(int value);

	// New methods that I added
	void ReturnToMenu();
	void ShowInstructions();


	// Adding these members to link difficulty with powerup options
	Difficulty mCurrentDifficulty;
	shared_ptr<GUILabel> mDifficultyTitle;
	vector<shared_ptr<GUILabel>> mDifficultyOptions;

	void CreateDifficultyMenu();
	void SetDifficulty(Difficulty difficulty);


	void ClearDifficultyMenu();
private:
	shared_ptr<Spaceship> mSpaceship;
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;

	uint mLevel;
	uint mAsteroidCount;

	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);
	shared_ptr<GameObject> CreateExplosion();

	const static uint SHOW_GAME_OVER = 0;
	const static uint START_NEXT_LEVEL = 1;
	const static uint CREATE_NEW_PLAYER = 2;

	ScoreKeeper mScoreKeeper;
	Player mPlayer;

	// members and member function declaration 
	GameState mGameState;
	vector<shared_ptr<GameObject>> mMenuAsteroids;
	shared_ptr<GUILabel> mTitleLabel;
	shared_ptr<GUILabel> mStartLabel;
	shared_ptr<GUILabel> mDifficultyLabel;
	shared_ptr<GUILabel> mInstructionsLabel;
	shared_ptr<GUILabel> mHighScoreLabel;
	shared_ptr<GUILabel> mInstructionsText;
	shared_ptr<GUILabel> mBackLabel;
	vector<shared_ptr<GUILabel>> mInstructionLines;
	void CreateMenu();
	void StartGame();
	void ShowGameGUI(bool show);
	void ClearMenuAsteroids();

	// for scoring (name then score ) 
	struct HighScoreEntry {
		string name;
		int score;

		bool operator<(const HighScoreEntry& other) const {
			return score > other.score;
		}
	};



	vector<HighScoreEntry> mHighScores;
	shared_ptr<GUILabel> mHighScoreTitleLabel;
	vector<shared_ptr<GUILabel>> mHighScoreLabels;
	shared_ptr<GUILabel> mEnterNameLabel;
	shared_ptr<GUILabel> mNameInputLabel;
	string mPlayerNameInput;
	bool mWaitingForNameInput;

	void LoadHighScores();
	void SaveHighScores();
	void ShowNameInputScreen();
	void AddHighScore(const string& name, int score);
	void UpdateNameInputDisplay();
	void ShowHighScoreTable();
	bool ShouldQualifyForHighScore();

	//variable for extra lives
	int mNextLifeScoreThreshold;
};

#endif