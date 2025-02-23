//
// Created by Vinnie on 23/02/2025.
//

#pragma once


class ApplicationLayer {
public:
    ApplicationLayer(class Application* application);
    virtual ~ApplicationLayer() = default;

    virtual void Begin() = 0;
    virtual void Update(float dt) = 0;
    virtual void End() = 0;

    Application* GetApplication() const { return owning_application; }

private:
    class Application* owning_application;

};

