#pragma once

export module Engine.GFX.View;
import Engine.Types;

export struct View
{
    Vector3 Position = {0,0,0};
    Matrix4 ViewMatrix = Matrix4(1.0f);
    Matrix4 ProjectionMatrix = Matrix4(1.0f);
};
