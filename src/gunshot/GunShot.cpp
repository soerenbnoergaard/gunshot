/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2015 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "DistrhoPlugin.hpp"

#include "utils.h"
#include "fftconvolver/FFTConvolver.h"
#include "fftconvolver/Utilities.h"

#define NUM_PARAMETERS 0
#define NUM_PROGRAMS 0
#define NUM_STATES 1

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------------------------------------------

/**
  Plugin that demonstrates the latency API in DPF.
 */
class GunShotPlugin : public Plugin
{
public:
    GunShotPlugin() : Plugin(NUM_PARAMETERS, NUM_PROGRAMS, NUM_STATES)
    {
        int err;
        sampleRateChanged(getSampleRate());
        err = plugin_state_reset(&state, true);
        /* err = plugin_state_init(&state, "/home/soren/vcs/gunshot/src/gunshot/test/test.wav"); */
        if (err) {
            throw "Could not reset state";
        }

        convolver_left.init(state.fft_block_size, (fftconvolver::Sample *)state.ir_left, state.ir_num_samples_per_channel);
        convolver_right.init(state.fft_block_size, (fftconvolver::Sample *)state.ir_right, state.ir_num_samples_per_channel);
    }

    ~GunShotPlugin() override
    {
        plugin_state_reset(&state, false);
    }

protected:
   /* --------------------------------------------------------------------------------------------------------
    * Information */

   /**
      Get the plugin label.
      This label is a short restricted name consisting of only _, a-z, A-Z and 0-9 characters.
    */
    const char* getLabel() const override
    {
        return "gunshot";
    }

   /**
      Get an extensive comment/description about the plugin.
    */
    const char* getDescription() const override
    {
        return "Convolution plugin";
    }

   /**
      Get the plugin author/maker.
    */
    const char* getMaker() const override
    {
        return "soerenbnoergaard";
    }

   /**
      Get the plugin homepage.
    */
    const char* getHomePage() const override
    {
        return "https://github.com/soerenbnoergaard/gunshot";
    }

   /**
      Get the plugin license name (a single line of text).
      For commercial plugins this should return some short copyright information.
    */
    const char* getLicense() const override
    {
        return "MIT";
    }

   /**
      Get the plugin version, in hexadecimal.
    */
    uint32_t getVersion() const override
    {
        return d_version(0, 0, 1);
    }

   /**
      Get the plugin unique Id.
      This value is used by LADSPA, DSSI and VST plugin formats.
    */
    int64_t getUniqueId() const override
    {
        /* soerenbnoergaard: I just made something up */
        return d_cconst('d', 'L', 'b', 'q');
    }

   /* --------------------------------------------------------------------------------------------------------
    * Init */

   /**
      Initialize the parameter @a index.
      This function will be called once, shortly after the plugin is created.
    */
    void initParameter(uint32_t index, Parameter& parameter) override
    {
    }

    /**
      Set the state key and default value of @a index.
      This function will be called once, shortly after the plugin is created.
    */
    void initState(uint32_t index, String& stateKey, String& defaultStateValue) override
    {
        // TODO: Serialize plugin_state_reset output as default state value.
        switch (index) {
        case 0:
            stateKey = "state";
            defaultStateValue = "";
            break;
        default:
            throw "Index out of range";
            break;
        }
    }

   /* --------------------------------------------------------------------------------------------------------
    * Internal data */

   /**
      Get the current value of a parameter.
      The host may call this function from any context, including realtime processing.
    */
    float getParameterValue(uint32_t index) const override
    {
    }

   /**
      Change a parameter value.
      The host may call this function from any context, including realtime processing.
      When a parameter is marked as automable, you must ensure no non-realtime operations are performed.
      @note This function will only be called for parameter inputs.
    */
    void setParameterValue(uint32_t index, float value) override
    {
    }

    /**
      Get the value of an internal state.
      The host may call this function from any non-realtime context.
    */
    String getState(const char* key) const override
    {
    }

    /**
      Change an internal state.
    */
    void setState(const char* key, const char* value) override
    {
    }

   /* --------------------------------------------------------------------------------------------------------
    * Audio/MIDI Processing */

   /**
      Run/process function for plugins without MIDI input.
      @note Some parameters might be null if there are no audio inputs or outputs.
    */
    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        const float* const inL  = inputs[0];
        const float* const inR  = inputs[1];
        float* outL = outputs[0];
        float* outR = outputs[1];

        convolver_left.process((fftconvolver::Sample *)inL, (fftconvolver::Sample *)outL, frames);
        convolver_right.process((fftconvolver::Sample *)inR, (fftconvolver::Sample *)outR, frames);
    }

   /* --------------------------------------------------------------------------------------------------------
    * Callbacks (optional) */

   /**
      Optional callback to inform the plugin about a sample rate change.
      This function will only be called when the plugin is deactivated.
    */
    void sampleRateChanged(double newSampleRate) override
    {
    }

    // -------------------------------------------------------------------------------------------------------

private:

    plugin_state_t state;
    fftconvolver::FFTConvolver convolver_left;
    fftconvolver::FFTConvolver convolver_right;

   /**
      Set our plugin class as non-copyable and add a leak detector just in case.
    */
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GunShotPlugin)
};

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

Plugin* createPlugin()
{
    return new GunShotPlugin();
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
