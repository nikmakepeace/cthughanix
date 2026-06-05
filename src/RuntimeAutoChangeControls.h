/** @file
 * Runtime auto-change control port and default implementation.
 */

#ifndef CTHUGHA_RUNTIME_AUTO_CHANGE_CONTROLS_H
#define CTHUGHA_RUNTIME_AUTO_CHANGE_CONTROLS_H

#include "RuntimeCommandSink.h"

class Option;

/** Controls runtime AutoChanger commands. */
class RuntimeAutoChangeControls {
public:
    /** Destroys the auto-change controls interface. */
    virtual ~RuntimeAutoChangeControls() { }

    /** Toggles AutoChanger's global lock state. */
    virtual void toggleLock() = 0;

    /**
     * Attempts to change an AutoChanger-owned option by relative offset.
     *
     * @param option Option to inspect and possibly change.
     * @param by Relative offset to apply.
     * @param changes Change flags to merge auto-change effects into.
     * @return Nonzero when the option belongs to AutoChanger behavior.
     */
    virtual int changeAutoChangeOptionBy(
        Option& option, int by, RuntimeChangeSet& changes) = 0;

    /**
     * Attempts to change an AutoChanger-owned option by value text.
     *
     * @param option Option to inspect and possibly change.
     * @param to Value text to select.
     * @param changes Change flags to merge auto-change effects into.
     * @return Nonzero when the option belongs to AutoChanger behavior.
     */
    virtual int changeAutoChangeOptionTo(
        Option& option, const char* to, RuntimeChangeSet& changes) = 0;
};

/**
 * RuntimeAutoChangeControls implementation backed by current AutoChanger globals.
 */
class DefaultRuntimeAutoChangeControls : public RuntimeAutoChangeControls {
public:
    /** Toggles AutoChanger's global lock state. */
    virtual void toggleLock();

    /**
     * Attempts to change an AutoChanger-owned option by relative offset.
     *
     * @param option Option to inspect and possibly change.
     * @param by Relative offset to apply.
     * @param changes Change flags to merge auto-change effects into.
     * @return Nonzero when the option belongs to AutoChanger behavior.
     */
    virtual int changeAutoChangeOptionBy(
        Option& option, int by, RuntimeChangeSet& changes);

    /**
     * Attempts to change an AutoChanger-owned option by value text.
     *
     * @param option Option to inspect and possibly change.
     * @param to Value text to select.
     * @param changes Change flags to merge auto-change effects into.
     * @return Nonzero when the option belongs to AutoChanger behavior.
     */
    virtual int changeAutoChangeOptionTo(
        Option& option, const char* to, RuntimeChangeSet& changes);
};

#endif
