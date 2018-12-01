### Using the sms interface

The Snorktracker can receive and send SMS using the GSM module. 
The SMS function must be activated under Settings and 
and the correct answer phone number must be entered.

The following commands are implemented:
Note: All commands are case insensitive.

#### Switch On or off

You can switch the system on or off if you send a
**on** or **off** sms to the sim-card phone number.
If the system accepted the command it sends a 

    **on -> OK** 
or 

    **off -> OK** 
back.

#### Send back the status of the system

You can query the system for the main information if you send
**status** to the Modul phone number.
The system should then response with the following result:

    Status: 
    Sim808 gsm connected
    Voltage: 12.2V
    Temperature: 26.5 C
    Humidity: 40%
    Pressure: 1006 hPa
    Altitude: 400 m
    Speed: 0 kmph
    Satellites: 5
    https://maps.google.com/maps?q=61.496052,23.7798"

#### Change the 'Power save mode'

If your system is in deep sleep it could be very tricky to catch the time when the system awakened.
It is much easier to send a 'popwer save mode off' via sms to the module.
To do that you have to send a **psm:off** to the system phone number and wait for a successful response

    **psm:off -> OK** 

Afterwards you can switch on the power save mode via the web interface or by sending a 
**psm** sms.

    **psm -> OK** 

#### Ask for the current gps position

If you want to know the gps position of your system you can send a simple sms with **gps**.
If the gps awake the next time and the gps tracking is active then you will receive a URL 
with the current gps position as an google request.

This URL can then easily be opened via your browsers in google and if everything works you 
will see the current location of the module on a Google map

    https://maps.google.com/maps?q=61.496052,23.7798"

#### Set some sms parameter

You can change the interval at which the system polls incoming SMS.
For this you can send i.e. the following command

**sms:15**

This change the interval to 15 seconds.
The response should be 

    sms:15 -> OK

#### Set some mqtt parameter

If you want to change the mqtt interval for sending data on moving or standing you can use the command sms
**mqtt:30:60**
This sets the interval on moving to 30 seconds and the interval on not moving to 30 seconds.
The response should be 

    mqtt:30:60 -> OK

### Change the response phone number

If you want to change the phone number to which the result should go then you can do this with:

**phone:123456789**

Then the new mobile phone number should receive the following result sms.

    phone:123456789 -> OK

#### Or send back the command list

If the system receives an invalid command then it sends back the list of commands:

    wrong command
    on
    off
    status
    psm[:off] - power saving mode
    gps[:15] - check every (sec)
    sms[:15] - check every (sec)
    mqtt[30:60] - (moving:standing (sec)
    phone:1234
 
