# Installation

## Precompiled Binaries

To install the precompiled binaries into `/usr/local`, run:

```sh
make install-pre
```

To change where CthughaNix is installed, edit `Makefile`.

The precompiled binaries are optimized for i486.

## Building from Source

To compile your own version of CthughaNix:

1. Run:

   ```sh
   ./configure
   ```

   For a list of possible options, run:

   ```sh
   ./configure --help
   ```

   See [Configure Architecture Option](#configure-architecture-option) for more
   information.

2. Run:

   ```sh
   make
   ```

   This produces all the necessary files. Depending on which compiler you use,
   you might get some warnings. These can be ignored.

3. Run:

   ```sh
   make install
   ```

   This installs CthughaNix on your system.

## Configure Architecture Option

The only `configure` option that is not self-explanatory is
`--with-arch=ARCH`. This option is mainly used to distinguish between
`pentiumpro` and `amdk6`.

Possible values for `ARCH` are:

- `386`
- `486`
- `pentium`
- `pentiumpro`
- `amdk6`

Use `--without-cpu` to disable any processor-specific optimization.

## Problems

- A possible problem with shared libraries is that you may not have the library
  without the version number. For example, if you have `libXaw.so.6` but no
  `libXaw.so`, add a link with:

  ```sh
  ln -s libXaw.so.6 libXaw.so
  ```

  Add such links for all affected shared libraries.

- If you have problems with sound reading, try the `--snd-method` option. Using
  a value of `1` or `2` might solve the problem.

  Please tell me if you continue to have problems, or if you know how to fix
  them.

- Take a look at the `TODO` file for known bugs and problems.

## Non-Linux Machines

- This release of CthughaNix should also work on some machines not running
  Linux. You may not have any sound, but you can try something like:

  ```sh
  --play /dev/audio --silent
  ```

- Please tell me if you run this version on a different operating system. Also
  tell me what you had to change to make it run.

- The network code might not compile on non-Linux machines. Use the configure
  option `--without-network` to disable all network functionality.
