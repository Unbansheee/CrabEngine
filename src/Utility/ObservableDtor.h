#pragma once
#include <rocket/rocket.hpp>

struct observable_dtor
{
    virtual ~observable_dtor();

    rocket::signal<void()> destroyed_signal;
};

inline observable_dtor::~observable_dtor()
{
    destroyed_signal.invoke();
}