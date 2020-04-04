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

START_NAMESPACE_DISTRHO

/**
  We need the rectangle class from DGL.
 */
using DGL_NAMESPACE::Rectangle;

// -----------------------------------------------------------------------------------------------------------

class GunShotUI : public UI
{
public:
    /**
      Get key name from an index.
    */
    static const char* getStateKeyFromIndex(const uint32_t index) noexcept
    {
        switch (index)
        {
        case 0: return "top-left";
        case 1: return "top-center";
        case 2: return "top-right";
        case 3: return "middle-left";
        case 4: return "middle-center";
        case 5: return "middle-right";
        case 6: return "bottom-left";
        case 7: return "bottom-center";
        case 8: return "bottom-right";
        }

        return "unknown";
    }

    /* constructor */
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
    }

   /**
      Mouse press event.
      This UI will de/activate blocks when you click them and report it as a state change to the plugin.
    */
    bool onMouse(const MouseEvent& ev) override
    {
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
