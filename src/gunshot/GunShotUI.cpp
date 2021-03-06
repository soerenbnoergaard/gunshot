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
#include "DistrhoDefines.h"
#include "Window.hpp"
#include "extra/String.hpp"

#include "log.h"
#include "utils.h"
#include "plugin_state.hpp"
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
    GunShotUI() : UI(800, 120)
    {
        fFont = createFontFromMemory("sans", dejavusans_ttf, dejavusans_ttf_length, false);
        error_message = "";
        filebrowser_start_dir = String();
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
        int err;
        plugin_state_t state;

        if (std::strcmp(key, "state") == 0) {
            err = plugin_state_deserialize(&state, (char *)value, std::strlen(value));
            if (err) {
                log_write("Error deserializing state in UI");
                return;
            }

            shown_filename = String(state.filename);
        }

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

        drawCenter(h/2 - 1.5*l, "GUNSHOT CONVOLVER", 0xff, 0x00, 0x00);
        drawCenter(h/2 - 0.5*l,  shown_filename, 0xff, 0xff, 0xff);
        drawCenter(h/2 + 0.5*l, "Click to load impulse response", 0x99, 0x99, 0x99);
        drawCenter(h/2 + 1.5*l,  error_message, 0xff, 0xff, 0x00);
    }

    void drawCenter(const float y, const char* const s, uint8_t r, uint8_t g, uint8_t b)
    {
        beginPath();
        fillColor(r, g, b);
        textAlign(ALIGN_CENTER|ALIGN_MIDDLE);
        text(getWidth()/2, y, s, NULL);
        closePath();
    }

    void setStartDirFromFileName(const char *filename)
    {
        int32_t n;
        int32_t last = 0;

#ifdef DISTRHO_OS_WINDOWS
        char sep = '\\';
#else
        char sep = '/';
#endif

        // Find last directory separator
        for (n = strlen(filename)-1; n >= 0; n--) {
            if (filename[n] == sep) {
                last = n;
                break;
            }
        }

        if (last <= 0) {
            return;
        }

        char *tmp = (char *)malloc(sizeof(char)*last + 1);
        if (tmp == NULL) {
            return;
        }

        memcpy(tmp, filename, sizeof(char)*last);
        tmp[last] = '\0';

        filebrowser_start_dir = String(tmp);
        free(tmp);
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

        // Set the startup directory for next time.
        setStartDirFromFileName(filename);

        // Load impulse response file
        log_write(String("File loaded from UI: ") + String(filename));
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
        shown_filename = String((char *)(filename + find_basename(filename)));

        // Clean up
        free(str);
        plugin_state_free(&state);
        error_message = "";

        repaint();
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
            o.startDir = filebrowser_start_dir;
            o.title = "Select impulse response";
            o.buttons.showPlaces = 2;
            w.openFileBrowser(o);
        }

        return true;
    }

    // -------------------------------------------------------------------------------------------------------

private:
    FontId fFont;
    String error_message;
    String shown_filename;
    String filebrowser_start_dir;

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
