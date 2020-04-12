#ifndef CONVOLVER_H
#define CONVOLVER_H

#include <stdint.h>
#include <atomic>

#include "extra/Thread.hpp"
#include "extra/Mutex.hpp"
#include "fftconvolver/TwoStageFFTConvolver.h"

// Subclass of Thread to get rid of some annoying descrutor error caused by unique_ptr.
class MyThread : public Thread
{
public:
    MyThread(const char *name) : Thread(name) {};
};

// Convolver based on KlangFalter's Convolver class, converted from Juce to
// DPF.
class Convolver : public fftconvolver::TwoStageFFTConvolver
{
public:
    Convolver();
    virtual ~Convolver();

protected:
    virtual void startBackgroundProcessing();
    virtual void waitForBackgroundProcessing();

private:
    friend class ConvolverBackgroundThread;

    std::unique_ptr<MyThread> _thread;
    Signal _backgroundProcessingFinishedEvent;
    Signal _backgroundProcessingStartedEvent;
};

#endif
