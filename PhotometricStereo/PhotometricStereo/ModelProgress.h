#pragma once

class ModelProgress
{
public:
    volatile int Min;
    volatile int Max;
    volatile int Current;
    volatile bool IsAlive;
    ModelProgress() : IsAlive(true) { }
};