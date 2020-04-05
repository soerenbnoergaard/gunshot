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
#include "StandaloneWindow.hpp"
#include "extra/String.hpp"
#include "utils.h"
#include "nfd.h"

#define MAX_PATH_LENGTH 2048

// Add reference to liked-in TTF font
extern uint8_t _binary_______dejavu_fonts_DejaVuSans_ttf_start;
extern uint8_t _binary_______dejavu_fonts_DejaVuSans_ttf_end;

const uint8_t *font_memory = &_binary_______dejavu_fonts_DejaVuSans_ttf_start;
const uint32_t font_memory_size = (uint32_t)(&_binary_______dejavu_fonts_DejaVuSans_ttf_end - &_binary_______dejavu_fonts_DejaVuSans_ttf_start);

START_NAMESPACE_DISTRHO

/**
  We need the rectangle class from DGL.
 */
using DGL_NAMESPACE::Rectangle;


// -----------------------------------------------------------------------------------------------------------

class MyFileBrowser : public StandaloneWindow
{
public:
    MyFileBrowser() : StandaloneWindow()
    {
        signal_file_selected = false;
    }

    int browse(char *output)
    {
        // Open browser and wait for it to close
        FileBrowserOptions o;
        openFileBrowser(o);
        exec();
        // show();

        // Return the file selected
        if (signal_file_selected) {
            signal_file_selected = false;
            std::strcpy(output, file_path);
            return 0;
        }
        else {
            // No file selected
            output = nullptr;
            return 1;
        }
    }

protected:
    void fileBrowserSelected(const char* filename) override
    {
        if (filename != nullptr) {
            std::strcpy(file_path, filename);
            signal_file_selected = true;
        }
        else {
            signal_file_selected = false;
        }
        close();
    }

public:
    char file_path[MAX_PATH_LENGTH];
    bool signal_file_selected;
};


// -----------------------------------------------------------------------------------------------------------

class GunShotUI : public UI
{
public:
    GunShotUI()
        : UI(512, 128)
    {
        /* fFont = createFontFromFile("sans", "/home/soren/vcs/gunshot/dejavu-fonts/DejaVuSans.ttf"); */
        fFont = createFontFromMemory("sans", font_memory, font_memory_size, false);
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
            int err;
            plugin_state_t state;
            char ir_path[MAX_PATH_LENGTH];

            repaint();

            // Browse for file
            err = file_browser.browse(ir_path);
            if (err) {
                log_write("No file selected\n");
                return true;
            }
            else {
                log_write(ir_path);
                log_write("\n");
            }

            // Load impulse response filimpulse response file
            err = plugin_state_init(&state, ir_path);

            if (err) {
                error_message = "ERROR: Supported formats: Uncompressed WAV and AIFF";
                return true;
            }

            // Convert state struct into string which is sent to the plugin
            char *str = NULL;
            uint32_t length = 0;
            err = plugin_state_serialize(&state, &str, &length);
            if (err) {
                error_message = "ERROR: Could not serialize impulse response";
                return true;
            }
            setState("state", String(str));

            // Clean up
            free(str);
            plugin_state_reset(&state, true, false);

            error_message = "";
        }

        return true;
    }

    // -------------------------------------------------------------------------------------------------------

private:
    FontId fFont;
    String error_message;
    MyFileBrowser file_browser;

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
