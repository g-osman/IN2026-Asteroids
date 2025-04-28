#include "GameUtil.h"
#include "GameWorld.h"
#include "Bullet.h"
#include "Spaceship.h"
#include "BoundingSphere.h"

using namespace std;

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/**  Default constructor. */

//Modified version of constructor for setting default values 
Spaceship::Spaceship()
	: GameObject("Spaceship"),
	mIsInvulnerable(false),
	mInvulnerabilityTime(0),
	mInvulnerabilityTimer(0),
	mBlinkInterval(100), 
	mIsVisible(true) 
{
}

/** Construct a spaceship with given position, velocity, acceleration, angle, and rotation. */
Spaceship::Spaceship(GLVector3f p, GLVector3f v, GLVector3f a, GLfloat h, GLfloat r)
	: GameObject("Spaceship", p, v, a, h, r), mThrust(0)
{
}

/** Copy constructor. */
Spaceship::Spaceship(const Spaceship& s)
	: GameObject(s), mThrust(0)
{
}

/** Destructor. */
Spaceship::~Spaceship(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Update this spaceship. */

// modified to Update invulnerability timer and braking system
void Spaceship::Update(int t)
{
	GameObject::Update(t);
	UpdateInvulnerability(t);

	if (mIsBraking) {
		ApplyBrake();
	}
}

/** Render this spaceship. */
// In Render() modified it to add blinking effect for 3 secs 
void Spaceship::Render(void)
{
	if (mIsInvulnerable && !mIsVisible) return; // Skip rendering during blink

	if (mSpaceshipShape.get() != NULL) mSpaceshipShape->Render();
	if ((mThrust > 0) && (mThrusterShape.get() != NULL)) {
		mThrusterShape->Render();
	}
	GameObject::Render();
}

/** Fire the rockets. */

void Spaceship::Thrust(float t)
{
	mThrust = t;
	if (t > 0) {
		mAcceleration.x = mThrust * cos(DEG2RAD * mAngle);
		mAcceleration.y = mThrust * sin(DEG2RAD * mAngle);
	}
	else {
		mAcceleration = GLVector3f(0, 0, 0);
	}
}

/** Set the rotation. */
void Spaceship::Rotate(float r)
{
	mRotation = r;
}

/** Shoot a bullet. */
void Spaceship::Shoot(void)
{
	// Check the world exists
	if (!mWorld) return;
	// Construct a unit length vector in the direction the spaceship is headed
	GLVector3f spaceship_heading(cos(DEG2RAD*mAngle), sin(DEG2RAD*mAngle), 0);
	spaceship_heading.normalize();
	// Calculate the point at the node of the spaceship from position and heading
	GLVector3f bullet_position = mPosition + (spaceship_heading * 4);
	// Calculate how fast the bullet should travel
	float bullet_speed = 30;
	// Construct a vector for the bullet's velocity
	GLVector3f bullet_velocity = mVelocity + spaceship_heading * bullet_speed;
	// Construct a new bullet
	shared_ptr<GameObject> bullet
		(new Bullet(bullet_position, bullet_velocity, mAcceleration, mAngle, 0, 2000));
	bullet->SetBoundingShape(make_shared<BoundingSphere>(bullet->GetThisPtr(), 2.0f));
	bullet->SetShape(mBulletShape);
	// Add the new bullet to the game world
	mWorld->AddObject(bullet);

}

bool Spaceship::CollisionTest(shared_ptr<GameObject> o)
{
	if (o->GetType() != GameObjectType("Asteroid")) return false;
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

//Modified this method to set a flag to ignore collision or not 
void Spaceship::OnCollision(const GameObjectList& objects)
{
	if (mIsInvulnerable && mInvulnerabilityEnabled) return; // Skip collision handling if invulnerable

	for (auto obj : objects) {
		if (obj->GetType() == GameObjectType("Asteroid")) {
			mWorld->FlagForRemoval(shared_from_this());
			break;
		}
	}
}
// new method to activate invulnerability
void Spaceship::ActivateInvulnerability(int duration)
{
	mIsInvulnerable = true;
	mInvulnerabilityTime = duration;
	mInvulnerabilityTimer = duration;
	mIsVisible = true;
}
// new method to Handle blinking and reseting it 
void Spaceship::UpdateInvulnerability(int time)
{
	if (!mIsInvulnerable) return;

	mInvulnerabilityTimer -= time;

	// Handle blinking
	if (mInvulnerabilityTimer % mBlinkInterval < time) {
		mIsVisible = !mIsVisible;
	}

	// End invulnerability
	if (mInvulnerabilityTimer <= 0) {
		mIsInvulnerable = false;
		mIsVisible = true;
	}

}
// new method for implementation of braking system

void Spaceship::ApplyBrake()
{
	if (mThrust <= 0) {
		mVelocity *= (1.0f - mBrakeIntensity * 0.05f);
	}
}