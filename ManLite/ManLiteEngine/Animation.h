// Animation.h
#pragma once
#include "Defs.h"

#define MAX_FRAMES 32

class Animation
{
public:
    float speed = 1.0f;
    ML_Rect frames[MAX_FRAMES];
    bool loop = true;
    bool pingpong = false;
    int totalFrames = 0;

private:
    float currentFrame = 0.0f;
    int pingpongDirection = 1;

public:
    void PushBack(const ML_Rect& rect) {
        if (totalFrames < MAX_FRAMES) {
            frames[totalFrames++] = rect;
        }
    }

    void Reset() {
        currentFrame = 0.0f;
        pingpongDirection = 1;
    }

    void Update(float deltaTime) {
        currentFrame += speed * deltaTime * 60.0f;

        if (pingpong) {
            if (currentFrame >= totalFrames) {
                pingpongDirection = -1;
                currentFrame = totalFrames - 1;
            }
            else if (currentFrame < 0) {
                pingpongDirection = 1;
                currentFrame = 0;
            }
        }
        else {
            if (loop) {
                currentFrame = fmod(currentFrame, totalFrames);
            }
            else {
                currentFrame = (currentFrame >= totalFrames) ? totalFrames - 1 : currentFrame;
            }
        }
    }

    ML_Rect GetCurrentFrame() const {
        int frameIndex = static_cast<int>(currentFrame);

        if (pingpong && pingpongDirection == -1) {
            frameIndex = totalFrames - frameIndex - 1;
        }

        return frames[frameIndex];
    }

    bool HasFinished() const {
        return !loop && !pingpong && (currentFrame >= totalFrames);
    }
};