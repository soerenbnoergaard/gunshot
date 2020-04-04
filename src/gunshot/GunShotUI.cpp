/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2019 Filipe Coelho <falktx@falktx.com>
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

#include "DistrhoUI.hpp"
#include "extra/String.hpp"
#include "utils.h"
#include "nfd.h"

START_NAMESPACE_DISTRHO

/**
  We need the rectangle class from DGL.
 */
using DGL_NAMESPACE::Rectangle;

// -----------------------------------------------------------------------------------------------------------

class GunShotUI : public UI
{
public:
    GunShotUI()
        : UI(512, 512)
    {
        // TODO explain why this is here
        setGeometryConstraints(128, 128, true);
    }

protected:
   /* --------------------------------------------------------------------------------------------------------
    * DSP/Plugin Callbacks */

   /**
      This plugin has no parameters, so we can safely ignore this.
    */
    void parameterChanged(uint32_t, float) override {}

   /**
      A state has changed on the plugin side.
      This is called by the host to inform the UI about state changes.
    */
    void stateChanged(const char* key, const char* value) override
    {
        repaint();
    }

   /* --------------------------------------------------------------------------------------------------------
    * Widget Callbacks */

   /**
      The OpenGL drawing function.
      This UI will draw a 3x3 grid, with on/off states according to plugin state.
    */
    void onDisplay() override
    {
        Rectangle<int> r;
        r.setWidth(getWidth());
        r.setHeight(getHeight());
        r.setX(0);
        r.setY(0);

        glColor3f(0.0f, 0.0f, 0.2f);

        r.draw();
    }

   /**
      Mouse press event.
      This UI will de/activate blocks when you click them and report it as a state change to the plugin.
    */
    bool onMouse(const MouseEvent& ev) override
    {
        Rectangle<int> r;
        r.setWidth(getWidth());
        r.setHeight(getHeight());
        r.setX(0);
        r.setY(0);

        if ((r.contains(ev.pos)) && (ev.press == true)) {
            repaint();


            // Toggle between two presets.
            //
            // Plugin state string is generated in the UI. The plugin itself
            // will have to do all calculations based on this.

            // Browse for file
            nfdchar_t *wav_path = NULL;
            nfdresult_t result = NFD_OpenDialog("wav", NULL, &wav_path);

            if (result == NFD_OKAY) {
                // Proceed
            }
            else if (result == NFD_CANCEL) {
                // User pressed cancel
                return true;
            }
            else {
                /* printf("Error: %s\n", NFD_GetError() ); */
                return true;
            }

            // Load impulse response filimpulse response file
            int err;
            plugin_state_t state;
            err = plugin_state_init(&state, wav_path);
            free(wav_path);

            if (err) {
                throw "Error resetting state";
            }

            // Convert state struct into string which is sent to the plugin
            char *str = NULL;
            uint32_t length = 0;
            err = plugin_state_serialize(&state, &str, &length);
            if (err) {
                throw "Error serializing state";
            }
            setState("state", String(str));

            // Clean up
            free(str);
            plugin_state_reset(&state, true, false);
        }

        return true;
    }

    // -------------------------------------------------------------------------------------------------------

private:

   /**
      Set our UI class as non-copyable and add a leak detector just in case.
    */
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GunShotUI)
};

/* ------------------------------------------------------------------------------------------------------------
 * UI entry point, called by DPF to create a new UI instance. */

UI* createUI()
{
    return new GunShotUI();
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
