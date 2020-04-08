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

#define NUM_PARAMETERS 2
#define NUM_PROGRAMS 0
#define NUM_STATES 1

#define PARAM_DRY 0
#define PARAM_WET 1


float convert_dB_to_linear(float x_dB)
{
    if (x_dB < -59.0) {
        return 0.0;
    } else {
        return pow(10.0, x_dB/20.0);
    }
}

START_NAMESPACE_DISTRHO

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
        err = plugin_state_init_dirac(&state, getSampleRate());
        if (err) {
            throw "Could not reset state";
        }

        convolver_left_write = &convolver_left_1;
        convolver_left_read = &convolver_left_2;

        convolver_right_write = &convolver_right_1;
        convolver_right_read = &convolver_right_2;

        signal_swap_buffers = false;

#ifdef GUNSHOT_LOG_FILE
        // log_init();
        log_write("Call: GunShotPlugin()");
#endif

    }

    ~GunShotPlugin() override
    {
        plugin_state_free(&state);
        convolver_left_1.reset();
        convolver_left_2.reset();
        convolver_right_1.reset();
        convolver_right_2.reset();
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
        log_write("Call: initParameter()");
        switch (index) {
        case PARAM_DRY:
            parameter.hints  = kParameterIsAutomable;
            parameter.name   = "Dry";
            parameter.symbol = "dry";
            parameter.unit   = "dB";
            parameter.ranges.def = -60.0f;
            parameter.ranges.min = -60.0f;
            parameter.ranges.max = 20.0f;

            param_dry_dB = parameter.ranges.def;
            param_dry_lin = convert_dB_to_linear(param_dry_dB);
            break;

        case PARAM_WET:
            parameter.hints  = kParameterIsAutomable;
            parameter.name   = "Wet";
            parameter.symbol = "wet";
            parameter.unit   = "dB";
            parameter.ranges.def = 0.0f;
            parameter.ranges.min = -60.0f;
            parameter.ranges.max = 20.0f;

            param_wet_dB = parameter.ranges.def;
            param_wet_lin = convert_dB_to_linear(param_wet_dB);
            break;

        default:
            break;
        }
    }

    /**
      Set the state key and default value of @a index.
      This function will be called once, shortly after the plugin is created.

      SBN: The only purpose of this functions is to provide default values for
      each state variable.
    */
    void initState(uint32_t index, String& stateKey, String& defaultStateValue) override
    {
        log_write("Call: initState()");

        int err;
        char *str = NULL;
        uint32_t length = 0;

        switch (index) {
        case 0:
            // Generate String-representation of default state
            err = plugin_state_init_dirac(&state, getSampleRate());
            if (err) {
                log_write("Error resetting state");
                return;
            }

            err = plugin_state_serialize(&state, &str, &length);
            if (err) {
                log_write("Error serializing state");
                return;
            }

            // Cache default value
            state_cache = String(str);

            // Clean up
            free(str);

            // Output the result
            stateKey = "state";
            defaultStateValue = state_cache;

            // Initialize convolution engines
            update();

            break;

        default:
            log_write("Index out of range");
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
        log_write("Call: getParameterValue()");
        switch (index) {
        case PARAM_DRY:
            return param_dry_dB;
            break;

        case PARAM_WET:
            return param_wet_dB;
            break;

        default:
            return 0.0;
            break;
        }
    }

   /**
      Change a parameter value.
      The host may call this function from any context, including realtime processing.
      When a parameter is marked as automable, you must ensure no non-realtime operations are performed.
      @note This function will only be called for parameter inputs.
    */
    void setParameterValue(uint32_t index, float value) override
    {
#ifdef GUNSHOT_LOG_FILE
        log_write("Call: setParameterValue()");
        char line[1024];
        sprintf(line, "parameter[%d] = %f", index, value);
        log_write(line);
#endif

        switch (index) {
        case PARAM_DRY:
            param_dry_dB = value;
            param_dry_lin = convert_dB_to_linear(value);
            break;
        case PARAM_WET:
            param_wet_dB = value;
            param_wet_lin = convert_dB_to_linear(value);
            break;
        default:
            break;
        }
    }

    /**
      Get the value of an internal state.
      The host may call this function from any non-realtime context.

      SBN: This function is called from the host to obtain state variables that
      it can store, e.g. in internal presets or in the project session.
     */
    String getState(const char* key) const override
    {
        log_write("Call: getState()");

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
        log_write("Call: setState()");
        // log_write(value);
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
        log_write("Call: update()");
        uint32_t n;
        uint32_t err;
        static SRC_DATA src_data_left;
        static SRC_DATA src_data_right;

        // Sample rate convert left channel
        src_data_left.data_in  = state.ir_left;
        src_data_right.data_in = state.ir_right;

        src_data_left.src_ratio  = getSampleRate() / state.ir_sample_rate_Hz;
        src_data_right.src_ratio = getSampleRate() / state.ir_sample_rate_Hz;

        src_data_left.input_frames  = state.ir_num_samples_per_channel;
        src_data_right.input_frames = state.ir_num_samples_per_channel;

        src_data_left.output_frames  = (uint32_t)(src_data_left.src_ratio  * state.ir_num_samples_per_channel) + 1;
        src_data_right.output_frames = (uint32_t)(src_data_right.src_ratio * state.ir_num_samples_per_channel) + 1;

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
        convolver_left_write->init(state.fft_block_size, (fftconvolver::Sample *)src_data_left.data_out, src_data_left.output_frames_gen);
        convolver_right_write->init(state.fft_block_size, (fftconvolver::Sample *)src_data_right.data_out, src_data_right.output_frames_gen);

        free(src_data_left.data_out);
        free(src_data_right.data_out);

        // Signal to the real time engine that the buffers are ready to be swapped.
        signal_swap_buffers = true;
    }

   /**
      Run/process function for plugins without MIDI input.
      @note Some parameters might be null if there are no audio inputs or outputs.
    */
    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        uint32_t n;
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

        for (n = 0; n < frames; n++) {
            outL[n] = param_dry_lin * inL[n] + param_wet_lin * outL[n];
            outR[n] = param_dry_lin * inR[n] + param_wet_lin * outR[n];
        }
    }

   /* --------------------------------------------------------------------------------------------------------
    * Callbacks (optional) */

   /**
      Optional callback to inform the plugin about a sample rate change.
      This function will only be called when the plugin is deactivated.
    */
    void sampleRateChanged(double newSampleRate) override
    {
        newSampleRate = newSampleRate;
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

    float param_dry_dB;
    float param_dry_lin;
    float param_wet_dB;
    float param_wet_lin;

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
