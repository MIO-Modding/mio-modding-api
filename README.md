# Modding API

A modding API for MIO: Memories in Orbit.

Documentation available [here](https://mio-modding.github.io/index.html).

## Cloning
```sh
git clone --recurse-submodules git@github.com:MIO-Modding/mio-modding-api.git
```

This command will clone the repository and any submodules (such as the theming for the documentation). If you've already pulled the repo and want to pull the submodules, run this command:

```sh
git submodule update --init
```

## Generating Documentation
```sh
# With doxygen installed:
doxygen ./docs/Doxyfile
```

This command will generate documentation from the Javadoc comments in the header file. To view the docs after running it, open `./docs/output/html/index.html`.