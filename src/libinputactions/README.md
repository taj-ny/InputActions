# libinputactions
Contains shared code for trigger handling. Not dependent on any specific compositor.

Not intended for use outside the Input Actions project.

# Adding support for other compositors
See [src/kwin](/src/kwin) for a reference implementation.

Classes to be implemented:
- ``libinputactions::InputBackend`` - Collects input events and optionally blocks them (**required**)
- ``libinputactions::InputEmitter`` - Emits input events (required by input actions)
- ``libinputactions::Keyboard`` - Clearing pressed modifiers (required by input actions inside triggers with keyboard modifiers)
- ``libinputactions::Pointer`` - Provides access to the pointer's position and allows changing it (required by start/end positions and mouse input actions)
- ``libinputactions::Window``, ``libinputactions::WindowProvider`` - Provides access to windows (required by conditions)

Implementing ``libinputactions::InputBackend`` is mandatory, other classes are optional.