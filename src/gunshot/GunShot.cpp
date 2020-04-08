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
#include "extra/Thread.hpp"

#include "utils.h"
#include "fftconvolver/FFTConvolver.h"
#include "fftconvolver/Utilities.h"
#include "samplerate.h"

#define NUM_PARAMETERS 0
#define NUM_PROGRAMS 0
#define NUM_STATES 1

START_NAMESPACE_DISTRHO

// Class prototypes
class GunShotPlugin;
class UpdateThread;

// -----------------------------------------------------------------------------------------------------------

class UpdateThread : Thread
{
public:
    UpdateThread() : Thread("UpdateThread")
    {
    }

    ~UpdateThread()
    {
    }

    void run() override
    {
        uint32_t n;
        uint32_t err;
        static SRC_DATA src_data_left;
        static SRC_DATA src_data_right;

        // Sample rate convert left channel
        src_data_left.data_in  = state->ir_left;
        src_data_right.data_in = state->ir_right;

        src_data_left.src_ratio  = sample_rate_Hz / state->ir_sample_rate_Hz;
        src_data_right.src_ratio = sample_rate_Hz / state->ir_sample_rate_Hz;

        src_data_left.input_frames  = state->ir_num_samples_per_channel;
        src_data_right.input_frames = state->ir_num_samples_per_channel;

        src_data_left.output_frames  = (uint32_t)(src_data_left.src_ratio  * state->ir_num_samples_per_channel) + 1;
        src_data_right.output_frames = (uint32_t)(src_data_right.src_ratio * state->ir_num_samples_per_channel) + 1;

        src_data_left.data_out  = nullptr;
        src_data_right.data_out = nullptr;

        src_data_left.data_out  = (float *)malloc(sizeof(float) * src_data_left.output_frames);
        src_data_right.data_out = (float *)malloc(sizeof(float) * src_data_right.output_frames);

        if (src_data_left.data_out == nullptr) {
            return;
        }
        if (src_data_right.data_out == nullptr) {
            return;
        }

        err = src_simple(&src_data_left, SRC_SINC_BEST_QUALITY, 1);
        if (err) {
            return;
        }
        err = src_simple(&src_data_right, SRC_SINC_BEST_QUALITY, 1);
        if (err) {
            return;
        }

        // Increasing the sample rate also increases the amplitude so the
        // impulse response is scaled down before initializing the
        // convolver.
        for (n = 0; n < src_data_left.output_frames_gen; n++) {
            src_data_left.data_out[n] /= src_data_left.src_ratio;
            src_data_right.data_out[n] /= src_data_right.src_ratio;
        }

        // Load impulse reponse into convolver
        convolver_left->init(state->fft_block_size, (fftconvolver::Sample *)src_data_left.data_out, src_data_left.output_frames_gen);
        convolver_right->init(state->fft_block_size, (fftconvolver::Sample *)src_data_right.data_out, src_data_right.output_frames_gen);

        free(src_data_left.data_out);
        free(src_data_right.data_out);

        // Signal to the real time engine that the buffers are ready to be swapped.
        *signal_swap_buffers = true;
    }

    void setup(float this_sample_rate_Hz, plugin_state_t *this_state, bool *this_signal_swap_buffers, 
             fftconvolver::FFTConvolver *this_convolver_left,
             fftconvolver::FFTConvolver *this_convolver_right)
    {
        log_write("Thread setup");
        // Copy input and output references so they are ready to run
        sample_rate_Hz = this_sample_rate_Hz;
        state = this_state;
        signal_swap_buffers = this_signal_swap_buffers;
        convolver_left = this_convolver_left;
        convolver_right = this_convolver_right;
    }

    void start()
    {
        log_write("Thread start");
        startThread();
    }

private:
    // Input and output pointers
    float sample_rate_Hz;
    plugin_state_t *state;
    bool *signal_swap_buffers;
    fftconvolver::FFTConvolver *convolver_left;
    fftconvolver::FFTConvolver *convolver_right;
};

// -----------------------------------------------------------------------------------------------------------

/**
  Convolution plugin with impulse reponse stored as internal state.
 */
class GunShotPlugin : public Plugin
{
public:
    GunShotPlugin() : Plugin(NUM_PARAMETERS, NUM_PROGRAMS, NUM_STATES)
    {
        int err;
        err = plugin_state_reset(&state, false, true);
        if (err) {
            throw "Could not reset state";
        }

        convolver_left_write = &convolver_left_1;
        convolver_left_read = &convolver_left_2;

        convolver_right_write = &convolver_right_1;
        convolver_right_read = &convolver_right_2;

        signal_swap_buffers = false;
        update_thread = new UpdateThread();

#ifdef GUNSHOT_LOG_FILE
        // log_init();
        log_write("Log started");
#endif
        sampleRateChanged(getSampleRate());
    }

