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
#include "log.h"
#include "biquad.h"
#include "plugin_state.hpp"
#include "convolver.hpp"

#include "fftconvolver/FFTConvolver.h"
#include "fftconvolver/TwoStageFFTConvolver.h"
#include "fftconvolver/Utilities.h"
#include "samplerate.h"

#define NUM_PARAMETERS 4
#define NUM_PROGRAMS 0
#define NUM_STATES 1

#define PARAM_DRY 0
#define PARAM_WET 1
#define PARAM_HIGHPASS 2
#define PARAM_LOWPASS 3

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

#ifdef GUNSHOT_LOG_FILE
        // log_init();
        log_write("Call: GunShotPlugin()");
#endif
        inL = NULL;
        inR = NULL;
        bufferSizeChanged(getBufferSize());
    }

    ~GunShotPlugin() override
    {
        plugin_state_free(&state);
        convolver_left.reset();
        convolver_right.reset();
        free(inL);
        free(inR);
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
        return d_version(0, 0, 2);
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

        case PARAM_HIGHPASS:
            parameter.hints  = kParameterIsAutomable;
            parameter.name   = "High pass";
            parameter.symbol = "highpass";
            parameter.unit   = "Hz";
            parameter.ranges.def = BIQUAD_MIN_Hz - 1.0;
            parameter.ranges.min = BIQUAD_MIN_Hz - 1.0;
            parameter.ranges.max = 1000.0f;

            param_highpass_Hz = parameter.ranges.def;

            if (param_highpass_Hz < BIQUAD_MIN_Hz) {
                param_highpass_data_left = biquad_calculate_nofilter(true);
            }
            else {
                param_highpass_data_left = biquad_calculate_highpass(param_highpass_Hz, getSampleRate(), true);
            }

            param_highpass_data_right = param_highpass_data_left;
            break;

        case PARAM_LOWPASS:
            parameter.hints  = kParameterIsAutomable;
            parameter.name   = "Low pass";
            parameter.symbol = "lowpass";
            parameter.unit   = "Hz";
            parameter.ranges.def = BIQUAD_MAX_Hz + 1.0;
            parameter.ranges.min = 200.0f;
            parameter.ranges.max = BIQUAD_MAX_Hz + 1.0;

            param_lowpass_Hz = parameter.ranges.def;

            if (param_lowpass_Hz > BIQUAD_MAX_Hz) {
                param_lowpass_data_left = biquad_calculate_nofilter(true);
            }
            else {
                param_lowpass_data_left = biquad_calculate_lowpass(param_lowpass_Hz, getSampleRate(), true);
            }

            param_lowpass_data_right = param_lowpass_data_left;
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

        case PARAM_HIGHPASS:
            return param_highpass_Hz;
            break;

        case PARAM_LOWPASS:
            return param_lowpass_Hz;
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

        case PARAM_HIGHPASS:
            param_highpass_Hz = value;

            if (param_highpass_Hz < BIQUAD_MIN_Hz) {
                param_highpass_data_left = biquad_calculate_nofilter(false);
            }
            else {
                param_highpass_data_left = biquad_calculate_highpass(param_highpass_Hz, getSampleRate(), false);
            }

            param_highpass_data_right = param_highpass_data_left;
            break;

        case PARAM_LOWPASS:
            param_lowpass_Hz = value;

            if (param_lowpass_Hz > BIQUAD_MAX_Hz) {
                param_lowpass_data_left = biquad_calculate_nofilter(false);
            }
            else {
                param_lowpass_data_left = biquad_calculate_lowpass(param_lowpass_Hz, getSampleRate(), false);
            }

            param_lowpass_data_right = param_lowpass_data_left;
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

        uint32_t fft_block_size_head = 1;
        while (fft_block_size_head < getBufferSize()) {
            fft_block_size_head *= 2;
        }
        uint32_t fft_block_size_tail = fft_block_size_head > 8192 ? fft_block_size_head : 8192;

        // Load impulse reponse into convolver
        convolver_left.init(fft_block_size_head, fft_block_size_tail, (fftconvolver::Sample *)src_data_left.data_out, src_data_left.output_frames_gen);
        convolver_right.init(fft_block_size_head, fft_block_size_tail, (fftconvolver::Sample *)src_data_right.data_out, src_data_right.output_frames_gen);

        free(src_data_left.data_out);
        free(src_data_right.data_out);
    }

   /**
      Run/process function for plugins without MIDI input.
      @note Some parameters might be null if there are no audio inputs or outputs.
    */
    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        uint32_t n;
        float* outL = outputs[0];
        float* outR = outputs[1];

        // There seems to be an issue when reading directly from the `inputs`
        // buffers, so instead, a local copy is created.
        memcpy(inL, inputs[0], sizeof(float)*frames);
        memcpy(inR, inputs[1], sizeof(float)*frames);

        // Real-time audio processing
        convolver_left.process((fftconvolver::Sample *)inL, (fftconvolver::Sample *)outL, frames);
        convolver_right.process((fftconvolver::Sample *)inR, (fftconvolver::Sample *)outR, frames);

        // Filter and mix
        for (n = 0; n < frames; n++) {

            // High-pass filter
            outL[n] = biquad_process_sample(&param_highpass_data_left,  outL[n]);
            outR[n] = biquad_process_sample(&param_highpass_data_right, outR[n]);

            // Low-pass filter
            outL[n] = biquad_process_sample(&param_lowpass_data_left,  outL[n]);
            outR[n] = biquad_process_sample(&param_lowpass_data_right, outR[n]);

            // Mix with dry signal
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
        // FIXME: Should the filter coefficients be updated? Or will the
        // parameters automatically be reset?
        newSampleRate = newSampleRate;
        update();
    }

   /**
      Optional callback to inform the plugin about a buffer size change.@n
      This function will only be called when the plugin is deactivated.
      @note This value is only a hint!@n
            Hosts might call run() with a higher or lower number of frames.
      @see getBufferSize()
    */
    void bufferSizeChanged(uint32_t newBufferSize)
    {
        if (inL != NULL) {
            free(inL);
        }
        if (inR != NULL) {
            free(inR);
        }
        inL = (float *)std::malloc(sizeof(float) * newBufferSize);
        inR = (float *)std::malloc(sizeof(float) * newBufferSize);
    }

    // -------------------------------------------------------------------------------------------------------

private:
    // Temporary input buffers.
    float *inL;
    float *inR;

    plugin_state_t state;
    String state_cache; // Serialized version of `state` which can be quickly returned in `getState()`.

    Convolver convolver_left;
    Convolver convolver_right;

    float param_dry_dB;
    float param_dry_lin;

    float param_wet_dB;
    float param_wet_lin;

    float param_highpass_Hz;
    biquad_t param_highpass_data_left;
    biquad_t param_highpass_data_right;

    float param_lowpass_Hz;
    biquad_t param_lowpass_data_left;
    biquad_t param_lowpass_data_right;

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
