//
// Created by Vinnie on 4/05/2025.
//

export module Engine.Input;

export enum class InputResult
{
 Ignored,
 Handled
};

export struct InputEvent
{
 enum class Type { Key, MouseMove, MouseButton, Scroll } type;

 union {
  struct { int key, scancode, action, mods; } key;
  struct { double x, y; } mouseMove;
  struct { int button, action, mods; } mouseButton;
  struct { double xoffset, yoffset; } scroll;
 };
};