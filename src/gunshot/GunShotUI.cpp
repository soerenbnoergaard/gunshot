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
#include "Window.hpp"
#include "extra/String.hpp"
#include "utils.h"
#include "dejavu-fonts/DejaVuSans.ttf.h"

#define MAX_PATH_LENGTH 2048

START_NAMESPACE_DISTRHO

/**
  We need the rectangle class from DGL.
 */
using DGL_NAMESPACE::Rectangle;

// -----------------------------------------------------------------------------------------------------------

class GunShotUI : public UI
{
public:
    GunShotUI() : UI(512, 128)
    {
        fFont = createFontFromMemory("sans", dejavusans_ttf, dejavusans_ttf_length, false);
        error_message = "";
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
      The NanoVG drawing function.
    */
    void onNanoDisplay() override
    {
        float h = getHeight(); // Window height
        float l = 20; // Line height
        fontSize(15.0f);

        beginPath();
        rect(0.0f, 0.0f, getWidth(), getHeight());
        fillColor(0, 0, 0);
        fill();
        closePath();

        drawCenter(h/2 - l, "GUNSHOT CONVOLVER", 0xff, 0x00, 0x00);
        drawCenter(h/2 + 0, "Click to load impulse response", 0xff, 0xff, 0xff);
        drawCenter(h/2 + l,  error_message, 0xff, 0xff, 0x00);
    }

    void drawCenter(const float y, const char* const s, uint8_t r, uint8_t g, uint8_t b)
    {
        beginPath();
        fillColor(r, g, b);
        textAlign(ALIGN_CENTER|ALIGN_MIDDLE);
        text(getWidth()/2, y, s, NULL);
        closePath();
    }

    /**
      File browser selected function.
    */
    void uiFileBrowserSelected(const char *filename) override
    {
        int err;
        plugin_state_t state;

        // Exit if no file is selected
        if (filename == nullptr) {
            return;
        }

        // Load impulse response file
        err = plugin_state_init(&state, filename);

        if (err) {
            error_message = "ERROR: Supported formats: Uncompressed WAV and AIFF";
            log_write(error_message);
            return;
        }

        // Convert state struct into string which is sent to the plugin
        char *str = NULL;
        uint32_t length = 0;
        err = plugin_state_serialize(&state, &str, &length);
        if (err) {
            error_message = "ERROR: Could not serialize impulse response";
            log_write(error_message);
            return;
        }
        setState("state", String(str));

        // Clean up
        free(str);
        plugin_state_free(&state);
        error_message = "";
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

            Window& w = getParentWindow();
            Window::FileBrowserOptions o;
            w.openFileBrowser(o);
        }

        return true;
    }

    // -------------------------------------------------------------------------------------------------------

private:
    FontId fFont;
    String error_message;

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
