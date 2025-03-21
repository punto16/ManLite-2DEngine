#include "Sprite2D.h"

#include "ResourceManagerEM.h"
#include "RendererEM.h"
#include "EngineCore.h"
#include "GameObject.h"

#include "Transform.h"
#include "mat3f.h"

Sprite2D::Sprite2D(std::weak_ptr<GameObject> container_go, const std::string& texture_path, std::string name, bool enable)
    : Component(container_go, ComponentType::Sprite, name, enable),
    texturePath(texture_path)
{
    textureID = ResourceManagerEM::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
}

Sprite2D::Sprite2D(const Sprite2D& component_to_copy, std::shared_ptr<GameObject> container_go)
    : Component(component_to_copy, container_go),
    texturePath(component_to_copy.texturePath)
{
    textureID = ResourceManagerEM::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
    this->tex_width = component_to_copy.tex_width;
    this->tex_height = component_to_copy.tex_height;
}

Sprite2D::~Sprite2D()
{
    if (texturePath.empty()) return;
    ResourceManagerEM::GetInstance().ReleaseTexture(texturePath);
}

void Sprite2D::Draw() {
    if (!enabled || textureID == 0) return;

    if (auto transform = GetContainerGO()->GetComponent<Transform>()) {
        // 1. Obtener componentes de la matriz mundo
        mat3f worldMatrix = transform->GetWorldMatrix();
        vec2f translation = worldMatrix.GetTranslation();
        float rotation = worldMatrix.GetRotation();
        vec2f originalScale = worldMatrix.GetScale();
        
        // 2. Umbral para considerar escalas iguales (ajustar según necesidad)
        const float epsilon = 0.001f;
        bool isUniformScale = std::abs(originalScale.x - originalScale.y) < epsilon;
        
        // 3. Aplicar corrección solo si la escala es uniforme
        vec2f adjustedScale = originalScale;
        if (isUniformScale) {
            float aspect = static_cast<float>(tex_width) / tex_height;
            adjustedScale.x = originalScale.y * aspect; // Mantenemos Y como referencia
        }
        
        // 4. Construir matriz final
        mat3f finalMatrix = mat3f::CreateTransformMatrix(
            translation,
            rotation,
            adjustedScale
        );
        
        // 5. Enviar al renderer
        engine->renderer_em->SubmitSprite(textureID, finalMatrix);
    }
}

void Sprite2D::SwapTexture(std::string new_path)
{
    if (!texturePath.empty()) ResourceManagerEM::GetInstance().ReleaseTexture(texturePath);
    texturePath = new_path;
    textureID = ResourceManagerEM::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
}