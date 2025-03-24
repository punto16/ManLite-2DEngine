// Animation.h
#pragma once
#include "Defs.h"
#include "filesystem"
#include "fstream"
#include "nlohmann/json.hpp"

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

    //serialization
    nlohmann::json Save() const {
        nlohmann::json animationJSON;
        animationJSON["speed"] = speed;
        animationJSON["loop"] = loop;
        animationJSON["pingpong"] = pingpong;
        animationJSON["totalFrames"] = totalFrames;

        for (int i = 0; i < totalFrames; ++i) {
            animationJSON["frames"][i]["x"] = frames[i].x;
            animationJSON["frames"][i]["y"] = frames[i].y;
            animationJSON["frames"][i]["w"] = frames[i].w;
            animationJSON["frames"][i]["h"] = frames[i].h;
        }

        return animationJSON;
    }
    void Load(const nlohmann::json& animationJSON) {
        speed = animationJSON["speed"];
        loop = animationJSON["loop"];
        pingpong = animationJSON["pingpong"];
        totalFrames = animationJSON["totalFrames"];

        for (int i = 0; i < totalFrames; ++i) {
            frames[i].x = animationJSON["frames"][i]["x"];
            frames[i].y = animationJSON["frames"][i]["y"];
            frames[i].w = animationJSON["frames"][i]["w"];
            frames[i].h = animationJSON["frames"][i]["h"];
        }
    }

    bool SaveToFile(const std::string& path) const {
        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << Save().dump(2);
        return true;
    }
    bool LoadFromFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;

        nlohmann::json animationJSON;
        try {
            file >> animationJSON;
        }
        catch (const nlohmann::json::parse_error& e) {
            return false;
        }

        Load(animationJSON);
        return true;
    }


    //getters // setters
    ML_Rect GetCurrentFrame() const {
        int frameIndex = static_cast<int>(currentFrame);
        if (frameIndex < 0 || frameIndex > MAX_FRAMES) return ML_Rect(0, 0, 0, 0);

        if (pingpong && pingpongDirection == -1) {
            frameIndex = totalFrames - frameIndex - 1;
        }

        return frames[frameIndex];
    }

    bool HasFinished() const {
        return !loop && !pingpong && (currentFrame >= totalFrames);
    }
};