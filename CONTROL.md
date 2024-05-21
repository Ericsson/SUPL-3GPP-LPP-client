# Control

Control commands can be used to configure/instruct the client to perform certain actions. The main reason for this system is to simplify the handling of cell IDs. This will allow the client to be more less specific and not require multiple modems. To use with `example-lpp` you need to specify a interface that control commands will be sent over, e.g., `--ctrl-tcp=localhost --ctrl-tcp-port=5432`. All commands are sent as a string with a leading `/` and ending with `\r\n`. See the example `ctrl-switch` which demonstrates how to send control commands to the client in C/C++.

## Commands

Here are the available commands:
* [CID](#cid)
* [IDENTITY](#identity)

### CID

Notify the client that the cell ID has changed. The client will then request (update) the assistance data for the new cell ID.

Usage: `/CID,<ACT>,<MNC>,<MCC>,<TAC>,<CID>`
* `<ACT>` - Access Technology (L: LTE, N: NR)
* `<MNC>` - Mobile Network Code
* `<MCC>` - Mobile Country Code
* `<TAC>` - Location Area Code (TAC) or Tracking Area Code (TAC)
* `<CID>` - Cell ID

Example: `/CID,L,240,01,0,3`

### IDENTITY

Provide the client with the IMSI, MSISDN, or IP address. 

Usage: `/IDENTITY,<TYPE>,<VALUE>`
* `<TYPE>` - Type of identity (IMSI, MSISDN, IP)
* `<VALUE>` - Identity value

Example: `/IDENTITY,IMSI,240010000000000`


