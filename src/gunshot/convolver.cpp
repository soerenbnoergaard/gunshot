#include "convolver.hpp"

class ConvolverBackgroundThread : public MyThread
{
public:
    explicit ConvolverBackgroundThread(Convolver& convolver) :
        MyThread("ConvolverBackgroundThread"),
        _convolver(convolver)
    {
        startThread();
    }

    virtual ~ConvolverBackgroundThread()
    {
        signalThreadShouldExit();
        _convolver._backgroundProcessingStartedEvent.signal();
        stopThread(1000);
    }

    virtual void run()
    {
        while (!shouldThreadExit())
        {
            _convolver._backgroundProcessingStartedEvent.wait();
            if (shouldThreadExit())
            {
                return;
            }
            _convolver.doBackgroundProcessing();
            _convolver._backgroundProcessingFinishedEvent.signal();
        }
    }

private:
    Convolver& _convolver;

    ConvolverBackgroundThread(const ConvolverBackgroundThread&);
    ConvolverBackgroundThread& operator=(const ConvolverBackgroundThread&);
};

Convolver::Convolver() :
    fftconvolver::TwoStageFFTConvolver(),
    _thread(),
    _backgroundProcessingFinishedEvent()
{
    _thread.reset(new ConvolverBackgroundThread(*this));
    _backgroundProcessingFinishedEvent.signal();
}

Convolver::~Convolver()
{
    _thread = nullptr;
}

void Convolver::startBackgroundProcessing()
{
    _backgroundProcessingStartedEvent.signal();
}

void Convolver::waitForBackgroundProcessing()
{
    _backgroundProcessingFinishedEvent.wait();
}