    ~GunShotPlugin() override
    {
        plugin_state_reset(&state, true, false);
        convolver_left_1.reset();
        convolver_left_2.reset();
        convolver_right_1.reset();
        convolver_right_2.reset();
        delete update_thread;
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
        return d_version(0, 0, 0);
    }

   /**
      Get the plugin unique Id.
      This value is used by LADSPA, DSSI and VST plugin formats.
    */
    int64_t getUniqueId() const override
    {
        // SBN: I just made something up
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

      SBN: The only purpose of this functions is to provide default values for
      each state variable.
    */
    void initState(uint32_t index, String& stateKey, String& defaultStateValue) override
    {
        log_write("Call: initState");
        // Generate String-representation of default state
        int err;
        plugin_state_t default_state;
        err = plugin_state_reset(&default_state, false, true);
        if (err) {
            log_write("Error resetting state");
            return;
        }

        char *str = NULL;
        uint32_t length = 0;
        err = plugin_state_serialize(&default_state, &str, &length);
        if (err) {
            log_write("Error serializing state");
            return;
        }

        state_cache = String(str);

        switch (index) {
        case 0:
            stateKey = "state";
            defaultStateValue = state_cache;
            break;
        default:
            log_write("Index out of range");
            break;
        }

        // Clean up
        free(str);
        update();
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

      SBN: This function is called from the host to obtain state variables that
      it can store, e.g. in internal presets or in the project session.
     */
    String getState(const char* key) const override
    {
        log_write("Call: getState");

        // Return the cached version of `state` instead of re-serializing it.
        if (std::strcmp(key, "state") == 0) {
            return state_cache;
        }
        else {
            return String("");
        }
        return String("");
    }

    /**
      Change an internal state.
      SBN: This function is called whenever the UI wants to change the internal
      state.
    */
    void setState(const char* key, const char* value) override
    {
        log_write("Call: setState");
        int err;
        if (std::strcmp(key, "state") == 0) {
            err = plugin_state_deserialize(&state, (char *)value, std::strlen(value));
            if (err) {
                log_write("Error deserializing state");
                return;
            }
            state_cache = String(value);
            update();
        }
    }

   /* --------------------------------------------------------------------------------------------------------
    * Audio/MIDI Processing */

   /**
      Update non-real-time parameters.
    */
    void update(void)
    {
        // 
        update_thread->setup(getSampleRate(),
                             &state,
                             &signal_swap_buffers, 
                             convolver_left_write,
                             convolver_right_write);

        // Start a thread. When this finishes, the `signal_swap_buffers` flag will be set.
        update_thread->start();
        // update_thread->run();
    }

   /**
      Run/process function for plugins without MIDI input.
      @note Some parameters might be null if there are no audio inputs or outputs.
    */
    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        const float* const inL = inputs[0];
        const float* const inR = inputs[1];
        float* outL = outputs[0];
        float* outR = outputs[1];

        if (signal_swap_buffers) {
            signal_swap_buffers = false;

            fftconvolver::FFTConvolver *tmp;

            tmp = convolver_left_read;
            convolver_left_read = convolver_left_write;
            convolver_left_write = tmp;

            tmp = convolver_right_read;
            convolver_right_read = convolver_right_write;
            convolver_right_write = tmp;
        }

        // Real-time audio processing
        convolver_left_read->process((fftconvolver::Sample *)inL, (fftconvolver::Sample *)outL, frames);
        convolver_right_read->process((fftconvolver::Sample *)inR, (fftconvolver::Sample *)outR, frames);
    }

   /* --------------------------------------------------------------------------------------------------------
    * Callbacks (optional) */

   /**
      Optional callback to inform the plugin about a sample rate change.
      This function will only be called when the plugin is deactivated.
    */
    void sampleRateChanged(double newSampleRate) override
    {
        update();
    }

    // -------------------------------------------------------------------------------------------------------

private:

    plugin_state_t state;
    String state_cache; // Serialized version of `state` which can be quickly returned in `getState()`.

    // Convolution engines.
    // These are double buffered so one can be updated in non-real-time while
    // the other is being used in real-time.
    fftconvolver::FFTConvolver convolver_left_1;
    fftconvolver::FFTConvolver convolver_left_2;
    fftconvolver::FFTConvolver *convolver_left_read;
    fftconvolver::FFTConvolver *convolver_left_write;

    fftconvolver::FFTConvolver convolver_right_1;
    fftconvolver::FFTConvolver convolver_right_2;
    fftconvolver::FFTConvolver *convolver_right_read;
    fftconvolver::FFTConvolver *convolver_right_write;

    bool signal_swap_buffers;

    UpdateThread *update_thread;
    friend class UpdateThread;

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
