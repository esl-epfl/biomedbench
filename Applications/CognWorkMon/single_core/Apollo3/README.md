# Random-projection Heartbeat Classifier

## Building and running

You can run this application on Apollo 3 assuming you have set up the SDK 3 or/and the board correctly.

### Apollo 3 Blue

Compile with
```sh
cd gcc
make all

```

Note because of relative paths in Makefile, the folder should be placed 4 layers in depth from the SDK home directory (i.e. ApolloSDK/boards/apollo3_evb/examples/<App_root_folder>).

Run by flashing the gcc/bin/CognWorkMon.bin file with JFlashLite. Program starting address should be 0xc000 in JFLash parameters.


## Data files

The input data are in src/lib/W14_1_mod_s1d1_f.h.


## Configuration file

In src/lib/eglass.h and src/lib/window_definitions.h you can find important configuration parameters like printing options.
