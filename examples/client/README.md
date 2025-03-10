# Example - Client

```
  ./example-client {OPTIONS}

    S3LC Client (4.0.1)

  OPTIONS:

      -?, --help                        Display this help menu
      -v, --version                     Display version information
      Location Server:
        -h[host], --ls-host=[host]        Location server hostname or IP address
        -p[port], --ls-port=[port]        Location server port
        --ls-disable                      Run without connecting to the location
                                          server
        --slp-host-cell                   Use host cell to resolve location
                                          server IP address
        --slp-host-imsi                   Use IMSI to resolve location server IP
                                          address
        --ls-shutdown-on-disconnect       Shutdown the client if the location
                                          server connection is lost
      Identity:
        --wait-for-identity               Wait for the identity to be provided
                                          via the control interface
        --msisdn=[msisdn]                 Mobile Subscriber ISDN
        --imsi=[imsi]                     International Mobile Subscriber
                                          Identity
        --ipv4=[ipv4]                     IPv4 address
      Assistance Data:
        --ad-disable                      Don't request assistance data
        --ad-type=[type]                  Type of assistance data to request
                                          One of: osr, ssr
        Cell Information:
          -c[mcc], --mcc=[mcc]              Mobile Country Code
          -n[mnc], --mnc=[mnc]              Mobile Network Code
          -t[tac], --lac=[tac], --tac=[tac] Tracking Area Code
          -i[ci], --ci=[ci]                 Cell Identity
          --nr-cell                         The cell specified is a 5G NR cell
          --wait-for-cell                   Wait for the cell information to be
                                            provided via the control interface
        GNSS:
          --ad-no-gps                       Do not request GPS assistance data
          --ad-no-glo                       Do not request GLONASS assistance
                                            data
          --ad-no-gal                       Do not request Galileo assistance
                                            data
          --ad-no-bds                       Do not request BDS assistance data
        OSR:
          --ad-osr-observations=[rate]      OSR Observations
                                            Default: 1
          --ad-osr-residuals=[rate]         OSR Residuals
                                            Default: 1
          --ad-osr-bias-information=[rate]  OSR Bias Information
                                            Default: 1
        SSR:
          --ad-ssr-clock=[rate]             SSR Clock
                                            Default: 5
          --ad-ssr-orbit=[rate]             SSR Orbit
                                            Default: 5
          --ad-ssr-ura=[rate]               SSR URA
                                            Default: 5
          --ad-ssr-code-bias=[rate]         SSR Code Bias
                                            Default: 5
          --ad-ssr-phase-bias=[rate]        SSR Phase Bias
                                            Default: 5
          --ad-ssr-stec=[rate]              SSR STEC
                                            Default: 30
          --ad-ssr-gridded=[rate]           SSR Gridded
                                            Default: 30
      Location Information:
        --li-unsolicited                  Send unsolicited provide location
                                          information messages to the location
                                          server
        --li-update-rate=[ms]             Update rate is determined by the
                                          location server. Setting this value
                                          will override the location server's
                                          update rate
                                          Default: 1000
        --li-no-nmea                      Do not use NMEA messages for location
                                          information
        --li-no-ubx                       Do not use UBX messages for location
                                          information
        --li-confidence-95-to-68          Scale the semi-major/semi-minor axes
                                          from 95% to 68% confidence
        --li-error-ellipse-68             Output error ellipse with 68%
                                          confidence instead of 39%
        --li-override-horizontal-confidence=[0.0-1.0]
                                          Override horizontal confidence for the
                                          error ellipse
        Fake Location:
          --li-fake-location                Enable fake location information
          --li-fake-latitude=[degree]       Fake Latitude
                                            Default: 69.06
          --li-fake-longitude=[degree]      Fake Longitude
                                            Default: 20.55
          --li-fake-altitude=[meter]        Fake Altitude
                                            Default: 0.0
      Input:
        --input=[input...]                Add an input interface.
                                          Usage: --input <type>:<arguments>
                                          Arguments:
                                            format=<fmt>[+<fmt>...]
                                          Types and their specific arguments:
                                            stdin:
                                            file:
                                              path=<path>
                                              bps=<bps>
                                            serial:
                                              device=<device>
                                              baudrate=<baudrate>
                                              databits=<5|6|7|8>
                                              stopbits=<1|2>
                                              parity=<none|odd|even>
                                            tcp-client:
                                              host=<host>
                                              port=<port>
                                              reconnect=<bool> (default=true)
                                              path=<path>
                                            tcp-server:
                                              listen=<addr> (default=0.0.0.0)
                                              port=<port>
                                              path=<path>
                                            udp-server:
                                              listen=<addr> (default=0.0.0.0)
                                              port=<port>
                                              path=<path>
                                          Formats:
                                            all, ubx, nmea, rtcm, ctrl,
                                          lpp-uper, lpp-uper-pad
      Output:
        --output=[output...]              Add an output interface.
                                          Usage: --output <type>:<arguments>
                                          Arguments:
                                            format=<fmt>[+<fmt>...]
                                          Types and their specific arguments:
                                            stdout:
                                            file:
                                              path=<path>
                                              append=<true|false>
                                            serial:
                                              device=<device>
                                              baudrate=<baudrate>
                                              databits=<5|6|7|8>
                                              stopbits=<1|2>
                                              parity=<none|odd|even>
                                            tcp-client:
                                              host=<host>
                                              port=<port>
                                              path=<path>
                                            udp-client:
                                              host=<host>
                                              port=<port>
                                              path=<path>
                                          Formats:
                                            all, ubx, nmea, rtcm, ctrl, spartn,
                                          lpp-xer, lpp-uper, lrf
                                          Examples:
                                            --output
                                          file:path=/tmp/output,format=ubx+nmea
      GNSS:
        --no-gps                          Disable GPS usage
        --no-glonass, --no-glo            Disable GLONASS usage
        --no-galileo, --no-gal            Disable Galileo usage
        --no-beidou, --no-bds             Disable BeiDou usage
      LPP to RTCM:
        --lpp2rtcm                        Enable LPP to RTCM conversion
        --l2r-no-gps                      Do not generate RTCM messages for GPS
        --l2r-no-glo                      Do not generate RTCM messages for
                                          GLONASS
        --l2r-no-gal                      Do not generate RTCM messages for
                                          Galileo
        --l2r-no-bds                      Do not generate RTCM messages for BDS
        --l2r-msm=[type]                  Which MSM type to generate
                                          One of: any, 4, 5, 6, 7
                                          Default: any
      LPP to framed RTCM:
        --lpp2fr                          Enable wrapping LPP messages in RTCM
                                          messages
        --l2f-id=[id]                     RTCM message ID for LPP messages
                                          Default: 355
        --l2f-rtcm                        Output as both 'lrf' and 'rtcm'
                                          messages
      LPP to SPARTN:
        --lpp2spartn                      Enable LPP to SPARTN conversion
        --l2s-no-gps                      Skip generating GPS messages
        --l2s-no-glo                      Skip generating GLONASS messages
        --l2s-no-gal                      Skip generating Galileo messages
        --l2s-no-bds                      Skip generating BeiDou messages
        SF???:
          --l2s-ura-override=[0-7],
          --l2s-sf024-override=[0-7]        Override URA (SF024). If the value
                                            is unknown, set to 0.
          --l2s-ura-default=[0-7],
          --l2s-sf024-default=[0-7]         Default URA (SF024), used when none
                                            is included in the LPP message. If
                                            the value is unknown, set to 0.
          --l2s-sf042-override=[0-7]        Override SF042. If the value is
                                            unknown, set to 0.
          --l2s-sf042-default=[0-7]         Default SF042, used when none is
                                            included in the LPP message. If the
                                            value is unknown, set to 0.
          --l2s-sf055-override=[0-15]       Override SF055. If the value is
                                            invalid, set to 0.
          --l2s-sf055-default=[0-15]        Default SF055, used when none is
                                            included in the LPP message. If the
                                            value is invalid, set to 0.
        Common:
          --l2s-no-generate-gad             Skip generating GAD messages
          --l2s-no-generate-ocb             Skip generating OCB messages
          --l2s-no-generate-hpac            Skip generating HPAC messages
          --l2s-flip-grid-bitmask           Flip the grid bitmask for incoming
                                            LPP messages
          --l2s-flip-clock-correction       Flip the sign of the clock
                                            correction
          --l2s-flip-orbit-correction       Flip the sign of the orbit
                                            correction
          --l2s-no-average-zenith-delay     Do not compute the average zenith
                                            delay
          --l2s-increasing-siou             Discard incoming LPP IoD values and
                                            use an increasing SIoU instead per
                                            HPAC
          --l2s-filter-by-residuals         Filter out satellites in ionosphere
                                            corrections that do not have
                                            residuals for all grid points
          --l2s-filter-by-ocb               Filter out satellites in ionosphere
                                            corrections that do not have OCB
                                            corrections
        Signal:
          --l2s-ignore-l2l                  Ignore biases from L2L signals
          --l2s-no-code-bias-translate      Do not translate between code biases
          --l2s-no-code-bias-correction-shift
                                            Do not apply correction shift to
                                            code biases when translating
          --l2s-no-phase-bias-translate     Do not translate between phase
                                            biases
          --l2s-no-phase-bias-correction-shift
                                            Do not apply correction shift to
                                            phase biases when translating
        Troposphere:
          --l2s-hydrostatic-in-zenith       Use the remaining hydrostatic delay
                                            residual in the per grid-point
                                            zenith residual
        Ionosphere:
          --l2s-stec-method=[method]        STEC method to use for the
                                            polynomial
                                            One of: default, discard, residual
                                            Default: default
          --l2s-no-stec-transform           Do not transform the STEC from LPP
                                            to SPARTN
          --l2s-stec-invalid-to-zero        Set invalid STEC values to zero
          --l2s-sf-stec-residuals           Flip the sign of the STEC residuals
          Coefficients:
            --l2s-sf-c00                      Flip the sign of the C00
                                              coefficient
            --l2s-sf-c01                      Flip the sign of the C01
                                              coefficient
            --l2s-sf-c10                      Flip the sign of the C10
                                              coefficient
            --l2s-sf-c11                      Flip the sign of the C11
                                              coefficient
      Tokoro:
        --tokoro                          Enable Tokoro generation
        --tkr-no-gps                      Skip generating GPS messages
        --tkr-no-glonass                  Skip generating GLONASS messages
        --tkr-no-galileo                  Skip generating Galileo messages
        --tkr-no-beidou                   Skip generating BeiDou messages
        --tkr-vrs-mode=[vrs-mode]         VRS mode
                                          One of: fixed, dynamic
                                          Default: dynamic
        --tkr-gen=[strategy]              Generation strategy
                                          One of: assistance, time-step,
                                          time-step-aligned
                                          Default: assistance
        --tkr-distance-threshold=[km]     Distance threshold for dynamic VRS
                                          mode (<= 0 means every time)
                                          Default: 5.0
        --tkr-fixed-itrf-x=[meter]        X coordinate for ITRF system in Fixed
                                          VRS mode
        --tkr-fixed-itrf-y=[meter]        Y coordinate for ITRF system in Fixed
                                          VRS mode
        --tkr-fixed-itrf-z=[meter]        Z coordinate for ITRF system in Fixed
                                          VRS mode
        --tkr-fixed-rtcm-x=[meter]        X coordinate for RTCM system in Fixed
                                          VRS mode
        --tkr-fixed-rtcm-y=[meter]        Y coordinate for RTCM system in Fixed
                                          VRS mode
        --tkr-fixed-rtcm-z=[meter]        Z coordinate for RTCM system in Fixed
                                          VRS mode
        --tkr-time-step=[seconds]         Time between each generated message,
                                          used with
                                          `--tkr-gen time-step` or
                                          `--tkr-gen time-step-aligned`
                                          Default: 1.0
        --tkr-no-sc                       Disable shapiro correction
        --tkr-no-pwc                      Disable phase windup correction
        --tkr-no-estc                     Disable earth solid tides correction
        --tkr-no-apvc                     Disable antenna phase variation
                                          correction
        --tkr-no-thc                      Disable tropospheric height correction
        --tkr-no-icc                      Disable Issue of Data consistency
                                          check for ephemeris
        --tkr-rtoc                        Use reception time in orbit correction
                                          instead of transmission time
        --tkr-ocit                        Use orbit corrections in satellite
                                          position evaluation
        --tkr-npw                         Enable negative phase windup
                                          correction
        --tkr-generate-rinex              Generate RINEX files
        --tkr-code-bias-optional          Disable code bias requirement
        --tkr-phase-bias-optional         Disable phase bias requirement
        --tkr-tropo-optional              Disable tropospheric correction
                                          requirement
        --tkr-iono-optional               Disable ionospheric correction
                                          requirement
        --tkr-use-tropospheric-model      Use tropospheric model if tropospheric
                                          correction are not available (requires
                                          tkr-tropo-optional)
        --tkr-use-ionospheric-height-correction
                                          Use ionospheric height correction
        --tkr-antex-file=[file]           Antex file for antenna phase variation
                                          correction
      Logging:
        --trace                           Set log level to trace
        --verbose                         Set log level to verbose
        --debug                           Set log level to debug
        --info                            Set log level to info
        --notice                          Set log level to notice
        --warning                         Set log level to warning
        --error                           Set log level to error
        --lm=[module...]                  <module>=<level>
      Data Tracing:
        --dt-device=[device]              Device
        --dt-server=[server]              Server
        --dt-port=[port]                  Port
                                          Default: 1883
        --dt-username=[username]          Username
        --dt-password=[password]          Password
        --dt-reliable                     Reliable
        --dt-disable-ssr-data             Disable SSR Data
```
