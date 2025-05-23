﻿module;
#pragma once
#include <rocket/rocket.hpp>

export module Engine.Object.ObservableDtor;

export struct observable_dtor
{
    virtual ~observable_dtor();

    rocket::signal<void()> destroyed_signal;
};

inline observable_dtor::~observable_dtor()
{
    destroyed_signal.invoke();
}