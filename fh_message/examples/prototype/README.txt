Field Hub Prototype

Purpose: A prototyping space for developing a field hub application.

TODO/Incomplete
    - Implement cobs/framing encoding
    - Implement additional page polling funtions (interval modes)
    - Shred pages into time-ordered record spool at sdaq
    - Implement get device id functions and utilize for page assignments at sdaq
    - Error conditions/disconnect behaviors
    - Replace polling with condition variables?
    - fix record time units
    - keep up-to-date with page format changes
    - support serial connection


Contents:

    /bbb:  A prototype field hub application

    /udaq: A simulated udaq application

    /sdaq: A simulated sdaq appliaction

Dependencies:

    ../../src/libfh_message.a
    ../shared/libfh_example.a

Build:

    cd ./bbb/src
    make

    cd ./udaq/src
    make

    cd ./sdaq/src
    make


Usage:

  Launch a simulated udaq:
  ./udaq -u "-d 0x01 -p 6660"

  Launch a field hub:
  ./bbb -p 7770 -u "-p 6660"

  Launch a simulated sdaq:
  ./sdaq -f "-p 7770"

Help:

./udaq -h
Usage: ./udaq [-u "udaq_specifier"] ...
       ./udaq [-h]

    -u  udaq_specifier      A quoted udaq configuration string:

                            "[-d dev_id] [-p port] [-r hit_rate] [-b buffer_size_bits] [-z page_size_bits]
                             [-e msg_encoding 1=plain 2=cobs_frame] [-t trace_option 0=none, 1=protocol,2=stream, 3=both]"

                            Required:
                            -d  dev_id       The device id as a 64-bit number in hexidecimal format

                            Optional:
                            -p  port         Udaq services comm port (DEFAULT=6660)
                            -r  hit_rate     The simulated hit rate (DEFAULT=100)
                            -b  buffer_sz    Size, in bits, of the hit buffer (DEFAULT=16)
                            -z  page_sz      Size, in bits, of a hit buffer page (DEFAULT=10)
                            -e  encoding     Sets the message encoding (DEFAULT=plain)
                            -t  trace        Sets tracing level (DEFAULT=none)

                            Examples:
                            ./udaq -u "-d 0x12345678abcdef01"
                            ./udaq -u "-d 0x01 -p 6660" -u "-d 0x02 -p 6661" -u "-d 0x02 -p 6662"
                            ./udaq -u "-d 0x01 -p 6660 -e 2" - u "-d 0x02 -p 6661 -e 2"
                            ./udaq -u "-d 0x01 -p 6660 -e 2 -r 10 -b 19 -z 8"

    -h  help                Print usage



./bbb -h
Usage: ./bbb [-b buffer_size_bits] [-z page_size_bits] [-p port] [-e encoding] [-t trace] -u [udaq_specifier]
       ./bbb [-h]
    -h  help                 Print usage
    -b  buffer_sz    Size, in bits, of the page buffer (DEFAULT=16)
    -z  page_sz      Size, in bits, of a page buffer page (DEFAULT=10)
    -p  port                 sdaq-service: Port to run sdaq interface on (DEFAULT=7770
    -e  encoding             sdaq-service: Set the message encoding for the sdaq interface (1=plain 2=cobs_frame DEFAULT=plain)
    -t  trace                sdaq-service: Set tracing level for the sdaq interface (0=none, 1=protocol, 2=stream, 3=both DEFAULT=none)
    -u  udaq_specifier       A quoted udaq configuration string:

                            "[-i ip_addr] [-p port] [-r hit_rate] [-g thread_group] [-e encoding] [-t trace]"

                            Required:
                            -p  port             Udaq services comm port

                            Optional:
                            -i  ip_addr          udaq services ip address (DEFAULT=127.0.0.1)
                            -g  thread_group     The thread group servicing the udaq (DEFAULT=0)
                            -e  encoding         Sets the message encoding (1=plain 2=cobs_frame DEFAULT=plain)
                            -t  trace            Sets tracing level (0=none, 1=protocol, 2=stream, 3=both DEFAULT=none)

Examples:
./bbb -u "-i 192.268.0.1 -p 6660"
./bbb -u "-i 192.268.0.1 -p 6660" -u "-i 192.268.0.1 -p 6661"
./bbb -b 16 -z 10 -u "-i 192.268.0.1 -p 6660 -g 0" -u "-i 192.268.0.1 -p 6661 -g 1"
./bbb -p 7770 -e 1 -t 0 -u "-i 192.268.0.1 -p 6660 -g 0 -e 2 -t 0" -u "-i 192.268.0.1 -p 6661 -g 1 -e 2 -t 0"



./sdaq -h
Usage: ./sdaq [-d run_duration] [-f "field_hub_spec"]...
Usage: ./sdaq [-h]

    -d  run_duration         Run duration in seconds (DEFAULT=4294967295)
    -f  field_hub_spec       A quoted config string for a field hub connection:

                            "[-i ip_addr] [-p port] [-e encoding] [-t trace]"

                            Optional:
                            -i  ip_addr      FieldHub services comm port (DEFAULT=127.0.0.1)
                            -p  port         FieldHub services comm port (DEFAULT=7770)
                            -e  encoding     Sets the message encoding (msg_encoding 1=plain 2=cobs_frame DEFAULT=plain)
                            -t  trace        Sets tracing level (trace_option 0=none 1=protocol 2=stream 3=both DEFAULT=none)

                            Examples:
                            ./sdaq -d 120 -f "-i 127.0.0.1 -p 7770"
                            ./sdaq -d 120 -f "-i 127.0.0.1 -p 7770 -e 1 -t 0" -f "-i 127.0.0.1 -p 7771 -e 1 -t 0"

    -h  help                Print usage

