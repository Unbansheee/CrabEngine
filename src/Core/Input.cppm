//
// Created by Vinnie on 4/05/2025.
//

export module Engine.Input;

export enum class InputResult {
    Ignored, // Input not consumed, continue traversing
    Handled  // Input consumed. Do not continue traversing
};


export struct InputEvent {
    enum class Type { Key, MouseMove, MouseButton, Scroll } type;

    union {
        struct {
            int key, scancode, action, mods;
        } key;

        struct {
            double x, y;
        } mouseMove;

        struct {
            int button, action, mods;
        } mouseButton;

        struct {
            double xoffset, yoffset;
        } scroll;
    };
};


// Interop struct for C#
#pragma pack(push, 1)
export struct InputEventInterop {
    int type; // maps to InputEvent::Type

    // Key input
    int key;
    int scancode;
    int action;
    int mods;

    // Mouse move
    double mouseX;
    double mouseY;

    // Mouse button
    int mouseButton;
    int mouseButtonAction;
    int mouseButtonMods;

    // Scroll
    double scrollX;
    double scrollY;


    operator InputEvent() const {
        InputEvent event{};

        switch (static_cast<InputEvent::Type>(type)) {
            case InputEvent::Type::Key:
                event.key.key = key;
                event.key.scancode = scancode;
                event.key.action = action;
                event.key.mods = mods;
                break;

            case InputEvent::Type::MouseMove:
                event.mouseMove.x = mouseX;
                event.mouseMove.y = mouseY;
                break;

            case InputEvent::Type::MouseButton:
                event.mouseButton.button = mouseButton;
                event.mouseButton.action = mouseButtonAction;
                event.mouseButton.mods = mouseButtonMods;
                break;

            case InputEvent::Type::Scroll:
                event.scroll.xoffset = scrollX;
                event.scroll.yoffset = scrollY;
                break;
        }

        return event;
    };

    InputEventInterop();

    InputEventInterop(const InputEvent &event) {
        type = static_cast<int>(event.type);
        switch (event.type) {
            case InputEvent::Type::Key:
                key = event.key.key;
                scancode = event.key.scancode;
                action = event.key.action;
                mods = event.key.mods;
                break;

            case InputEvent::Type::MouseMove:
                mouseX = event.mouseMove.x;
                mouseY = event.mouseMove.y;
                break;

            case InputEvent::Type::MouseButton:
                mouseButton = event.mouseButton.button;
                mouseButtonAction = event.mouseButton.action;
                mouseButtonMods = event.mouseButton.mods;
                break;

            case InputEvent::Type::Scroll:
                scrollX = event.scroll.xoffset;
                scrollY = event.scroll.yoffset;
                break;
        }
    }
};
#pragma pack(pop)
