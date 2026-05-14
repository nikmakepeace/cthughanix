# CthughaNix - An Oscilloscope on Acid

## Relicensing and the Future

This is a relicensing, renaming, and modernization of the Cthugha-L
distribution. Much remains to be done, though if you are using Linux/x86 you
should be able to enjoy Cthugha immediately.

Please see the `TODO` file for more information.

For details on the new licence, see the `COPYING` file.

The application's web site is
[http://cthughanix.sourceforge.net/](http://cthughanix.sourceforge.net/).

## How to Install

See `INSTALL`.

## Some Tips

- `xcthugha` does not react to `-geometry` in the normal way. Use the `-D` and
  `--position` options.

- Take a look at the documentation. I know it is quite long and not very well
  written, but it is still useful.

- Joystick movement of the display can be controlled by a joystick in this
  version. Use `--no-joystick` for automatic, random movement.

- Example of nice full-screen options for X:

  ```sh
  ./xcthugha -D 5 --full-screen --play /export/home/brandon/Shared/Beach\ Boys\ -\ Good\ Vibrations.mp3
  ```

## Bugs

See the `TODO` file. Known bugs and problems are usually listed there.

Information on new bugs, and sometimes even how to fix some old ones, can be
found on the SourceForge bug tracker:

[https://sourceforge.net/tracker/?group_id=201465&atid=977671](https://sourceforge.net/tracker/?group_id=201465&atid=977671)

If you find a new bug or problem, please submit a bug report on the SourceForge
bug tracker.

If you have problems with sound, such as sound reading errors, use the
`--snd-method` option. Setting a value of `3` should work.

## Contact

- E-mail: <brandon.barker@gmail.com>
- WWW: [http://brandonbarker.blogspot.com](http://brandonbarker.blogspot.com)
- Official bug page:
  [https://sourceforge.net/tracker/?group_id=201465&atid=977671](https://sourceforge.net/tracker/?group_id=201465&atid=977671)
- Official WWW page:
  [http://cthughanix.sourceforge.net/](http://cthughanix.sourceforge.net/)
- Official newsgroup: `alt.graphics.cthugha`

If you have any questions left, feel free to send me an email.
