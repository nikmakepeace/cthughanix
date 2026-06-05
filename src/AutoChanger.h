/** @file
 * Automatic scene-change policy driven by audio metrics and acoustic context.
 */

#ifndef __AUTO_CHANGER_H
#define __AUTO_CHANGER_H

class RuntimeCommandSink;
class AcousticContext;
class AutoChangeSettings;
struct AudioMetrics;

class AutoChanger {
    RuntimeCommandSink& runtimeCommands;
    const AutoChangeSettings& settings;
    AcousticContext& acousticContextValue;

    int quietSince;
    int waitTime;
    int lastChange;

public:
    /**
     * Creates the automatic scene changer.
     *
     * @param runtimeCommands_ Runtime command sink used for automatic scene
     *        mutations. The referenced object must outlive this AutoChanger.
     * @param settings_ Automatic scene-change settings. The referenced object
     *        must outlive this AutoChanger.
     * @param acousticContext_ Rolling acoustic state used for fire-triggered
     *        scene changes. The referenced object must outlive this AutoChanger.
     */
    AutoChanger(RuntimeCommandSink& runtimeCommands_,
        const AutoChangeSettings& settings_,
        AcousticContext& acousticContext_);

    /** Releases automatic scene-change policy state. */
    ~AutoChanger();

    /**
     * Runs one automatic-change policy step using supplied audio metrics.
     *
     * @param metrics Metrics for the current visual audio frame.
     */
    void operator()(const AudioMetrics& metrics);

    /**
     * Applies the selected automatic change action.
     *
     * Uses the little option to choose between changing one eligible EffectControl
     * and changing the whole unlocked scene option set. Exact entry selection is
     * still owned by the runtime command handler and legacy EffectControl policy.
     */
    void change();

    /**
     * @return Pointer to static status text for the interface. The text is
     *         overwritten on the next status() call.
     */
    const char* status() const;
};

#endif
