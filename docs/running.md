[← MMB4M](../README.md)

# Running MMB4M

```sh
mmbasic prog.bas a b c    # run and exit, arguments in MM.CMDLINE$
mmbasic -i prog.bas       # run, then stay at the prompt
mmbasic -d ~/basic        # start in that directory
mmbasic -s "Colour Maximite 2" prog.bas
mmbasic -l Debug          # log level: None, Debug, Info, Warning, Error
mmbasic -v                # version and copyright
mmbasic -h                # all options
```

Nothing is logged unless you ask. `-l Info` or `-l Debug` writes `mmb4l.log` into the working directory. `MMDIR` in your environment does the same as `-d`.

Programs open files by bare name, relative to the working directory.

## The first launch

macOS marks every download and refuses to run a marked file without an Apple developer certificate. This version has none:

```sh
xattr -dr com.apple.quarantine mmbasic
```

Once per download. `chmod +x` is not needed, the executable bit survives unzipping.

Notarisation would remove the step for everyone. It needs an Apple Developer account and a Developer ID certificate instead of the current ad-hoc signature.

## Putting it on the PATH

`mmbasic` works from any directory once the file sits somewhere your shell looks.

```sh
mkdir -p ~/bin && mv mmbasic ~/bin/
```

`~/bin` is not on the `PATH` by default; `export PATH="$HOME/bin:$PATH"` in `~/.zshrc` puts it there.

If you build it yourself, link instead of copy and a rebuild takes effect at once:

```sh
ln -sfn "$PWD/bin/mmbasic" ~/bin/mmbasic
```

## Your settings

MMBasic keeps its own things in `~/.mmbasic/`: `mmbasic.options` holds the options you set and kept, `mmbasic.history` the lines you typed at the prompt, and the nano configuration lives there too if you installed it. `OPTION RESET ALL` puts every option back; deleting the directory starts you from nothing.

## At the prompt

`QUIT` leaves. Ctrl-C does not: it stops the running program and gives you the prompt back.

Tab completes a filename as far as it can, and rings the bell when it cannot.

`!` runs a shell command, short for `SYSTEM`. `!ls` lists the directory. `!cd foo` is the exception and becomes `CHDIR "foo"`, because every other `SYSTEM` command runs in a forked process where a directory change would be thrown away.

What follows `!` is passed on as it stands. Variables and expressions in it are not evaluated.

## Programs that run themselves

A `.bas` file can start with a shebang:

```basic
#!/usr/local/bin/mmbasic
PRINT "Hello"
```

```sh
chmod 755 hello.bas
./hello.bas
```

The path must be absolute and must be where `mmbasic` really is, which `which mmbasic` prints. The line above assumes `/usr/local/bin`. The file needs Unix line endings; with CRLF the shell says `bad interpreter: No such file or directory`.
