#include "Animator.h"

Animator::Animator() : animMap(), animQueue(), currentAnimation(), playingAnimation(false), interruptible(true) {

}

Animator& Animator::operator=(const Animator& a) 
{
	// Deep copies maps
	this->animMap = std::map<std::string, Animation>(a.animMap);
	this->animQueue = std::queue<std::string>(a.animQueue);
	this->currentAnimation = nullptr;
	this->playingAnimation = false;
	this->interruptible = true;
	// Adjusts pointers
	for (auto& anim : animMap) {
		anim.second.SetAnimator(this);
	}
	return *this;
}

void Animator::AnimEndCallback() {
	interruptible = true;
	// No queued animations
	if (animQueue.empty()) {
		playingAnimation = false;
		return;
	}
	// Play top member of animation queue
	Animation* queuedAnim = &animMap[animQueue.front()];
	ActivateAnimation(queuedAnim, queuedAnim->Interruptible());
	animQueue.pop();
}


void Animator::AddAnimation(const std::string name, Animation animation) {
	animation.SetAnimator(this);
	animation.SetAnimEndCallback(&Animator::AnimEndCallback);
	animMap[name] = animation;
}

void Animator::PlayOnce(const std::string& animation, bool looping, bool interruptible) {
	// TODO: Spinning on a map access? I don't think so...
	// -- also passing string = 2 map accesses --> pass pointer instead?
	if (currentAnimation != &animMap[animation]) PlayAnimation(animation, looping, interruptible);
}

void Animator::PlayAnimation(const std::string& animation, bool looping=false, bool interruptible=true) {
	if (looping && !interruptible) {
		std::stringstream msg;
		msg << "animation " << animation << " is looping and uninterruptable";
		Log::LogWarning(msg.str());
	}
	// Animation doesn't exist in the map
	if (animMap.find(animation) == animMap.end()) {
		std::stringstream msg;
		msg << "could not find animation: " << animation;
		Log::LogError(msg.str());
		return;
	}
	// Set animation properties
	// Pitfall of this is that you can't queue two
	// of the same animation with different settings
	Animation* anim = &(animMap[animation]);
	anim->SetLooping(looping);
	anim->SetInterruptible(interruptible);
	// An uninterruptible animation is playing
	if (playingAnimation && !this->interruptible) {
        animQueue.push(animation);
		return;
	}
	// An interuptible animation is playing -- stop it
	if (playingAnimation) currentAnimation->Stop();

	ActivateAnimation(anim, interruptible);
}

void Animator::ActivateAnimation(Animation* animation, bool interruptible) {
	// Activate the animation
	playingAnimation = true;
	currentAnimation = animation;
	this->interruptible = interruptible;
	animation->Play();
}


void Animator::Update(float dt) {
	currentAnimation->Update(dt);
}

const Frame& Animator::GetCurrentFrame() { return currentAnimation->GetCurrentFrame(); }
