#ifndef __PANEL_INSPECTOR_H__
#define __PANEL_INSPECTOR_H__
#pragma once

#include "GuiPanel.h"
#include "string"

class GameObject;

class PanelInspector : public Panel
{
public:
	PanelInspector(PanelType type, std::string name, bool enabled);
	~PanelInspector();

	bool Update();

	void GeneralOptions(GameObject& go);
	void PrefabOptions(GameObject& go);
	//components inpsector
	void TransformOptions(GameObject& go);
	void CameraOptions(GameObject& go);
	void SpriteOptions(GameObject& go);
	void AnimatorOptions(GameObject& go);
	void AudioSourceOptions(GameObject& go);
	void Collider2DOptions(GameObject& go);
	void CanvasOptions(GameObject& go);

	//add components last
	void AddComponent(GameObject& go);

private:
	std::string selected_animation;
	float image_animation_size = 100.0f;
	float currentFrame;


	std::string selected_audio;
};

#endif // !__PANEL_INSPECTOR_H__