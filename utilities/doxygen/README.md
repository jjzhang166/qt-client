## Dependencies:

- You must have Doxygen 1.6 or later
- You must have the Qt docs installed in `QTDIR/doc/html`

## Steps to run:

```Shell
  $ git clone https://github.com/xtuple/qt-client
  $ git clone https://github.com/xtuple/qt-sample-scripts
  $ cd qt-client
  $ git checkout the_proper_build_branch_or_tag
  $ qmake
  $ make
  $ edit utilities/doxygen/Doxyfile.public
      # set PROJECT_NUMBER to match version.cpp
      # fix the URL in TAGFILES to match the Qt version we're using
  $ doxygen utilities/doxygen/Doxyfile.public
```

Then publish the resulting `html` directory via the `desktop-programmer-ref` directory in [xtuple.github.io](https://github.com/xtuple/xtuple.github.io).

**Note:** The `make` step above is important. It compiles the `.ui` files, which in turn allows Doxygen to document the screen fields.

## Doxygen comment guidelines

- Document in the `.cpp` file instead of the `.h` as much as possible.
- If the only items in the `.cpp` are class members, start the file with `@class`. Precede globals and statics in the `.cpp` with `@addtogroup` and wrap them in `@{` and `@}`.
- Use `/**` to start doxygen comments, not `/*!`
- Use `@`-style directives, not backslashes
- Use `@brief` instead of relying on Doxygen to guess.
- Put `%` in front of words that you're using casually that happen to be names of classes/functions/... so they don't get converted to links.
- Tag order is important. Put the tags you use in the following order:

```
/** @brief Short description of methodX.

    Long description of methodX.

    @dontinclude samplefile.js
      bits of samplefile.js using @skip, @until, ...

    More long description

    @param x Describe x

    @return Description of return value and its semantics

    @see RelatedClass
    @see samplefile.js
 */
int methodX(QString x) { ... }

/** @example */
```

## TODO

Look at turning on `COLLABORATION_GRAPH`. This would be more useful
for people working in the core. One problem with it, though, is
that the diagrams are really hairy - we have a lot of interdependencies.
On the other hand, this would provide good data on ways we could
simplify the code.

Look at turning off `EXTRACT_ALL` once we've documented everything.

Look into documenting stored procedures. We might have to write a
parser for PL/PGSQL and a filter to convert `--` comments to C- or
C++-style comments. The filter could be run on all source files
with a `.sql` suffix.  We might be able to do the same with `COMMENTS`
on tables and their columns and/or views and their columns. Either
we should create `COMMENTS` on functions or include Doxygen comments
inside the function text so they're always visible.
