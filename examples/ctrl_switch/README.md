# Example - Control Switch Example

This example demonstrates how to create a simple controller. This controller will send the arguments as commands, one by one (in a loop). Use with example-lpp with `--ctrl-tcp localhost --ctrl-tcp-port 13226`.

## Usage

```
  ./example-ctrl-switch [commands...] {OPTIONS}

    Control Switch Example (3.4.4) - This example demonstrates how to create a
    simple controller. This controller will send the input commands one by one
    (in a loop). Use with example-lpp with `--ctrl-tcp localhost --ctrl-tcp-port
    13226`.

  OPTIONS:

      -?, --help                        Display this help menu
      -v, --version                     Display version information
      Arguments:
        commands...                       List of commands
      "--" can be used to terminate flag options and force all following
      arguments to be treated as positional options
```
