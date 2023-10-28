#include "core/Renderer.h"
#include "core/Transform.h"

#include "raylib.h"

void Myriad::Renderer::Draw(Myriad::Transform t)
{
    DrawCircle(t.position().x, t.position().y, 5, {100, 50, 0, 255});
}
